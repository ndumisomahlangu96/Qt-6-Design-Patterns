#ifndef CAFFEINATOR3____H
#define CAFFEINATOR3____H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include "Interface/iBrew.h"

class Caffeinator3000 : public QObject , public iBrew
{
    Q_OBJECT
public:
    explicit Caffeinator3000(QObject *parent = nullptr);

signals:

    // iBrew interface
public:
    void addIngredients(QStringList &list) override;
    void brew() override;
    void marketing();
};

#endif // CAFFEINATOR3____H
