#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. Window settings
    this->resize(430, 203);
    this->setWindowTitle("Student Information");

    // 2. Central widget
    centralwidget = new QWidget(this);
    this->setCentralWidget(centralwidget);

    // 3. Main layout (gridLayout_2)
    gridLayout_2 = new QGridLayout(centralwidget);

    // 4. Inner layout for form inputs (gridLayout)
    gridLayout = new QGridLayout();

    // --- Row 0: Student Number ---
    labelStudentNumber = new QLabel("🆔 Student Number:", centralwidget);
    lineEditStudentNumber = new QLineEdit(centralwidget);
    lineEditStudentNumber->setPlaceholderText("Please enter your 4-digit student number (e.g., 1234)");

    // Add Input Mask and Regular Expression Validator for Student Number
    // Enforces 4 required digits
    QRegularExpression rxStudent("^[0-9]{4}$");
    QRegularExpressionValidator *valStudent = new QRegularExpressionValidator(rxStudent, this);
    lineEditStudentNumber->setValidator(valStudent);

    gridLayout->addWidget(labelStudentNumber, 0, 0);
    gridLayout->addWidget(lineEditStudentNumber, 0, 1);

    // --- Row 1: Module Code ---
    labelModuleCode = new QLabel("📚 Module Code:", centralwidget);
    lineEditModuleCode = new QLineEdit(centralwidget);
    // Updated placeholder to reflect the 7-character rule instead of 6
    lineEditModuleCode->setPlaceholderText("Enter a 7-character module code (e.g., COS3711).");

    // Add Input Mask and Regular Expression Validator for Module Code
    // Mask rule: 3 uppercase letters, a 1-3 digit, 2 digits, and 1 final alphanumeric character.
    QRegularExpression rxModule("^[A-Z]{3}[1-3][0-9]{2}[A-Za-z0-9]$");
    QRegularExpressionValidator *valModule = new QRegularExpressionValidator(rxModule, this);
    lineEditModuleCode->setValidator(valModule);

    gridLayout->addWidget(labelModuleCode, 1, 0);
    gridLayout->addWidget(lineEditModuleCode, 1, 1);

    // --- Row 2: Mark ---
    labelMark = new QLabel("🎓 Mark:", centralwidget);
    spinBoxMark = new QSpinBox(centralwidget);
    spinBoxMark->setMaximum(100);

    gridLayout->addWidget(labelMark, 2, 0);
    gridLayout->addWidget(spinBoxMark, 2, 1);

    // 5. Add inner layout to main layout
    gridLayout_2->addLayout(gridLayout, 0, 0);

    // 6. Add Student button
    pushButtonAddStudent = new QPushButton("✅Add Student", centralwidget);
    pushButtonAddStudent->setAutoDefault(true);

    gridLayout_2->addWidget(pushButtonAddStudent, 1, 0);

    // 7. Menubar and Statusbar
    menubar = new QMenuBar(this);
    this->setMenuBar(menubar);

    statusbar = new QStatusBar(this);
    this->setStatusBar(statusbar);

    connect(pushButtonAddStudent,&QPushButton::clicked,this,&MainWindow::AddStudentClick);
}

MainWindow::~MainWindow() = default;

void MainWindow::AddStudentClick()
{
    // Extract the text and integer from the specific GUI components
    QString studentNum = lineEditStudentNumber->text();
    QString moduleCode = lineEditModuleCode->text();
    int mark = spinBoxMark->value();

    // Bind a QTextStream directly to the standard output (stdout)
    QTextStream out(stdout);

    // Pass the comma-separated string and use Qt::endl
    // Qt::endl automatically flushes the buffer, pushing data to QProcess!
    out << studentNum << "," << moduleCode << "," << mark << Qt::endl;
}
