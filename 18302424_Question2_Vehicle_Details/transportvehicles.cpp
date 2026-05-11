#include "transportvehicles.h"

TransportVehicles::TransportVehicles(QObject *parent)
    : VehicleDetails(parent), m_capacityKg(1000.0) // Default 1000 kg
{}

TransportVehicles::TransportVehicles(const QString &model, int year, double capacityKg, QObject *parent)
    : VehicleDetails(model, year, parent)
{
    setCapacityKg(capacityKg);
}

double TransportVehicles::capacityKg() const { return m_capacityKg; }

void TransportVehicles::setCapacityKg(double capacityKg) {
    // Ensure capacity cannot be negative
    m_capacityKg = (capacityKg >= 0.0) ? capacityKg : 0.0;
}

QString TransportVehicles::getDetails() const {
    // Call the base class output and append specific details
    return VehicleDetails::getDetails() + QString(", Capacity: %1 kg").arg(m_capacityKg);
}