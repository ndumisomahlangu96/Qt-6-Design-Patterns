#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <cmath>

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
 *
 * Extends QSqlRelationalTableModel to:
 * 1. Validate direct user edits made inside the QTableView ledger before persistence.
 * 2. Highlight statistically anomalous expense rows via Qt::BackgroundRole when
 *    Anomaly Detection mode is enabled.
 */
class ExpenseTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the ExpenseTableModel bound to a given SQLite database.
     */
    ExpenseTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
        : QSqlRelationalTableModel(parent, db) {}

    /**
     * @brief Updates the anomaly detection thresholds and triggers a visual repaint.
     *
     * Emits dataChanged exclusively for visual/tooltip roles so connected chart-update
     * handlers do not enter a recursive refresh loop.
     *
     * @param enabled       Whether anomalous rows should be highlighted.
     * @param mean          Dataset mean (mu).
     * @param stdDev        Dataset standard deviation (sigma).
     * @param iqrUpperFence Upper Tukey fence (Q3 + 1.5 * IQR).
     * @param valid         True when dataset contains enough records for valid statistics.
     */
    void setAnomalyThresholds(bool enabled, double mean, double stdDev, double iqrUpperFence, bool valid)
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

    /**
     * @brief Evaluates whether a given transaction amount qualifies as an outlier.
     *
     * Flags values satisfying either |x - mu| > 2*sigma or x > Q3 + 1.5*IQR.
     */
    bool isAmountOutlier(double amount) const
    {
        if (!hasValidStats) {
            return false;
        }
        constexpr double eps = 1e-9;
        const bool isZOutlier = (statStdDev > eps) && (std::abs(amount - statMean) > (2.0 * statStdDev + eps));
        const bool isIqrOutlier = amount > (statIqrUpperFence + eps);
        return isZOutlier || isIqrOutlier;
    }

    /**
     * @brief Overrides model data retrieval to style and annotate anomalous rows.
     *
     * Highlights the entire row in soft crimson when Anomaly Detection is active
     * and the row's Amount (column 5) exceeds statistical outlier bounds.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
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

    /**
     * @brief Validates cell edits before delegating to QSqlRelationalTableModel::setData().
     *
     * Enforces two rules when role == Qt::EditRole:
     * - Column 5 (Amount): Must parse as a valid non-negative floating-point number.
     * - Columns 2, 3, 4 (Project, Category, Description): Must not be empty/whitespace-only.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (role == Qt::EditRole) {
            // Column 5: Validate that Amount is numeric and non-negative
            if (index.column() == 5) {
                bool ok;
                double amount = value.toDouble(&ok);
                if (!ok || amount < 0) {
                    return false;
                }
            }
            // Columns 2, 3, 4: Prevent blank Project, Category, or Description entries
            else if (index.column() == 2 || index.column() == 3 || index.column() == 4) {
                QString str = value.toString().trimmed();
                if (str.isEmpty()) {
                    return false;
                }
            }
        }
        return QSqlRelationalTableModel::setData(index, value, role);
    }

private:
    bool highlightAnomalies = false;
    bool hasValidStats = false;
    double statMean = 0.0;
    double statStdDev = 0.0;
    double statIqrUpperFence = 0.0;
};

/**
 * @brief Primary application window for the Finance Manager.
 *
 * Coordinates three core layers:
 * 1. Persistence: SQLite database schema initialization, CRUD operations, and CSV file I/O.
 * 2. Presentation: Tabbed interface (Data Entry, Ledger Reports, Visual Dashboard), menus, and toolbar.
 * 3. Analytics: Real-time Pie, Line, and Stacked Bar charts, distribution histograms,
 *    anomaly highlighting, and statistical summaries.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    /// Ensures the SQLite database connection is cleanly closed when the window closes.
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

    // --- Analytics & Chart Interactivity ---
    void onPieSliceClicked(QPieSlice *slice);
    void onCalculateTotalSpending();
    void onGenerateStats();
    void onShowStatisticalDistribution();
    void onToggleAnomalyDetection(bool checked);
    void onChartTypeChanged(int index);
    void updateChart();

private:
    // --- Database & File Tracking ---
    QString currentFilePath;          ///< Path to the currently opened/saved external text/CSV file.
    QSqlDatabase db;                  ///< Active SQLite database connection.
    ExpenseTableModel *expenseModel;  ///< Relational model bridging SQLite and the ledger QTableView.

    // --- Data Entry & Ledger UI Widgets ---
    QComboBox *projectCombo;
    QComboBox *categoryCombo;
    QDateEdit *dateEdit;
    QDoubleSpinBox *amountSpin;
    QLineEdit *descriptionEdit;
    QTableView *ledgerView;
    QTabWidget *mainTabWidget;
    QAction *anomalyDetectionAction = nullptr; ///< Toggle action for real-time outlier row highlighting.

    // --- Find & Replace Panel & State ---
    QWidget *findReplaceWidget;
    QLineEdit *findInput;
    QLineEdit *replaceInput;
    QModelIndexList currentMatches;   ///< List of table cell indices matching the current search query.
    int currentMatchIndex = -1;       ///< Active index within currentMatches (-1 if none).
    void performSearch();
    void highlightCurrentMatch();

    // --- Visual Dashboard: Pie & Line Charts ---
    QComboBox *chartTypeCombo;
    QStackedWidget *chartStack;       ///< Switches between Pie, Line, and Stacked Bar chart views.
    QChart *pieChart;
    QChartView *pieChartView;
    QPieSeries *categorySeries;
    QChart *lineChart;
    QChartView *lineChartView;
    QLineSeries *timeSeries;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;

    // --- Visual Dashboard: Stacked Bar Chart ---
    QChart *barChart;
    QChartView *barChartView;
    QStackedBarSeries *barSeries;
    QBarCategoryAxis *barAxisX;
    QValueAxis *barAxisY;

    // --- Initialization, Statistics & Persistence Helpers ---
    void setupDatabase();
    void setupModel();
    void setupFlexibleUI();
    void setupMenuBar();
    int getOrInsertForeignKey(const QString &tableName, const QString &value);
    bool saveDatabaseToFile(const QString &filePath);
    static double calculatePercentile(const QVector<double> &sortedValues, double percentile);
    ExpenseStats computeExpenseStats() const;
    void refreshAnomalyHighlighting();
};
#endif // MAINWINDOW_H