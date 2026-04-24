#include "dialog.h"
#include "ui_dialog.h"

/*
 * To dynamically generate the root home path for the current user across different operating systems
 *  (Windows, macOS, Linux) in the Qt framework, you can use the QDir::homePath() function.
 */

#include <QDir> // Required for QDir::homePath()

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // QDir::homePath() automatically fetches the correct home directory
    // for Windows, macOS, and Linux
    QString sPath = QDir::homePath();

    dirmodel = new QFileSystemModel(this);

    // Optional: You will likely want to set the root path for your model next
    dirmodel->setRootPath(sPath);
    ui->treeView->setModel(dirmodel);

}

Dialog::~Dialog()
{
    delete ui;
}
