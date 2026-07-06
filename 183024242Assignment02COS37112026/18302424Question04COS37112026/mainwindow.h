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
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QList>
#include <QMessageBox>
#include <QValidator>
#include <QApplication>
#include "shape.h"

// ==========================================
// MEMENTO DESIGN PATTERN
// ==========================================

class Memento {
    // 2. Declare the originator a friend.
    friend class MainWindow;
private:
    // UI State
    QString shapeType;
    int penWidth;
    int prop1;
    int prop2;
    QString penColor;
    QString fillColor;

    // Application State
    QList<Shape*> savedShapes;
    int currentIndex;

    // Keep the constructor private so ONLY MainWindow can create a Memento
    Memento(QString sType, int pWidth, int p1, int p2, QString pColor, QString fColor, const QList<Shape*>& shapes, int cIndex)
        : shapeType(sType), penWidth(pWidth), prop1(p1), prop2(p2), penColor(pColor), fillColor(fColor), currentIndex(cIndex) {

        // Deep copy the list so Memento holds a separate, safe state
        for (Shape* s : shapes) {
            savedShapes.append(s->clone());
        }
    }

public:
    // ADD THIS 'public' ACCESS MODIFIER
    // Allow Caretaker to delete the Memento when a new one is saved or the app closes
    ~Memento() {
        qDeleteAll(savedShapes);
        savedShapes.clear();
    }
};

class Caretaker {
private:
    Memento* memento; // 5. Caretaker holds on to (but cannot peek into) the Memento.
public:
    Caretaker() : memento(nullptr) {}
    ~Caretaker() { delete memento; }

    void save(Memento* m) {
        if (memento) {
            delete memento; // Allow only a single backup point
        }
        memento = m;
    }

    Memento* getMemento() const {
        return memento;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCreateShapeClicked();
    void onShapeSelectionChanged(const QString &shape);

    // Navigation Slots
    void onPreviousShapeClicked();
    void onNextShapeClicked();

    // NEW: Slots for Memento
    void onBackupShapeClicked();
    void onRestoreShapeClicked();

private:
    Caretaker *caretaker; // NEW: Caretaker instance
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

    QPushButton *pushButtonPreviousShape;
    QPushButton *pushButtonNextShape;
    QPushButton *pushButtonCreateShape;
    QPushButton *pushButtonBackup;
    QPushButton *pushButtonRestore;

    // XML parsing and startup helper functions
    void addShape(QDomDocument& document, QDomElement& root, const QString& type, const QString& pw, const QString& pc, const QString& fc, const QString& p1, const QString& p2);
    void ListElements(const QDomElement& root);
    void loadShapesOnStartup();

    // Helper function to map combobox string to QColor
    QColor getColorFromName(const QString &name);

    // Helper function to update button states
    void updateNavigationButtons();
};
#endif // MAINWINDOW_H