#ifndef MYEASYDEVICE_H
#define MYEASYDEVICE_H

#include <QObject>
#include <QBuffer>
#include "iEasyDevice.h"

class MyEasyDevice : public QObject, public iEasyDevice
{
    Q_OBJECT
public:
    explicit MyEasyDevice(QObject *parent = nullptr);

signals:

    // iEasyDevice interface
public:
    void toDevice(QByteArray &data) override;
    QByteArray fromDevice() override;

private:
    QBuffer m_buffer;
};

#endif // MYEASYDEVICE_H
