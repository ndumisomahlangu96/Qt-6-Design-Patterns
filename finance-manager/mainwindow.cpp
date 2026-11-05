#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupModel();
    setupFlexibleUI();
    setupMenuBar(); // Initialize the menu bar

    // Load previously saved data on startup
    loadData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuBar()
{
    // --- File Menu ---
    QMenu *fileMenu = ui->menubar->addMenu("File");

    QAction *newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("Clear current records and start fresh");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewData);
    fileMenu->addAction(newAction);

    // --- Open File Action ---
    QAction *openAction = new QAction("Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an existing expense CSV file");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);
    // ------------------------

    QAction *saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save current expense records to disk");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);
    fileMenu->addAction(saveAction);

    // ---  Save As Action ---
    QAction *saveAsAction = new QAction("Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip("Save current expense records to a new file location");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveDataAs);
    fileMenu->addAction(saveAsAction);
    // --------------------------

    QAction *exportAction = new QAction("Export to CSV...", this);
    exportAction->setStatusTip("Export all records to a CSV file");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCSV);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);

    // --- Edit Menu ---
    QMenu *editMenu = ui->menubar->addMenu("Edit");

    QAction *deleteAction = new QAction("Delete Selected Row", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Delete the currently selected record from the ledger");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    editMenu->addAction(deleteAction);

    // --- Help Menu ---
    QMenu *helpMenu = ui->menubar->addMenu("Help");

    QAction *aboutAction = new QAction("About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    event->accept();
}

QString MainWindow::getDataFilePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return path + "/expenses.csv"; // Updated extension to .csv
}

void MainWindow::saveData()
{
    QFile file(getDataFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // Write CSV header
        out << "Date,Project/Site,Category,Description,Amount\n";

        // Write CSV rows
        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QString text = expenseModel->item(row, col)->text();
                // Escape any internal quotes by doubling them
                text.replace("\"", "\"\"");
                // Wrap every field in quotes to safely handle internal commas
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        ui->statusbar->showMessage("Data saved successfully.", 3000);
    } else {
        ui->statusbar->showMessage("Failed to save data.", 3000);
    }
}

void MainWindow::saveDataAs()
{
    // Open the Save As dialog box
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", QDir::homePath(), "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return; // User canceled the dialog
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // Write CSV header
        out << "Date,Project/Site,Category,Description,Amount\n";

        // Write CSV rows
        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QString text = expenseModel->item(row, col)->text();
                // Escape any internal quotes by doubling them
                text.replace("\"", "\"\"");
                // Wrap every field in quotes to safely handle internal commas
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        ui->statusbar->showMessage("File saved successfully to new location.", 3000);
    } else {
        QMessageBox::warning(this, "Save As Error", "Could not write to the selected file.");
    }
}

void MainWindow::openFile()
{
    // Open the Open File dialog box
    QString fileName = QFileDialog::getOpenFileName(this, "Open Expense File", QDir::homePath(), "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return; // User canceled the dialog
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error", "Could not open the selected file.");
        return;
    }

    // Clear current data before loading the new file
    expenseModel->removeRows(0, expenseModel->rowCount());

    QTextStream in(&file);
    bool isFirstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();

        // Skip the header row
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        QStringList fields;
        QString currentField;
        bool inQuotes = false;

        // Basic CSV parser to respect quoted fields containing commas
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '\"') {
                if (i + 1 < line.length() && line[i+1] == '\"') {
                    currentField.append('\"');
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField);
                currentField.clear();
            } else {
                currentField.append(c);
            }
        }
        fields.append(currentField);

        // Ensure we have exactly 5 columns before adding to the model
        if (fields.size() >= 5) {
            QList<QStandardItem *> newRow;
            for (int col = 0; col < 5; ++col) {
                newRow << new QStandardItem(fields[col]);
            }
            expenseModel->appendRow(newRow);
        }
    }
    file.close();

    // Refresh the dashboard with the newly loaded data
    updateChart();
    ui->statusbar->showMessage("File opened successfully.", 3000);
}

void MainWindow::loadData()
{
    QFile file(getDataFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // File might not exist on first run
    }

    QTextStream in(&file);
    bool isFirstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();

        // Skip the header row
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        QStringList fields;
        QString currentField;
        bool inQuotes = false;

        // Basic CSV parser to respect quoted fields containing commas
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '\"') {
                // Peek ahead to handle escaped quotes ("")
                if (i + 1 < line.length() && line[i+1] == '\"') {
                    currentField.append('\"');
                    i++; // Skip the second quote
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField);
                currentField.clear();
            } else {
                currentField.append(c);
            }
        }
        fields.append(currentField); // Append the last field

        // Ensure we have exactly 5 columns before adding to the model
        if (fields.size() >= 5) {
            QList<QStandardItem *> newRow;
            for (int col = 0; col < 5; ++col) {
                newRow << new QStandardItem(fields[col]);
            }
            expenseModel->appendRow(newRow);
        }
    }
    file.close();
    updateChart();
}

void MainWindow::setupModel()
{
    expenseModel = new QStandardItemModel(0, 5, this);
    expenseModel->setHorizontalHeaderLabels({"Date", "Project/Site", "Category", "Description", "Amount"});
}

void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    QTabWidget *tabWidget = new QTabWidget(this);

    // --- TAB 1: Expense Entry ---
    QWidget *entryTab = new QWidget();
    QVBoxLayout *entryLayout = new QVBoxLayout(entryTab);
    QGroupBox *formGroup = new QGroupBox("New Expense Record");
    QFormLayout *formLayout = new QFormLayout(formGroup);

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);

    projectCombo = new QComboBox();
    projectCombo->setEditable(true);
    projectCombo->addItems({"Site A", "Property 102", "General Overhead"});

    categoryCombo = new QComboBox();
    categoryCombo->setEditable(true);
    categoryCombo->addItems({"Raw Materials", "Maintenance", "Marketing", "Fuel","Human Labour"});

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");

    descriptionEdit = new QLineEdit();

    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Project:", projectCombo);
    formLayout->addRow("Expense Category:", categoryCombo);
    formLayout->addRow("Description:", descriptionEdit);
    formLayout->addRow("Amount:", amountSpin);

    QPushButton *saveBtn = new QPushButton("Record Expense");
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onAddExpenseClicked);

    entryLayout->addWidget(formGroup);
    entryLayout->addWidget(saveBtn, 0, Qt::AlignRight);
    entryLayout->addStretch();

    // --- TAB 2: Ledger View ---
    QWidget *ledgerTab = new QWidget();
    QVBoxLayout *ledgerLayout = new QVBoxLayout(ledgerTab);

    ledgerView = new QTableView();
    ledgerView->setModel(expenseModel);
    ledgerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ledgerView->setAlternatingRowColors(true);
    ledgerLayout->addWidget(ledgerView);

    // --- TAB 3: Visual Dashboard ---
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardTab);

    categorySeries = new QPieSeries();
    expenseChart = new QChart();
    expenseChart->addSeries(categorySeries);
    expenseChart->setTitle("Expenses by Category");
    expenseChart->setAnimationOptions(QChart::SeriesAnimations);
    expenseChart->legend()->setAlignment(Qt::AlignRight);

    chartView = new QChartView(expenseChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    dashboardLayout->addWidget(chartView);

    // Add tabs
    tabWidget->addTab(entryTab, "Data Entry");
    tabWidget->addTab(ledgerTab, "Ledger Reports");
    tabWidget->addTab(dashboardTab, "Visual Dashboard");

    mainLayout->addWidget(tabWidget);
}

void MainWindow::onAddExpenseClicked()
{
    QList<QStandardItem *> newRow;
    newRow << new QStandardItem(dateEdit->date().toString("yyyy-MM-dd"));
    newRow << new QStandardItem(projectCombo->currentText());
    newRow << new QStandardItem(categoryCombo->currentText());
    newRow << new QStandardItem(descriptionEdit->text());
    newRow << new QStandardItem(QString::number(amountSpin->value(), 'f', 2));

    expenseModel->appendRow(newRow);

    descriptionEdit->clear();
    amountSpin->setValue(0.0);

    // Refresh the chart with the new data
    updateChart();
}

void MainWindow::updateChart()
{
    categorySeries->clear();
    QMap<QString, double> categoryTotals;

    // Aggregate totals by category
    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        QString category = expenseModel->item(row, 2)->text();
        double amount = expenseModel->item(row, 4)->text().toDouble();
        categoryTotals[category] += amount;
    }

    // Populate the pie series
    for (auto it = categoryTotals.begin(); it != categoryTotals.end(); ++it) {
        QPieSlice *slice = categorySeries->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }
}

void MainWindow::onNewData()
{
    // Ask for confirmation before clearing data
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Data", "Are you sure you want to clear all expense records?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        expenseModel->removeRows(0, expenseModel->rowCount());
        updateChart();
        ui->statusbar->showMessage("All records cleared.", 3000);
    }
}

void MainWindow::onExportCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export Expenses", QDir::homePath(), "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // Write CSV headers
        out << "Date,Project/Site,Category,Description,Amount\n";

        // Write CSV rows
        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                // Wrap in quotes to safely handle commas inside descriptions
                rowData << "\"" + expenseModel->item(row, col)->text() + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        ui->statusbar->showMessage("Data exported to CSV successfully.", 3000);
    } else {
        QMessageBox::warning(this, "Export Error", "Could not write to the selected file.");
    }
}

void MainWindow::onExit()
{
    // This will trigger the closeEvent() which automatically calls saveData()
    this->close();
}

void MainWindow::onDeleteSelected()
{
    // Fetch currently selected items from the ledger view
    QModelIndexList selectedRows = ledgerView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a row in the Ledger Reports tab to delete.");
        return;
    }

    // Remove row from model (start from the back to avoid shifting index issues if multiple are supported later)
    for (int i = selectedRows.count() - 1; i >= 0; i--) {
        expenseModel->removeRow(selectedRows.at(i).row());
    }

    updateChart();
    ui->statusbar->showMessage("Record deleted.", 3000);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Finance Manager",
                       "<b>Finance Manager</b><br><br>"
                       "A simple, Qt-based application for tracking project expenses, "
                       "viewing ledgers, and analyzing financial data.");
}