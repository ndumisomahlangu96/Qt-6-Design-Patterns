#include "studentlist.h"

StudentList::StudentList(QObject *parent)
    : QObject{parent}
{}

StudentList::~StudentList()
{
    // Clean up dynamic memory
    qDeleteAll(m_students);
    m_students.clear();
}

StudentList& StudentList::getInstance()
{
    // Meyers' Singleton: instantiated on first use and guaranteed to be destroyed
    static StudentList instance;
    return instance;
}

void StudentList::addStudent(Student* student)
{
    if (student) {
        m_students.append(student);
    }
}

QList<Student*> StudentList::getList() const
{
    return m_students;
}

int StudentList::findStudent(const QString& studentNumber) const
{
    for (int i = 0; i < m_students.size(); ++i) {
        if (m_students.at(i)->getNumber() == studentNumber) {
            return i;
        }
    }
    return -1; // Return -1 if student does not exist
}

Student* StudentList::getStudent(int index) const
{
    if (index >= 0 && index < m_students.size()) {
        return m_students.at(index);
    }
    return nullptr;
}

int StudentList::size() const
{
    return m_students.size();
}