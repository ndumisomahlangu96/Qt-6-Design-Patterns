#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setup UI components
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    btnStartProcess = new QPushButton("▶️Run Question01GetStudentInfo", this);
    textDisplay = new QTextEdit(this);
    textDisplay->setReadOnly(true); // Ensures the display remains read-only

    layout->addWidget(btnStartProcess);
    layout->addWidget(textDisplay);
    setCentralWidget(centralWidget);

    // Initialize logic class
    studentInfo = new GetStudentInformation(this);

    // Connect Button to Process Start
    connect(btnStartProcess, &QPushButton::clicked, studentInfo, &GetStudentInformation::startSeparateProgram);

    // Connect Process Output to GUI Display
    connect(studentInfo, &GetStudentInformation::outputGUI, this, &MainWindow::displayOutput);

    this->setWindowTitle("Student Information");
    this->setMinimumSize(400, 300);
}

MainWindow::~MainWindow() = default;

void MainWindow::displayOutput(const QString &output)
{
    // Clean any trailing newline characters captured from QProcess
    QString cleanOutput = output.trimmed();

    // Assuming Question01 outputs data in a CSV format: "StudentNum,ModuleCode,Mark"
    QStringList studentData = cleanOutput.split(",");

    if (studentData.size() >= 3) {
        // Standard format to separate different Student Information
        QString formattedRecord = QString(
                                      "========================================\n"
                                      " STUDENT INFORMATION RECORD\n"
                                      "========================================\n"
                                      " 🆔 Student Number : %1\n"
                                      " 📚 Module Code    : %2\n"
                                      " 🎓 Mark           : %3\n"
                                      "========================================\n"
                                      ).arg(studentData[0].trimmed(), studentData[1].trimmed(), studentData[2].trimmed());

        // Append the cleanly formatted string to the text edit area
        textDisplay->append(formattedRecord);
    } else {
        // Fallback in case Question01 outputs standard informational text or errors
        textDisplay->append(cleanOutput);
    }
}

