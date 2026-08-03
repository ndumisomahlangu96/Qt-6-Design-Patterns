#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include "getstudentinformation.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void displayOutput(const QString &output);

private:
    QPushButton *btnStartProcess;
    QTextEdit *textDisplay;
    GetStudentInformation *studentInfo;
};
#endif // MAINWINDOW_H