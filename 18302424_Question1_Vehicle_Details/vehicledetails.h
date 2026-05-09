#ifndef VEHICLEDETAILS_H
#define VEHICLEDETAILS_H

#include <QObject>
#include <QString>      // The class provides a Unicode character string.
#include <QDate>        // The class provides date functions.

class VehicleDetails : public QObject
{
    Q_OBJECT
public:
    // Default constructor and parameterized constructor
    explicit VehicleDetails(QObject *parent = nullptr);
    VehicleDetails(const QString &model, int year, QObject *parent = nullptr);

    // Getters and Setters
    QString model() const;
    void setModel(const QString &model);

    int year() const;
    void setYear(int year);

    // Virtual function to output details polymorphically
    virtual QString getDetails() const;

private:
    QString m_model;
    int m_year;
};

#endif // VEHICLEDETAILS_H