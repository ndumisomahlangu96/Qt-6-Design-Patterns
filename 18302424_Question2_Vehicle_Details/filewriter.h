#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QObject>
#include <QString>      // The class provides a Unicode character string.

class FileWriter : public QObject
{
    Q_OBJECT
public:
    // Matches the UML signature
    explicit FileWriter(QObjectList *olist, QString fname, QObject *parent = nullptr);

    // Writes reflective properties to file and returns number of items written
    int writer();

private:
    QObjectList *m_list;
    QString m_filename;
};

#endif // FILEWRITER_H