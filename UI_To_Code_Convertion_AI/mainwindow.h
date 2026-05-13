#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>       // The class provides a generic model for storing custom data.
#include <QHeaderView>              // Include QHeaderView to access header settings.
#include <QDate>                    // Include QDate to get the current year.
#include <QStyledItemDelegate>      // Add for custom table view editors.
#include <QSpinBox>                 // Add for the custom Year spinbox in the table.
#include <QSortFilterProxyModel>    // The class provides support for sorting and filtering data passed between another model and a view.

// Required Qt Headers for UI Elements
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTableView>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QIcon>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

    // Add custom slot to catch when table data is manually edited.
    void on_modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);

    // Add custom slot for the Add button
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();
    void on_pushButtonFind_clicked();
    void on_pushButtonReset_clicked();

    // Add slots to edit the view of database.
    void on_radioButtonAscendingOrder_clicked();

    void on_radioButtonDescendingOrder_clicked();

    // Close the QApplication.
    void on_actionClose_triggered();



private:
    QStandardItemModel *databaseModel;
    QSortFilterProxyModel *proxyModel;

    // --- Add these UI pointers so they are accessible in your slots ---
    QLineEdit *lineEditAuthor;
    QSpinBox *spinBoxYear;
    QLineEdit *lineEditTitle;
    QLineEdit *lineEditJournal;
    QLineEdit *lineEditVolume;
    QLineEdit *lineEditIssue;
    QLineEdit *lineEditPages;
    QTableView *tableViewDatabase;
    QComboBox *comboBoxSort;
    QComboBox *comboBoxFilter;
    QLineEdit *lineEditWildcard;
    QRadioButton *radioButtonAscendingOrder;
    QRadioButton *radioButtonDescendingOrder;
    QPushButton *pushButtonAdd;
    QPushButton *pushButtonRemove;
    QPushButton *pushButtonFind;
    QPushButton *pushButtonReset;

    // Setup Actions
    QAction *actionAdd;
    QAction *actionRemove;
    QAction *actionFind;
    QAction *actionReset;
    QAction *actionSortAscendingOrder;
    QAction *actionSortDescendingOrder;
    QAction *actionClose;


    // Setup Menus
    QMenuBar *menubar;
    QMenu *menuDatabase;
    QMenu *menuView;

    // Setup Statusbar
    QStatusBar *statusbar;

};
#endif // MAINWINDOW_H
