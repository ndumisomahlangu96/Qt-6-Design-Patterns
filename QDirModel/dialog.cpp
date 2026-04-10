#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    model = new QFileSystemModel(this);
    model->setReadOnly(false);

    // 1. Get the user's home directory path
    QString userPath = QDir::homePath();

    // 2. Tell the model to start reading the file system at this path
    model->setRootPath(userPath);
    ui->treeView->setModel(model);

    // 3. Tell the view to focus only on this specific directory
    ui->treeView->setRootIndex(model->index(userPath));
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

