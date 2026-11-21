#include "mainwindow.h"

/**
 * @brief Constructs the MainWindow and bootstraps all application subsystems.
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
 * @brief Fetches all remaining SQLite batches into expenseModel so proxy filtering and charts see 100% of records.
 */
void MainWindow::fetchAllModelRows()
{
    if (!expenseModel) {
        return;
    }
    while (expenseModel->canFetchMore()) {
        expenseModel->fetchMore();
    }
}

/**
 * @brief Configures the relational table model and inserts ExpenseFilterProxyModel on top of it.
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
    fetchAllModelRows();

    // Insert custom multi-criteria proxy model between expenseModel and ledgerView
    expenseProxyModel = new ExpenseFilterProxyModel(this);
    expenseProxyModel->setSourceModel(expenseModel);
    expenseProxyModel->setDynamicSortFilter(true);

    // Refresh filter bounds and charts on actual data edits (ignore pure visual-role updates)
    connect(expenseModel, &QSqlTableModel::dataChanged, this,
            [this](const QModelIndex &, const QModelIndex &, const QList<int> &roles) {
                if (roles.isEmpty() || roles.contains(Qt::DisplayRole) || roles.contains(Qt::EditRole)) {
                    syncFilterControlsWithDatabase(false);
                    updateChart();
                }
            });
}

/**
 * @brief Builds the application menu bar (File, Edit, Help) and top toolbar.
 */
void MainWindow::setupMenuBar()
{
    QStyle *style = QApplication::style();
    // --- QAction functions for data manipulation and data analysis ---
    QAction *newAction = new QAction(style->standardIcon(QStyle::SP_FileIcon), "Clear Database", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onClearDatabase);

    QAction *openAction = new QAction(style->standardIcon(QStyle::SP_DirOpenIcon), "Open File ...", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    QAction *saveCurrentAction = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
    saveCurrentAction->setShortcut(QKeySequence::Save);
    connect(saveCurrentAction, &QAction::triggered, this, &MainWindow::onSaveCurrentFile);

    QAction *saveAction = new QAction(style->standardIcon(QStyle::SP_ArrowDown), "Save to File ...", this);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);

    QAction *exitAction = new QAction(style->standardIcon(QStyle::SP_DialogCloseButton), "Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

    QAction *deleteAction = new QAction(style->standardIcon(QStyle::SP_TrashIcon), "Delete Selected Record", this);
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);

    QAction *sortAction = new QAction(style->standardIcon(QStyle::SP_BrowserReload), "Sort Ledger...", this);
    connect(sortAction, &QAction::triggered, this, &MainWindow::onSortLedger);

    QAction *findAction = new QAction(QIcon::fromTheme("edit-find-replace", style->standardIcon(QStyle::SP_FileDialogContentsView)), "Find and Replace...", this);
    findAction->setShortcut(QKeySequence::Find);
    connect(findAction, &QAction::triggered, this, &MainWindow::onToggleFindReplace);

    QAction *totalSpendingAction = new QAction(style->standardIcon(QStyle::SP_DialogApplyButton), "Total Spending", this);
    connect(totalSpendingAction, &QAction::triggered, this, &MainWindow::onCalculateTotalSpending);

    QAction *statsAction = new QAction(style->standardIcon(QStyle::SP_FileDialogDetailedView), "Ledger Statistics", this);
    connect(statsAction, &QAction::triggered, this, &MainWindow::onGenerateStats);

    QAction *statDistAction = new QAction(style->standardIcon(QStyle::SP_FileDialogInfoView), "Statistical Distribution", this);
    connect(statDistAction, &QAction::triggered, this, &MainWindow::onShowStatisticalDistribution);

    anomalyDetectionAction = new QAction(style->standardIcon(QStyle::SP_MessageBoxWarning), "Anomaly Detection", this);
    anomalyDetectionAction->setCheckable(true);
    anomalyDetectionAction->setToolTip("Highlight outlier transactions (|x - μ| > 2σ or > Q3 + 1.5×IQR) in the ledger");
    connect(anomalyDetectionAction, &QAction::toggled, this, &MainWindow::onToggleAnomalyDetection);

    liveFilterAction = new QAction(
        QIcon::fromTheme("view-filter", style->standardIcon(QStyle::SP_FileDialogListView)),
        "Live Multi-Criteria Filtering",
        this);
    liveFilterAction->setCheckable(true);
    liveFilterAction->setToolTip("Filter ledger and Visual Dashboard in real time by date range, projects, categories, amount range, and keyword");
    connect(liveFilterAction, &QAction::toggled, this, &MainWindow::onToggleLiveFiltering);

    // --- File Menu ---
    QMenu *fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveCurrentAction);
    fileMenu->addAction(saveAction);

    fileMenu->addSeparator();

    fileMenu->addAction(exitAction);

    // --- Edit Menu ---
    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(deleteAction);

    // --- View Menu ---
    QMenu *viewMenu = menuBar()->addMenu("View");
    viewMenu->addAction(sortAction);
    viewMenu->addAction(findAction);

    viewMenu->addSeparator();

    viewMenu->addAction(totalSpendingAction);
    viewMenu->addAction(statDistAction);
    viewMenu->addAction(anomalyDetectionAction);

    viewMenu->addSeparator();

    viewMenu->addAction(liveFilterAction);

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

    // Basic function for data.
    mainToolBar->addAction(newAction);
    mainToolBar->addAction(deleteAction);

    mainToolBar->addSeparator();

    // Basic functions for data analysis.
    mainToolBar->addAction(totalSpendingAction);
    mainToolBar->addAction(statsAction);
    mainToolBar->addAction(statDistAction);
    mainToolBar->addAction(anomalyDetectionAction);

    mainToolBar->addSeparator();

    // Advanced functions for data analysis.
    mainToolBar->addAction(liveFilterAction);
}

/**
 * @brief Constructs the collapsible Live Multi-Criteria Filter Panel and the three-tab workspace.
 */
void MainWindow::setupFlexibleUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget());

    // =========================================================================
    // COLLAPSIBLE LIVE MULTI-CRITERIA FILTER PANEL (shared by Ledger & Dashboard)
    // =========================================================================
    filterPanelWidget = new QGroupBox("Live Multi-Criteria Filtering (QSortFilterProxyModel)", this);
    QVBoxLayout *filterOuterLayout = new QVBoxLayout(filterPanelWidget);
    filterOuterLayout->setSpacing(6);

    // Row 1: Keyword search, Date range (start/end), Match status, Reset
    QHBoxLayout *filterTopRow = new QHBoxLayout();

    QLabel *keywordLabel = new QLabel("Keyword:");
    filterKeywordEdit = new QLineEdit();
    filterKeywordEdit->setPlaceholderText("Search description, project, category, date, or amount...");
    filterKeywordEdit->setClearButtonEnabled(true);
    connect(filterKeywordEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterCriteriaChanged);

    filterDateEnabledCheck = new QCheckBox("Date Range:");
    filterDateEnabledCheck->setChecked(true);
    connect(filterDateEnabledCheck, &QCheckBox::toggled, this, &MainWindow::onFilterCriteriaChanged);

    filterStartDateEdit = new QDateEdit(QDate::currentDate());
    filterStartDateEdit->setCalendarPopup(true);
    filterStartDateEdit->setDisplayFormat("yyyy-MM-dd");
    connect(filterStartDateEdit, &QDateEdit::dateChanged, this, &MainWindow::onFilterStartDateChanged);

    QLabel *dateToLabel = new QLabel("to");

    filterEndDateEdit = new QDateEdit(QDate::currentDate());
    filterEndDateEdit->setCalendarPopup(true);
    filterEndDateEdit->setDisplayFormat("yyyy-MM-dd");
    connect(filterEndDateEdit, &QDateEdit::dateChanged, this, &MainWindow::onFilterEndDateChanged);

    filterStatusLabel = new QLabel("Showing 0 of 0 records");
    filterStatusLabel->setStyleSheet("font-weight: bold;");

    QPushButton *resetFiltersBtn = new QPushButton("Reset Filters");
    connect(resetFiltersBtn, &QPushButton::clicked, this, &MainWindow::onResetFilters);

    filterTopRow->addWidget(keywordLabel);
    filterTopRow->addWidget(filterKeywordEdit, 2);
    filterTopRow->addSpacing(10);
    filterTopRow->addWidget(filterDateEnabledCheck);
    filterTopRow->addWidget(filterStartDateEdit);
    filterTopRow->addWidget(dateToLabel);
    filterTopRow->addWidget(filterEndDateEdit);
    filterTopRow->addSpacing(10);
    filterTopRow->addWidget(filterStatusLabel);
    filterTopRow->addWidget(resetFiltersBtn);

    filterOuterLayout->addLayout(filterTopRow);

    // Row 2: Multi-select Projects, Multi-select Categories, Min/Max Amount Sliders
    QHBoxLayout *filterBottomRow = new QHBoxLayout();

    // Projects multi-select list
    QVBoxLayout *projFilterLayout = new QVBoxLayout();
    QHBoxLayout *projHeaderLayout = new QHBoxLayout();
    projHeaderLayout->addWidget(new QLabel("Projects:"));
    projHeaderLayout->addStretch();
    QPushButton *projAllBtn = new QPushButton("All");
    QPushButton *projNoneBtn = new QPushButton("None");
    projAllBtn->setMaximumWidth(45);
    projNoneBtn->setMaximumWidth(50);
    connect(projAllBtn, &QPushButton::clicked, this, &MainWindow::onSelectAllProjects);
    connect(projNoneBtn, &QPushButton::clicked, this, &MainWindow::onClearAllProjects);
    projHeaderLayout->addWidget(projAllBtn);
    projHeaderLayout->addWidget(projNoneBtn);

    filterProjectsList = new QListWidget();
    filterProjectsList->setMaximumHeight(85);
    filterProjectsList->setSelectionMode(QAbstractItemView::NoSelection);
    connect(filterProjectsList, &QListWidget::itemChanged, this, &MainWindow::onFilterCriteriaChanged);

    projFilterLayout->addLayout(projHeaderLayout);
    projFilterLayout->addWidget(filterProjectsList);

    // Categories multi-select list
    QVBoxLayout *catFilterLayout = new QVBoxLayout();
    QHBoxLayout *catHeaderLayout = new QHBoxLayout();
    catHeaderLayout->addWidget(new QLabel("Categories:"));
    catHeaderLayout->addStretch();
    QPushButton *catAllBtn = new QPushButton("All");
    QPushButton *catNoneBtn = new QPushButton("None");
    catAllBtn->setMaximumWidth(45);
    catNoneBtn->setMaximumWidth(50);
    connect(catAllBtn, &QPushButton::clicked, this, &MainWindow::onSelectAllCategories);
    connect(catNoneBtn, &QPushButton::clicked, this, &MainWindow::onClearAllCategories);
    catHeaderLayout->addWidget(catAllBtn);
    catHeaderLayout->addWidget(catNoneBtn);

    filterCategoriesList = new QListWidget();
    filterCategoriesList->setMaximumHeight(85);
    filterCategoriesList->setSelectionMode(QAbstractItemView::NoSelection);
    connect(filterCategoriesList, &QListWidget::itemChanged, this, &MainWindow::onFilterCriteriaChanged);

    catFilterLayout->addLayout(catHeaderLayout);
    catFilterLayout->addWidget(filterCategoriesList);

    // Min / Max Amount Sliders
    QGridLayout *amountSliderLayout = new QGridLayout();
    QLabel *minSliderLabel = new QLabel("Min Amount:");
    minAmountSlider = new QSlider(Qt::Horizontal);
    minAmountValueLabel = new QLabel("R 0.00");
    minAmountValueLabel->setMinimumWidth(90);
    minAmountValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(minAmountSlider, &QSlider::valueChanged, this, &MainWindow::onMinAmountSliderChanged);

    QLabel *maxSliderLabel = new QLabel("Max Amount:");
    maxAmountSlider = new QSlider(Qt::Horizontal);
    maxAmountValueLabel = new QLabel("R 100.00");
    maxAmountValueLabel->setMinimumWidth(90);
    maxAmountValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(maxAmountSlider, &QSlider::valueChanged, this, &MainWindow::onMaxAmountSliderChanged);

    amountSliderLayout->addWidget(minSliderLabel, 0, 0);
    amountSliderLayout->addWidget(minAmountSlider, 0, 1);
    amountSliderLayout->addWidget(minAmountValueLabel, 0, 2);
    amountSliderLayout->addWidget(maxSliderLabel, 1, 0);
    amountSliderLayout->addWidget(maxAmountSlider, 1, 1);
    amountSliderLayout->addWidget(maxAmountValueLabel, 1, 2);

    filterBottomRow->addLayout(projFilterLayout, 1);
    filterBottomRow->addLayout(catFilterLayout, 1);
    filterBottomRow->addLayout(amountSliderLayout, 2);

    filterOuterLayout->addLayout(filterBottomRow);
    filterPanelWidget->setVisible(false);
    mainLayout->addWidget(filterPanelWidget);

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

    QRegularExpression nameRegex("^[a-zA-Z0-9\\s\\-_]+$");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator(nameRegex, this);
    projectCombo->lineEdit()->setValidator(nameValidator);
    categoryCombo->lineEdit()->setValidator(nameValidator);

    reloadLookupCombos();

    amountSpin = new QDoubleSpinBox();
    amountSpin->setMaximum(9999999.99);
    amountSpin->setPrefix("R ");

    descriptionEdit = new QLineEdit();
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
    ledgerView->setModel(expenseProxyModel);
    ledgerView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ledgerView->setAlternatingRowColors(true);
    ledgerView->hideColumn(0); // Hide the internal auto-increment primary key column
    ledgerLayout->addWidget(ledgerView);

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

    // --- Chart 0: Category Pie Chart ---
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

    // --- Chart 1: Cash Flow Line Chart ---
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

    // --- Chart 2: Stacked Bar Chart ---
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

    mainLayout->addWidget(mainTabWidget, 1);

    syncFilterControlsWithDatabase(true);
}

/**
 * @brief Reloads the Data Entry project/category combo boxes from SQLite.
 */
void MainWindow::reloadLookupCombos()
{
    if (!projectCombo || !categoryCombo) {
        return;
    }

    const QString prevProj = projectCombo->currentText();
    const QString prevCat = categoryCombo->currentText();

    projectCombo->clear();
    QSqlQuery q("SELECT name FROM projects ORDER BY name ASC");
    while (q.next()) {
        projectCombo->addItem(q.value(0).toString());
    }
    if (!prevProj.isEmpty()) {
        projectCombo->setCurrentText(prevProj);
    }

    categoryCombo->clear();
    q.exec("SELECT name FROM categories ORDER BY name ASC");
    while (q.next()) {
        categoryCombo->addItem(q.value(0).toString());
    }
    if (!prevCat.isEmpty()) {
        categoryCombo->setCurrentText(prevCat);
    }
}

/**
 * @brief Synchronizes filter lists, date bounds, and amount slider ranges with SQLite contents.
 */
void MainWindow::syncFilterControlsWithDatabase(bool resetSelections)
{
    if (!filterPanelWidget) {
        return;
    }

    // 1. Sync multi-select Projects list
    {
        QSet<QString> prevAll;
        QSet<QString> prevChecked;
        for (int i = 0; i < filterProjectsList->count(); ++i) {
            const QListWidgetItem *item = filterProjectsList->item(i);
            prevAll.insert(item->text());
            if (item->checkState() == Qt::Checked) {
                prevChecked.insert(item->text());
            }
        }
        const bool allPreviouslyChecked = prevAll.isEmpty() || (prevChecked.size() == prevAll.size());

        QSignalBlocker blocker(filterProjectsList);
        filterProjectsList->clear();

        QSqlQuery q("SELECT name FROM projects ORDER BY name ASC");
        while (q.next()) {
            const QString name = q.value(0).toString();
            QListWidgetItem *item = new QListWidgetItem(name, filterProjectsList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            const bool isNew = !prevAll.contains(name);
            const bool checked = resetSelections || allPreviouslyChecked || prevChecked.contains(name) || isNew;
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        }
    }

    // 2. Sync multi-select Categories list
    {
        QSet<QString> prevAll;
        QSet<QString> prevChecked;
        for (int i = 0; i < filterCategoriesList->count(); ++i) {
            const QListWidgetItem *item = filterCategoriesList->item(i);
            prevAll.insert(item->text());
            if (item->checkState() == Qt::Checked) {
                prevChecked.insert(item->text());
            }
        }
        const bool allPreviouslyChecked = prevAll.isEmpty() || (prevChecked.size() == prevAll.size());

        QSignalBlocker blocker(filterCategoriesList);
        filterCategoriesList->clear();

        QSqlQuery q("SELECT name FROM categories ORDER BY name ASC");
        while (q.next()) {
            const QString name = q.value(0).toString();
            QListWidgetItem *item = new QListWidgetItem(name, filterCategoriesList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            const bool isNew = !prevAll.contains(name);
            const bool checked = resetSelections || allPreviouslyChecked || prevChecked.contains(name) || isNew;
            item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
        }
    }

    // 3. Query dataset date & amount bounds
    QDate dbMinDate = QDate::currentDate();
    QDate dbMaxDate = QDate::currentDate();
    double dbMaxAmount = 100.0;

    QSqlQuery boundsQuery("SELECT MIN(date_val), MAX(date_val), MAX(amount) FROM expenses");
    if (boundsQuery.next()) {
        const QDate minD = QDate::fromString(boundsQuery.value(0).toString(), "yyyy-MM-dd");
        const QDate maxD = QDate::fromString(boundsQuery.value(1).toString(), "yyyy-MM-dd");
        if (minD.isValid()) {
            dbMinDate = minD;
        }
        if (maxD.isValid()) {
            dbMaxDate = maxD;
        }
        if (!boundsQuery.value(2).isNull()) {
            dbMaxAmount = std::max(1.0, boundsQuery.value(2).toDouble());
        }
    }

    // 4. Sync Date Range controls
    if (resetSelections || !dateFilterCustomized) {
        QSignalBlocker startBlocker(filterStartDateEdit);
        QSignalBlocker endBlocker(filterEndDateEdit);
        filterStartDateEdit->setDate(dbMinDate);
        filterEndDateEdit->setDate(dbMaxDate);

        if (resetSelections) {
            QSignalBlocker checkBlocker(filterDateEnabledCheck);
            filterDateEnabledCheck->setChecked(true);
            dateFilterCustomized = false;
        }
    }

    // 5. Sync Min/Max Amount Sliders
    const bool wasMinAtBottom = (minAmountSlider->maximum() == 0) || (minAmountSlider->value() <= minAmountSlider->minimum());
    const bool wasMaxAtTop = (maxAmountSlider->maximum() == 0) || (maxAmountSlider->value() >= maxAmountSlider->maximum());
    const double prevMinVal = minAmountSlider->value() / amountSliderScale;
    const double prevMaxVal = maxAmountSlider->value() / amountSliderScale;

    amountSliderScale = (dbMaxAmount > 10000000.0) ? 1.0 : 100.0;
    const int newMaxTicks = std::max(100, static_cast<int>(std::ceil(dbMaxAmount * amountSliderScale)));
    const int step = std::max(1, static_cast<int>(amountSliderScale));
    const int pageStep = std::max(step, newMaxTicks / 20);

    {
        QSignalBlocker minBlocker(minAmountSlider);
        QSignalBlocker maxBlocker(maxAmountSlider);

        minAmountSlider->setRange(0, newMaxTicks);
        maxAmountSlider->setRange(0, newMaxTicks);
        minAmountSlider->setSingleStep(step);
        minAmountSlider->setPageStep(pageStep);
        maxAmountSlider->setSingleStep(step);
        maxAmountSlider->setPageStep(pageStep);

        const int newMinTick = (resetSelections || wasMinAtBottom)
                                   ? 0
                                   : std::clamp(static_cast<int>(std::round(prevMinVal * amountSliderScale)), 0, newMaxTicks);
        const int newMaxTick = (resetSelections || wasMaxAtTop)
                                   ? newMaxTicks
                                   : std::clamp(static_cast<int>(std::round(prevMaxVal * amountSliderScale)), newMinTick, newMaxTicks);

        minAmountSlider->setValue(newMinTick);
        maxAmountSlider->setValue(newMaxTick);
    }
    updateAmountSliderLabels();

    if (resetSelections) {
        QSignalBlocker kwBlocker(filterKeywordEdit);
        filterKeywordEdit->clear();
    }

    applyFilterCriteriaToProxy();
}

/**
 * @brief Pushes the current UI filter controls into ExpenseFilterProxyModel.
 */
void MainWindow::applyFilterCriteriaToProxy()
{
    if (!expenseProxyModel || !filterPanelWidget) {
        return;
    }

    QSet<QString> selectedProjects;
    const bool filterProjects = filterProjectsList->count() > 0;
    for (int i = 0; i < filterProjectsList->count(); ++i) {
        const QListWidgetItem *item = filterProjectsList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedProjects.insert(item->text());
        }
    }

    QSet<QString> selectedCategories;
    const bool filterCategories = filterCategoriesList->count() > 0;
    for (int i = 0; i < filterCategoriesList->count(); ++i) {
        const QListWidgetItem *item = filterCategoriesList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedCategories.insert(item->text());
        }
    }

    const double minAmt = minAmountSlider->value() / amountSliderScale;
    const double maxAmt = maxAmountSlider->value() / amountSliderScale;

    expenseProxyModel->setFilterCriteria(
        filterDateEnabledCheck->isChecked(),
        filterStartDateEdit->date(),
        filterEndDateEdit->date(),
        filterProjects,
        selectedProjects,
        filterCategories,
        selectedCategories,
        minAmt,
        maxAmt,
        filterKeywordEdit->text());

    updateFilterStatusLabel();
}

/**
 * @brief Refreshes the currency labels paired with the Min/Max amount sliders.
 */
void MainWindow::updateAmountSliderLabels()
{
    if (!minAmountValueLabel || !maxAmountValueLabel) {
        return;
    }
    minAmountValueLabel->setText(QString("R %1").arg(minAmountSlider->value() / amountSliderScale, 0, 'f', 2));
    maxAmountValueLabel->setText(QString("R %1").arg(maxAmountSlider->value() / amountSliderScale, 0, 'f', 2));
}

/**
 * @brief Displays how many ledger records match the active filter slice.
 */
void MainWindow::updateFilterStatusLabel()
{
    if (!filterStatusLabel || !expenseModel || !expenseProxyModel) {
        return;
    }
    filterStatusLabel->setText(
        QString("Showing %1 of %2 records")
            .arg(expenseProxyModel->rowCount())
            .arg(expenseModel->rowCount()));
}

/**
 * @brief Batch-updates check states in a filter QListWidget without redundant repaints.
 */
void MainWindow::setListItemsChecked(QListWidget *listWidget, Qt::CheckState state)
{
    if (!listWidget) {
        return;
    }
    {
        QSignalBlocker blocker(listWidget);
        for (int i = 0; i < listWidget->count(); ++i) {
            listWidget->item(i)->setCheckState(state);
        }
    }
    onFilterCriteriaChanged();
}

/**
 * @brief Toggles the Live Multi-Criteria Filter panel and proxy filtering mode.
 */
void MainWindow::onToggleLiveFiltering(bool checked)
{
    filterPanelWidget->setVisible(checked);
    expenseProxyModel->setFilteringEnabled(checked);

    if (checked) {
        syncFilterControlsWithDatabase(false);
        onFilterCriteriaChanged();
        statusBar()->showMessage("Live Multi-Criteria Filtering active.", 3000);
    } else {
        updateChart();
        updateFilterStatusLabel();
        if (findReplaceWidget && findReplaceWidget->isVisible()) {
            performSearch();
        }
        statusBar()->showMessage("Live Multi-Criteria Filtering disabled (showing all records).", 3000);
    }
}

/**
 * @brief Applies updated filter criteria and refreshes the Visual Dashboard in real time.
 */
void MainWindow::onFilterCriteriaChanged()
{
    filterStartDateEdit->setEnabled(filterDateEnabledCheck->isChecked());
    filterEndDateEdit->setEnabled(filterDateEnabledCheck->isChecked());

    applyFilterCriteriaToProxy();
    updateChart();

    if (findReplaceWidget && findReplaceWidget->isVisible()) {
        performSearch();
    }
}

void MainWindow::onFilterStartDateChanged(const QDate &date)
{
    dateFilterCustomized = true;
    if (date > filterEndDateEdit->date()) {
        QSignalBlocker blocker(filterEndDateEdit);
        filterEndDateEdit->setDate(date);
    }
    onFilterCriteriaChanged();
}

void MainWindow::onFilterEndDateChanged(const QDate &date)
{
    dateFilterCustomized = true;
    if (date < filterStartDateEdit->date()) {
        QSignalBlocker blocker(filterStartDateEdit);
        filterStartDateEdit->setDate(date);
    }
    onFilterCriteriaChanged();
}

void MainWindow::onMinAmountSliderChanged(int value)
{
    if (value > maxAmountSlider->value()) {
        QSignalBlocker blocker(maxAmountSlider);
        maxAmountSlider->setValue(value);
    }
    updateAmountSliderLabels();
    onFilterCriteriaChanged();
}

void MainWindow::onMaxAmountSliderChanged(int value)
{
    if (value < minAmountSlider->value()) {
        QSignalBlocker blocker(minAmountSlider);
        minAmountSlider->setValue(value);
    }
    updateAmountSliderLabels();
    onFilterCriteriaChanged();
}

void MainWindow::onResetFilters()
{
    syncFilterControlsWithDatabase(true);
    updateChart();
    if (findReplaceWidget && findReplaceWidget->isVisible()) {
        performSearch();
    }
    statusBar()->showMessage("All filter criteria reset.", 3000);
}

void MainWindow::onSelectAllProjects()
{
    setListItemsChecked(filterProjectsList, Qt::Checked);
}

void MainWindow::onClearAllProjects()
{
    setListItemsChecked(filterProjectsList, Qt::Unchecked);
}

void MainWindow::onSelectAllCategories()
{
    setListItemsChecked(filterCategoriesList, Qt::Checked);
}

void MainWindow::onClearAllCategories()
{
    setListItemsChecked(filterCategoriesList, Qt::Unchecked);
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
 * @brief Calculates distribution and anomaly metrics from the active ExpenseFilterProxyModel slice.
 */
ExpenseStats MainWindow::computeExpenseStats() const
{
    ExpenseStats stats;
    if (!expenseProxyModel) {
        return stats;
    }

    const int rowCount = expenseProxyModel->rowCount();
    stats.sortedAmounts.reserve(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        const double val = expenseProxyModel->data(expenseProxyModel->index(row, 5), Qt::EditRole).toDouble();
        stats.sortedAmounts.append(val);
        stats.sum += val;
    }

    std::sort(stats.sortedAmounts.begin(), stats.sortedAmounts.end());
    stats.count = stats.sortedAmounts.size();
    if (stats.count == 0) {
        return stats;
    }

    stats.min = stats.sortedAmounts.first();
    stats.max = stats.sortedAmounts.last();
    stats.mean = stats.sum / static_cast<double>(stats.count);

    stats.q1 = calculatePercentile(stats.sortedAmounts, 0.25);
    stats.median = calculatePercentile(stats.sortedAmounts, 0.50);
    stats.q3 = calculatePercentile(stats.sortedAmounts, 0.75);
    stats.iqr = stats.q3 - stats.q1;

    double sumSqDiff = 0.0;
    for (double val : stats.sortedAmounts) {
        const double diff = val - stats.mean;
        sumSqDiff += diff * diff;
    }
    stats.stdDev = std::sqrt(sumSqDiff / static_cast<double>(stats.count));

    stats.zLowerThreshold = stats.mean - 2.0 * stats.stdDev;
    stats.zUpperThreshold = stats.mean + 2.0 * stats.stdDev;
    stats.iqrUpperFence = stats.q3 + 1.5 * stats.iqr;

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
    const QString dateStr = dateEdit->date().toString("yyyy-MM-dd");
    const QString projectName = projectCombo->currentText().trimmed();
    const QString categoryName = categoryCombo->currentText().trimmed();
    const QString description = descriptionEdit->text();
    const double amount = amountSpin->value();

    const int projId = getOrInsertForeignKey("projects", projectName);
    const int catId = getOrInsertForeignKey("categories", categoryName);

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO expenses (date_val, project_id, category_id, description, amount) "
                        "VALUES (:date, :proj, :cat, :desc, :amt)");
    insertQuery.bindValue(":date", dateStr);
    insertQuery.bindValue(":proj", projId);
    insertQuery.bindValue(":cat", catId);
    insertQuery.bindValue(":desc", description);
    insertQuery.bindValue(":amt", amount);

    if (insertQuery.exec()) {
        if (!projectName.isEmpty() && projectCombo->findText(projectName) == -1) {
            projectCombo->addItem(projectName);
        }
        if (!categoryName.isEmpty() && categoryCombo->findText(categoryName) == -1) {
            categoryCombo->addItem(categoryName);
        }

        expenseModel->select();
        fetchAllModelRows();
        syncFilterControlsWithDatabase(false);
        updateChart();
        descriptionEdit->clear();
        amountSpin->setValue(0.0);
        statusBar()->showMessage("Expense recorded successfully to database.", 3000);
    } else {
        QMessageBox::warning(this, "Database Error", "Failed to insert record: " + insertQuery.lastError().text());
    }
}

/**
 * @brief Aggregates the active filtered slice from ExpenseFilterProxyModel and rebuilds all three dashboard charts.
 */
void MainWindow::updateChart()
{
    if (!expenseProxyModel || !categorySeries || !timeSeries || !barSeries) {
        return;
    }

    // Disable chart animations while actively scrubbing sliders for smooth 60fps feedback
    const bool isScrubbing = (minAmountSlider && minAmountSlider->isSliderDown()) ||
                             (maxAmountSlider && maxAmountSlider->isSliderDown());
    const QChart::AnimationOptions animOpt = isScrubbing ? QChart::NoAnimation : QChart::SeriesAnimations;
    pieChart->setAnimationOptions(animOpt);
    lineChart->setAnimationOptions(animOpt);
    barChart->setAnimationOptions(animOpt);

    const bool isFiltered = expenseProxyModel->isFilteringEnabled();
    pieChart->setTitle(isFiltered ? "Expenses by Category (Filtered)" : "Expenses by Category");
    lineChart->setTitle(isFiltered ? "Cash Flow Over Time (Filtered)" : "Cash Flow Over Time");
    barChart->setTitle(isFiltered ? "Expenses Grouped by Project (Filtered)"
                                  : "Expenses Grouped by Project (Stacked by Category)");

    categorySeries->clear();
    timeSeries->clear();
    barSeries->clear();
    barAxisX->clear();

    QMap<QString, double> categoryTotals;
    QMap<QDate, double> dailyTotals;
    QSet<QString> projectSet;
    QMap<QString, QMap<QString, double>> categoryProjectValues;
    QMap<QString, double> projectTotals;

    const int rowCount = expenseProxyModel->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const QString dateStr = expenseProxyModel->data(expenseProxyModel->index(row, 1), Qt::DisplayRole).toString();
        const QString project = expenseProxyModel->data(expenseProxyModel->index(row, 2), Qt::DisplayRole).toString();
        const QString category = expenseProxyModel->data(expenseProxyModel->index(row, 3), Qt::DisplayRole).toString();
        const double amount = expenseProxyModel->data(expenseProxyModel->index(row, 5), Qt::EditRole).toDouble();

        categoryTotals[category] += amount;

        const QDate parsedDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (parsedDate.isValid()) {
            dailyTotals[parsedDate] += amount;
        }

        projectSet.insert(project);
        categoryProjectValues[category][project] += amount;
        projectTotals[project] += amount;
    }

    // 1. Pie Chart: Aggregate spending per category
    for (auto it = categoryTotals.constBegin(); it != categoryTotals.constEnd(); ++it) {
        QPieSlice *slice = categorySeries->append(it.key(), it.value());
        slice->setLabelVisible(true);
        slice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }

    // 2. Line Chart: Daily spending totals over time (QMap<QDate, double> is naturally sorted ascending)
    double maxDailyAmount = 0.0;
    QDateTime minDate;
    QDateTime maxDate;

    for (auto it = dailyTotals.constBegin(); it != dailyTotals.constEnd(); ++it) {
        const QDateTime dt(it.key(), QTime(0, 0));
        const double dailyTotal = it.value();

        if (minDate.isNull() || dt < minDate) minDate = dt;
        if (maxDate.isNull() || dt > maxDate) maxDate = dt;
        if (dailyTotal > maxDailyAmount) maxDailyAmount = dailyTotal;

        timeSeries->append(dt.toMSecsSinceEpoch(), dailyTotal);
    }

    if (!minDate.isNull()) {
        if (minDate == maxDate) {
            minDate = minDate.addDays(-1);
            maxDate = maxDate.addDays(1);
        }
        axisX->setRange(minDate, maxDate);
        axisY->setRange(0.0, maxDailyAmount > 0.0 ? maxDailyAmount * 1.1 : 100.0);
    } else {
        const QDateTime today(QDate::currentDate(), QTime(0, 0));
        axisX->setRange(today.addDays(-1), today.addDays(1));
        axisY->setRange(0.0, 100.0);
    }

    // 3. Stacked Bar Chart: Project totals segmented by category
    QStringList projectNames = projectSet.values();
    std::sort(projectNames.begin(), projectNames.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });

    if (!projectNames.isEmpty()) {
        barAxisX->append(projectNames);
    } else {
        barAxisX->append(isFiltered ? "No Matching Records" : "No Projects");
    }

    double maxBarHeight = 0.0;
    for (double total : std::as_const(projectTotals)) {
        if (total > maxBarHeight) {
            maxBarHeight = total;
        }
    }
    barAxisY->setRange(0.0, maxBarHeight > 0.0 ? maxBarHeight * 1.1 : 100.0);

    for (auto it = categoryProjectValues.constBegin(); it != categoryProjectValues.constEnd(); ++it) {
        QBarSet *set = new QBarSet(it.key());
        for (const QString &projName : std::as_const(projectNames)) {
            *set << it.value().value(projName, 0.0);
        }
        barSeries->append(set);
    }

    updateFilterStatusLabel();
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
        fetchAllModelRows();
        projectCombo->clear();
        categoryCombo->clear();
        syncFilterControlsWithDatabase(true);
        updateChart();
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

    reloadLookupCombos();
    expenseModel->select();
    fetchAllModelRows();
    syncFilterControlsWithDatabase(false);
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
 * @brief Triggers window shutdown (delegating cleanup to closeEvent).
 */
void MainWindow::onExit()
{
    this->close();
}

/**
 * @brief Deletes all currently selected rows from the Ledger Reports table via proxy-to-source mapping.
 */
void MainWindow::onDeleteSelected()
{
    const QModelIndexList selectedProxyRows = ledgerView->selectionModel()->selectedRows();
    if (selectedProxyRows.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select a row in the Ledger Reports tab to delete.");
        return;
    }

    QVector<int> sourceRows;
    sourceRows.reserve(selectedProxyRows.size());
    for (const QModelIndex &proxyIdx : selectedProxyRows) {
        const QModelIndex srcIdx = expenseProxyModel->mapToSource(proxyIdx);
        if (srcIdx.isValid()) {
            sourceRows.append(srcIdx.row());
        }
    }

    std::sort(sourceRows.begin(), sourceRows.end(), std::greater<int>());
    sourceRows.erase(std::unique(sourceRows.begin(), sourceRows.end()), sourceRows.end());

    for (int srcRow : std::as_const(sourceRows)) {
        expenseModel->removeRow(srcRow);
    }

    expenseModel->submitAll();
    expenseModel->select();
    fetchAllModelRows();
    syncFilterControlsWithDatabase(false);
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
 * @brief Prompts the user for a column and sort direction, then re-sorts via ExpenseFilterProxyModel.
 */
void MainWindow::onSortLedger()
{
    QStringList columns = {"Date", "Project/Site", "Category", "Description", "Amount"};
    bool ok = false;
    QString item = QInputDialog::getItem(this, "Sort Ledger", "Select variable to sort by:", columns, 0, false, &ok);

    if (ok && !item.isEmpty()) {
        const int columnIndex = columns.indexOf(item) + 1;
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Sort Order", QString("Sort in Ascending order?"), QMessageBox::Yes | QMessageBox::No);

        expenseProxyModel->sort(columnIndex, (reply == QMessageBox::Yes) ? Qt::AscendingOrder : Qt::DescendingOrder);
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
 * @brief Searches all visible proxy ledger cells for case-insensitive matches to findInput.
 */
void MainWindow::performSearch()
{
    currentMatches.clear();
    currentMatchIndex = -1;
    ledgerView->clearSelection();

    const QString targetText = findInput->text();
    if (targetText.isEmpty() || !expenseProxyModel) {
        return;
    }

    for (int row = 0; row < expenseProxyModel->rowCount(); ++row) {
        for (int col = 1; col < expenseProxyModel->columnCount(); ++col) {
            const QModelIndex proxyIdx = expenseProxyModel->index(row, col);
            if (expenseProxyModel->data(proxyIdx).toString().contains(targetText, Qt::CaseInsensitive)) {
                currentMatches.append(proxyIdx);
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
    const QModelIndex index = currentMatches.at(currentMatchIndex);
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
 * @brief Highlights every cell in currentMatches simultaneously in the table view.
 */
void MainWindow::onSelectAllMatches()
{
    ledgerView->clearSelection();
    for (const QModelIndex &index : std::as_const(currentMatches)) {
        ledgerView->selectionModel()->select(index, QItemSelectionModel::Select);
    }
}

/**
 * @brief Replaces the matched substring inside the currently highlighted visible cell.
 */
void MainWindow::onReplace()
{
    if (currentMatches.isEmpty() || currentMatchIndex < 0 || currentMatchIndex >= currentMatches.size()) return;

    const QModelIndex proxyIdx = currentMatches.at(currentMatchIndex);
    QString currentText = expenseProxyModel->data(proxyIdx).toString();
    currentText.replace(findInput->text(), replaceInput->text(), Qt::CaseInsensitive);

    expenseProxyModel->setData(proxyIdx, currentText);
    expenseModel->submitAll();
    syncFilterControlsWithDatabase(false);
    updateChart();
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
 * @brief Replaces all visible occurrences of findInput with replaceInput across the filtered ledger.
 */
void MainWindow::onReplaceAll()
{
    const QString targetText = findInput->text();
    if (targetText.isEmpty() || !expenseProxyModel) return;

    // Snapshot matching source indices so mid-loop filter changes do not skip rows
    QList<QPersistentModelIndex> matchedSourceIndices;
    for (int row = 0; row < expenseProxyModel->rowCount(); ++row) {
        for (int col = 1; col < expenseProxyModel->columnCount(); ++col) {
            const QModelIndex proxyIdx = expenseProxyModel->index(row, col);
            if (expenseProxyModel->data(proxyIdx).toString().contains(targetText, Qt::CaseInsensitive)) {
                matchedSourceIndices.append(QPersistentModelIndex(expenseProxyModel->mapToSource(proxyIdx)));
            }
        }
    }

    int replacedCount = 0;
    for (const QPersistentModelIndex &srcIdx : std::as_const(matchedSourceIndices)) {
        if (!srcIdx.isValid()) {
            continue;
        }
        QString text = expenseModel->data(srcIdx).toString();
        text.replace(targetText, replaceInput->text(), Qt::CaseInsensitive);
        if (expenseModel->setData(srcIdx, text)) {
            replacedCount++;
        }
    }

    if (replacedCount > 0) {
        expenseModel->submitAll();
        syncFilterControlsWithDatabase(false);
        updateChart();
        statusBar()->showMessage(QString("Successfully replaced %1 occurrence(s).").arg(replacedCount), 5000);
        performSearch();
    }
}

/**
 * @brief Opens a modal drill-down pie chart showing project spending within a clicked category (respecting active filters).
 */
void MainWindow::onPieSliceClicked(QPieSlice *slice)
{
    const QString category = slice->label().section(':', 0, 0).trimmed();

    QMap<QString, double> projectTotals;
    for (int row = 0; row < expenseProxyModel->rowCount(); ++row) {
        const QString rowCat = expenseProxyModel->data(expenseProxyModel->index(row, 3), Qt::DisplayRole).toString();
        if (rowCat == category) {
            const QString rowProj = expenseProxyModel->data(expenseProxyModel->index(row, 2), Qt::DisplayRole).toString();
            const double amount = expenseProxyModel->data(expenseProxyModel->index(row, 5), Qt::EditRole).toDouble();
            projectTotals[rowProj] += amount;
        }
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(category + " - Project Breakdown");
    dialog->resize(500, 400);

    QPieSeries *projectSeries = new QPieSeries();
    for (auto it = projectTotals.constBegin(); it != projectTotals.constEnd(); ++it) {
        QPieSlice *pSlice = projectSeries->append(it.key(), it.value());
        pSlice->setLabelVisible(true);
        pSlice->setLabel(QString("%1: R %2").arg(it.key()).arg(it.value(), 0, 'f', 2));
    }

    QChart *projectChart = new QChart();
    projectChart->addSeries(projectSeries);
    projectChart->setTitle(expenseProxyModel->isFilteringEnabled()
                               ? QString("Budget Distribution for '%1' (Filtered)").arg(category)
                               : QString("Budget Distribution for '%1'").arg(category));
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
 * @brief Calculates the grand total of all visible expenses (respecting active filters).
 */
void MainWindow::onCalculateTotalSpending()
{
    double total = 0.0;
    for (int row = 0; row < expenseProxyModel->rowCount(); ++row) {
        total += expenseProxyModel->data(expenseProxyModel->index(row, 5), Qt::EditRole).toDouble();
    }

    const QString formattedTotal = QString::number(total, 'f', 2);
    const QString scope = expenseProxyModel->isFilteringEnabled() ? "filtered records" : "all records";
    statusBar()->showMessage("Total Spending (" + scope + "): R " + formattedTotal, 5000);
    QMessageBox::information(this, "Total Spending",
                             QString("The total spending for %1 is:\n\nR %2").arg(scope, formattedTotal));
}

/**
 * @brief Computes expanded descriptive and distributional ledger statistics for the active view.
 */
void MainWindow::onGenerateStats()
{
    const ExpenseStats stats = computeExpenseStats();
    if (stats.count == 0) {
        QMessageBox::information(this, "Ledger Statistics", "No matching records to analyze.");
        return;
    }

    QMap<QString, int> categoryCounts;
    QMap<QString, int> projectCounts;
    for (int row = 0; row < expenseProxyModel->rowCount(); ++row) {
        projectCounts[expenseProxyModel->data(expenseProxyModel->index(row, 2), Qt::DisplayRole).toString()]++;
        categoryCounts[expenseProxyModel->data(expenseProxyModel->index(row, 3), Qt::DisplayRole).toString()]++;
    }

    QString topCategory = "N/A";
    int maxCatCount = 0;
    for (auto it = categoryCounts.constBegin(); it != categoryCounts.constEnd(); ++it) {
        if (it.value() > maxCatCount) {
            maxCatCount = it.value();
            topCategory = it.key();
        }
    }

    QString topProject = "N/A";
    int maxProjCount = 0;
    for (auto it = projectCounts.constBegin(); it != projectCounts.constEnd(); ++it) {
        if (it.value() > maxProjCount) {
            maxProjCount = it.value();
            topProject = it.key();
        }
    }

    const QString heading = expenseProxyModel->isFilteringEnabled()
                                ? "<h3>Ledger Statistical Summary (Filtered Slice)</h3>"
                                : "<h3>Ledger Statistical Summary</h3>";

    QString statsMessage = heading + QString(
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
 */
void MainWindow::onShowStatisticalDistribution()
{
    const ExpenseStats stats = computeExpenseStats();
    if (stats.count == 0) {
        QMessageBox::information(this, "Statistical Distribution", "No matching records to analyze.");
        return;
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(expenseProxyModel->isFilteringEnabled()
                               ? "Statistical Distribution Analysis (Filtered Slice)"
                               : "Statistical Distribution Analysis");
    dialog->resize(720, 520);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

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
    histChart->setTitle(expenseProxyModel->isFilteringEnabled()
                            ? "Expense Amount Frequency Distribution (Filtered)"
                            : "Expense Amount Frequency Distribution");
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
        QMessageBox::information(this, "Anomaly Detection", "No matching records to analyze.");
        anomalyDetectionAction->setChecked(false);
        return;
    }

    expenseModel->setAnomalyThresholds(true, stats.mean, stats.stdDev, stats.iqrUpperFence, true);
    mainTabWidget->setCurrentIndex(1);

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

void ExpenseTableModel::setAnomalyThresholds(bool enabled, double mean, double stdDev, double iqrUpperFence, bool valid)
{
    highlightAnomalies = enabled;
    statMean = mean;
    statStdDev = stdDev;
    statIqrUpperFence = iqrUpperFence;
    hasValidStats = valid;

    if (rowCount() > 0 && columnCount() > 0) {
        emit dataChanged(index(0, 0),
                         index(rowCount() - 1, columnCount() - 1),
                         {Qt::BackgroundRole, Qt::ForegroundRole, Qt::ToolTipRole});
    }
}

bool ExpenseTableModel::isAmountOutlier(double amount) const
{
    if (!hasValidStats) {
        return false;
    }
    constexpr double eps = 1e-9;
    const bool isZOutlier = (statStdDev > eps) && (std::abs(amount - statMean) > (2.0 * statStdDev + eps));
    const bool isIqrOutlier = amount > (statIqrUpperFence + eps);
    return isZOutlier || isIqrOutlier;
}

QVariant ExpenseTableModel::data(const QModelIndex &index, int role) const
{
    if (highlightAnomalies && hasValidStats && index.isValid() &&
        (role == Qt::BackgroundRole || role == Qt::ForegroundRole || role == Qt::ToolTipRole)) {
        bool ok = false;
        const QModelIndex amountIdx = this->index(index.row(), 5);
        const double amount = QSqlRelationalTableModel::data(amountIdx, Qt::EditRole).toDouble(&ok);

        if (ok && isAmountOutlier(amount)) {
            if (role == Qt::BackgroundRole) {
                return QBrush(QColor(255, 220, 220));
            }
            if (role == Qt::ForegroundRole) {
                return QBrush(QColor(135, 0, 0));
            }
            if (role == Qt::ToolTipRole) {
                constexpr double eps = 1e-9;
                const bool isZOutlier = (statStdDev > eps) && (std::abs(amount - statMean) > (2.0 * statStdDev + eps));
                const bool isIqrOutlier = amount > (statIqrUpperFence + eps);

                QStringList reasons;
                if (isZOutlier) {
                    const double zScore = (amount - statMean) / statStdDev;
                    reasons << QString("|z| = %1 (> 2σ)").arg(std::abs(zScore), 0, 'f', 2);
                }
                if (isIqrOutlier) {
                    reasons << QString("Exceeds Q3 + 1.5×IQR (R %1)").arg(statIqrUpperFence, 0, 'f', 2);
                }
                return QString("Outlier Transaction: %1").arg(reasons.join(" & "));
            }
        }
    }
    return QSqlRelationalTableModel::data(index, role);
}

bool ExpenseTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (index.column() == 5) {
            bool ok = false;
            const double amount = value.toDouble(&ok);
            if (!ok || amount < 0.0) {
                return false;
            }
        } else if (index.column() == 2 || index.column() == 3 || index.column() == 4) {
            if (value.toString().trimmed().isEmpty()) {
                return false;
            }
        }
    }
    return QSqlRelationalTableModel::setData(index, value, role);
}

void ExpenseFilterProxyModel::setFilteringEnabled(bool enabled)
{
    if (m_filteringEnabled == enabled) {
        return;
    }
    m_filteringEnabled = enabled;
    invalidateFilter();
}

bool ExpenseFilterProxyModel::isFilteringEnabled() const
{
    return m_filteringEnabled;
}

void ExpenseFilterProxyModel::setFilterCriteria(bool useDateRange, const QDate &startDate, const QDate &endDate, bool filterProjects, const QSet<QString> &selectedProjects, bool filterCategories, const QSet<QString> &selectedCategories, double minAmount, double maxAmount, const QString &keyword)
{
    m_useDateRange = useDateRange;
    m_startDate = startDate;
    m_endDate = endDate;
    m_filterProjects = filterProjects;
    m_selectedProjects = selectedProjects;
    m_filterCategories = filterCategories;
    m_selectedCategories = selectedCategories;
    m_minAmount = minAmount;
    m_maxAmount = maxAmount;
    m_keyword = keyword.trimmed();

    if (m_filteringEnabled) {
        invalidateFilter();
    }
}

bool ExpenseFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!m_filteringEnabled || !sourceModel()) {
        return true;
    }

    const QModelIndex dateIdx = sourceModel()->index(sourceRow, 1, sourceParent);
    const QModelIndex projIdx = sourceModel()->index(sourceRow, 2, sourceParent);
    const QModelIndex catIdx  = sourceModel()->index(sourceRow, 3, sourceParent);
    const QModelIndex descIdx = sourceModel()->index(sourceRow, 4, sourceParent);
    const QModelIndex amtIdx  = sourceModel()->index(sourceRow, 5, sourceParent);

    const QString dateStr = sourceModel()->data(dateIdx, Qt::DisplayRole).toString();
    const QString project = sourceModel()->data(projIdx, Qt::DisplayRole).toString();
    const QString category = sourceModel()->data(catIdx, Qt::DisplayRole).toString();
    const QString description = sourceModel()->data(descIdx, Qt::DisplayRole).toString();

    bool amountOk = false;
    const double amount = sourceModel()->data(amtIdx, Qt::EditRole).toDouble(&amountOk);

    // 1. Date range filter
    if (m_useDateRange) {
        const QDate rowDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (!rowDate.isValid()) {
            return false;
        }
        if (m_startDate.isValid() && rowDate < m_startDate) {
            return false;
        }
        if (m_endDate.isValid() && rowDate > m_endDate) {
            return false;
        }
    }

    // 2. Multi-select Projects filter
    if (m_filterProjects && !m_selectedProjects.contains(project)) {
        return false;
    }

    // 3. Multi-select Categories filter
    if (m_filterCategories && !m_selectedCategories.contains(category)) {
        return false;
    }

    // 4. Min/Max Amount range filter
    constexpr double eps = 1e-9;
    if (!amountOk || amount < (m_minAmount - eps) || amount > (m_maxAmount + eps)) {
        return false;
    }

    // 5. Case-insensitive Keyword search across visible columns
    if (!m_keyword.isEmpty()) {
        const QString amountStr = QString::number(amount, 'f', 2);
        const bool matchesKeyword =
            dateStr.contains(m_keyword, Qt::CaseInsensitive) ||
            project.contains(m_keyword, Qt::CaseInsensitive) ||
            category.contains(m_keyword, Qt::CaseInsensitive) ||
            description.contains(m_keyword, Qt::CaseInsensitive) ||
            amountStr.contains(m_keyword, Qt::CaseInsensitive);
        if (!matchesKeyword) {
            return false;
        }
    }

    return true;
}

bool ExpenseFilterProxyModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    if (sourceLeft.column() == 5) {
        return sourceModel()->data(sourceLeft, Qt::EditRole).toDouble() <
               sourceModel()->data(sourceRight, Qt::EditRole).toDouble();
    }
    if (sourceLeft.column() == 1) {
        const QDate leftDate = QDate::fromString(
            sourceModel()->data(sourceLeft, Qt::DisplayRole).toString(), "yyyy-MM-dd");
        const QDate rightDate = QDate::fromString(
            sourceModel()->data(sourceRight, Qt::DisplayRole).toString(), "yyyy-MM-dd");
        if (leftDate.isValid() && rightDate.isValid()) {
            return leftDate < rightDate;
        }
    }
    return QString::localeAwareCompare(
               sourceModel()->data(sourceLeft, Qt::DisplayRole).toString(),
               sourceModel()->data(sourceRight, Qt::DisplayRole).toString()) < 0;
}
