#ifndef TRANSPORTVEHICLES_H
#define TRANSPORTVEHICLES_H

#include "vehicledetails.h"

class TransportVehicles : public VehicleDetails
{
    Q_OBJECT
    Q_PROPERTY(double capacityKg READ capacityKg WRITE setCapacityKg)
public:
    explicit TransportVehicles(QObject *parent = nullptr);
    TransportVehicles(const QString &model, int year, double capacityKg, QObject *parent = nullptr);

    double capacityKg() const;
    void setCapacityKg(double capacityKg);

    // Override the base class method
    QString getDetails() const override;

private:
    double m_capacityKg;
};

#endif // TRANSPORTVEHICLES_H