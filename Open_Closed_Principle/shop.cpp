#include "shop.h"

Shop::Shop(QObject *parent)
    : QObject{parent}
{}

void Shop::sellDrink(QStringList list, iBrew *machine)
{
    qInfo() << "Take the order?";
    machine->addIngredients(list);
    machine->brew();
    qInfo() << "Take payment.";
    qInfo() << "Give the customer their order.";
}
