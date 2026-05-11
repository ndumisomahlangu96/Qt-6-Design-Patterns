#include "vehicledetails.h"

VehicleDetails::VehicleDetails(QObject *parent)
    : QObject{parent}, m_model("Unknown Model"), m_year(2000) // Default values
{}

VehicleDetails::VehicleDetails(const QString &model, int year, QObject *parent)
    : QObject{parent}, m_model(model)
{
    setYear(year);
}

QString VehicleDetails::model() const
{
    return m_model;
}

void VehicleDetails::setModel(const QString &model)
{
    m_model = model;
}

int VehicleDetails::year() const
{
    return m_year;
}

void VehicleDetails::setYear(int year) {
    // Validating for a reasonable year (1886 was the first car, up to next year's models)
    int currentYear = QDate::currentDate().year();
    if (year >= 1886 && year <= currentYear + 1) {
        m_year = year;
    } else {
        m_year = 2000; // Fallback to a default reasonable value
    }
}

QString VehicleDetails::getDetails() const
{
    return QString("Model: %1, Year: %2").arg(m_model).arg(m_year);
}