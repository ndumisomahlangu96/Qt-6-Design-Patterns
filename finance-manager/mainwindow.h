#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <cmath>
#include <limits>

#include <QMainWindow>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSortFilterProxyModel>
#include <QPersistentModelIndex>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QListWidget>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QHeaderView>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QStyle>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QLabel>
#include <QDateTime>
#include <QTime>
#include <QStackedWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QSignalBlocker>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

#include <QtCharts/QBarSeries>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>

/**
 * @brief Container for descriptive, distributional, and anomaly-detection statistics.
 */
struct ExpenseStats
{
    int count = 0;
    double sum = 0.0;
    double mean = 0.0;
    double min = 0.0;
    double max = 0.0;
    double q1 = 0.0;
    double median = 0.0;
    double q3 = 0.0;
    double iqr = 0.0;
    double stdDev = 0.0;
    double zLowerThreshold = 0.0;
    double zUpperThreshold = 0.0;
    double iqrUpperFence = 0.0;
    int outlierCount = 0;
    QVector<double> sortedAmounts;
};

/**
 * @brief Custom relational SQL table model with inline edit validation and anomaly highlighting.
 */
class ExpenseTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
public:
    explicit ExpenseTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db) {}

    /**
     * @brief Updates the anomaly detection thresholds and triggers a visual repaint.
     */
    void setAnomalyThresholds(bool enabled, double mean, double stdDev, double iqrUpperFence, bool valid);

    /**
     * @brief Evaluates whether a given transaction amount qualifies as an outlier.
     */
    bool isAmountOutlier(double amount) const;

    /**
     * @brief Overrides model data retrieval to style and annotate anomalous rows.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Validates cell edits before delegating to QSqlRelationalTableModel::setData().
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    bool highlightAnomalies = false;
    bool hasValidStats = false;
    double statMean = 0.0;
    double statStdDev = 0.0;
    double statIqrUpperFence = 0.0;
};

/**
 * @brief Multi-criteria proxy model sitting between ExpenseTableModel and QTableView.
 *
 * Filters rows simultaneously by:
 * 1. Date range (start/end QDate)
 * 2. Multi-selected Projects
 * 3. Multi-selected Categories
 * 4. Min/Max Amount bounds
 * 5. Case-insensitive Keyword search across all visible columns
 */
class ExpenseFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ExpenseFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent) {}

    void setFilteringEnabled(bool enabled);

    bool isFilteringEnabled() const;

    void setFilterCriteria(bool useDateRange,
                           const QDate &startDate,
                           const QDate &endDate,
                           bool filterProjects,
                           const QSet<QString> &selectedProjects,
                           bool filterCategories,
                           const QSet<QString> &selectedCategories,
                           double minAmount,
                           double maxAmount,
                           const QString &keyword);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

private:
    bool m_filteringEnabled = false;
    bool m_useDateRange = false;
    QDate m_startDate;
    QDate m_endDate;
    bool m_filterProjects = false;
    QSet<QString> m_selectedProjects;
    bool m_filterCategories = false;
    QSet<QString> m_selectedCategories;
    double m_minAmount = 0.0;
    double m_maxAmount = std::numeric_limits<double>::max();
    QString m_keyword;
};

/**
 * @brief Primary application window for the Finance Manager.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // --- Record Management & File I/O ---
    void onAddExpenseClicked();
    void onOpenFile();
    void onSaveCurrentFile();
    void onSaveFile();
    void onClearDatabase();
    void onExit();
    void onDeleteSelected();
    void onAbout();
    void onSortLedger();

    // --- Ledger Find & Replace ---
    void onToggleFindReplace();
    void onFindNext();
    void onFindPrevious();
    void onSelectAllMatches();
    void onReplace();
    void onReplaceAndFind();
    void onReplaceAll();

    // --- Live Multi-Criteria Filtering ---
    void onToggleLiveFiltering(bool checked);
    void onFilterCriteriaChanged();
    void onFilterStartDateChanged(const QDate &date);
    void onFilterEndDateChanged(const QDate &date);
    void onMinAmountSliderChanged(int value);
    void onMaxAmountSliderChanged(int value);
    void onResetFilters();
    void onSelectAllProjects();
    void onClearAllProjects();
    void onSelectAllCategories();
    void onClearAllCategories();

    // --- Analytics & Chart Interactivity ---
    void onPieSliceClicked(QPieSlice *slice);
    void onCalculateTotalSpending();
    void onGenerateStats();
    void onShowStatisticalDistribution();
    void onToggleAnomalyDetection(bool checked);
    void onChartTypeChanged(int index);
    void updateChart();

private:
    // --- Database & Models ---
    QString currentFilePath;
    QSqlDatabase db;
    ExpenseTableModel *expenseModel = nullptr;
    ExpenseFilterProxyModel *expenseProxyModel = nullptr;

    // --- Data Entry & Ledger UI Widgets ---
    QComboBox *projectCombo = nullptr;
    QComboBox *categoryCombo = nullptr;
    QDateEdit *dateEdit = nullptr;
    QDoubleSpinBox *amountSpin = nullptr;
    QLineEdit *descriptionEdit = nullptr;
    QTableView *ledgerView = nullptr;
    QTabWidget *mainTabWidget = nullptr;
    QAction *anomalyDetectionAction = nullptr;
    QAction *liveFilterAction = nullptr;

    // --- Live Multi-Criteria Filter Panel ---
    QGroupBox *filterPanelWidget = nullptr;
    QCheckBox *filterDateEnabledCheck = nullptr;
    QDateEdit *filterStartDateEdit = nullptr;
    QDateEdit *filterEndDateEdit = nullptr;
    QListWidget *filterProjectsList = nullptr;
    QListWidget *filterCategoriesList = nullptr;
    QSlider *minAmountSlider = nullptr;
    QSlider *maxAmountSlider = nullptr;
    QLabel *minAmountValueLabel = nullptr;
    QLabel *maxAmountValueLabel = nullptr;
    QLineEdit *filterKeywordEdit = nullptr;
    QLabel *filterStatusLabel = nullptr;
    double amountSliderScale = 100.0;
    bool dateFilterCustomized = false;

    // --- Find & Replace Panel & State ---
    QWidget *findReplaceWidget = nullptr;
    QLineEdit *findInput = nullptr;
    QLineEdit *replaceInput = nullptr;
    QModelIndexList currentMatches;
    int currentMatchIndex = -1;
    void performSearch();
    void highlightCurrentMatch();

    // --- Visual Dashboard: Pie & Line Charts ---
    QComboBox *chartTypeCombo = nullptr;
    QStackedWidget *chartStack = nullptr;
    QChart *pieChart = nullptr;
    QChartView *pieChartView = nullptr;
    QPieSeries *categorySeries = nullptr;
    QChart *lineChart = nullptr;
    QChartView *lineChartView = nullptr;
    QLineSeries *timeSeries = nullptr;
    QDateTimeAxis *axisX = nullptr;
    QValueAxis *axisY = nullptr;

    // --- Visual Dashboard: Stacked Bar Chart ---
    QChart *barChart = nullptr;
    QChartView *barChartView = nullptr;
    QStackedBarSeries *barSeries = nullptr;
    QBarCategoryAxis *barAxisX = nullptr;
    QValueAxis *barAxisY = nullptr;

    // --- Initialization, Filtering, Statistics & Persistence Helpers ---
    void setupDatabase();
    void setupModel();
    void setupFlexibleUI();
    void setupMenuBar();
    void fetchAllModelRows();
    void reloadLookupCombos();
    void syncFilterControlsWithDatabase(bool resetSelections);
    void applyFilterCriteriaToProxy();
    void updateAmountSliderLabels();
    void updateFilterStatusLabel();
    void setListItemsChecked(QListWidget *listWidget, Qt::CheckState state);
    int getOrInsertForeignKey(const QString &tableName, const QString &value);
    bool saveDatabaseToFile(const QString &filePath);
    static double calculatePercentile(const QVector<double> &sortedValues, double percentile);
    ExpenseStats computeExpenseStats() const;
    void refreshAnomalyHighlighting();
};
#endif // MAINWINDOW_H