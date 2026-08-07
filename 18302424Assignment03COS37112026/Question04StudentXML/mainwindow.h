#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QWidget>
#include <QStringList>
#include <QLineEdit>
#include <QSpinBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "getstudentinformation.h"
#include "student.h"
#include "studentlist.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void displayOutput(const QString &output);

    // GUI button click handlers
    void displayStudentRecord();
    void displayAverage();
    void checkGraduation();

    // New slot for loading XML data
    void loadStudentRecordXML();

private:
    QPushButton *btnStartProcess;
    QTextEdit *textDisplay;
    GetStudentInformation *studentInfo;

    // GUI Input Fields
    QLineEdit *editStudentNumber;

    // GUI Action Buttons
    QPushButton *btnDisplayRecord;
    QPushButton *btnGetAverage;
    QPushButton *btnCheckGraduation;

    // New GUI Action Button for XML
    QPushButton *btnLoadStudentRecordXMl;
};
#endif // MAINWINDOW_H