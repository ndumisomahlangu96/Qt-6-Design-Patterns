#include <QCoreApplication>
#include <QDebug>
#include "tempconverter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    tempconverter t;

    int value = 32;
    qInfo() << "c to f" << t.celsiusToFahrenheit(value);
    qInfo() << "f to c" << t.fahrenheitToCelsius(value);

    return a.exec();
}
