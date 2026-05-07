#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>       // The class provides a generic model for storing custom data.
#include <QHeaderView>              // Include QHeaderView to access header settings.
#include <QDate>                    // Include QDate to get the current year.
#include <QStyledItemDelegate>      // Add for custom table view editors.
#include <QSpinBox>                 // Add for the custom Year spinbox in the table.
#include <QSortFilterProxyModel>    // The class provides support for sorting and filtering data passed between another model and a view.

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Add slots to manage the database.
    void on_pushButtonAdd_clicked();

    void on_pushButtonRemove_clicked();

    // Add custom slot to catch when table data is manually edited.
    void on_modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);

    // Add slots to edit the view of database.
    void on_pushButtonFind_clicked();

    void on_pushButtonReset_clicked();

    void on_radioButtonAscendingOrder_clicked();

    void on_radioButtonDescendingOrder_clicked();

    // Add slots to manage the database.
    void on_actionAdd_triggered();

    void on_actionRemove_triggered();

    // Add slots to edit the view of database.
    void on_actionFind_triggered();

    void on_actionReset_triggered();

    void on_actionSortAscendingOrder_triggered();

    void on_actionSortDescendingOrder_triggered();

    void on_actionClose_triggered();

private:
    Ui::MainWindow *ui;
    QStandardItemModel *databaseModel;
    QSortFilterProxyModel *proxyModel;

};
#endif // MAINWINDOW_H
