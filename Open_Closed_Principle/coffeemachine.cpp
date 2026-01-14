#include "coffeemachine.h"

CoffeeMachine::CoffeeMachine(QObject *parent)
    : QObject{parent}
{}


void CoffeeMachine::addIngredients(QStringList &list)
{
    foreach(QString item, list)
    {
        qInfo() << "Adding: " << item;
    }
}

void CoffeeMachine::brew()
{
    qInfo() << "Heating";
    qInfo() << "Pour";
    qInfo() << "Beep";
}
