#ifndef PASSENGERVEHICLES_H
#define PASSENGERVEHICLES_H

#include "vehicledetails.h"

class PassengerVehicles : public VehicleDetails
{
    Q_OBJECT
public:
    explicit PassengerVehicles(QObject *parent = nullptr);
    PassengerVehicles(const QString &model, int year, int passengers, QObject *parent = nullptr);

    int passengers() const;
    void setPassengers(int passengers);

    // Override the base class method
    QString getDetails() const override;

private:
    int m_passengers;
};

#endif // PASSENGERVEHICLES_H