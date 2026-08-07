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

    // Ensure this is public so MainWindow can execute it
    void loadFromXML();

private:
    // Private constructor and destructor
    explicit StudentList(QObject *parent = nullptr);
    ~StudentList() override;

    // New helper functions to handle XML Serialization
    void saveToXML() const;

    QList<Student*> m_students;
    const QString xmlFilePath = "students.xml"; // Target XML file
};

#endif // STUDENTLIST_H