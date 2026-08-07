#include "studentlist.h"
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QDebug>

StudentList::StudentList(QObject *parent)
    : QObject{parent}
{
    // Auto-load details into memory on instantiation
    loadFromXML();
}

StudentList::~StudentList()
{
    // Auto-save details when the application exits
    saveToXML();

    // Clean up dynamic memory
    qDeleteAll(m_students);
    m_students.clear();
}

StudentList& StudentList::getInstance()
{
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
    return -1;
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

// ------------------------------------------------------------------
// NEW LOGIC: Reading from XML (Deserialization)
// ------------------------------------------------------------------
void StudentList::loadFromXML()
{
    // Clear the existing list before loading to avoid duplicates if button is clicked multiple times
    qDeleteAll(m_students);
    m_students.clear();

    QFile file("students.xml");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot read XML file.";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "StudentList") {
        return;
    }

    QDomNodeList studentNodes = root.elementsByTagName("student");
    for (int i = 0; i < studentNodes.count(); ++i) {
        QDomNode studentNode = studentNodes.at(i);
        if (studentNode.isElement()) {
            QDomElement studentElem = studentNode.toElement();

            // Get student number
            QString stdNum = studentElem.elementsByTagName("number").at(0).toElement().text();
            Student* student = new Student(stdNum);

            // Get modules
            QDomNodeList moduleNodes = studentElem.elementsByTagName("module");
            for (int j = 0; j < moduleNodes.count(); ++j) {
                QDomElement modElem = moduleNodes.at(j).toElement();
                QString code = modElem.elementsByTagName("code").at(0).toElement().text();
                int mark = modElem.elementsByTagName("mark").at(0).toElement().text().toInt();

                student->addModule(code, mark);
            }
            m_students.append(student);
        }
    }
}

// ------------------------------------------------------------------
// NEW LOGIC: Writing to XML using DOM Approach (Serialization)
// ------------------------------------------------------------------
void StudentList::saveToXML() const
{
    QDomDocument doc;

    // Create the root element `<StudentList>`
    QDomElement root = doc.createElement("StudentList");
    doc.appendChild(root);

    // Iterating populates the DOM tree. If `m_students` is empty, this is skipped,
    // seamlessly producing a valid but empty <StudentList/> root element.
    for (Student* s : m_students) {
        QDomElement studentElem = doc.createElement("student");

        // Setup `<number>`
        QDomElement numElem = doc.createElement("number");
        numElem.appendChild(doc.createTextNode(s->getNumber()));
        studentElem.appendChild(numElem);

        // Setup `<modules>`
        QDomElement modulesElem = doc.createElement("modules");
        QHash<QString, int> modules = s->getModules();
        QHashIterator<QString, int> it(modules);

        while (it.hasNext()) {
            it.next();
            QDomElement moduleElem = doc.createElement("module");

            // Setup `<code>`
            QDomElement codeElem = doc.createElement("code");
            codeElem.appendChild(doc.createTextNode(it.key()));
            moduleElem.appendChild(codeElem);

            // Setup `<mark>`
            QDomElement markElem = doc.createElement("mark");
            markElem.appendChild(doc.createTextNode(QString::number(it.value())));
            moduleElem.appendChild(markElem);

            modulesElem.appendChild(moduleElem);
        }

        studentElem.appendChild(modulesElem);
        root.appendChild(studentElem);
    }

    // Write structure to file cleanly
    QFile file(xmlFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << doc.toString();
        file.close();
    }
}