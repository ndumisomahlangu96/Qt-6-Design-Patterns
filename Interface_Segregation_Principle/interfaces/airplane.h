#ifndef AIRPLANE_H
#define AIRPLANE_H

#include <QObject>
#include <QDebug>
#include "interfaces/iFlying.h"


class airplane : public QObject
{
    Q_OBJECT
public:
    explicit airplane(QObject *parent = nullptr);

signals:
};

#endif // AIRPLANE_H
