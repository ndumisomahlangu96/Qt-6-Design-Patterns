#include "student.h"

Student::Student(QObject *parent) : QObject{parent} {}

Student::Student(const QString& stdNum) : studentNumber(stdNum) {}

void Student::setNumber(const QString &stdNum)
{
    studentNumber = stdNum;
}

QString Student::getNumber() const
{
    return studentNumber;
}

void Student::addModule(const QString &moduleCode, int mark)
{
    // Inserts or updates the module mark[cite: 7]
    modulesAndMarks.insert(moduleCode, mark);
}

QHash<QString, int> Student::getModules() const
{
    return modulesAndMarks;
}

double Student::average() const
{
    if (modulesAndMarks.isEmpty()) return 0.0;

    double sum = 0.0;
    for (int mark : modulesAndMarks.values()) {
        sum += mark;
    }
    return sum / modulesAndMarks.size();
}

bool Student::graduate() const
{
    int passedCount = 0;
    int firstYearCount = 0;
    int thirdYearCount = 0;

    QHashIterator<QString, int> i(modulesAndMarks);
    while (i.hasNext()) {
        i.next();

        // Assuming >= 50 is a pass
        if (i.value() >= 50) {
            passedCount++;
            QString code = i.key();

            // Validate module code length based on rules:
            // 3 Upper Alpha + 1 Year Digit + 2 Digits + 1 Char = 7 characters total
            if (code.length() == 7) {
                QChar yearChar = code.at(3); // The 4th character denotes the year
                if (yearChar == '1') {
                    firstYearCount++;
                } else if (yearChar == '3') {
                    thirdYearCount++;
                }
            }
        }
    }

    // Must pass at least 5 modules, <= 2 first-year modules, and >= 1 third-year module
    return (passedCount >= 5 && firstYearCount <= 2 && thirdYearCount >= 1);
}

void Student::printMarks() const
{
    qDebug() << "Marks for Student:" << studentNumber;
    QHashIterator<QString, int> i(modulesAndMarks);
    while (i.hasNext()) {
        i.next();
        qDebug() << "  Module:" << i.key() << "| Mark:" << i.value();
    }
}