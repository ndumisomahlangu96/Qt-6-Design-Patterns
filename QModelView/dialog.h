#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    QStringListModel *model;

private slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonInsert_clicked();

    void on_pushButtonDelete_clicked();

private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
