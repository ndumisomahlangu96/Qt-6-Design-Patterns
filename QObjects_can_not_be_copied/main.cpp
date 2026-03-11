#include <QCoreApplication>
#include "cat.h"
#include <QDebug>

void test(Cat &cat)
{
    qInfo() << "Addr" << &cat;
}

void test2(Cat *cat)
{
    qInfo() << "Ptr" << cat;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Cat kitty;
    kitty.setObjectName("kitty");

    Cat death;
    death.setObjectName("The grim reaper");

    test(kitty);
    test2(&kitty);

    test(death);
    test2(&death);

    return app.exec();
}
