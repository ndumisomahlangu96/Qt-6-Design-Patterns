#include "filewriter.h"
#include <QFile>
#include <QTextStream>      // The class provides a convenient interface for reading and writing text.
#include <QMetaObject>      // The class contains meta-information about Qt objects.
#include <QMetaProperty>    // The class provides meta-data about a property.
#include <QDebug>           // The class provides an output stream for debugging information.

FileWriter::FileWriter(QObjectList *olist, QString fname, QObject *parent)
    : QObject{parent}, m_list(olist), m_filename(fname)
{
}

int FileWriter::writer()
{
    // Safety check if the list is null
    if (!m_list) return 0;

    QFile file(m_filename);

    // Open the file in write mode, overriding existing content
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for writing:" << m_filename;
        return 0;
    }

    QTextStream out(&file);
    int count = 0;

    // Iterate through the QObject pointers in the list
    for (QObject *obj : *m_list) {
        if (!obj) continue;

        const QMetaObject *metaObj = obj->metaObject();

        // Dynamically get the class name (Type of vehicle)
        out << "Vehicle Type: " << metaObj->className() << "\n";

        // Iterate through all registered properties
        // We start at QObject::staticMetaObject.propertyCount() to skip
        // base QObject properties (like "objectName") and only get our custom ones.
        int propertyOffset = QObject::staticMetaObject.propertyCount();
        for (int i = propertyOffset; i < metaObj->propertyCount(); ++i) {
            QMetaProperty prop = metaObj->property(i);

            // Extract the property name and value dynamically
            QString propName = prop.name();
            QVariant propValue = prop.read(obj);

            out << "  - " << propName << ": " << propValue.toString() << "\n";
        }
        out << "----------------------------------------\n";
        count++;
    }

    file.close();
    return count;
}