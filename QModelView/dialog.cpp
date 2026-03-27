#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QStringList list;
    list << "cats" << "dogs" << "birds";

    model->setStringList(list);

    ui->listView->setModel(model);
    ui->comboBox->setModel(model);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonAdd_clicked()
{
    // Add


}


void Dialog::on_pushButtonInsert_clicked()
{
    // Insert


}


void Dialog::on_pushButtonDelete_clicked()
{
    // Delete


}

