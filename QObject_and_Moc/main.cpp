#include <QCoreApplication>
#include "test.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Test test;

    // Connect the signal and slots
    QObject::connect(&test,&Test::close,&app,&QCoreApplication::quit,Qt::QueuedConnection);
    test.dostuff();


    int value = app.exec();

    qInfo() << "Exit value:" << value;

    return value;
}
