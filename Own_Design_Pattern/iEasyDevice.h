#ifndef IEASYDEVICE_H
#define IEASYDEVICE_H

#include <QIODevice>
#include <QByteArray>

class iEasyDevice {
public:
    virtual void toDevice(QByteArray &data) = 0;
    virtual QByteArray fromDevice() = 0;
};

#endif // IEASYDEVICE_H
