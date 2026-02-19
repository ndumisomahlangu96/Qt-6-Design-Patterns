#ifndef BIRD_H
#define BIRD_H

#include <QObject>
#include <QDebug>
#include "interfaces/iFlying.h"


class bird : public QObject, public iFlying
{
    Q_OBJECT
public:
    explicit bird(QObject *parent = nullptr);

    // iFlying interface
    void fly() override;
    void land() override;

signals:
};

#endif // BIRD_H
