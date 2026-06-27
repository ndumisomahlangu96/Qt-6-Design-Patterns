#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

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
    void on_pushButton_clicked();

private:
    Ui::Dialog *ui;
    QStandardItemModel *model;
    QString Filename;
    void ReadFile();
    void WriteFile();
};
#endif // DIALOG_H
