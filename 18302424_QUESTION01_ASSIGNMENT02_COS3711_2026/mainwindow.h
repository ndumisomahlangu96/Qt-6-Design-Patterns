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

    // Helper function to map combobox string to QColor
    QColor getColorFromName(const QString &name);
};
#endif // MAINWINDOW_H