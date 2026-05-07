#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_buttonBox_accepted()
{
    accept();
}


void Dialog::on_buttonBox_rejected()
{
    reject();
}

void Dialog::add()
{

}

void Dialog::remove()
{

}

void Dialog::init()
{

}

