#include "myeasydevice.h"

MyEasyDevice::MyEasyDevice(QObject *parent)
    : QObject{parent}
{}


void MyEasyDevice::toDevice(QByteArray &data)
{
    if (!m_buffer.isOpen())
    {
        m_buffer.open(QIODevice::ReadWrite);
    }
    m_buffer.write(data);
}

QByteArray MyEasyDevice::fromDevice()
{
    int pos = m_buffer.pos();
    m_buffer.seek(0);
    QByteArray data = m_buffer.readAll();
    return data;
}

