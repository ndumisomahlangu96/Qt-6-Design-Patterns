#include "shape.h"

// ==========================================
// CONSTRUCTORS WITH INITIALIZER LISTS
// ==========================================
// The syntax ": penWidth(pWidth)..." is a member initializer list.
// It is more efficient than assigning values inside the constructor body because
// it initializes the variables at the exact moment of creation.

Shape::Shape(int pWidth, QColor pColour, QColor fColour)
    : penWidth(pWidth), penColour(pColour), fillColour(fColour){}

Shape1Property::Shape1Property(int pWidth, QColor pColour, QColor fColour, int prop1)
    : Shape(pWidth, pColour, fColour), property1(prop1) {}

Shape2Property::Shape2Property(int pWidth, QColor pColour, QColor fColour, int prop1, int prop2)
    : Shape1Property(pWidth, pColour, fColour, prop1), property2(prop2) {}

// ==========================================
// DRAW IMPLEMENTATIONS (Polymorphism)
// ==========================================
// Each specific shape knows how to draw itself using Qt's QGraphicsScene API.

// --- Circle Implementation ---
Circle::Circle(int pWidth, QColor pColour, QColor fColour, int radius)
    : Shape1Property(pWidth, pColour, fColour, radius) {}

void Circle::draw(QGraphicsScene *scene)
{
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour); // Brush handles the inside color of the shape
    // QGraphicsScene::addEllipse takes (x, y, width, height).
    // For a perfect circle, width and height are the same (property1).
    scene->addEllipse(0,0,property1,property1,pen,brush);
}

// --- Square Implementation ---
Square::Square(int pWidth, QColor pColour, QColor fColour, int side)
    : Shape1Property(pWidth, pColour, fColour, side) {}

void Square::draw(QGraphicsScene *scene)
{
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    // addRect takes (x, y, width, height).
    scene->addRect(0,0,property1,property1,pen,brush);
}

// --- Ellipse Implementation ---
Ellipse::Ellipse(int pWidth,QColor pColour, QColor fColour, int width, int height)
    : Shape2Property(pWidth, pColour, fColour, width, height) {}

void Ellipse::draw(QGraphicsScene *scene)
{
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    // Uses both property1 (width) and property2 (height) to allow for stretched shapes.
    scene->addEllipse(0,0,property1,property2,pen,brush);
}

// --- Rectangle Implementation ---
Rectangle::Rectangle(int pWidth, QColor pColour, QColor fColour, int width, int height)
    : Shape2Property(pWidth, pColour, fColour, width, height) {}

void Rectangle::draw(QGraphicsScene *scene)
{
    QPen pen(penColour, penWidth);
    QBrush brush(fillColour);
    scene->addRect(0,0,property1,property2,pen,brush);
}

// ==========================================
// MEMENTO CLONE IMPLEMENTATIONS
// ==========================================
Shape* Circle::clone() const {
    return new Circle(penWidth, penColour, fillColour, property1);
}

Shape* Square::clone() const {
    return new Square(penWidth, penColour, fillColour, property1);
}

Shape* Ellipse::clone() const {
    return new Ellipse(penWidth, penColour, fillColour, property1, property2);
}

Shape* Rectangle::clone() const {
    return new Rectangle(penWidth, penColour, fillColour, property1, property2);
}