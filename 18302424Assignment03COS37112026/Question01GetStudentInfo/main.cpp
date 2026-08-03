/*
 * STUDENT NUMBER: 18302424
 * AUTHOR: KUTLWANO NDUMISO MAHLANGU
 * PROGRAM: STUDENT INFORMATION
 * DATE: 2026/08/01
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
