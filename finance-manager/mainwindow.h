#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QStandardItemModel>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QCloseEvent>
#include <QDomDocument>
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QHeaderView>
#include <QMap>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QKeySequence>

// New Includes for standard application features
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>

// QtCharts headers
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // Slots to connect to QMenuBar action
    void onAddExpenseClicked();
    void saveData();
    void saveDataAs();
    void openFile();
    void onNewData();
    void onExportCSV();
    void onExit();
    void onDeleteSelected();
    void onAbout();
    void onSortLedger();
    void onFindAndReplace();
    void onPieSliceClicked(QPieSlice *slice);
    void onCalculateTotalSpending();
    void onGenerateStats();

private:
    // Data Model
    QStandardItemModel *expenseModel;
    QString currentFilePath; // Add this variable to track the active file

    // UI Components
    QComboBox *projectCombo;
    QComboBox *categoryCombo;
    QDateEdit *dateEdit;
    QDoubleSpinBox *amountSpin;
    QLineEdit *descriptionEdit;
    QTableView *ledgerView;

    // Chart Components
    QChart *expenseChart;
    QPieSeries *categorySeries;
    QChartView *chartView;

    void setupFlexibleUI();
    void setupModel();
    void updateChart();
    void setupMenuBar(); // Helper for menu setup

    // Storage
    QString getDataFilePath();
};
#endif // MAINWINDOW_H