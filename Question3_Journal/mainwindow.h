#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QStandardItemModel>
#include <QHeaderView>              // Include QHeaderView to access header settings.
#include <QDate>                    // Include QDate to get the current year.
#include <QStyledItemDelegate>      // Add for custom table view editors.
#include <QSpinBox>                 // Add for the custom Year spinbox in the table.

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
    void on_pushButtonAdd_clicked();

    void on_pushButtonRemove_clicked();

    // Add custom slot to catch when table data is manually edited
    void on_modelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *databaseModel;

};
#endif // MAINWINDOW_H
