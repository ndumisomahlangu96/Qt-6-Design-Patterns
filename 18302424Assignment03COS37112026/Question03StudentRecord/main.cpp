/*
 * STUDENT NUMBER: 18302424
 * AUTHOR: KUTLWANO NDUMISO MAHLANGU
 * PROGRAM: STUDENT ACADEMIC RECORD
 * DATE: 2026/08/04
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
