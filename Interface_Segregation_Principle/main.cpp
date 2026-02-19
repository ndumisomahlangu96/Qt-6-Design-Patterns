/*
 * What
 * Interface Segregation Principle
 *
 * Define
 * Clients should not be forced to depend upon interfaces that they do not use
 *
 * Why
 * Adding too much to an interface could cause un-needed code, and undesirable effects.
 *
 * Example
 * Flying things
 */

#include <QCoreApplication>
#include "interfaces/iFlying.h"
#include "interfaces/iMachine.h"
#include "bird.h"
#include "airplane.h"


void trip (iFlying *obj)
{
    obj->fly();
    obj->land();
}

void checkFuel(iMachine *obj)
{
    obj->refuel();
}

void testBird()
{
    bird Bird;
    trip(&Bird);
}

void testPlane()
{
    airplane Plane;
    trip(&Plane);
    checkFuel(&Plane);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    testBird();
    testPlane();

    return a.exec();
}
