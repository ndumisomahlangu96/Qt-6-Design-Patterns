#ifndef BIRD_H
#define BIRD_H

#include <QObject>

class bird : public QObject
{
    Q_OBJECT
public:
    explicit bird(QObject *parent = nullptr);

signals:
};

#endif // BIRD_H
