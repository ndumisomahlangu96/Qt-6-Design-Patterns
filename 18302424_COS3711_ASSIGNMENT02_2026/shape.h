#ifndef SHAPE_H
#define SHAPE_H

#include <QColor>
#include <QGraphicsScene>

// Abstract Base Class
class Shape {
protected:
    int penWidth;
    QColor penColour;
    QColor fillColour;

public:
    Shape(int pWidth, QColor pColour, QColor fColour);
    virtual ~Shape() = default;

    // Pure virtual function making this an abstract class
    virtual void draw(QGraphicsScene* scene) = 0;
};

// Abstract Class for Shapes with 1 Property (Circle, Square)
class Shape1Property : public Shape {
protected:
    int property1;

public:
    Shape1Property(int pWidth, QColor pColour, QColor fColour, int prop1);
    virtual void draw(QGraphicsScene* scene) = 0;
};

// Abstract Class for Shapes with 2 Properties (Ellipse, Rectangle)
class Shape2Property : public Shape1Property {
protected:
    int property2;

public:
    Shape2Property(int pWidth, QColor pColour, QColor fColour, int prop1, int prop2);
    virtual void draw(QGraphicsScene* scene) = 0;
};

// Concrete Class: Circle
class Circle : public Shape1Property {
public:
    Circle(int pWidth, QColor pColour, QColor fColour, int radius);
    void draw(QGraphicsScene* scene) override;
};

// Concrete Class: Square
class Square : public Shape1Property {
public:
    Square(int pWidth, QColor pColour, QColor fColour, int side);
    void draw(QGraphicsScene* scene) override;
};

// Concrete Class: Ellipse
class Ellipse : public Shape2Property {
public:
    Ellipse(int pWidth, QColor pColour, QColor fColour, int width, int height);
    void draw(QGraphicsScene* scene) override;
};

// Concrete Class: Rectangle
class Rectangle : public Shape2Property {
public:
    Rectangle(int pWidth, QColor pColour, QColor fColour, int width, int height);
    void draw(QGraphicsScene* scene) override;
};

#endif // SHAPE_H