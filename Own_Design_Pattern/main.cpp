/*
 * What
 * Make your own pattern
 *
 * Description
 * Making a pattern is easy-make your own!
 *
 * Why
 * To show a pattern is really just a simpler way ot doing things.
 *
 * Example
 * EasyDevice Pattern
 * (I totally made that up)
 */

#include <QDebug>
#include <QCoreApplication>
#include "myeasydevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyEasyDevice device;

    for (int i = 0; i < 10; i++)
    {
        QString num = QString::number(i);
        QByteArray data = num.toUtf8();
        device.toDevice(data);
    }

    qInfo() << device.fromDevice();
    return a.exec();
}
