#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // FIX: Initialize the model before using it
    model = new QStringListModel(this);

    QStringList list;
    list << "cats" << "dogs" << "birds";

    model->setStringList(list);

    ui->listView->setModel(model);
    ui->comboBox->setModel(model);

    ui->listView->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonAdd_clicked()
{
    // Add
    int row = model->rowCount();
    model->insertRows(row,1);

    QModelIndex index = model->index(row);

    ui->listView->setCurrentIndex(index);
    ui->listView->edit(index);


}


void Dialog::on_pushButtonInsert_clicked()
{
    // Insert
    int row = ui->listView->currentIndex().row();
    model->insertRows(row,1);

    QModelIndex index = model->index(row);

    ui->listView->setCurrentIndex(index);
    ui->listView->edit(index);

}


void Dialog::on_pushButtonDelete_clicked()
{
    // Delete
    model->removeRows(ui->listView->currentIndex().row(),1);


}

