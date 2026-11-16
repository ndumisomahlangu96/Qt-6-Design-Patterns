#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Create the central widget programmatically (No UI file needed)
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    // Set the window title explicitly in C++
    setWindowTitle("Finance Manager");

    setupModel();
    setupFlexibleUI();
    setupMenuBar(); // Initialize the menu bar
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupMenuBar()
{
    // Fetch the standard application style to access built-in icons
    QStyle *style = QApplication::style();

    // --- File Menu ---
    QMenu *fileMenu = menuBar()->addMenu("File");

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
    QMenu *editMenu = menuBar()->addMenu("Edit");

    QAction *deleteAction = new QAction(style->standardIcon(QStyle::SP_TrashIcon), "Delete Selected Record", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setStatusTip("Delete the currently selected record from the ledger");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    editMenu->addAction(deleteAction);

    // --- Sort Action ---
    QAction *sortAction = new QAction(style->standardIcon(QStyle::SP_BrowserReload), "Sort Ledger...", this);
    sortAction->setStatusTip("Sort the ledger by a specific variable");
    connect(sortAction, &QAction::triggered, this, &MainWindow::onSortLedger);
    editMenu->addAction(sortAction);

    // ---  Find and Replace Action ---
    QAction *findAction = new QAction(QIcon::fromTheme("edit-find-replace", style->standardIcon(QStyle::SP_FileDialogContentsView)), "Find and Replace...", this);
    findAction->setShortcut(QKeySequence::Find); // Using standard Ctrl+F shortcut
    connect(findAction, &QAction::triggered, this, &MainWindow::onToggleFindReplace);
    editMenu->addAction(findAction);

    // --- Help Menu ---
    QMenu *helpMenu = menuBar()->addMenu("Help");

    QAction *aboutAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxInformation), "About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);

    // --- Main Toolbar ---
    QToolBar *mainToolBar = new QToolBar("Main Toolbar", this);
    mainToolBar->setMovable(false);
    this->addToolBar(Qt::TopToolBarArea, mainToolBar);

    mainToolBar->addAction(newAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addAction(saveAction);
    mainToolBar->addSeparator();

    QAction *totalSpendingAction = new QAction(style->standardIcon(QStyle::SP_DialogApplyButton), "Total Spending", this);
    totalSpendingAction->setStatusTip("Calculate the total of all expenses in the ledger");
    connect(totalSpendingAction, &QAction::triggered, this, &MainWindow::onCalculateTotalSpending);

    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->addAction(totalSpendingAction);

    // --- Ledger Statistics Action ---
    QAction *statsAction = new QAction(style->standardIcon(QStyle::SP_FileDialogDetailedView), "Ledger Statistics", this);
    statsAction->setStatusTip("Generate statistical summary of the current ledger data");
    connect(statsAction, &QAction::triggered, this, &MainWindow::onGenerateStats);
    mainToolBar->addAction(statsAction);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveData();
    event->accept();
}

QString MainWindow::getDataFilePath()
{
    // Retrieve the application's current working directory
    QString path = QDir::currentPath();

    // Return the path with a .txt extension
    return path + "/expenses.txt";
}

void MainWindow::saveData()
{
    QString savePath = currentFilePath.isEmpty() ? getDataFilePath() : currentFilePath;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Date,Project/Site,Category,Description,Amount\n";

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
        currentFilePath = savePath;
        statusBar()->showMessage("Data saved successfully to: " + currentFilePath, 3000);
    }
    else
    {
        statusBar()->showMessage("Failed to save data.", 3000);
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
        out << "Date,Project/Site,Category,Description,Amount\n";

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
        currentFilePath = fileName;
        statusBar()->showMessage("File saved successfully to new location.", 3000);
    } else {
        QMessageBox::warning(this, "Save As Error", "Could not write to the selected file.");
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Expense File", QDir::homePath(), "All Files (*);;CSV Files (*.csv)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Error", "Could not open the selected file.");
        return;
    }

    currentFilePath = fileName;
    expenseModel->removeRows(0, expenseModel->rowCount());

    QTextStream in(&file);
    bool isFirstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }

        QStringList fields;
        QString currentField;
        bool inQuotes = false;

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

        if (fields.size() >= 5) {
            QList<QStandardItem *> newRow;
            for (int col = 0; col < 5; ++col) {
                QStandardItem *item = new QStandardItem(fields[col]);
                if (col == 4) {
                    item->setData(fields[col].toDouble(), Qt::UserRole);
                } else {
                    item->setData(fields[col], Qt::UserRole);
                }
                newRow << item;
            }
            expenseModel->appendRow(newRow);
        }
    }
    file.close();

    updateChart();
    statusBar()->showMessage("File opened successfully.", 3000);
}

void MainWindow::setupModel()
{
    expenseModel = new QStandardItemModel(0, 5, this);
    expenseModel->setHorizontalHeaderLabels({"Date", "Project/Site", "Category", "Description", "Amount"});
    expenseModel->setSortRole(Qt::UserRole);
    connect(expenseModel, &QStandardItemModel::itemChanged, this, &MainWindow::updateChart);
}

void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    // FIX: Initialize the class member instead of a local 'QTabWidget *tabWidget'
    mainTabWidget = new QTabWidget(this);

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

    // --- BUILD THE FIND/REPLACE WIDGET ---
    findReplaceWidget = new QWidget();
    QGridLayout *searchLayout = new QGridLayout(findReplaceWidget);
    searchLayout->setContentsMargins(0, 5, 0, 0); // Little margin at top to separate from table

    // Row 0: Find
    QLabel *findLabel = new QLabel("Find:");
    findInput = new QLineEdit();
    findInput->setPlaceholderText("Find");
    findInput->setClearButtonEnabled(true);

    // Detect when text changes to re-run the search
    connect(findInput, &QLineEdit::textChanged, this, &MainWindow::performSearch);

    QPushButton *btnFindPrev = new QPushButton("Find Previous");
    QPushButton *btnFindNext = new QPushButton("Find Next");
    QPushButton *btnSelectAll = new QPushButton("Select All");

    connect(btnFindPrev, &QPushButton::clicked, this, &MainWindow::onFindPrevious);
    connect(btnFindNext, &QPushButton::clicked, this, &MainWindow::onFindNext);
    connect(btnSelectAll, &QPushButton::clicked, this, &MainWindow::onSelectAllMatches);

    searchLayout->addWidget(findLabel, 0, 0);
    searchLayout->addWidget(findInput, 0, 1);
    searchLayout->addWidget(btnFindPrev, 0, 2);
    searchLayout->addWidget(btnFindNext, 0, 3);
    searchLayout->addWidget(btnSelectAll, 0, 4);

    // Row 1: Replace
    QLabel *replaceLabel = new QLabel("Replace with:");
    replaceInput = new QLineEdit();

    QPushButton *btnReplace = new QPushButton("Replace");
    QPushButton *btnReplaceFind = new QPushButton("Replace && Find");
    QPushButton *btnReplaceAll = new QPushButton("Replace All");

    connect(btnReplace, &QPushButton::clicked, this, &MainWindow::onReplace);
    connect(btnReplaceFind, &QPushButton::clicked, this, &MainWindow::onReplaceAndFind);
    connect(btnReplaceAll, &QPushButton::clicked, this, &MainWindow::onReplaceAll);

    searchLayout->addWidget(replaceLabel, 1, 0);
    searchLayout->addWidget(replaceInput, 1, 1);
    searchLayout->addWidget(btnReplace, 1, 2);
    searchLayout->addWidget(btnReplaceFind, 1, 3);
    searchLayout->addWidget(btnReplaceAll, 1, 4);

    // Add search widget to the bottom of the ledger layout, initially hidden
    ledgerLayout->addWidget(findReplaceWidget);
    findReplaceWidget->setVisible(false);

    // --- TAB 3: Visual Dashboard ---
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardTab);

    // Dropdown to switch charts
    chartTypeCombo = new QComboBox();
    chartTypeCombo->addItems({"Expenses by Category (Pie Chart)", "Cash Flow over Time (Line Chart)"});
    connect(chartTypeCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onChartTypeChanged);
    dashboardLayout->addWidget(chartTypeCombo);

    chartStack = new QStackedWidget();

    // 1. Setup Pie Chart
    categorySeries = new QPieSeries();
    connect(categorySeries, &QPieSeries::clicked, this, &MainWindow::onPieSliceClicked);

    pieChart = new QChart();
    pieChart->addSeries(categorySeries);
    pieChart->setTitle("Expenses by Category");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->setAlignment(Qt::AlignRight);

    pieChartView = new QChartView(pieChart);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    chartStack->addWidget(pieChartView);

    // 2. Setup Time-Series Line Chart
    timeSeries = new QLineSeries();
    timeSeries->setName("Total Expenses");

    lineChart = new QChart();
    lineChart->addSeries(timeSeries);
    lineChart->setTitle("Cash Flow Over Time");
    lineChart->setAnimationOptions(QChart::SeriesAnimations);

    axisX = new QDateTimeAxis();
    axisX->setTickCount(5);
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    lineChart->addAxis(axisX, Qt::AlignBottom);
    timeSeries->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setLabelFormat("R %.2f");
    axisY->setTitleText("Amount Spent");
    lineChart->addAxis(axisY, Qt::AlignLeft);
    timeSeries->attachAxis(axisY);

    lineChartView = new QChartView(lineChart);
    lineChartView->setRenderHint(QPainter::Antialiasing);
    chartStack->addWidget(lineChartView);

    dashboardLayout->addWidget(chartStack);

    mainTabWidget->addTab(entryTab, "Data Entry");
    mainTabWidget->addTab(ledgerTab, "Ledger Reports");
    mainTabWidget->addTab(dashboardTab, "Visual Dashboard");

    mainLayout->addWidget(mainTabWidget);
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
    amountItem->setData(amountSpin->value(), Qt::UserRole);
    newRow << amountItem;

    expenseModel->appendRow(newRow);

    descriptionEdit->clear();
    amountSpin->setValue(0.0);

    updateChart();
}

void MainWindow::updateChart()
{
    // 1. Clear both existing chart series
    categorySeries->clear();
    timeSeries->clear();

    QMap<QString, double> categoryTotals;
    QMap<QDateTime, double> dateTotals;

    // 2. Extract data from the model
    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        // --- Pie Chart Data (Category) ---
        QString category = expenseModel->item(row, 2)->text();
        double amount = expenseModel->item(row, 4)->text().toDouble();
        categoryTotals[category] += amount;

        // --- Line Chart Data (Date) ---
        QString dateStr = expenseModel->item(row, 0)->text();
        QDateTime date = QDateTime::fromString(dateStr, "yyyy-MM-dd");
        if (date.isValid()) {
            // QMap automatically sorts by QDateTime (chronologically)
            dateTotals[date] += amount;
        }
    }

    // 3. Populate Pie Chart
    for (auto it = categoryTotals.begin(); it != categoryTotals.end(); ++it) {
        QPieSlice *slice = categorySeries->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }

    // 4. Populate Line Chart and Scale Axes
    if (!dateTotals.isEmpty()) {
        double maxAmount = 0.0;

        // Populate the series using milliseconds since epoch for QDateTimeAxis
        for (auto it = dateTotals.begin(); it != dateTotals.end(); ++it) {
            timeSeries->append(it.key().toMSecsSinceEpoch(), it.value());
            if (it.value() > maxAmount) {
                maxAmount = it.value();
            }
        }

        // Dynamically adjust the X and Y axes ranges so the chart draws properly
        QDateTime minDate = dateTotals.firstKey();
        QDateTime maxDate = dateTotals.lastKey();

        // If there's only one date, add some padding to the X-axis so it draws a visible point
        if (minDate == maxDate) {
            minDate = minDate.addDays(-1);
            maxDate = maxDate.addDays(1);
        }

        axisX->setRange(minDate, maxDate);
        axisY->setRange(0, maxAmount * 1.1); // Add 10% headroom to the top of the graph
    }
}
void MainWindow::onNewData()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Data", "Are you sure you want to clear all expense records?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        expenseModel->removeRows(0, expenseModel->rowCount());
        updateChart();
        statusBar()->showMessage("All records cleared.", 3000);
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
        out << "Date,Project/Site,Category,Description,Amount\n";

        for (int row = 0; row < expenseModel->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < expenseModel->columnCount(); ++col) {
                rowData << "\"" + expenseModel->item(row, col)->text() + "\"";
            }
            out << rowData.join(",") << "\n";
        }

        file.close();
        statusBar()->showMessage("Data exported to CSV successfully.", 3000);
    } else {
        QMessageBox::warning(this, "Export Error", "Could not write to the selected file.");
    }
}

void MainWindow::onExit()
{
    this->close();
}

void MainWindow::onDeleteSelected()
{
    QModelIndexList selectedRows = ledgerView->selectionModel()->selectedRows();

    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a row in the Ledger Reports tab to delete.");
        return;
    }

    for (int i = selectedRows.count() - 1; i >= 0; i--) {
        expenseModel->removeRow(selectedRows.at(i).row());
    }

    updateChart();
    statusBar()->showMessage("Record deleted.", 3000);
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
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok;

    QString item = QInputDialog::getItem(this, "Sort Ledger",
                                         "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        int columnIndex = columns.indexOf(item);
        if (columnIndex != -1) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Sort Order",
                                          QString("Sort the '%1' column in Ascending order?\n(Select 'No' for Descending)").arg(item),
                                          QMessageBox::Yes | QMessageBox::No);

            Qt::SortOrder order = (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder;
            expenseModel->sort(columnIndex, order);

            statusBar()->showMessage(QString("Ledger sorted by %1.").arg(item), 3000);
        }
    }
}

// --- NEW SEARCH LOGIC AND SLOT IMPLEMENTATIONS ---

void MainWindow::onToggleFindReplace()
{
    // Force switch to the Ledger tab (index 1) to see the search bar
    mainTabWidget->setCurrentIndex(1);

    bool isVisible = findReplaceWidget->isVisible();
    findReplaceWidget->setVisible(!isVisible);

    if (!isVisible) {
        findInput->setFocus();
        findInput->selectAll();
    } else {
        // Clear selection when closing
        ledgerView->clearSelection();
    }
}

void MainWindow::performSearch()
{
    currentMatches.clear();
    currentMatchIndex = -1;
    ledgerView->clearSelection();

    QString targetText = findInput->text();
    if (targetText.isEmpty()) {
        return;
    }

    // Scan the entire model for matches
    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        for (int col = 0; col < expenseModel->columnCount(); ++col) {
            QModelIndex index = expenseModel->index(row, col);
            QString cellData = expenseModel->data(index).toString();

            if (cellData.contains(targetText, Qt::CaseInsensitive)) {
                currentMatches.append(index);
            }
        }
    }

    if (!currentMatches.isEmpty()) {
        currentMatchIndex = 0;
        highlightCurrentMatch();
    }
}

void MainWindow::highlightCurrentMatch()
{
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) {
        return;
    }

    QModelIndex index = currentMatches.at(currentMatchIndex);
    ledgerView->setCurrentIndex(index);
    ledgerView->scrollTo(index);
}

void MainWindow::onFindNext()
{
    if (currentMatches.isEmpty()) return;

    currentMatchIndex++;
    if (currentMatchIndex >= currentMatches.size()) {
        currentMatchIndex = 0; // Wrap around to top
    }
    highlightCurrentMatch();
}

void MainWindow::onFindPrevious()
{
    if (currentMatches.isEmpty()) return;

    currentMatchIndex--;
    if (currentMatchIndex < 0) {
        currentMatchIndex = currentMatches.size() - 1; // Wrap around to bottom
    }
    highlightCurrentMatch();
}

void MainWindow::onSelectAllMatches()
{
    ledgerView->clearSelection();
    for (const QModelIndex &index : std::as_const(currentMatches)) {
        ledgerView->selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

void MainWindow::onReplace()
{
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) {
        return;
    }

    QModelIndex index = currentMatches.at(currentMatchIndex);
    QString currentText = expenseModel->data(index).toString();
    QString targetText = findInput->text();
    QString replaceText = replaceInput->text();

    currentText.replace(targetText, replaceText, Qt::CaseInsensitive);

    // Update the item
    QStandardItem *item = expenseModel->itemFromIndex(index);
    item->setText(currentText);

    if (index.column() == 4) {
        item->setData(currentText.toDouble(), Qt::UserRole);
    } else {
        item->setData(currentText, Qt::UserRole);
    }

    // We modified the data, we must search again to update the match list
    performSearch();
}

void MainWindow::onReplaceAndFind()
{
    onReplace();
    onFindNext();
}

void MainWindow::onReplaceAll()
{
    QString targetText = findInput->text();
    QString replaceText = replaceInput->text();

    if (targetText.isEmpty()) return;

    int replacedCount = 0;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        for (int col = 0; col < expenseModel->columnCount(); ++col) {
            QStandardItem *item = expenseModel->item(row, col);
            QString currentText = item->text();

            if (currentText.contains(targetText, Qt::CaseInsensitive)) {
                currentText.replace(targetText, replaceText, Qt::CaseInsensitive);
                item->setText(currentText);

                if (col == 4) {
                    item->setData(currentText.toDouble(), Qt::UserRole);
                } else {
                    item->setData(currentText, Qt::UserRole);
                }

                replacedCount++;
            }
        }
    }

    if (replacedCount > 0) {
        statusBar()->showMessage(QString("Successfully replaced %1 occurrence(s).").arg(replacedCount), 5000);
        performSearch(); // Refresh highlight list
    } else {
        QMessageBox::information(this, "Not Found", "Could not find '" + targetText + "' in the database.");
    }
}

void MainWindow::onPieSliceClicked(QPieSlice *slice)
{
    QString category = slice->label().section(':', 0, 0).trimmed();
    QMap<QString, double> projectTotals;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        QString rowCategory = expenseModel->item(row, 2)->text();
        if (rowCategory == category) {
            QString project = expenseModel->item(row, 1)->text();
            double amount = expenseModel->item(row, 4)->text().toDouble();
            projectTotals[project] += amount;
        }
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(category + " - Project Breakdown");
    dialog->resize(500, 400);

    QPieSeries *projectSeries = new QPieSeries();

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

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(pChartView);

    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dialog->exec();
    delete dialog;
}

void MainWindow::onCalculateTotalSpending()
{
    double total = 0.0;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        double amount = expenseModel->item(row, 4)->data(Qt::UserRole).toDouble();
        total += amount;
    }

    QString formattedTotal = QString::number(total, 'f', 2);
    statusBar()->showMessage("Total Spending: R " + formattedTotal, 5000);

    QMessageBox::information(this, "Total Spending",
                             QString("The total spending for all records is:\n\nR %1").arg(formattedTotal));
}

void MainWindow::onGenerateStats()
{
    int rowCount = expenseModel->rowCount();
    if (rowCount == 0) {
        QMessageBox::information(this, "Ledger Statistics", "The ledger is currently empty. Add data to view statistics.");
        return;
    }

    double minExpense = expenseModel->item(0, 4)->data(Qt::UserRole).toDouble();
    double maxExpense = minExpense;
    double totalExpense = 0.0;

    QMap<QString, int> categoryCounts;
    QMap<QString, int> projectCounts;

    for (int row = 0; row < rowCount; ++row) {
        double amount = expenseModel->item(row, 4)->data(Qt::UserRole).toDouble();
        QString project = expenseModel->item(row, 1)->text();
        QString category = expenseModel->item(row, 2)->text();

        if (amount < minExpense) minExpense = amount;
        if (amount > maxExpense) maxExpense = amount;
        totalExpense += amount;

        categoryCounts[category]++;
        projectCounts[project]++;
    }

    double avgExpense = totalExpense / rowCount;

    QString topCategory;
    int maxCatCount = 0;
    for (auto it = categoryCounts.begin(); it != categoryCounts.end(); ++it) {
        if (it.value() > maxCatCount) {
            maxCatCount = it.value();
            topCategory = it.key();
        }
    }

    QString topProject;
    int maxProjCount = 0;
    for (auto it = projectCounts.begin(); it != projectCounts.end(); ++it) {
        if (it.value() > maxProjCount) {
            maxProjCount = it.value();
            topProject = it.key();
        }
    }

    QString statsMessage = QString(
                               "<h3>Ledger Statistical Summary</h3>"
                               "<b>Total Records Analyzed:</b> %1<br><br>"
                               "<b>Average Expense:</b> R %2<br>"
                               "<b>Highest Single Expense:</b> R %3<br>"
                               "<b>Lowest Single Expense:</b> R %4<br><br>"
                               "<b>Most Frequent Category:</b> %5 (%6 entries)<br>"
                               "<b>Most Frequent Project:</b> %7 (%8 entries)"
                               ).arg(rowCount)
                               .arg(avgExpense, 0, 'f', 2)
                               .arg(maxExpense, 0, 'f', 2)
                               .arg(minExpense, 0, 'f', 2)
                               .arg(topCategory).arg(maxCatCount)
                               .arg(topProject).arg(maxProjCount);

    QMessageBox::information(this, "Ledger Statistics", statsMessage);
}

void MainWindow::onChartTypeChanged(int index)
{
    chartStack->setCurrentIndex(index);
}