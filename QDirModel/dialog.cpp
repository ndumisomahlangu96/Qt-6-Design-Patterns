#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    model = new QFileSystemModel(this);
    model->setReadOnly(false);

    ui->treeView->setModel(model);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonMakeDirectory_clicked()
{
    // Make Directory

}


void Dialog::on_pushButtonDelete_clicked()
{
    // Delete

}

