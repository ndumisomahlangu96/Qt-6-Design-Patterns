/*
 * PROGRAM NAME: QUESTION2_VEHICLE_DETAILS
 * AUTHOR: KUTLWANO NDUMISO MAHLANGU
 * STUDENT NUMBER: 18302424
 * DATE: 2026/05/11
 */

#include <QCoreApplication>
#include <QDebug>
#include "vehicledetails.h"
#include "passengervehicles.h"
#include "transportvehicles.h"
#include "filewriter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. Create a parent object to manage the list of vehicles automatically
    QObject vehicleManager;

    // 2. Test by creating fully initialized vehicles
    new PassengerVehicles("Toyota Camry", 2021, 5, &vehicleManager);
    new TransportVehicles("Ford Transit", 2018, 1500.5, &vehicleManager);

    // 3. Test by creating a vehicle using the default constructor
    new PassengerVehicles(&vehicleManager);

    // 4. Test by creating a default vehicle and then using setters
    TransportVehicles *customTransport = new TransportVehicles(&vehicleManager);
    customTransport->setModel("Mercedes Sprinter");
    customTransport->setYear(2023);
    customTransport->setCapacityKg(3000.0);

    qInfo() << "--- Fleet Vehicle Details (Console Output) ---";

    // 5. Outputting the values in the list to the console
    for (QObject *obj : vehicleManager.children()) {
        VehicleDetails *vehicle = qobject_cast<VehicleDetails*>(obj);
        if (vehicle) {
            qInfo().noquote() << vehicle->getDetails();
        }
    }

    // --- NEW: Using FileWriter with Reflection ---
    qInfo() << "\n--- Writing to File using Reflection ---";

    // Extract the children list from the parent object manager
    QObjectList vehicleList = vehicleManager.children();

    // Instantiate FileWriter per UML specifications
    FileWriter writer(&vehicleList, "filename.txt");

    // Call writer() and capture the records written
    int recordsWritten = writer.writer();

    qInfo() << "Successfully wrote" << recordsWritten << "records to filename.txt.";

    return 0;
}