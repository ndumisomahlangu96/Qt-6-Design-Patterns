#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    setWindowTitle("Finance Manager");

    setupDatabase();
    setupModel();
    setupFlexibleUI();
    setupMenuBar();

    updateChart();
}

MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Close the database connection if it's open before the application exits
    if (db.isOpen()) {
        db.close();
    }

    // Accept the event to allow the window to close
    event->accept();
}

void MainWindow::setupDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::currentPath() + "/expenses.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS projects (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE)");
    query.exec("CREATE TABLE IF NOT EXISTS categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT UNIQUE)");
    query.exec("CREATE TABLE IF NOT EXISTS expenses ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "date_val TEXT, "
               "project_id INTEGER, "
               "category_id INTEGER, "
               "description TEXT, "
               "amount REAL, "
               "FOREIGN KEY(project_id) REFERENCES projects(id), "
               "FOREIGN KEY(category_id) REFERENCES categories(id))");
}

void MainWindow::setupModel()
{
    expenseModel = new ExpenseTableModel(this, db); // Using our custom model class
    expenseModel->setTable("expenses");

    // Set relations for Foreign Keys
    expenseModel->setRelation(2, QSqlRelation("projects", "id", "name"));
    expenseModel->setRelation(3, QSqlRelation("categories", "id", "name"));

    expenseModel->setHeaderData(1, Qt::Horizontal, "Date");
    expenseModel->setHeaderData(2, Qt::Horizontal, "Project/Site");
    expenseModel->setHeaderData(3, Qt::Horizontal, "Category");
    expenseModel->setHeaderData(4, Qt::Horizontal, "Description");
    expenseModel->setHeaderData(5, Qt::Horizontal, "Amount");

    expenseModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    expenseModel->select();

    connect(expenseModel, &QSqlTableModel::dataChanged, this, &MainWindow::updateChart);
}

void MainWindow::setupMenuBar()
{
    QStyle *style = QApplication::style();

    QMenu *fileMenu = menuBar()->addMenu("File");

    QAction *newAction = new QAction(style->standardIcon(QStyle::SP_FileIcon), "Clear Database", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onClearDatabase);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(style->standardIcon(QStyle::SP_DirOpenIcon), "Open File ...", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addAction(openAction);

    // NEW: Save to currently opened file (Ctrl+S)
    QAction *saveCurrentAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
    saveCurrentAction->setShortcut(QKeySequence::Save); // Ctrl+S
    connect(saveCurrentAction, &QAction::triggered, this, &MainWindow::onSaveCurrentFile);
    fileMenu->addAction(saveCurrentAction);

    QAction *saveAction = new QAction(style->standardIcon(QStyle::SP_ArrowDown), "Save to File ...", this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    fileMenu->addAction(saveAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);

    QMenu *editMenu = menuBar()->addMenu("Edit");
    QAction *deleteAction = new QAction(style->standardIcon(QStyle::SP_TrashIcon), "Delete Selected Record", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    editMenu->addAction(deleteAction);

    QAction *sortAction = new QAction(style->standardIcon(QStyle::SP_BrowserReload), "Sort Ledger...", this);
    connect(sortAction, &QAction::triggered, this, &MainWindow::onSortLedger);
    editMenu->addAction(sortAction);

    QAction *findAction = new QAction(QIcon::fromTheme("edit-find-replace", style->standardIcon(QStyle::SP_FileDialogContentsView)), "Find and Replace...", this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::onToggleFindReplace);
    editMenu->addAction(findAction);

    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxInformation), "About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);

    QToolBar *mainToolBar = new QToolBar("Main Toolbar", this);
    mainToolBar->setMovable(false);
    this->addToolBar(Qt::TopToolBarArea, mainToolBar);

    mainToolBar->addAction(newAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addSeparator();

    QAction *totalSpendingAction = new QAction(style->standardIcon(QStyle::SP_DialogApplyButton), "Total Spending", this);
    connect(totalSpendingAction, &QAction::triggered, this, &MainWindow::onCalculateTotalSpending);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mainToolBar->addAction(totalSpendingAction);

    QAction *statsAction = new QAction(style->standardIcon(QStyle::SP_FileDialogDetailedView), "Ledger Statistics", this);
    connect(statsAction, &QAction::triggered, this, &MainWindow::onGenerateStats);
    mainToolBar->addAction(statsAction);
}

void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
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

    categoryCombo = new QComboBox();
    categoryCombo->setEditable(true);

    // Apply Validator to Combo Boxes to prevent special characters in Names
    QRegularExpression nameRegex("^[a-zA-Z0-9\\s\\-_]+$");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    projectCombo->lineEdit()->setValidator(nameValidator);
    categoryCombo->lineEdit()->setValidator(nameValidator);

    // Populate combos from DB
    QSqlQuery q("SELECT name FROM projects");
    while(q.next()) projectCombo->addItem(q.value(0).toString());
    q.exec("SELECT name FROM categories");
    while(q.next()) categoryCombo->addItem(q.value(0).toString());

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");
    // Note: QDoubleSpinBox inherently validates numeric entry. No extra validator needed here.

    descriptionEdit = new QLineEdit();
    // Apply Validator to Description to prevent SQL injection artifacts and garbage strings
    QRegularExpression descRegex("^[a-zA-Z0-9\\s\\-_.,'\"()]+$");
    QRegularExpressionValidator *descValidator = new QRegularExpressionValidator(descRegex, this);
    descriptionEdit->setValidator(descValidator);

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
    ledgerView->setModel(expenseModel); // Reverted back to expenseModel
    ledgerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ledgerView->setAlternatingRowColors(true);
    ledgerView->hideColumn(0); // Hide the primary key ID column
    ledgerLayout->addWidget(ledgerView);

    // --- FIND / REPLACE WIDGET ---
    findReplaceWidget = new QWidget();
    QGridLayout *searchLayout = new QGridLayout(findReplaceWidget);
    searchLayout->setContentsMargins(0, 5, 0, 0);

    QLabel *findLabel = new QLabel("Find:");
    findInput = new QLineEdit();
    findInput->setPlaceholderText("Find");
    findInput->setClearButtonEnabled(true);
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

    ledgerLayout->addWidget(findReplaceWidget);
    findReplaceWidget->setVisible(false);

    // --- TAB 3: Visual Dashboard ---
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardTab);

    chartTypeCombo = new QComboBox();
    // NEW: Added the Stacked Bar chart option to the combo box
    chartTypeCombo->addItems({"Expenses by Category (Pie Chart)",
                              "Cash Flow over Time (Line Chart)",
                              "Expenses by Project & Category (Stacked Bar)"});
    connect(chartTypeCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onChartTypeChanged);
    dashboardLayout->addWidget(chartTypeCombo);

    chartStack = new QStackedWidget();

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

    // --- NEW: Stacked Bar Chart Setup ---
    barSeries = new QStackedBarSeries();
    barChart = new QChart();
    barChart->addSeries(barSeries);
    barChart->setTitle("Expenses Grouped by Project (Stacked by Category)");
    barChart->setAnimationOptions(QChart::SeriesAnimations);

    barAxisX = new QBarCategoryAxis();
    barChart->addAxis(barAxisX, Qt::AlignBottom);
    barSeries->attachAxis(barAxisX);

    barAxisY = new QValueAxis();
    barAxisY->setLabelFormat("R %.2f");
    barAxisY->setTitleText("Total Amount");
    barChart->addAxis(barAxisY, Qt::AlignLeft);
    barSeries->attachAxis(barAxisY);

    barChartView = new QChartView(barChart);
    barChartView->setRenderHint(QPainter::Antialiasing);
    chartStack->addWidget(barChartView);

    dashboardLayout->addWidget(chartStack);

    mainTabWidget->addTab(entryTab, "Data Entry");
    mainTabWidget->addTab(ledgerTab, "Ledger Reports");
    mainTabWidget->addTab(dashboardTab, "Visual Dashboard");

    mainLayout->addWidget(mainTabWidget);
}

int MainWindow::getOrInsertForeignKey(const QString &tableName, const QString &value)
{
    QSqlQuery query;
    query.prepare(QString("SELECT id FROM %1 WHERE name = :name").arg(tableName));
    query.bindValue(":name", value);
    query.exec();

    if (query.next()) {
        return query.value(0).toInt();
    }

    query.prepare(QString("INSERT INTO %1 (name) VALUES (:name)").arg(tableName));
    query.bindValue(":name", value);
    query.exec();
    return query.lastInsertId().toInt();
}

bool MainWindow::saveDatabaseToFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error", "Could not save to file: " + filePath);
        return false;
    }

    QTextStream out(&file);
    out << "Date,Project/Site,Category,Description,Amount\n";

    QSqlQuery query("SELECT e.date_val, p.name, c.name, e.description, e.amount "
                    "FROM expenses e "
                    "JOIN projects p ON e.project_id = p.id "
                    "JOIN categories c ON e.category_id = c.id");

    while (query.next()) {
        out << "\"" << query.value(0).toString() << "\",\""
            << query.value(1).toString() << "\",\""
            << query.value(2).toString() << "\",\""
            << query.value(3).toString() << "\",\""
            << query.value(4).toString() << "\"\n";
    }

    file.close();
    return true;
}

void MainWindow::onAddExpenseClicked()
{
    QString dateStr = dateEdit->date().toString("yyyy-MM-dd");
    QString projectName = projectCombo->currentText();
    QString categoryName = categoryCombo->currentText();
    QString description = descriptionEdit->text();
    double amount = amountSpin->value();

    int projId = getOrInsertForeignKey("projects", projectName);
    int catId = getOrInsertForeignKey("categories", categoryName);

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO expenses (date_val, project_id, category_id, description, amount) "
                        "VALUES (:date, :proj, :cat, :desc, :amt)");
    insertQuery.bindValue(":date", dateStr);
    insertQuery.bindValue(":proj", projId);
    insertQuery.bindValue(":cat", catId);
    insertQuery.bindValue(":desc", description);
    insertQuery.bindValue(":amt", amount);

    if (insertQuery.exec()) {
        expenseModel->select(); // Refresh the table model
        updateChart();
        descriptionEdit->clear();
        amountSpin->setValue(0.0);
        statusBar()->showMessage("Expense recorded successfully to database.", 3000);
    } else {
        QMessageBox::warning(this, "Database Error", "Failed to insert record: " + insertQuery.lastError().text());
    }
}

void MainWindow::updateChart()
{
    categorySeries->clear();
    timeSeries->clear();
    barSeries->clear(); // Clear existing bar series
    barAxisX->clear();  // Clear existing X-axis categories

    // 1. Existing Pie Chart Logic
    QSqlQuery pieQuery("SELECT c.name, SUM(e.amount) "
                       "FROM expenses e "
                       "JOIN categories c ON e.category_id = c.id "
                       "GROUP BY c.name");

    while (pieQuery.next()) {
        QString cat = pieQuery.value(0).toString();
        double total = pieQuery.value(1).toDouble();
        QPieSlice *slice = categorySeries->append(cat, total);
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: R %2").arg(cat).arg(total, 0, 'f', 2));
    }

    // 2. Existing Line Chart Logic
    QSqlQuery lineQuery("SELECT date_val, SUM(amount) FROM expenses GROUP BY date_val ORDER BY date_val ASC");
    double maxAmount = 0.0;
    QDateTime minDate, maxDate;

    while (lineQuery.next()) {
        QDateTime date = QDateTime::fromString(lineQuery.value(0).toString(), "yyyy-MM-dd");
        double dailyTotal = lineQuery.value(1).toDouble();

        if (minDate.isNull() || date < minDate) minDate = date;
        if (maxDate.isNull() || date > maxDate) maxDate = date;
        if (dailyTotal > maxAmount) maxAmount = dailyTotal;

        timeSeries->append(date.toMSecsSinceEpoch(), dailyTotal);
    }

    if (!minDate.isNull()) {
        if (minDate == maxDate) {
            minDate = minDate.addDays(-1);
            maxDate = maxDate.addDays(1);
        }
        axisX->setRange(minDate, maxDate);
        axisY->setRange(0, maxAmount * 1.1);
    }

    // --- NEW: Stacked Bar Chart Logic ---

    // Get ordered list of projects for the X-Axis categories
    QStringList projectNames;
    QSqlQuery projQuery("SELECT name FROM projects ORDER BY name");
    while (projQuery.next()) {
        projectNames << projQuery.value(0).toString();
    }

    if (!projectNames.isEmpty()) {
        barAxisX->append(projectNames);
    } else {
        barAxisX->append("No Projects");
    }

    // Fetch data grouped by Category and Project
    QSqlQuery barQuery("SELECT c.name, p.name, SUM(e.amount) "
                       "FROM expenses e "
                       "JOIN categories c ON e.category_id = c.id "
                       "JOIN projects p ON e.project_id = p.id "
                       "GROUP BY c.name, p.name");

    // Map: Category Name -> (Map: Project Name -> Amount)
    QMap<QString, QMap<QString, double>> categoryProjectValues;
    QMap<QString, double> projectTotals;
    double maxBarHeight = 0.0;

    while (barQuery.next()) {
        QString catName = barQuery.value(0).toString();
        QString projName = barQuery.value(1).toString();
        double amount = barQuery.value(2).toDouble();

        categoryProjectValues[catName][projName] = amount;
        projectTotals[projName] += amount;
    }

    // Find the maximum bar height (tallest project stack) for scaling the Y-axis
    for (double total : std::as_const(projectTotals)) {
        if (total > maxBarHeight) maxBarHeight = total;
    }
    barAxisY->setRange(0, maxBarHeight > 0 ? maxBarHeight * 1.1 : 100);

    // Create a QBarSet for each Category
    for (auto it = categoryProjectValues.constBegin(); it != categoryProjectValues.constEnd(); ++it) {
        QString catName = it.key();
        QBarSet *set = new QBarSet(catName);

        // Populate the set in the exact order of the X-axis project list
        for (const QString &projName : projectNames) {
            *set << it.value().value(projName, 0.0);
        }

        barSeries->append(set);
    }
}

void MainWindow::onClearDatabase()
{
    if (QMessageBox::question(this, "Clear Data", "Delete all database records?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q;
        q.exec("DELETE FROM expenses");
        q.exec("DELETE FROM projects");
        q.exec("DELETE FROM categories");

        expenseModel->select();
        updateChart();
        projectCombo->clear();
        categoryCombo->clear();
        statusBar()->showMessage("All database records cleared.", 3000);
    }
}

void MainWindow::onOpenFile()
{
    // Updated file filter to include Text Files and All Files
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
                                                    "All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    currentFilePath = fileName; // <-- Track the opened file path

    QTextStream in(&file);
    bool isFirstLine = true;
    db.transaction();

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (isFirstLine) { isFirstLine = false; continue; }

        QStringList fields = line.split(","); // Simple split for demonstration
        if (fields.size() >= 5) {
            QString date = fields[0].remove('\"');
            QString proj = fields[1].remove('\"');
            QString cat = fields[2].remove('\"');
            QString desc = fields[3].remove('\"');
            double amt = fields[4].remove('\"').toDouble();

            int projId = getOrInsertForeignKey("projects", proj);
            int catId = getOrInsertForeignKey("categories", cat);

            QSqlQuery iq;
            iq.prepare("INSERT INTO expenses (date_val, project_id, category_id, description, amount) VALUES (?, ?, ?, ?, ?)");
            iq.addBindValue(date); iq.addBindValue(projId); iq.addBindValue(catId); iq.addBindValue(desc); iq.addBindValue(amt);
            iq.exec();
        }
    }
    db.commit();
    file.close();

    expenseModel->select();
    updateChart();
    statusBar()->showMessage("File imported to database successfully.", 3000);
}

void MainWindow::onSaveCurrentFile()
{
    if (currentFilePath.isEmpty()) {
        // No file opened yet; fall back to Save As dialog
        onSaveFile();
        return;
    }

    if (saveDatabaseToFile(currentFilePath)) {
        statusBar()->showMessage("Saved to " + currentFilePath, 3000);
    }
}


void MainWindow::onSaveFile()
{
    QString initialDir = currentFilePath.isEmpty() ? QDir::homePath() : currentFilePath;
    QString fileName = QFileDialog::getSaveFileName(this, "Save Expenses Data", initialDir, "Textfile (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    if (saveDatabaseToFile(fileName)) {
        currentFilePath = fileName;
        statusBar()->showMessage("Data saved to file successfully.", 3000);
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
    expenseModel->submitAll();
    expenseModel->select();
    updateChart();
    statusBar()->showMessage("Record deleted.", 3000);
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Finance Manager", "<b>Finance Manager (Database Edition)</b><br><br>Qt SQL application for tracking expenses.");
}

void MainWindow::onSortLedger()
{
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok;
    QString item = QInputDialog::getItem(this, "Sort Ledger", "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        int columnIndex = columns.indexOf(item) + 1; // +1 to account for hidden ID column
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Sort Order",
                                                                  QString("Sort in Ascending order?"), QMessageBox::Yes | QMessageBox::No);

        expenseModel->setSort(columnIndex, (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder);
        expenseModel->select();
    }
}

void MainWindow::onToggleFindReplace()
{
    mainTabWidget->setCurrentIndex(1);
    bool isVisible = findReplaceWidget->isVisible();
    findReplaceWidget->setVisible(!isVisible);

    if (!isVisible) {
        findInput->setFocus();
        findInput->selectAll();
    } else {
        ledgerView->clearSelection();
    }
}

void MainWindow::performSearch()
{
    currentMatches.clear();
    currentMatchIndex = -1;
    ledgerView->clearSelection();

    QString targetText = findInput->text();
    if (targetText.isEmpty()) return;

    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        for (int col = 1; col < expenseModel->columnCount(); ++col) {
            QModelIndex index = expenseModel->index(row, col);
            if (expenseModel->data(index).toString().contains(targetText, Qt::CaseInsensitive)) {
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
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) return;
    QModelIndex index = currentMatches.at(currentMatchIndex);
    ledgerView->setCurrentIndex(index);
    ledgerView->scrollTo(index);
}

void MainWindow::onFindNext()
{
    if (currentMatches.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex + 1) % currentMatches.size();
    highlightCurrentMatch();
}

void MainWindow::onFindPrevious()
{
    if (currentMatches.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex - 1 < 0) ? currentMatches.size() - 1 : currentMatchIndex - 1;
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
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) return;

    QModelIndex index = currentMatches.at(currentMatchIndex);
    QString currentText = expenseModel->data(index).toString();
    currentText.replace(findInput->text(), replaceInput->text(), Qt::CaseInsensitive);

    expenseModel->setData(index, currentText);
    expenseModel->submitAll();
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
    if (targetText.isEmpty()) return;

    int replacedCount = 0;
    for (int row = 0; row < expenseModel->rowCount(); ++row) {
        for (int col = 1; col < expenseModel->columnCount(); ++col) {
            QModelIndex index = expenseModel->index(row, col);
            QString text = expenseModel->data(index).toString();
            if (text.contains(targetText, Qt::CaseInsensitive)) {
                text.replace(targetText, replaceInput->text(), Qt::CaseInsensitive);
                expenseModel->setData(index, text);
                replacedCount++;
            }
        }
    }

    if (replacedCount > 0) {
        expenseModel->submitAll();
        statusBar()->showMessage(QString("Successfully replaced %1 occurrence(s).").arg(replacedCount), 5000);
        performSearch();
    }
}

void MainWindow::onPieSliceClicked(QPieSlice *slice)
{
    QString category = slice->label().section(':', 0, 0).trimmed();

    QSqlQuery q;
    q.prepare("SELECT p.name, SUM(e.amount) "
              "FROM expenses e "
              "JOIN projects p ON e.project_id = p.id "
              "JOIN categories c ON e.category_id = c.id "
              "WHERE c.name = :cat "
              "GROUP BY p.name");
    q.bindValue(":cat", category);
    q.exec();

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(category + " - Project Breakdown");
    dialog->resize(500, 400);

    QPieSeries *projectSeries = new QPieSeries();
    while (q.next()) {
        QPieSlice *pSlice = projectSeries->append(q.value(0).toString(), q.value(1).toDouble());
        pSlice->setLabelVisible(true);
        pSlice->setLabel(QString("%1: R %2").arg(q.value(0).toString()).arg(q.value(1).toDouble(), 0, 'f', 2));
    }

    QChart *projectChart = new QChart();
    projectChart->addSeries(projectSeries);
    projectChart->setTitle(QString("Budget Distribution for '%1'").arg(category));
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
    QSqlQuery q("SELECT SUM(amount) FROM expenses");
    if (q.next()) {
        QString formattedTotal = QString::number(q.value(0).toDouble(), 'f', 2);
        statusBar()->showMessage("Total Spending: R " + formattedTotal, 5000);
        QMessageBox::information(this, "Total Spending", "The total spending for all records is:\n\nR " + formattedTotal);
    }
}

void MainWindow::onGenerateStats()
{
    QSqlQuery q;
    q.exec("SELECT COUNT(*), AVG(amount), MAX(amount), MIN(amount) FROM expenses");

    if (!q.next() || q.value(0).toInt() == 0) {
        QMessageBox::information(this, "Ledger Statistics", "The ledger is currently empty.");
        return;
    }

    int rowCount = q.value(0).toInt();
    double avgExpense = q.value(1).toDouble();
    double maxExpense = q.value(2).toDouble();
    double minExpense = q.value(3).toDouble();

    q.exec("SELECT c.name, COUNT(*) as c_count FROM expenses e JOIN categories c ON e.category_id = c.id GROUP BY c.name ORDER BY c_count DESC LIMIT 1");
    QString topCategory = q.next() ? q.value(0).toString() : "N/A";
    int maxCatCount = q.value(1).toInt();

    q.exec("SELECT p.name, COUNT(*) as p_count FROM expenses e JOIN projects p ON e.project_id = p.id GROUP BY p.name ORDER BY p_count DESC LIMIT 1");
    QString topProject = q.next() ? q.value(0).toString() : "N/A";
    int maxProjCount = q.value(1).toInt();

    QString statsMessage = QString("<h3>Ledger Statistical Summary</h3>"
                                   "<b>Total Records Analyzed:</b> %1<br><br>"
                                   "<b>Average Expense:</b> R %2<br>"
                                   "<b>Highest Single Expense:</b> R %3<br>"
                                   "<b>Lowest Single Expense:</b> R %4<br><br>"
                                   "<b>Most Frequent Category:</b> %5 (%6 entries)<br>"
                                   "<b>Most Frequent Project:</b> %7 (%8 entries)")
                               .arg(rowCount).arg(avgExpense, 0, 'f', 2)
                               .arg(maxExpense, 0, 'f', 2).arg(minExpense, 0, 'f', 2)
                               .arg(topCategory).arg(maxCatCount)
                               .arg(topProject).arg(maxProjCount);

    QMessageBox::information(this, "Ledger Statistics", statsMessage);
}

void MainWindow::onChartTypeChanged(int index)
{
    chartStack->setCurrentIndex(index);
}