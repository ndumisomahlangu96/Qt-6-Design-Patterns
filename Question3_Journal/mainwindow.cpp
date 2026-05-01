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
// ----------------------------------------------------------------------


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Setup the table view model.
    databaseModel = new QStandardItemModel(this);
    databaseModel->setHorizontalHeaderLabels({"Author", "Year", "Title", "Journal", "Volume", "Issue", "Pages"});

    ui->tableViewDatabase->setModel(databaseModel);

    // Autosize the columns to fit headers and all row content
    ui->tableViewDatabase->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // Alternatively, if you want the last column to stretch and fill remaining space:
    ui->tableViewDatabase->horizontalHeader()->setStretchLastSection(true);

    // Prevent the UI year spinbox from exceeding the current year.
    ui->spinBoxYear->setMaximum(QDate::currentDate().year());
    ui->spinBoxYear->setValue(QDate::currentDate().year());

    // Apply the custom delegate to Column 1 ("Year") to restrict in-table edits
    ui->tableViewDatabase->setItemDelegateForColumn(1, new YearDelegate(this));

    // Connect the model's data change signal to our custom slot for dynamic color updates
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
    QString volume = ui->spinBoxVolume->text();
    QString issue = ui->spinBoxIssue->text();
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

    // 4. Append the new row to the model.
    databaseModel->appendRow(newRow);

    // 5. (Optional) Clear the line edits for the next entry.
    ui->lineEditAuthor->clear();
    ui->lineEditTitle->clear();
    ui->lineEditJournal->clear();
    ui->lineEditPages->clear();

    // Reset spinboxes to defaults.
    ui->spinBoxYear->setValue(QDate::currentDate().year());
    ui->spinBoxVolume->setValue(0);
    ui->spinBoxIssue->setValue(0);
}

void MainWindow::on_pushButtonRemove_clicked()
{
    if (databaseModel->rowCount() == 0) return;

    QModelIndex currentIndex = ui->tableViewDatabase->currentIndex();

    if (currentIndex.isValid()) {
        databaseModel->removeRow(currentIndex.row());
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
