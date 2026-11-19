#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QHeaderView>
#include <QMap>
#include <QDir>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QLabel>
#include <QDateTime>
#include <QStackedWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>

// Custom Model to validate edits made directly in the QTableView
class ExpenseTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
public:
    ExpenseTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db) {}

    // Inline implementation to fix the "undefined reference" error
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole) {
            // Column 5: Amount
            if (index.column() == 5) {
                bool ok;
                double amount = value.toDouble(&ok);
                if (!ok || amount < 0) {
                    return false;
                }
            }
            // Columns 2, 3, 4: Project, Category, Description
            else if (index.column() == 2 || index.column() == 3 || index.column() == 4) {
                QString str = value.toString().trimmed();
                if (str.isEmpty()) {
                    return false;
                }
            }
        }
        return QSqlRelationalTableModel::setData(index, value, role);
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddExpenseClicked();
    void onOpenFile();
    void onSaveCurrentFile(); // <-- Add this
    void onSaveFile();
    void onClearDatabase();
    void onExit();
    void onDeleteSelected();
    void onAbout();
    void onSortLedger();

    void onToggleFindReplace();
    void onFindNext();
    void onFindPrevious();
    void onSelectAllMatches();
    void onReplace();
    void onReplaceAndFind();
    void onReplaceAll();

    void onPieSliceClicked(QPieSlice *slice);
    void onCalculateTotalSpending();
    void onGenerateStats();
    void onChartTypeChanged(int index);
    void updateChart();

private:
    QString currentFilePath;
    QSqlDatabase db;
    ExpenseTableModel *expenseModel;

    QComboBox *projectCombo;
    QComboBox *categoryCombo;
    QDateEdit *dateEdit;
    QDoubleSpinBox *amountSpin;
    QLineEdit *descriptionEdit;
    QTableView *ledgerView;
    QTabWidget *mainTabWidget;

    QWidget *findReplaceWidget;
    QLineEdit *findInput;
    QLineEdit *replaceInput;

    QModelIndexList currentMatches;
    int currentMatchIndex = -1;
    void performSearch();
    void highlightCurrentMatch();

    QComboBox *chartTypeCombo;
    QStackedWidget *chartStack;
    QChart *pieChart;
    QChartView *pieChartView;
    QPieSeries *categorySeries;
    QChart *lineChart;
    QChartView *lineChartView;
    QLineSeries *timeSeries;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;

    // Stacked Bar Chart Variables
    QChart *barChart;
    QChartView *barChartView;
    QStackedBarSeries *barSeries;
    QBarCategoryAxis *barAxisX;
    QValueAxis *barAxisY;

    void setupDatabase();
    void setupModel();
    void setupFlexibleUI();
    void setupMenuBar();
    int getOrInsertForeignKey(const QString &tableName, const QString &value);
    bool saveDatabaseToFile(const QString &filePath); // <-- Add this
};
#endif // MAINWINDOW_H