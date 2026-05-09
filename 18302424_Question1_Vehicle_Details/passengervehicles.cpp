#include "passengervehicles.h"

PassengerVehicles::PassengerVehicles(QObject *parent)
    : VehicleDetails(parent), m_passengers(4) // Default 4 passengers
{}

PassengerVehicles::PassengerVehicles(const QString &model, int year, int passengers, QObject *parent)
    : VehicleDetails(model, year, parent)
{
    setPassengers(passengers);
}

int PassengerVehicles::passengers() const
{
    return m_passengers;
}

void PassengerVehicles::setPassengers(int passengers)
{
    // Ensure at least 1 passenger (the driver)
    m_passengers = (passengers > 0) ? passengers : 1;
}

QString PassengerVehicles::getDetails() const
{
    // Call the base class output and append specific details to avoid redundant code
    return VehicleDetails::getDetails() + QString(", Passengers: %1").arg(m_passengers);
}