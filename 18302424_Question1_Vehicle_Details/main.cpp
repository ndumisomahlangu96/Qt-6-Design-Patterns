/*
 * PROGRAM NAME: QUESTION1_VEHICLE_DETAILS
 * AUTHOR: KUTLWANO NDUMISO MAHLANGU
 * STUDENT NUMBER: 18302424
 * DATE: 2026/05/06
 */

#include <QCoreApplication>
#include <QDebug>
#include "vehicledetails.h"
#include "passengervehicles.h"
#include "transportvehicles.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. Create a parent object to manage the list of vehicles automatically
    QObject vehicleManager;

    // 2. Test by creating fully initialized vehicles
    // The `&vehicleManager` is passed as the parent
    new PassengerVehicles("Toyota Camry", 2021, 5, &vehicleManager);
    new TransportVehicles("Ford Transit", 2018, 1500.5, &vehicleManager);

    // 3. Test by creating a vehicle using the default constructor
    new PassengerVehicles(&vehicleManager);

    // 4. Test by creating a default vehicle and then using setters
    TransportVehicles *customTransport = new TransportVehicles(&vehicleManager);
    customTransport->setModel("Mercedes Sprinter");
    customTransport->setYear(2023);
    customTransport->setCapacityKg(3000.0);

    qInfo() << "--- Fleet Vehicle Details ---";

    // 5. Outputting the values in the list to the console
    // vehicleManager.children() returns a QObjectList containing all instantiated vehicles
    for (QObject *obj : vehicleManager.children()) {

        // Downcast back to our base class to access the virtual getDetails() method
        VehicleDetails *vehicle = qobject_cast<VehicleDetails*>(obj);

        if (vehicle) {
            // .noquote() removes the surrounding double-quotes from the qDebug output
            qInfo().noquote() << vehicle->getDetails();
        }
    }

    return 0;
}
