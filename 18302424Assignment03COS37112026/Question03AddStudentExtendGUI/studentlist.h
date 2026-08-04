#ifndef STUDENTLIST_H
#define STUDENTLIST_H

#include <QObject>
#include <QList>
#include "student.h"

class StudentList : public QObject
{
    Q_OBJECT
public:
    // Singleton access method
    static StudentList& getInstance();

    // Delete copy constructor and assignment operator to enforce Singleton
    StudentList(const StudentList&) = delete;
    StudentList& operator=(const StudentList&) = delete;

    // Required list functionalities
    void addStudent(Student* student);
    QList<Student*> getList() const;
    int findStudent(const QString& studentNumber) const;
    Student* getStudent(int index) const;
    int size() const;

private:
    // Private constructor and destructor
    explicit StudentList(QObject *parent = nullptr);
    ~StudentList() override;

    QList<Student*> m_students;
};

#endif // STUDENTLIST_H