#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QFileSystemModel>

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

private slots:
    void on_pushButtonMakeDirectory_clicked();

    void on_pushButtonDelete_clicked();

private:
    Ui::Dialog *ui;
    QFileSystemModel *model; // Updated type here
};
#endif // DIALOG_H
