#ifndef STUDENT_H
#define STUDENT_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QDebug>

class Student : public QObject
{
    Q_OBJECT
public:
    explicit Student(QObject *parent = nullptr);
    Student(const QString& stdNum);

    // Required Getters and Setters
    void setNumber(const QString& stdNum);
    QString getNumber() const;

    // Required Module management methods
    void addModule(const QString& moduleCode, int mark);
    QHash<QString, int> getModules() const;

    // Academic calculations
    double average() const;
    bool graduate() const;

    void printMarks() const;

private:
    QString studentNumber;
    QHash<QString, int> modulesAndMarks;
};

#endif // STUDENT_H