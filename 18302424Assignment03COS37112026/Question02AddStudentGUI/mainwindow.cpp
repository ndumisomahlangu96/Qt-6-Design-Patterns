#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Setup UI components
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    btnStartProcess = new QPushButton("Run Question01GetStudentInfo", this);
    textDisplay = new QTextEdit(this);
    textDisplay->setReadOnly(true);

    layout->addWidget(btnStartProcess);
    layout->addWidget(textDisplay);
    setCentralWidget(centralWidget);

    // Initialize logic class
    studentInfo = new GetStudentInformation(this);

    // Connect Button to Process Start
    connect(btnStartProcess, &QPushButton::clicked, studentInfo, &GetStudentInformation::startSeparateProgram);

    // Connect Process Output to GUI Display
    connect(studentInfo, &GetStudentInformation::outputGUI, this, &MainWindow::displayOutput);

    setMinimumSize(400, 300);
}

MainWindow::~MainWindow() = default;

void MainWindow::displayOutput(const QString &output)
{
    // Append the captured output to the text edit area
    textDisplay->append(output);
}