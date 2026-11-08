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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuBar()
{
    // Fetch the standard application style to access built-in icons
    QStyle *style = QApplication::style();

    // --- File Menu ---
    QMenu *fileMenu = ui->menubar->addMenu("File");

    QAction *newAction = new QAction(style->standardIcon(QStyle::SP_FileIcon), "New", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip("Clear current records and start fresh");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewData);
    fileMenu->addAction(newAction);

    // --- Open File Action ---
    QAction *openAction = new QAction(style->standardIcon(QStyle::SP_DirOpenIcon), "Open...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an existing expense CSV file");
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);
    // ------------------------

    QAction *saveAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("Save current expense records to disk");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);
    fileMenu->addAction(saveAction);

    // ---  Save As Action ---
    QAction *saveAsAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save As...", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip("Save current expense records to a new file location");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveDataAs);
    fileMenu->addAction(saveAsAction);
    // --------------------------

    QAction *exportAction = new QAction(style->standardIcon(QStyle::SP_ArrowDown), "Export to CSV...", this);
    exportAction->setStatusTip("Export all records to a CSV file");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCSV);
    fileMenu->addAction(exportAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);

    // --- Edit Menu ---
    QMenu *editMenu = ui->menubar->addMenu("Edit");

    QAction *deleteAction = new QAction(style->standardIcon(QStyle::SP_TrashIcon), "Delete Selected Rocord", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Delete the currently selected record from the ledger");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    editMenu->addAction(deleteAction);

    // --- Sort Action ---
    QAction *sortAction = new QAction(style->standardIcon(QStyle::SP_BrowserReload), "Sort Ledger...", this);
    sortAction->setStatusTip("Sort the ledger by a specific variable");
    connect(sortAction, &QAction::triggered, this, &MainWindow::onSortLedger);
    editMenu->addAction(sortAction);

    // --- Find Action ---
    // Attempts to load the standard OS magnifier icon ("edit-find"), defaults to a list view icon if unavailable
    QAction *findAction = new QAction(QIcon::fromTheme("edit-find", style->standardIcon(QStyle::SP_FileDialogContentsView)), "Find...", this);
    findAction->setShortcut(QKeySequence::Find);
    findAction->setStatusTip("Search the database for a specific variable");
    connect(findAction, &QAction::triggered, this, &MainWindow::onFind);
    editMenu->addAction(findAction);

    // --- Help Menu ---
    QMenu *helpMenu = ui->menubar->addMenu("Help");

    QAction *aboutAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxInformation), "About", this);
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
    // Use the currently opened file path, or default to the app data location if empty
    QString savePath = currentFilePath.isEmpty() ? getDataFilePath() : currentFilePath;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        // Write CSV header
        out << "Date,Project/Site,Category,Description,Amount\n";

        // Write CSV rows
        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QString text = expenseModel->item(row, col)->text();
                text.replace("\"", "\"\"");
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();

        // Update the member variable in case it was the first default save
        currentFilePath = savePath;

        ui->statusbar->showMessage("Data saved successfully to: " + currentFilePath, 3000);
    }
    else
    {
        ui->statusbar->showMessage("Failed to save data.", 3000);
    }
}

void MainWindow::saveDataAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", QDir::homePath(), "CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
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
                text.replace("\"", "\"\"");
                rowData << "\"" + text + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();

        // Update the current file path to the new save location
        currentFilePath = fileName;

        ui->statusbar->showMessage("File saved successfully to new location.", 3000);
    } else {
        QMessageBox::warning(this, "Save As Error", "Could not write to the selected file.");
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Expense File", QDir::homePath(), "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error", "Could not open the selected file.");
        return;
    }

    // Set the current file path to the newly opened file
    currentFilePath = fileName;

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
                QStandardItem *item = new QStandardItem(fields[col]);

                if (col == 4) {
                    // Store Amount column as a double for sorting calculations
                    item->setData(fields[col].toDouble(), Qt::UserRole);
                } else {
                    // Store Date, Project/Site, Category, and Description as strings
                    item->setData(fields[col], Qt::UserRole);
                }

                newRow << item;
            }
            expenseModel->appendRow(newRow);
        }
    }
    file.close();

    // Refresh the dashboard with the newly loaded data
    updateChart();
    ui->statusbar->showMessage("File opened successfully.", 3000);
}

void MainWindow::setupModel()
{
    expenseModel = new QStandardItemModel(0, 5, this);
    expenseModel->setHorizontalHeaderLabels({"Date", "Project/Site", "Category", "Description", "Amount"});

    // Enable sorting by actual data types (double for amount, string for others)
    expenseModel->setSortRole(Qt::UserRole);

    // Automatically update the visual dashboard when manual edits are made in the ledger view
    connect(expenseModel, &QStandardItemModel::itemChanged, this, &MainWindow::updateChart);
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
    projectCombo->addItems({"Rental Property", "Residential Property", "General Overhead"});

    categoryCombo = new QComboBox();
    categoryCombo->setEditable(true);
    categoryCombo->addItems({"Raw Materials", "Maintenance", "Equipment", "Fuel","Human Labour"});

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");

    descriptionEdit = new QLineEdit();

    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Project:", projectCombo);
    formLayout->addRow("Expense Category:", categoryCombo);
    formLayout->addRow("Description:", descriptionEdit);
    formLayout->addRow("Amount:", amountSpin);

    // Add a standard save/apply icon to the button
    QPushButton *saveBtn = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_DialogOkButton), "Record Expense");
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

    // Connect the slice click signal to our new drill-down slot
    connect(categorySeries, &QPieSeries::clicked, this, &MainWindow::onPieSliceClicked);

    expenseChart = new QChart();
    expenseChart->addSeries(categorySeries);
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

    QStandardItem *dateItem = new QStandardItem(dateEdit->date().toString("yyyy-MM-dd"));
    dateItem->setData(dateItem->text(), Qt::UserRole);
    newRow << dateItem;

    QStandardItem *projectItem = new QStandardItem(projectCombo->currentText());
    projectItem->setData(projectItem->text(), Qt::UserRole);
    newRow << projectItem;

    QStandardItem *categoryItem = new QStandardItem(categoryCombo->currentText());
    categoryItem->setData(categoryItem->text(), Qt::UserRole);
    newRow << categoryItem;

    QStandardItem *descItem = new QStandardItem(descriptionEdit->text());
    descItem->setData(descItem->text(), Qt::UserRole);
    newRow << descItem;

    QStandardItem *amountItem = new QStandardItem(QString::number(amountSpin->value(), 'f', 2));
    amountItem->setData(amountSpin->value(), Qt::UserRole); // Store explicitly as double for numeric sorting
    newRow << amountItem;

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

void MainWindow::onSortLedger()
{
    // Define the available variables (columns) for sorting
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok;

    // Prompt the user to select a variable using QInputDialog
    QString item = QInputDialog::getItem(this, "Sort Ledger",
                                         "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        int columnIndex = columns.indexOf(item);
        if (columnIndex != -1) {
            // Prompt the user to select the sorting order using QMessageBox
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Sort Order",
                                          QString("Sort the '%1' column in Ascending order?\n(Select 'No' for Descending)").arg(item),
                                          QMessageBox::Yes | QMessageBox::No);

            Qt::SortOrder order = (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder;

            // Sort the underlying data model
            expenseModel->sort(columnIndex, order);

            ui->statusbar->showMessage(QString("Ledger sorted by %1.").arg(item), 3000);
        }
    }
}

void MainWindow::onFind()
{
    bool ok;
    QString searchText = QInputDialog::getText(this, "Find Variable",
                                               "Enter text or number to find:", QLineEdit::Normal,
                                               "", &ok);

    if (ok && !searchText.isEmpty()) {
        QModelIndex foundIndex;

        // Iterate through rows and columns to find the variable
        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                QModelIndex index = expenseModel->index(row, col);
                // Check if the cell contains the searched string (case-insensitive)
                if (index.data().toString().contains(searchText, Qt::CaseInsensitive)) {
                    foundIndex = index;
                    break;
                }
            }
            if (foundIndex.isValid()) {
                break; // Stop searching once the first match is found
            }
        }

        if (foundIndex.isValid()) {
            // Automatically switch to the Ledger Reports tab (Index 1) so the user can see the highlighted variable
            QTabWidget *tabWidget = this->findChild<QTabWidget*>();
            if (tabWidget) {
                tabWidget->setCurrentIndex(1);
            }

            // Highlight the cell and scroll to it
            ledgerView->selectionModel()->select(foundIndex, QItemSelectionModel::ClearAndSelect);
            ledgerView->scrollTo(foundIndex);

            ui->statusbar->showMessage("Found match for '" + searchText + "'.", 3000);
        } else {
            QMessageBox::information(this, "Not Found", "Could not find '" + searchText + "' in the database.");
        }
    }
}

void MainWindow::onPieSliceClicked(QPieSlice *slice)
{
    // The slice label is formatted as "Category: R Amount".
    // We extract the exact category name by splitting at the colon.
    QString category = slice->label().section(':', 0, 0).trimmed();

    QMap<QString, double> projectTotals;

    // Scan the model and aggregate spending by Project for the clicked Category
    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        QString rowCategory = expenseModel->item(row, 2)->text();
        if (rowCategory == category) {
            QString project = expenseModel->item(row, 1)->text();
            double amount = expenseModel->item(row, 4)->text().toDouble();
            projectTotals[project] += amount;
        }
    }

    // Create a new modal dialog to display the secondary breakdown chart
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(category + " - Project Breakdown");
    dialog->resize(500, 400);

    QPieSeries *projectSeries = new QPieSeries();

    // Populate the sub-chart series
    for (auto it = projectTotals.begin(); it != projectTotals.end(); ++it) {
        QPieSlice *pSlice = projectSeries->append(it.key(), it.value());
        pSlice->setLabelVisible(true);
        pSlice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }

    QChart *projectChart = new QChart();
    projectChart->addSeries(projectSeries);
    projectChart->setTitle(QString("Budget Distribution for '%1'").arg(category));
    projectChart->setAnimationOptions(QChart::SeriesAnimations);
    projectChart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *pChartView = new QChartView(projectChart);
    pChartView->setRenderHint(QPainter::Antialiasing);

    // Layout the dialog
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(pChartView);

    // Add a close button
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    // Execute the dialog modally
    dialog->exec();

    // Clean up
    delete dialog;
}