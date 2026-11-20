#include "mainwindow.h"

/**
 * @brief Constructs the MainWindow and bootstraps all application subsystems.
 *
 * Sets up the central widget, initializes the SQLite database and relational
 * table model, builds the tabbed UI and menus/toolbar, and performs an initial
 * render of all dashboard charts.
 */
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

/**
 * @brief Destructor that ensures the SQLite database connection is closed safely.
 */
MainWindow::~MainWindow()
{
    if (db.isOpen()) {
        db.close();
    }
}

/**
 * @brief Handles the window close event to release database resources cleanly.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (db.isOpen()) {
        db.close();
    }

    event->accept();
}

/**
 * @brief Opens the local SQLite database and creates the normalized schema if needed.
 *
 * Connects to `expenses.db` in the current working directory and ensures three
 * tables exist: `projects`, `categories`, and `expenses`.
 */
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

/**
 * @brief Configures the relational table model backing the Ledger Reports view.
 *
 * Maps foreign key columns (`project_id` and `category_id`) to display their
 * human-readable names, sets column headers, enables immediate persistence on
 * field edits (`OnFieldChange`), and connects value changes to `updateChart()`.
 */
void MainWindow::setupModel()
{
    expenseModel = new ExpenseTableModel(this, db);
    expenseModel->setTable("expenses");

    // Map foreign key columns (2 = project_id, 3 = category_id) to their lookup names
    expenseModel->setRelation(2, QSqlRelation("projects", "id", "name"));
    expenseModel->setRelation(3, QSqlRelation("categories", "id", "name"));

    expenseModel->setHeaderData(1, Qt::Horizontal, "Date");
    expenseModel->setHeaderData(2, Qt::Horizontal, "Project/Site");
    expenseModel->setHeaderData(3, Qt::Horizontal, "Category");
    expenseModel->setHeaderData(4, Qt::Horizontal, "Description");
    expenseModel->setHeaderData(5, Qt::Horizontal, "Amount");

    expenseModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    expenseModel->select();

    // Only refresh charts/stats on actual data edits (ignore pure visual-role updates)
    connect(expenseModel, &QSqlTableModel::dataChanged, this,
            [this](const QModelIndex &, const QModelIndex &, const QList<int> &roles) {
                if (roles.isEmpty() || roles.contains(Qt::DisplayRole) || roles.contains(Qt::EditRole)) {
                    updateChart();
                }
            });
}

/**
 * @brief Builds the application menu bar (File, Edit, Help) and top toolbar.
 *
 * Registers keyboard shortcuts, assigns standard Qt icons, and adds analytics
 * actions including Total Spending, Ledger Statistics, Statistical Distribution,
 * and Anomaly Detection to the main toolbar.
 */
void MainWindow::setupMenuBar()
{
    QStyle *style = QApplication::style();

    // --- File Menu ---
    QMenu *fileMenu = menuBar()->addMenu("File");

    QAction *newAction = new QAction(style->standardIcon(QStyle::SP_FileIcon), "Clear Database", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onClearDatabase);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(style->standardIcon(QStyle::SP_DirOpenIcon), "Open File ...", this);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addAction(openAction);

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

    // --- Edit Menu ---
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

    // --- Help Menu ---
    QMenu *helpMenu = menuBar()->addMenu("Help");
    QAction *aboutAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxInformation), "About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);

    // --- Main Toolbar ---
    QToolBar *mainToolBar = new QToolBar("Main Toolbar", this);
    mainToolBar->setMovable(false);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    this->addToolBar(Qt::TopToolBarArea, mainToolBar);

    mainToolBar->addAction(newAction);
    mainToolBar->addAction(openAction);
    mainToolBar->addSeparator();

    QAction *totalSpendingAction = new QAction(style->standardIcon(QStyle::SP_DialogApplyButton), "Total Spending", this);
    connect(totalSpendingAction, &QAction::triggered, this, &MainWindow::onCalculateTotalSpending);
    mainToolBar->addAction(totalSpendingAction);

    QAction *statsAction = new QAction(style->standardIcon(QStyle::SP_FileDialogDetailedView), "Ledger Statistics", this);
    connect(statsAction, &QAction::triggered, this, &MainWindow::onGenerateStats);
    mainToolBar->addAction(statsAction);

    QAction *statDistAction = new QAction(style->standardIcon(QStyle::SP_FileDialogInfoView), "Statistical Distribution", this);
    connect(statDistAction, &QAction::triggered, this, &MainWindow::onShowStatisticalDistribution);
    mainToolBar->addAction(statDistAction);

    anomalyDetectionAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxWarning), "Anomaly Detection", this);
    anomalyDetectionAction->setCheckable(true);
    anomalyDetectionAction->setToolTip("Highlight outlier transactions (|x - μ| > 2σ or > Q3 + 1.5×IQR) in the ledger");
    connect(anomalyDetectionAction, &QAction::toggled, this, &MainWindow::onToggleAnomalyDetection);
    mainToolBar->addAction(anomalyDetectionAction);
}

/**
 * @brief Constructs the main three-tab user interface inside the central widget.
 *
 * - Tab 1 ("Data Entry"): Input form with regex-validated fields and currency spin box.
 * - Tab 2 ("Ledger Reports"): Editable QTableView paired with a collapsible Find/Replace bar.
 * - Tab 3 ("Visual Dashboard"): Chart selector driving a QStackedWidget of Pie, Line,
 *   and Stacked Bar charts.
 */
void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());
    mainTabWidget = new QTabWidget(this);

    // =========================================================================
    // TAB 1: Expense Entry Form
    // =========================================================================
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

    // Restrict project and category names to alphanumeric characters, spaces, hyphens, and underscores
    QRegularExpression nameRegex("^[a-zA-Z0-9\\s\\-_]+$");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    projectCombo->lineEdit()->setValidator(nameValidator);
    categoryCombo->lineEdit()->setValidator(nameValidator);

    // Pre-populate combo boxes with existing projects and categories from the database
    QSqlQuery q("SELECT name FROM projects");
    while(q.next()) projectCombo->addItem(q.value(0).toString());
    q.exec("SELECT name FROM categories");
    while(q.next()) categoryCombo->addItem(q.value(0).toString());

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");

    descriptionEdit = new QLineEdit();
    // Restrict description text to safe alphanumeric and standard punctuation characters
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

    // =========================================================================
    // TAB 2: Ledger View & Collapsible Find/Replace Bar
    // =========================================================================
    QWidget *ledgerTab = new QWidget();
    QVBoxLayout *ledgerLayout = new QVBoxLayout(ledgerTab);

    ledgerView = new QTableView();
    ledgerView->setModel(expenseModel);
    ledgerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ledgerView->setAlternatingRowColors(true);
    ledgerView->hideColumn(0); // Hide the internal auto-increment primary key column
    ledgerLayout->addWidget(ledgerView);

    // Build Find / Replace widget (hidden by default until toggled via Edit -> Find)
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

    // =========================================================================
    // TAB 3: Visual Dashboard (Pie, Line, and Stacked Bar Charts)
    // =========================================================================
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashboardLayout = new QVBoxLayout(dashboardTab);

    chartTypeCombo = new QComboBox();
    chartTypeCombo->addItems({"Expenses by Category (Pie Chart)",
                              "Cash Flow over Time (Line Chart)",
                              "Expenses by Project & Category (Stacked Bar)"});
    connect(chartTypeCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onChartTypeChanged);
    dashboardLayout->addWidget(chartTypeCombo);

    chartStack = new QStackedWidget();

    // --- Chart 0: Category Pie Chart (clickable for project drill-down) ---
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

    // --- Chart 1: Cash Flow Line Chart (daily totals over time) ---
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

    // --- Chart 2: Stacked Bar Chart (expenses by project, segmented by category) ---
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

/**
 * @brief Retrieves the primary key ID for a lookup value, inserting it if absent.
 */
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

/**
 * @brief Exports all joined expense records from SQLite to a CSV-formatted file.
 */
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

/**
 * @brief Computes a linearly interpolated percentile on a pre-sorted dataset.
 *
 * Uses inclusive linear interpolation: idx = p * (n - 1).
 *
 * @param sortedValues Ascending-sorted numeric sample values.
 * @param percentile   Fractional quantile in [0.0, 1.0] (e.g., 0.25, 0.50, 0.75).
 * @return Interpolated quantile value, or 0.0 if empty.
 */
double MainWindow::calculatePercentile(const QVector<double> &sortedValues, double percentile)
{
    if (sortedValues.isEmpty()) {
        return 0.0;
    }
    if (sortedValues.size() == 1) {
        return sortedValues.first();
    }

    const double pos = percentile * static_cast<double>(sortedValues.size() - 1);
    const int lower = static_cast<int>(std::floor(pos));
    const int upper = static_cast<int>(std::ceil(pos));
    const double weight = pos - static_cast<double>(lower);

    return sortedValues[lower] + weight * (sortedValues[upper] - sortedValues[lower]);
}

/**
 * @brief Queries all expense amounts and calculates distribution & anomaly metrics.
 *
 * Computes count, sum, mean (mu), min, max, Q1, median (Q2), Q3, IQR,
 * population standard deviation (sigma), outlier bounds (mu +/- 2*sigma and
 * Q3 + 1.5*IQR), and total flagged outlier count.
 */
ExpenseStats MainWindow::computeExpenseStats() const
{
    ExpenseStats stats;
    QSqlQuery q("SELECT amount FROM expenses ORDER BY amount ASC");
    while (q.next()) {
        const double val = q.value(0).toDouble();
        stats.sortedAmounts.append(val);
        stats.sum += val;
    }

    stats.count = stats.sortedAmounts.size();
    if (stats.count == 0) {
        return stats;
    }

    stats.min = stats.sortedAmounts.first();
    stats.max = stats.sortedAmounts.last();
    stats.mean = stats.sum / static_cast<double>(stats.count);

    // Quartiles and Interquartile Range
    stats.q1 = calculatePercentile(stats.sortedAmounts, 0.25);
    stats.median = calculatePercentile(stats.sortedAmounts, 0.50);
    stats.q3 = calculatePercentile(stats.sortedAmounts, 0.75);
    stats.iqr = stats.q3 - stats.q1;

    // Population Standard Deviation (sigma)
    double sumSqDiff = 0.0;
    for (double val : stats.sortedAmounts) {
        const double diff = val - stats.mean;
        sumSqDiff += diff * diff;
    }
    stats.stdDev = std::sqrt(sumSqDiff / static_cast<double>(stats.count));

    // Anomaly Thresholds
    stats.zLowerThreshold = stats.mean - 2.0 * stats.stdDev;
    stats.zUpperThreshold = stats.mean + 2.0 * stats.stdDev;
    stats.iqrUpperFence = stats.q3 + 1.5 * stats.iqr;

    // Count outlier transactions matching either |x - mu| > 2*sigma or x > Q3 + 1.5*IQR
    constexpr double eps = 1e-9;
    for (double val : stats.sortedAmounts) {
        const bool isZOutlier = (stats.stdDev > eps) && (std::abs(val - stats.mean) > (2.0 * stats.stdDev + eps));
        const bool isIqrOutlier = val > (stats.iqrUpperFence + eps);
        if (isZOutlier || isIqrOutlier) {
            stats.outlierCount++;
        }
    }

    return stats;
}

/**
 * @brief Re-synchronizes ExpenseTableModel anomaly thresholds if Anomaly Detection is active.
 */
void MainWindow::refreshAnomalyHighlighting()
{
    if (!anomalyDetectionAction || !anomalyDetectionAction->isChecked()) {
        return;
    }

    const ExpenseStats stats = computeExpenseStats();
    const bool valid = stats.count > 0;
    expenseModel->setAnomalyThresholds(valid, stats.mean, stats.stdDev, stats.iqrUpperFence, valid);
}

/**
 * @brief Inserts a new expense record from the Data Entry form into SQLite.
 */
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
        expenseModel->select(); // Refresh the ledger table model
        updateChart();
        descriptionEdit->clear();
        amountSpin->setValue(0.0);
        statusBar()->showMessage("Expense recorded successfully to database.", 3000);
    } else {
        QMessageBox::warning(this, "Database Error", "Failed to insert record: " + insertQuery.lastError().text());
    }
}

/**
 * @brief Queries SQLite aggregates, rebuilds all three dashboard charts, and refreshes anomaly highlights.
 */
void MainWindow::updateChart()
{
    categorySeries->clear();
    timeSeries->clear();
    barSeries->clear();
    barAxisX->clear();

    // 1. Pie Chart: Aggregate spending per category
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

    // 2. Line Chart: Daily spending totals over time
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

    // 3. Stacked Bar Chart: Project totals segmented by category
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

    QSqlQuery barQuery("SELECT c.name, p.name, SUM(e.amount) "
                       "FROM expenses e "
                       "JOIN categories c ON e.category_id = c.id "
                       "JOIN projects p ON e.project_id = p.id "
                       "GROUP BY c.name, p.name");

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

    for (double total : std::as_const(projectTotals)) {
        if (total > maxBarHeight) maxBarHeight = total;
    }
    barAxisY->setRange(0, maxBarHeight > 0 ? maxBarHeight * 1.1 : 100);

    for (auto it = categoryProjectValues.constBegin(); it != categoryProjectValues.constEnd(); ++it) {
        QString catName = it.key();
        QBarSet *set = new QBarSet(catName);

        for (const QString &projName : projectNames) {
            *set << it.value().value(projName, 0.0);
        }

        barSeries->append(set);
    }

    // Keep live anomaly highlights in sync whenever data changes
    refreshAnomalyHighlighting();
}

/**
 * @brief Purges all records from the database after user confirmation.
 */
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

/**
 * @brief Imports expense records from an external CSV/text file into SQLite.
 */
void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
                                                    "All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }

    currentFilePath = fileName;

    QTextStream in(&file);
    bool isFirstLine = true;
    db.transaction();

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (isFirstLine) { isFirstLine = false; continue; }

        QStringList fields = line.split(",");
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

/**
 * @brief Saves database contents directly to the currently tracked file (Ctrl+S).
 */
void MainWindow::onSaveCurrentFile()
{
    if (currentFilePath.isEmpty()) {
        onSaveFile();
        return;
    }

    if (saveDatabaseToFile(currentFilePath)) {
        statusBar()->showMessage("Saved to " + currentFilePath, 3000);
    }
}

/**
 * @brief Prompts for a destination path and exports the database ("Save to File...").
 */
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

/**
 * @brief Triggers window shutdown (delegating cleanup to `closeEvent`).
 */
void MainWindow::onExit()
{
    this->close();
}

/**
 * @brief Deletes all currently selected rows from the Ledger Reports table.
 */
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

/**
 * @brief Displays basic application version and purpose information.
 */
void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Finance Manager", "<b>Finance Manager (Database Edition)</b><br><br>Qt SQL application for tracking expenses.");
}

/**
 * @brief Prompts the user for a column and sort direction, then re-sorts the ledger model.
 */
void MainWindow::onSortLedger()
{
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok;
    QString item = QInputDialog::getItem(this, "Sort Ledger", "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        int columnIndex = columns.indexOf(item) + 1;
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Sort Order",
                                                                  QString("Sort in Ascending order?"), QMessageBox::Yes | QMessageBox::No);

        expenseModel->setSort(columnIndex, (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder);
        expenseModel->select();
    }
}

/**
 * @brief Switches to the Ledger tab and toggles visibility of the Find & Replace bar.
 */
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

/**
 * @brief Searches all visible ledger cells for case-insensitive matches to `findInput`.
 */
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

/**
 * @brief Focuses and scrolls the ledger view to the currently active search match.
 */
void MainWindow::highlightCurrentMatch()
{
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) return;
    QModelIndex index = currentMatches.at(currentMatchIndex);
    ledgerView->setCurrentIndex(index);
    ledgerView->scrollTo(index);
}

/**
 * @brief Advances the active search highlight to the next match (wrapping around).
 */
void MainWindow::onFindNext()
{
    if (currentMatches.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex + 1) % currentMatches.size();
    highlightCurrentMatch();
}

/**
 * @brief Moves the active search highlight to the previous match (wrapping around).
 */
void MainWindow::onFindPrevious()
{
    if (currentMatches.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex - 1 < 0) ? currentMatches.size() - 1 : currentMatchIndex - 1;
    highlightCurrentMatch();
}

/**
 * @brief Highlights every cell in `currentMatches` simultaneously in the table view.
 */
void MainWindow::onSelectAllMatches()
{
    ledgerView->clearSelection();
    for (const QModelIndex &index : std::as_const(currentMatches)) {
        ledgerView->selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

/**
 * @brief Replaces the matched substring inside the currently highlighted cell.
 */
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

/**
 * @brief Replaces the current match and immediately advances to the next match.
 */
void MainWindow::onReplaceAndFind()
{
    onReplace();
    onFindNext();
}

/**
 * @brief Replaces all occurrences of `findInput` with `replaceInput` across the ledger.
 */
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

/**
 * @brief Opens a modal drill-down pie chart showing project spending within a clicked category.
 */
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

/**
 * @brief Calculates the grand total of all expenses and displays it to the user.
 */
void MainWindow::onCalculateTotalSpending()
{
    QSqlQuery q("SELECT SUM(amount) FROM expenses");
    if (q.next()) {
        QString formattedTotal = QString::number(q.value(0).toDouble(), 'f', 2);
        statusBar()->showMessage("Total Spending: R " + formattedTotal, 5000);
        QMessageBox::information(this, "Total Spending", "The total spending for all records is:\n\nR " + formattedTotal);
    }
}

/**
 * @brief Computes expanded descriptive and distributional ledger statistics.
 *
 * Reports count, mean, median, Q1, Q3, IQR, standard deviation (sigma), min, max,
 * outlier thresholds, flagged outlier count, and most frequent category/project.
 */
void MainWindow::onGenerateStats()
{
    const ExpenseStats stats = computeExpenseStats();
    if (stats.count == 0) {
        QMessageBox::information(this, "Ledger Statistics", "The ledger is currently empty.");
        return;
    }

    QSqlQuery q;
    // Identify the most frequently occurring category
    q.exec("SELECT c.name, COUNT(*) as c_count FROM expenses e JOIN categories c ON e.category_id = c.id GROUP BY c.name ORDER BY c_count DESC LIMIT 1");
    QString topCategory = q.next() ? q.value(0).toString() : "N/A";
    int maxCatCount = q.value(1).toInt();

    // Identify the most frequently occurring project
    q.exec("SELECT p.name, COUNT(*) as p_count FROM expenses e JOIN projects p ON e.project_id = p.id GROUP BY p.name ORDER BY p_count DESC LIMIT 1");
    QString topProject = q.next() ? q.value(0).toString() : "N/A";
    int maxProjCount = q.value(1).toInt();

    QString statsMessage = QString(
                               "<h3>Ledger Statistical Summary</h3>"
                               "<b>Total Records Analyzed:</b> %1<br>"
                               "<b>Total Spend:</b> R %2<br><br>"
                               "<b>Mean (μ):</b> R %3<br>"
                               "<b>Median (Q2):</b> R %4<br>"
                               "<b>Standard Deviation (σ):</b> R %5<br>"
                               "<b>Quartiles (Q1 / Q3):</b> R %6 / R %7<br>"
                               "<b>Interquartile Range (IQR):</b> R %8<br>"
                               "<b>Lowest / Highest Expense:</b> R %9 / R %10<br><br>"
                               "<b>Outlier Upper Bounds:</b> R %11 (μ + 2σ) | R %12 (Q3 + 1.5×IQR)<br>"
                               "<b>Flagged Anomalies:</b> %13 record(s)<br><br>"
                               "<b>Most Frequent Category:</b> %14 (%15 entries)<br>"
                               "<b>Most Frequent Project:</b> %16 (%17 entries)")
                               .arg(stats.count)
                               .arg(stats.sum, 0, 'f', 2)
                               .arg(stats.mean, 0, 'f', 2)
                               .arg(stats.median, 0, 'f', 2)
                               .arg(stats.stdDev, 0, 'f', 2)
                               .arg(stats.q1, 0, 'f', 2)
                               .arg(stats.q3, 0, 'f', 2)
                               .arg(stats.iqr, 0, 'f', 2)
                               .arg(stats.min, 0, 'f', 2)
                               .arg(stats.max, 0, 'f', 2)
                               .arg(stats.zUpperThreshold, 0, 'f', 2)
                               .arg(stats.iqrUpperFence, 0, 'f', 2)
                               .arg(stats.outlierCount)
                               .arg(topCategory).arg(maxCatCount)
                               .arg(topProject).arg(maxProjCount);

    QMessageBox::information(this, "Ledger Statistics", statsMessage);
}

/**
 * @brief Displays a dedicated Statistical Distribution dialog with a frequency histogram.
 *
 * Visualizes how expense amounts are distributed across dynamically sized bins
 * alongside quartiles (Q1, Median, Q3), IQR, mean, standard deviation, and skewness.
 */
void MainWindow::onShowStatisticalDistribution()
{
    const ExpenseStats stats = computeExpenseStats();
    if (stats.count == 0) {
        QMessageBox::information(this, "Statistical Distribution", "The ledger is currently empty.");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Statistical Distribution Analysis");
    dialog->resize(720, 520);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Determine rough distribution skew based on Mean vs Median
    QString skewLabel = "Approximately Symmetric";
    if (stats.stdDev > 1e-9) {
        if (stats.mean > stats.median + 0.25 * stats.stdDev) {
            skewLabel = "Right-Skewed (High-Value Tail)";
        } else if (stats.mean < stats.median - 0.25 * stats.stdDev) {
            skewLabel = "Left-Skewed (Low-Value Tail)";
        }
    }

    QLabel *summaryLabel = new QLabel(QString(
                                          "<b>Distribution Profile (%1 records):</b> %2<br>"
                                          "<b>Mean (μ):</b> R %3 &nbsp;|&nbsp; <b>Median:</b> R %4 &nbsp;|&nbsp; <b>Std Dev (σ):</b> R %5<br>"
                                          "<b>Q1:</b> R %6 &nbsp;|&nbsp; <b>Q3:</b> R %7 &nbsp;|&nbsp; <b>IQR:</b> R %8 &nbsp;|&nbsp; "
                                          "<b>Outliers Detected:</b> %9")
                                          .arg(stats.count)
                                          .arg(skewLabel)
                                          .arg(stats.mean, 0, 'f', 2)
                                          .arg(stats.median, 0, 'f', 2)
                                          .arg(stats.stdDev, 0, 'f', 2)
                                          .arg(stats.q1, 0, 'f', 2)
                                          .arg(stats.q3, 0, 'f', 2)
                                          .arg(stats.iqr, 0, 'f', 2)
                                          .arg(stats.outlierCount));
    summaryLabel->setWordWrap(true);
    layout->addWidget(summaryLabel);

    // Build dynamic histogram bins using Sturges' rule (clamped to 4..10 bins)
    const double range = stats.max - stats.min;
    const int binCount = (range < 1e-9)
                             ? 1
                             : std::clamp(static_cast<int>(std::ceil(std::log2(stats.count) + 1.0)), 4, 10);

    QVector<int> binCounts(binCount, 0);
    QStringList binLabels;

    if (binCount == 1) {
        binCounts[0] = stats.count;
        binLabels << QString("R %1").arg(stats.min, 0, 'f', 2);
    } else {
        const double binWidth = range / static_cast<double>(binCount);
        for (int i = 0; i < binCount; ++i) {
            const double low = stats.min + i * binWidth;
            const double high = (i == binCount - 1) ? stats.max : (low + binWidth);
            binLabels << QString("R %1–%2").arg(low, 0, 'f', 0).arg(high, 0, 'f', 0);
        }

        for (double val : stats.sortedAmounts) {
            int idx = static_cast<int>((val - stats.min) / binWidth);
            if (idx >= binCount) {
                idx = binCount - 1;
            }
            binCounts[idx]++;
        }
    }

    QBarSet *freqSet = new QBarSet("Transaction Frequency");
    int maxFreq = 0;
    for (int c : binCounts) {
        *freqSet << c;
        if (c > maxFreq) maxFreq = c;
    }

    QBarSeries *histSeries = new QBarSeries();
    histSeries->append(freqSet);

    QChart *histChart = new QChart();
    histChart->addSeries(histSeries);
    histChart->setTitle("Expense Amount Frequency Distribution");
    histChart->setAnimationOptions(QChart::SeriesAnimations);
    histChart->legend()->setVisible(false);

    QBarCategoryAxis *histAxisX = new QBarCategoryAxis();
    histAxisX->append(binLabels);
    histAxisX->setTitleText("Amount Range (ZAR)");
    histChart->addAxis(histAxisX, Qt::AlignBottom);
    histSeries->attachAxis(histAxisX);

    QValueAxis *histAxisY = new QValueAxis();
    histAxisY->setLabelFormat("%d");
    histAxisY->setRange(0, std::max(1, maxFreq + 1));
    histAxisY->setTitleText("Number of Transactions");
    histChart->addAxis(histAxisY, Qt::AlignLeft);
    histSeries->attachAxis(histAxisY);

    QChartView *histChartView = new QChartView(histChart);
    histChartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(histChartView);

    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    dialog->exec();
    delete dialog;
}

/**
 * @brief Toggles statistical outlier highlighting in the Ledger Reports table.
 *
 * When enabled, computes mean, standard deviation, and Tukey upper fence (Q3 + 1.5*IQR),
 * switches to the Ledger Reports tab, highlights anomalous rows in ExpenseTableModel,
 * and reports the detection summary.
 *
 * @param checked True to activate anomaly highlighting; false to clear it.
 */
void MainWindow::onToggleAnomalyDetection(bool checked)
{
    if (!checked) {
        expenseModel->setAnomalyThresholds(false, 0.0, 0.0, 0.0, false);
        statusBar()->showMessage("Anomaly highlighting cleared.", 3000);
        return;
    }

    const ExpenseStats stats = computeExpenseStats();
    if (stats.count == 0) {
        QMessageBox::information(this, "Anomaly Detection", "The ledger is currently empty.");
        anomalyDetectionAction->setChecked(false);
        return;
    }

    expenseModel->setAnomalyThresholds(true, stats.mean, stats.stdDev, stats.iqrUpperFence, true);
    mainTabWidget->setCurrentIndex(1); // Switch to Ledger Reports tab to show highlights

    const QString summary = QString(
                                "<h3>Anomaly Detection Active</h3>"
                                "<b>Records Scanned:</b> %1<br>"
                                "<b>Flagged Outliers:</b> %2<br><br>"
                                "<b>Active Rules:</b><br>"
                                "• Z-Score: |x − μ| &gt; 2σ (Outside R %3 – R %4)<br>"
                                "• Tukey Fence: x &gt; Q3 + 1.5×IQR (Above R %5)<br><br>"
                                "Anomalous rows are highlighted in red in the Ledger Reports table. "
                                "Hover over a highlighted row for details.")
                                .arg(stats.count)
                                .arg(stats.outlierCount)
                                .arg(std::max(0.0, stats.zLowerThreshold), 0, 'f', 2)
                                .arg(stats.zUpperThreshold, 0, 'f', 2)
                                .arg(stats.iqrUpperFence, 0, 'f', 2);

    statusBar()->showMessage(QString("Anomaly Detection active: %1 outlier(s) highlighted.").arg(stats.outlierCount), 5000);
    QMessageBox::information(this, "Anomaly Detection", summary);
}

/**
 * @brief Switches the visible chart in the Visual Dashboard tab.
 */
void MainWindow::onChartTypeChanged(int index)
{
    chartStack->setCurrentIndex(index);
}