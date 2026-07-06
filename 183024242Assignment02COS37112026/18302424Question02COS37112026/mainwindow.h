#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QWidget>
#include <QColor>
#include <QMessageBox>
#include <QValidator>
#include "shape.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCreateShapeClicked();
    void onShapeSelectionChanged(const QString &shape);

    // --- New Navigation Slots ---
    void onPreviousShapeClicked();
    void onNextShapeClicked();

private:
    QGraphicsScene *scene;

    // --- UI Pointers ---
    QComboBox *comboBoxShape;
    QComboBox *comboBoxPenColor;
    QComboBox *comboBoxFillColor;
    QSpinBox *spinBoxProperty1;
    QSpinBox *spinBoxProperty2;
    QSpinBox *spinBoxPenWidth;
    QLabel *labelProperty1;
    QLabel *labelProperty2;
    QGraphicsView *graphicsShapeView;

    // --- Promoted to class members ---
    QPushButton *pushButtonPreviousShape;
    QPushButton *pushButtonNextShape;
    QPushButton *pushButtonCreateShape;

    // Helper function to map combobox string to QColor
    QColor getColorFromName(const QString &name);

    // Helper function to update button states
    void updateNavigationButtons();
};
#endif // MAINWINDOW_H