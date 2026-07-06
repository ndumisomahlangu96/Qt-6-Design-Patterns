#ifndef SHAPE_H
#define SHAPE_H

#include <QColor>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>

// ==========================================
// ABSTRACT BASE CLASS
// ==========================================
// This class defines the common interface and properties for ALL shapes.
class Shape
{
protected:
    // Protected means derived classes (Circle, Square) can access these directly,
    // but outside code cannot.
    int penWidth;
    QColor penColour;
    QColor fillColour;

public:
    Shape(int pWidth, QColor pColour, QColor fColour);

    // A virtual destructor ensures that when a derived class object is deleted
    // via a base class pointer, the derived class's destructor is called properly,
    // preventing memory leaks.
    virtual ~Shape() = default;

    // Pure virtual function ("= 0"). This makes 'Shape' an Abstract Class.
    // You cannot instantiate a 'Shape' directly. Any class inheriting from Shape
    // MUST implement this function to compile.
    virtual void draw(QGraphicsScene *scene) = 0;
};

// ==========================================
// INTERMEDIATE ABSTRACT CLASSES
// ==========================================
// Handles shapes that only need a single dimension (e.g., radius for a circle, side for a square).
class Shape1Property : public Shape {
protected:
    int property1;

public:
    Shape1Property(int pWidth, QColor pColour, QColor fColour, int prop1);
    // Note: draw() is still pure virtual here, so this class is also abstract.
    virtual void draw(QGraphicsScene *scene) = 0;
};

// Handles shapes that need two dimensions (e.g., width AND height for a rectangle).
class Shape2Property : public Shape1Property {
protected:
    int property2;

public:
    Shape2Property(int pWidth, QColor pColour, QColor fColour, int prop1, int prop2);
    virtual void draw(QGraphicsScene *scene) = 0;
};

// ==========================================
// CONCRETE CLASSES
// ==========================================
// These classes can actually be instantiated because they provide an implementation for draw().

class Circle : public Shape1Property {
public:
    Circle(int pWidth, QColor pColour, QColor fColour, int radius);
    void draw(QGraphicsScene *scene) override; // 'override' ensures we match the base class signature
};

class Square : public Shape1Property {
public:
    Square(int pWidth, QColor pColour, QColor fColour, int side);
    void draw(QGraphicsScene *scene) override;
};

class Ellipse : public Shape2Property {
public:
    Ellipse(int pWidth, QColor pColour, QColor fColour, int width, int height);
    void draw(QGraphicsScene *scene) override;
};

class Rectangle : public Shape2Property {
public:
    Rectangle(int pWidth, QColor pColour, QColor fColour, int width, int height);
    void draw(QGraphicsScene *scene) override;
};

#endif // SHAPE_H