#ifndef SHOP_H
#define SHOP_H

#include <QObject>
#include <QDebug>
#include "Interface/iBrew.h"

class Shop : public QObject
{
    Q_OBJECT
public:
    explicit Shop(QObject *parent = nullptr);
    void sellDrink(QStringList list, iBrew *machine);

signals:
};

#endif // SHOP_H
