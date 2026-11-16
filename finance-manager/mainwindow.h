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
#include <QLabel>
#include <QDateTime>
#include <QStackedWidget>
#include <utility>

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
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
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

    // Updated slot for toggling the find widget
    void onToggleFindReplace();

    // Slots for the Find/Replace widget buttons
    void onFindNext();
    void onFindPrevious();
    void onSelectAllMatches();
    void onReplace();
    void onReplaceAndFind();
    void onReplaceAll();

    void onPieSliceClicked(QPieSlice *slice);
    void onCalculateTotalSpending();
    void onGenerateStats();

    // Slot for switching visualizations
    void onChartTypeChanged(int index);

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
    QTabWidget *mainTabWidget; // Track the tab widget to switch tabs

    // --- Find/Replace Widget Components ---
    QWidget *findReplaceWidget;
    QLineEdit *findInput;
    QLineEdit *replaceInput;

    // Search state tracking
    QModelIndexList currentMatches;
    int currentMatchIndex = -1;
    void performSearch();
    void highlightCurrentMatch();

    // Chart Components
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

    void setupFlexibleUI();
    void setupModel();
    void updateChart();
    void setupMenuBar(); // Helper for menu setup

    // Storage
    QString getDataFilePath();
};
#endif // MAINWINDOW_H