#include "caffeinator3000.h"

Caffeinator3000::Caffeinator3000(QObject *parent)
    : QObject{parent}
{}


void Caffeinator3000::addIngredients(QStringList &list)
{
    foreach (QString item,list)
    {
        qInfo() << "Adding: " << item;
    }
}

void Caffeinator3000::brew()
{
    qInfo() << "Heating";
    marketing();
    qInfo() << "Pour";
    qInfo() << "Complete";
}

void Caffeinator3000::marketing()
{
    for (int i = 0; i < 10; i++)
    {
        qInfo() << "Fire!";
        qInfo() << "Loud MUSIC!!!";
        qInfo() << "super-inject-caffinee";
    }
}
