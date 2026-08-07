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
    // Inserts or updates the module mark
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

    // Regex to validate the module format:
    // ^[A-Z]{3}     : Exactly 3 uppercase alphabetic characters
    // [123]         : A 1, 2, or 3 indicating the year
    // \\d{2}        : Exactly 2 digits
    // [A-Za-z0-9]$  : A final alphabetic or numeric character
    QRegularExpression moduleValidator("^[A-Z]{3}[123]\\d{2}[A-Za-z0-9]$");

    QHashIterator<QString, int> i(modulesAndMarks);
    while (i.hasNext()) {
        i.next();

        // Check if the mark is a pass (>= 50) and if the module code matches the specific pattern
        if (i.value() >= 50 && moduleValidator.match(i.key()).hasMatch()) {
            passedCount++;

            // The 4th character denotes the year
            QChar yearChar = i.key().at(3);

            if (yearChar == '1') {
                firstYearCount++;
            } else if (yearChar == '3') {
                thirdYearCount++;
            }
        }
    }

    // Must pass at least 5 modules in total, with at most 2 first-year modules, and at least 1 third-year module
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