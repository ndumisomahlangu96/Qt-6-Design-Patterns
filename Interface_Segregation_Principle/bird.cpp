#include "bird.h"

bird::bird(QObject *parent)
    : QObject{parent}
{}

void bird::fly()
{
    qInfo() << this << "Flying";
}

void bird::land()
{
    qInfo() << this << "Land";
}
