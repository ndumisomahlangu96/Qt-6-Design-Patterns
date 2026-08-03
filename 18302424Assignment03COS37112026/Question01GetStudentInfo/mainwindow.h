#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QMenuBar>
#include <QStatusBar>
#include <QIcon>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // UI Components mapped from mainwindow.ui
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;

    QLabel *labelStudentNumber;
    QLineEdit *lineEditStudentNumber;

    QLabel *labelModuleCode;
    QLineEdit *lineEditModuleCode;

    QLabel *labelMark;
    QSpinBox *spinBoxMark;

    QPushButton *pushButtonAddStudent;

    QMenuBar *menubar;
    QStatusBar *statusbar;
};
#endif // MAINWINDOW_H
