#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

// QtCharts headers
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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
    void onFind(); // New find slot added here
    void onPieSliceClicked(QPieSlice *slice); // New slot for interactivity

private:
    Ui::MainWindow *ui;

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