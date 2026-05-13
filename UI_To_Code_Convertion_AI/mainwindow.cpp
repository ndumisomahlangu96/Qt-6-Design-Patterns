#include "mainwindow.h"


// --- Custom Delegate to restrict Year editing inside the Table View ---
class YearDelegate : public QStyledItemDelegate {
public:
    YearDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(QDate::currentDate().year()); // Restrict max value to current year
        return editor;
    }
};

// ----------------------------------------------------------------------
// Custom Slot to dynamically change color if the table is manually edited
// ----------------------------------------------------------------------
void MainWindow::on_modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles)
{
    // Fix 1: Tell the compiler we are intentionally not using these parameters
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    // Only proceed if the user specifically edited the Year column (index 1)
    if (topLeft.column() == 1) {
        int row = topLeft.row();
        int year = databaseModel->data(topLeft).toInt();
        int currentYear = QDate::currentDate().year();

        QBrush rowColor;
        if (currentYear - year > 10) {
            rowColor = QBrush(Qt::red);
        } else if (currentYear - year <= 5) {
            rowColor = QBrush(Qt::green);
        }

        // Block signals to prevent an infinite loop while we automatically update backgrounds
        databaseModel->blockSignals(true);

        for (int col = 0; col < databaseModel->columnCount(); ++col) {
            QStandardItem *item = databaseModel->item(row, col);
            if (item) {
                if (rowColor.style() != Qt::NoBrush) {
                    item->setBackground(rowColor);
                } else {
                    // Fix 2: Use an empty QBrush to clear the background instead of QVariant
                    item->setBackground(QBrush());
                }
            }
        }

        databaseModel->blockSignals(false);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. Setup Main Window Properties
    this->resize(767, 406);
    this->setWindowTitle("Journals");
    // Note: ThemeIcon is available in recent Qt 6 versions. If using an older version,
    // you may need to use string equivalents like QIcon::fromTheme("computer").
    this->setWindowIcon(QIcon::fromTheme(QIcon::ThemeIcon::Computer));

    // 2. Setup Central Widget and Main Layout
    QWidget *centralwidget = new QWidget(this);
    this->setCentralWidget(centralwidget);
    QGridLayout *gridLayout_5 = new QGridLayout(centralwidget);

    // --- Row 0, Column 0: Input Form ---
    QGridLayout *gridLayout_4 = new QGridLayout();

    QLabel *labelAuthor = new QLabel("Author:", centralwidget);
    lineEditAuthor = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelAuthor, 0, 0);
    gridLayout_4->addWidget(lineEditAuthor, 0, 1);

    QLabel *labelYear = new QLabel("Year:", centralwidget);
    spinBoxYear = new QSpinBox(centralwidget);
    spinBoxYear->setMinimum(0);
    spinBoxYear->setMaximum(10000);
    spinBoxYear->setValue(2025);
    gridLayout_4->addWidget(labelYear, 1, 0);
    gridLayout_4->addWidget(spinBoxYear, 1, 1);

    QLabel *labelTitle = new QLabel("Title:", centralwidget);
    lineEditTitle = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelTitle, 2, 0);
    gridLayout_4->addWidget(lineEditTitle, 2, 1);

    QLabel *labelJournal = new QLabel("Journal:", centralwidget);
    lineEditJournal = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelJournal, 3, 0);
    gridLayout_4->addWidget(lineEditJournal, 3, 1);

    QLabel *labelVolume = new QLabel("Volume:", centralwidget);
    lineEditVolume = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelVolume, 4, 0);
    gridLayout_4->addWidget(lineEditVolume, 4, 1);

    QLabel *labelIssue = new QLabel("Issue:", centralwidget);
    lineEditIssue = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelIssue, 5, 0);
    gridLayout_4->addWidget(lineEditIssue, 5, 1);

    QLabel *labelPages = new QLabel("Pages:", centralwidget);
    lineEditPages = new QLineEdit(centralwidget);
    gridLayout_4->addWidget(labelPages, 6, 0);
    gridLayout_4->addWidget(lineEditPages, 6, 1);

    gridLayout_5->addLayout(gridLayout_4, 0, 0);

    // --- Row 0, Column 1: Database View ---
    tableViewDatabase = new QTableView(centralwidget);
    gridLayout_5->addWidget(tableViewDatabase, 0, 1);

    // --- Row 1, Column 0: Add/Remove Buttons ---
    QGridLayout *gridLayout = new QGridLayout();

    pushButtonAdd = new QPushButton("Add", centralwidget);
    pushButtonAdd->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
    pushButtonAdd->setAutoDefault(true);
    pushButtonAdd->setDefault(true);
    gridLayout->addWidget(pushButtonAdd, 0, 0);

    pushButtonRemove = new QPushButton("Remove", centralwidget);
    pushButtonRemove->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove));
    gridLayout->addWidget(pushButtonRemove, 1, 0);

    gridLayout_5->addLayout(gridLayout, 1, 0);

    // --- Row 1, Column 1 (rowspan 3): Sorting GroupBox ---
    QGroupBox *groupBoxSort = new QGroupBox("Sort:", centralwidget);
    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBoxSort); // Replaced absolute geometry with dynamic layout

    QGridLayout *gridLayout_6 = new QGridLayout();
    QLabel *labelSort = new QLabel("Select column to sort in database view:", groupBoxSort);
    comboBoxSort = new QComboBox(groupBoxSort);
    comboBoxSort->addItems({"Author", "Year", "Title", "Journal", "Volume", "Issue", "Pages"});
    gridLayout_6->addWidget(labelSort, 0, 0);
    gridLayout_6->addWidget(comboBoxSort, 0, 1);
    groupBoxLayout->addLayout(gridLayout_6);

    QGridLayout *gridLayout_7 = new QGridLayout();
    radioButtonAscendingOrder = new QRadioButton("Ascending Order", groupBoxSort);
    radioButtonDescendingOrder = new QRadioButton("Descending Order", groupBoxSort);
    gridLayout_7->addWidget(radioButtonAscendingOrder, 0, 0);
    gridLayout_7->addWidget(radioButtonDescendingOrder, 1, 0);
    groupBoxLayout->addLayout(gridLayout_7);

    gridLayout_5->addWidget(groupBoxSort, 1, 1, 3, 1); // Rowspan 3, Colspan 1

    // --- Row 2, Column 0: Filter controls ---
    QGridLayout *gridLayout_2 = new QGridLayout();

    QLabel *labelFilter = new QLabel("Select column to filter in database view:", centralwidget);
    comboBoxFilter = new QComboBox(centralwidget);
    comboBoxFilter->addItems({"Author", "Year", "Title", "Journal", "Volume", "Issue"});
    gridLayout_2->addWidget(labelFilter, 0, 0, 1, 2); // Colspan 2
    gridLayout_2->addWidget(comboBoxFilter, 0, 2);

    QLabel *labelFind = new QLabel("Find:", centralwidget);
    lineEditWildcard = new QLineEdit(centralwidget);
    gridLayout_2->addWidget(labelFind, 1, 0);
    gridLayout_2->addWidget(lineEditWildcard, 1, 1, 1, 2); // Colspan 2

    gridLayout_5->addLayout(gridLayout_2, 2, 0);

    // --- Row 3, Column 0: Find/Reset Buttons ---
    QGridLayout *gridLayout_3 = new QGridLayout();

    pushButtonFind = new QPushButton("Find", centralwidget);
    pushButtonFind->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditFind));
    gridLayout_3->addWidget(pushButtonFind, 0, 0);

    pushButtonReset = new QPushButton("Reset", centralwidget);
    pushButtonReset->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewRestore));
    gridLayout_3->addWidget(pushButtonReset, 0, 1);

    gridLayout_5->addLayout(gridLayout_3, 3, 0);

    // 3. Setup Actions
    actionAdd = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd), "Add", this);
    actionRemove = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove", this);
    actionFind = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::EditFind), "Find", this);
    actionReset = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ViewRestore), "Reset", this);
    actionSortAscendingOrder = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::GoUp), "Sort Ascending Order", this);
    actionSortDescendingOrder = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::GoDown), "Sort Descending Order", this);
    actionClose = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ProcessStop), "Close", this);

    // 4. Setup Menus
    menubar = this->menuBar();

    menuDatabase = menubar->addMenu("Database");
    menuDatabase->addAction(actionAdd);
    menuDatabase->addAction(actionRemove);
    menuDatabase->addSeparator();
    menuDatabase->addAction(actionClose);

    menuView = menubar->addMenu("View");
    menuView->addAction(actionFind);
    menuView->addAction(actionReset);
    menuView->addSeparator();
    menuView->addAction(actionSortAscendingOrder);
    menuView->addAction(actionSortDescendingOrder);

    // 5. Setup Status Bar
    statusbar = new QStatusBar(this);
    this->setStatusBar(statusbar);

    // Setup the table view model.
    databaseModel = new QStandardItemModel(this);
    databaseModel->setHorizontalHeaderLabels({"Author", "Year", "Title", "Journal", "Volume", "Issue", "Pages"});

    // --- PROXY MODEL SETUP ---
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(databaseModel);

    // Set the proxy model to the table view instead of the databaseModel
    tableViewDatabase->setModel(proxyModel);
    // -----------------------------

    spinBoxYear->setMaximum(QDate::currentDate().year());
    spinBoxYear->setValue(QDate::currentDate().year());
    tableViewDatabase->setItemDelegateForColumn(1, new YearDelegate(this));
    connect(databaseModel, &QStandardItemModel::dataChanged, this, &MainWindow::on_modelDataChanged);

    // MANUALLY CONNECT THE SIGNAL TO YOUR NEW SLOT:
    connect(pushButtonAdd, &QPushButton::clicked, this, &MainWindow::on_pushButtonAdd_clicked);
    connect(pushButtonRemove, &QPushButton::clicked, this, &MainWindow::on_pushButtonRemove_clicked);
    connect(actionAdd, &QAction::triggered, this, &MainWindow::on_pushButtonAdd_clicked);
    connect(actionRemove, &QAction::triggered, this, &MainWindow::on_pushButtonRemove_clicked);
    connect(actionClose, &QAction::triggered, this, &MainWindow::on_actionClose_triggered);
    connect(pushButtonFind, &QPushButton::clicked, this, &MainWindow::on_pushButtonFind_clicked);
    connect(pushButtonReset, &QPushButton::clicked, this, &MainWindow::on_pushButtonReset_clicked);
    connect(actionFind, &QAction::triggered, this, &MainWindow::on_pushButtonFind_clicked);
    connect(actionReset, &QAction::triggered, this, &MainWindow::on_pushButtonReset_clicked);
    connect(radioButtonAscendingOrder, &QRadioButton::clicked, this, &MainWindow::on_radioButtonAscendingOrder_clicked);
    connect(radioButtonDescendingOrder, &QRadioButton::clicked, this, &MainWindow::on_radioButtonDescendingOrder_clicked);
    connect(actionSortAscendingOrder, &QAction::triggered, this, &MainWindow::on_radioButtonAscendingOrder_clicked);
    connect(actionSortDescendingOrder, &QAction::triggered, this, &MainWindow::on_radioButtonDescendingOrder_clicked);
}

MainWindow::~MainWindow() = default;

void MainWindow::on_pushButtonAdd_clicked()
{
    // 1. Get the data from the UI widgets.
    QString author = lineEditAuthor->text();
    QString year = spinBoxYear->text();
    QString title = lineEditTitle->text();
    QString journal = lineEditJournal->text();
    QString volume = lineEditVolume->text();
    QString issue = lineEditIssue->text();
    QString pages = lineEditPages->text();

    // 2. Create a list of QStandardItems to represent a single row.
    QList<QStandardItem *> newRow;
    newRow << new QStandardItem(author)
           << new QStandardItem(year)
           << new QStandardItem(title)
           << new QStandardItem(journal)
           << new QStandardItem(volume)
           << new QStandardItem(issue)
           << new QStandardItem(pages);

    // 3. Determine Row Highlight Color based on the Year
    int currentYear = QDate::currentDate().year();
    int itemYear = year.toInt();
    QBrush rowColor;

    if (currentYear - itemYear > 10) {
        rowColor = QBrush(Qt::red);
    } else if (currentYear - itemYear <= 5) {
        rowColor = QBrush(Qt::green);
    }

    // Apply the color to all items in the new row
    if (rowColor.style() != Qt::NoBrush) {
        for (QStandardItem *item : newRow) {
            item->setBackground(rowColor);
        }
    }

    // Autosize the columns to fit headers and all row content
    tableViewDatabase->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Alternatively, if you want the last column to stretch and fill remaining space:
    tableViewDatabase->horizontalHeader()->setStretchLastSection(true);

    // 4. Append the new row to the model.
    databaseModel->appendRow(newRow);

    // 5. Clear the line edits for the next entry.
    lineEditAuthor->clear();
    lineEditTitle->clear();
    lineEditJournal->clear();
    lineEditPages->clear();
    lineEditVolume->clear();
    lineEditIssue->clear();

    // Reset spinboxes to defaults. (Fixed typo from 'uspinBoxYear')
    spinBoxYear->setValue(QDate::currentDate().year());
}

// --- CRITICAL FIX FOR REMOVE BUTTON ---
void MainWindow::on_pushButtonRemove_clicked()
{
    if (proxyModel->rowCount() == 0) return;

    // We must get the index from the TableView (which now uses the proxy model)
    QModelIndex proxyIndex = tableViewDatabase->currentIndex();

    if (proxyIndex.isValid()) {
        // Map the proxy index back to the underlying database model index
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        databaseModel->removeRow(sourceIndex.row());
        tableViewDatabase->clearSelection();
    } else {
        databaseModel->removeRow(databaseModel->rowCount() - 1);
    }
}


// --- FILTERING LOGIC ---
void MainWindow::on_pushButtonFind_clicked()
{
    // 1. Get the wildcard text from the line edit
    QString filterText = lineEditWildcard->text();

    // 2. Get the selected column from the combo box
    int filterColumn = comboBoxFilter->currentIndex();

    // 3. Set the column we want to filter on
    proxyModel->setFilterKeyColumn(filterColumn);

    // 4. Create a Qt 6 compatible regular expression from the wildcard string
    QRegularExpression regExp(QRegularExpression::fromWildcard(filterText));

    // Optional: Make the search case-insensitive for better user experience
    regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    // 5. Apply the filter
    proxyModel->setFilterRegularExpression(regExp);
}

void MainWindow::on_pushButtonReset_clicked()
{
    // Clear the search field
    lineEditWildcard->clear();

    // Clear the filter by passing an empty QRegularExpression
    proxyModel->setFilterRegularExpression(QRegularExpression());
}

void MainWindow::on_radioButtonAscendingOrder_clicked()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Ascending Order
    proxyModel->sort(sortColumn, Qt::AscendingOrder);
}

void MainWindow::on_radioButtonDescendingOrder_clicked()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Descending Order
    proxyModel->sort(sortColumn, Qt::DescendingOrder);
}

// Closes the MainWindow.
void MainWindow::on_actionClose_triggered()
{
    // Since this is the main application window, closing it will terminate the QApplication.
    this->close();
}
