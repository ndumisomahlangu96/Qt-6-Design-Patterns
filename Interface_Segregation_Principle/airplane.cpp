#include "airplane.h"

airplane::airplane(QObject *parent)
    : QObject{parent}
{}

void airplane::refuel()
{
    qInfo() << this << "Refueling";
}

void airplane::fly()
{
    qInfo() << this << "Flying";
}

void airplane::land()
{
    qInfo() << this << "Land";
}
