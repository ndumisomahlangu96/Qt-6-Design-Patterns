#include "shape.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>

// Base Constructors
Shape::Shape(int pWidth, QColor pColour, QColor fColour)
    : penWidth(pWidth), penColour(pColour), fillColour(fColour) {}

Shape1Property::Shape1Property(int pWidth, QColor pColour, QColor fColour, int prop1)
    : Shape(pWidth, pColour, fColour), property1(prop1) {}

Shape2Property::Shape2Property(int pWidth, QColor pColour, QColor fColour, int prop1, int prop2)
    : Shape1Property(pWidth, pColour, fColour, prop1), property2(prop2) {}

// --- Circle Implementation ---
Circle::Circle(int pWidth, QColor pColour, QColor fColour, int radius)
    : Shape1Property(pWidth, pColour, fColour, radius) {}

void Circle::draw(QGraphicsScene* scene) {
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    // X, Y, Width, Height (Width and Height are both 'property1' / radius)
    scene->addEllipse(0, 0, property1, property1, pen, brush);
}

// --- Square Implementation ---
Square::Square(int pWidth, QColor pColour, QColor fColour, int side)
    : Shape1Property(pWidth, pColour, fColour, side) {}

void Square::draw(QGraphicsScene* scene) {
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    scene->addRect(0, 0, property1, property1, pen, brush);
}

// --- Ellipse Implementation ---
Ellipse::Ellipse(int pWidth, QColor pColour, QColor fColour, int width, int height)
    : Shape2Property(pWidth, pColour, fColour, width, height) {}

void Ellipse::draw(QGraphicsScene* scene) {
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    scene->addEllipse(0, 0, property1, property2, pen, brush);
}

// --- Rectangle Implementation ---
Rectangle::Rectangle(int pWidth, QColor pColour, QColor fColour, int width, int height)
    : Shape2Property(pWidth, pColour, fColour, width, height) {}

void Rectangle::draw(QGraphicsScene* scene) {
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    scene->addRect(0, 0, property1, property2, pen, brush);
}