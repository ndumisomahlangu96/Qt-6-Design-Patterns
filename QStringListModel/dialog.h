#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QStringListModel>
#include <QPushButton>
#include <QInputDialog>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void add();
    void remove();

private:
    QStringList *list;
    QStringListModel *model;

    void init();


private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
