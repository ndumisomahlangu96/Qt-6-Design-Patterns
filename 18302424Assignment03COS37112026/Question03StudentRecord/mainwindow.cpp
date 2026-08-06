#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Setup input form for Student Information
    QFormLayout *formLayout = new QFormLayout();
    editStudentNumber = new QLineEdit(this);
    // Add Input Mask and Regular Expression Validator for Student Number
    // Enforces 4 required digits.
    QRegularExpression rexpStudent("^[0-9]{4}");
    QRegularExpressionValidator *rvalStudent = new QRegularExpressionValidator(rexpStudent,this);
    editStudentNumber->setValidator(rvalStudent);
    editStudentNumber->setPlaceholderText("Please enter your 4-digit student number (e.g., 1234)");
    formLayout->addRow("🆔 Student Number:", editStudentNumber);

    // Setup horizontal layout for action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    btnDisplayRecord = new QPushButton("📋🔍 Display Record", this);
    btnGetAverage = new QPushButton("📊🔍 Get Average", this);
    btnCheckGraduation = new QPushButton("🎓✅ Check Graduation", this);

    buttonLayout->addWidget(btnDisplayRecord);
    buttonLayout->addWidget(btnGetAverage);
    buttonLayout->addWidget(btnCheckGraduation);

    // Existing components
    btnStartProcess = new QPushButton("▶️Run Question01GetStudentInfo.exe To Add Student Data", this);
    textDisplay = new QTextEdit(this);
    textDisplay->setReadOnly(true);

    // Assemble the main vertical layout
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(btnStartProcess);
    mainLayout->addWidget(textDisplay);

    setCentralWidget(centralWidget);

    // Initialize logic class
    studentInfo = new GetStudentInformation(this);

    // Connect standard process buttons
    connect(btnStartProcess, &QPushButton::clicked, studentInfo, &GetStudentInformation::startSeparateProgram);
    connect(studentInfo, &GetStudentInformation::outputGUI, this, &MainWindow::displayOutput);

    // Connect new GUI operation buttons
    connect(btnDisplayRecord, &QPushButton::clicked, this, &MainWindow::displayStudentRecord);
    connect(btnGetAverage, &QPushButton::clicked, this, &MainWindow::displayAverage);
    connect(btnCheckGraduation, &QPushButton::clicked, this, &MainWindow::checkGraduation);

    this->setWindowTitle("Student Academic Record System");
    this->setMinimumSize(550, 450);
}

MainWindow::~MainWindow() = default;

void MainWindow::displayOutput(const QString &output)
{
    // Clean any trailing newline characters captured from QProcess
    QString cleanOutput = output.trimmed();

    // Assuming Question01 outputs data in a CSV format: "StudentNum,ModuleCode,Mark"
    QStringList studentData = cleanOutput.split(",");

    if (studentData.size() >= 3) {
        QString stdNum = studentData[0].trimmed();
        QString modCode = studentData[1].trimmed();
        int mark = studentData[2].trimmed().toInt();

        // 1. Search for the student in the singleton list
        int index = StudentList::getInstance().findStudent(stdNum);
        Student* currentStudent = nullptr;

        // 2. Fetch the student if they exist, or create and append if they don't
        if (index != -1) {
            currentStudent = StudentList::getInstance().getStudent(index);
        } else {
            currentStudent = new Student(stdNum);
            StudentList::getInstance().addStudent(currentStudent);
        }

        // 3. Save the module and integer mark
        currentStudent->addModule(modCode, mark);

        // Standard format to separate different Student Information
        QString formattedRecord = QString(
                                      "========================================\n"
                                      " STUDENT INFORMATION RECORD\n"
                                      "========================================\n"
                                      " 🆔 Student Number : %1\n"
                                      " 📚 Module Code    : %2\n"
                                      " 🎓 Mark           : %3\n"
                                      "========================================\n"
                                      ).arg(stdNum, modCode, QString::number(mark));

        textDisplay->append(formattedRecord);
    } else {
        textDisplay->append(cleanOutput);
    }
}

// Logic to lookup and display a full student record
void MainWindow::displayStudentRecord()
{
    QString stdNum = editStudentNumber->text().trimmed();
    int index = StudentList::getInstance().findStudent(stdNum);

    if (index == -1) {
        textDisplay->append("❌ Error: Cannot display record. Student '" + stdNum + "' does not exist.\n");
        return;
    }

    Student* student = StudentList::getInstance().getStudent(index);
    QString record = QString("========================================\n"
                             " 🎓 RECORD FOR STUDENT: %1\n"
                             "========================================\n").arg(stdNum);

    QHash<QString, int> modules = student->getModules();
    if (modules.isEmpty()) {
        record += " No modules completed yet.\n";
    } else {
        QHashIterator<QString, int> i(modules);
        while (i.hasNext()) {
            i.next();
            record += QString(" 📚 Module: %1 | Mark: %2\n").arg(i.key(), QString::number(i.value()));
        }
    }
    record += "========================================\n";
    textDisplay->append(record);
}

// Logic to fetch the average mark
void MainWindow::displayAverage()
{
    QString stdNum = editStudentNumber->text().trimmed();
    int index = StudentList::getInstance().findStudent(stdNum);

    if (index == -1) {
        textDisplay->append("❌ Error: Cannot calculate average. Student '" + stdNum + "' does not exist.\n");
        return;
    }

    Student* student = StudentList::getInstance().getStudent(index);
    double avg = student->average();
    textDisplay->append(QString("📊 Average for student number %1 is: %2\n").arg(stdNum, QString::number(avg, 'f', 2)));
}

// Logic to check Graduation eligibility against defined rules
void MainWindow::checkGraduation()
{
    QString stdNum = editStudentNumber->text().trimmed();
    int index = StudentList::getInstance().findStudent(stdNum);

    if (index == -1) {
        textDisplay->append("❌ Error: Cannot check graduation. Student '" + stdNum + "' does not exist.\n");
        return;
    }

    Student* student = StudentList::getInstance().getStudent(index);

    // Checks if passed 5 modules (max 2 first-year, min 1 third-year)
    bool qualifies = student->graduate();

    if (qualifies) {
        textDisplay->append(QString("🎉 Student %1 QUALIFIES for graduation!\n").arg(stdNum));
    } else {
        textDisplay->append(QString("⚠️ Student %1 DOES NOT qualify for graduation.\n").arg(stdNum));
    }
}