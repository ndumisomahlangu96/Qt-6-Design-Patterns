#include "mainwindow.h"
#include "ui_mainwindow.h"

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup the table view model.
    databaseModel = new QStandardItemModel(this);
    databaseModel->setHorizontalHeaderLabels({"Author", "Year", "Title", "Journal", "Volume", "Issue", "Pages"});

    // --- PROXY MODEL SETUP ---
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(databaseModel);

    // Set the proxy model to the table view instead of the databaseModel
    ui->tableViewDatabase->setModel(proxyModel);
    // -----------------------------

    ui->spinBoxYear->setMaximum(QDate::currentDate().year());
    ui->spinBoxYear->setValue(QDate::currentDate().year());
    ui->tableViewDatabase->setItemDelegateForColumn(1, new YearDelegate(this));
    connect(databaseModel, &QStandardItemModel::dataChanged, this, &MainWindow::on_modelDataChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonAdd_clicked()
{
    // 1. Get the data from the UI widgets.
    QString author = ui->lineEditAuthor->text();
    QString year = ui->spinBoxYear->text();
    QString title = ui->lineEditTitle->text();
    QString journal = ui->lineEditJournal->text();
    QString volume = ui->lineEditVolume->text();
    QString issue = ui->lineEditIssue->text();
    QString pages = ui->lineEditPages->text();

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
    ui->tableViewDatabase->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Alternatively, if you want the last column to stretch and fill remaining space:
    ui->tableViewDatabase->horizontalHeader()->setStretchLastSection(true);

    // 4. Append the new row to the model.
    databaseModel->appendRow(newRow);

    // 5. Clear the line edits for the next entry.
    ui->lineEditAuthor->clear();
    ui->lineEditTitle->clear();
    ui->lineEditJournal->clear();
    ui->lineEditPages->clear();
    ui->lineEditVolume->clear();
    ui->lineEditIssue->clear();

    // Reset spinboxes to defaults.
    ui->spinBoxYear->setValue(QDate::currentDate().year());
}

// --- CRITICAL FIX FOR REMOVE BUTTON ---
void MainWindow::on_pushButtonRemove_clicked()
{
    if (proxyModel->rowCount() == 0) return;

    // We must get the index from the TableView (which now uses the proxy model)
    QModelIndex proxyIndex = ui->tableViewDatabase->currentIndex();

    if (proxyIndex.isValid()) {
        // Map the proxy index back to the underlying database model index
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        databaseModel->removeRow(sourceIndex.row());
        ui->tableViewDatabase->clearSelection();
    } else {
        databaseModel->removeRow(databaseModel->rowCount() - 1);
    }
}

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

// --- FILTERING LOGIC ---
void MainWindow::on_pushButtonFind_clicked()
{
    // 1. Get the wildcard text from the line edit
    QString filterText = ui->lineEditWildcard->text();

    // 2. Get the selected column from the combo box
    int filterColumn = ui->comboBoxFilter->currentIndex();

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
    ui->lineEditWildcard->clear();

    // Clear the filter by passing an empty QRegularExpression
    proxyModel->setFilterRegularExpression(QRegularExpression());
}

void MainWindow::on_radioButtonAscendingOrder_clicked()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = ui->comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Ascending Order
    proxyModel->sort(sortColumn, Qt::AscendingOrder);
}

void MainWindow::on_radioButtonDescendingOrder_clicked()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = ui->comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Descending Order
    proxyModel->sort(sortColumn, Qt::DescendingOrder);
}

void MainWindow::on_actionAdd_triggered()
{
    // 1. Get the data from the UI widgets.
    QString author = ui->lineEditAuthor->text();
    QString year = ui->spinBoxYear->text();
    QString title = ui->lineEditTitle->text();
    QString journal = ui->lineEditJournal->text();
    QString volume = ui->lineEditVolume->text();
    QString issue = ui->lineEditIssue->text();
    QString pages = ui->lineEditPages->text();

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
    ui->tableViewDatabase->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Alternatively, if you want the last column to stretch and fill remaining space:
    ui->tableViewDatabase->horizontalHeader()->setStretchLastSection(true);

    // 4. Append the new row to the model.
    databaseModel->appendRow(newRow);

    // 5. Clear the line edits for the next entry.
    ui->lineEditAuthor->clear();
    ui->lineEditTitle->clear();
    ui->lineEditJournal->clear();
    ui->lineEditPages->clear();
    ui->lineEditVolume->clear();
    ui->lineEditIssue->clear();

    // Reset spinboxes to defaults.
    ui->spinBoxYear->setValue(QDate::currentDate().year());
}

// --- CRITICAL FIX FOR REMOVE BUTTON ---
void MainWindow::on_actionRemove_triggered()
{
    if (proxyModel->rowCount() == 0) return;

    // We must get the index from the TableView (which now uses the proxy model)
    QModelIndex proxyIndex = ui->tableViewDatabase->currentIndex();

    if (proxyIndex.isValid()) {
        // Map the proxy index back to the underlying database model index
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        databaseModel->removeRow(sourceIndex.row());
        ui->tableViewDatabase->clearSelection();
    } else {
        databaseModel->removeRow(databaseModel->rowCount() - 1);
    }
}

// --- FILTERING LOGIC ---
void MainWindow::on_actionFind_triggered()
{
    // 1. Get the wildcard text from the line edit
    QString filterText = ui->lineEditWildcard->text();

    // 2. Get the selected column from the combo box
    int filterColumn = ui->comboBoxFilter->currentIndex();

    // 3. Set the column we want to filter on
    proxyModel->setFilterKeyColumn(filterColumn);

    // 4. Create a Qt 6 compatible regular expression from the wildcard string
    QRegularExpression regExp(QRegularExpression::fromWildcard(filterText));

    // Optional: Make the search case-insensitive for better user experience
    regExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    // 5. Apply the filter
    proxyModel->setFilterRegularExpression(regExp);
}

void MainWindow::on_actionReset_triggered()
{
    // Clear the search field
    ui->lineEditWildcard->clear();

    // Clear the filter by passing an empty QRegularExpression
    proxyModel->setFilterRegularExpression(QRegularExpression());
}

void MainWindow::on_actionSortAscendingOrder_triggered()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = ui->comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Ascending Order
    proxyModel->sort(sortColumn, Qt::AscendingOrder);
}

void MainWindow::on_actionSortDescendingOrder_triggered()
{
    // 1. Get the selected column index from the combo box
    int sortColumn = ui->comboBoxSort->currentIndex();

    // 2. Sort the proxy model based on that column in Descending Order
    proxyModel->sort(sortColumn, Qt::DescendingOrder);
}

// Closes the MainWindow.
void MainWindow::on_actionClose_triggered()
{
    // Since this is the main application window, closing it will terminate the QApplication.
    this->close();
}
