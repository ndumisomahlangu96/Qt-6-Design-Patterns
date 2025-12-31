#ifndef TEMPCONVERTER_H
#define TEMPCONVERTER_H

#include <QObject>

class tempconverter : public QObject
{
    Q_OBJECT
public:
    explicit tempconverter(QObject *parent = nullptr);
    int celsiusToFahrenheit(int value);
    int fahrenheitToCelsius(int value);

signals:
};

#endif // TEMPCONVERTER_H
