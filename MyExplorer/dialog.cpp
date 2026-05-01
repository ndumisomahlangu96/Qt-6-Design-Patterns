#include "dialog.h"
#include "ui_dialog.h"

/*
 * To dynamically generate the root home path for the current user across different operating systems
 *  (Windows, macOS, Linux) in the Qt framework, you can use the QDir::homePath() function.
 */

#include <QDir> // Required for QDir::homePath()
#include <QHeaderView> // 1. Add this include for QHeaderView

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // Add this line to enable the maximize and minimize buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

    // QDir::homePath() automatically fetches the correct home directory
    // for Windows, macOS, and Linux
    QString sPath = QDir::homePath();

    // Setup the directory model.
    dirmodel = new QFileSystemModel(this);
    dirmodel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    // Optional: You will likely want to set the root path for your model next
    dirmodel->setRootPath(sPath);
    ui->treeView->setModel(dirmodel);

    // 2. Add this line to automatically resize all columns to fit their content
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Setup the file model.
    filemodel = new QFileSystemModel(this);
    filemodel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    filemodel->setRootPath(sPath);
    ui->listView->setModel(filemodel);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_treeView_clicked(const QModelIndex &index)
{
    QString sPath = dirmodel->fileInfo(index).absoluteFilePath();
    ui->listView->setRootIndex(filemodel->setRootPath(sPath));
}

