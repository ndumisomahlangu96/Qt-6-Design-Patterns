#include "mainwindow.h"
#include <QList>

// ==========================================
// SINGLETON: SHAPE MANAGER
// ==========================================
class ShapeManager {
private:
    QList<Shape*> shapes;
    int currentIndex;

    // Private constructor ensures no outside instantiation
    ShapeManager() : currentIndex(-1) {}

    // Destructor cleans up dynamically allocated shapes
    ~ShapeManager() {
        qDeleteAll(shapes);
        shapes.clear();
    }

    // Delete copy constructor and assignment operator
    ShapeManager(const ShapeManager&) = delete;
    ShapeManager& operator=(const ShapeManager&) = delete;

public:
    // Global access point to the single instance
    static ShapeManager& getInstance() {
        static ShapeManager instance;
        return instance;
    }

    void addShape(Shape* shape) {
        // If we add a new shape while in the middle of history, clear the "future" shapes
        while (shapes.size() - 1 > currentIndex) {
            delete shapes.takeLast();
        }
        shapes.append(shape);
        currentIndex = shapes.size() - 1;
    }

    Shape* getPrevious() {
        if (currentIndex > 0) {
            currentIndex--;
            return shapes[currentIndex];
        }
        return nullptr;
    }

    Shape* getNext() {
        if (currentIndex < shapes.size() - 1) {
            currentIndex++;
            return shapes[currentIndex];
        }
        return nullptr;
    }

    Shape* getCurrent() {
        if (currentIndex >= 0 && currentIndex < shapes.size()) {
            return shapes[currentIndex];
        }
        return nullptr;
    }

    bool hasPrevious() const { return currentIndex > 0; }
    bool hasNext() const { return currentIndex < shapes.size() - 1; }
};

// ==========================================
// MAIN WINDOW IMPLEMENTATION
// ==========================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scene(new QGraphicsScene(this))
{
    this->setWindowTitle("Shapes");
    this->resize(458, 406);

    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    // --- Top Layout ---
    QGridLayout *topLayout = new QGridLayout();

    QLabel *labelShape = new QLabel("Shape:");
    comboBoxShape = new QComboBox();
    comboBoxShape->addItems({"Circle", "Square", "Ellipse", "Rectangle"});

    QLabel *labelPenWidth = new QLabel("Pen width:");
    spinBoxPenWidth = new QSpinBox();
    spinBoxPenWidth->setMaximum(100000000);

    labelProperty1 = new QLabel("Property 1");
    spinBoxProperty1 = new QSpinBox();
    spinBoxProperty1->setMaximum(100000000);

    labelProperty2 = new QLabel("Property 2");
    spinBoxProperty2 = new QSpinBox();
    spinBoxProperty2->setMaximum(100000000);

    topLayout->addWidget(labelShape, 0, 0);
    topLayout->addWidget(comboBoxShape, 0, 1);
    topLayout->addWidget(labelProperty1, 0, 2);
    topLayout->addWidget(spinBoxProperty1, 0, 3);

    topLayout->addWidget(labelPenWidth, 1, 0);
    topLayout->addWidget(spinBoxPenWidth, 1, 1);
    topLayout->addWidget(labelProperty2, 1, 2);
    topLayout->addWidget(spinBoxProperty2, 1, 3);

    // --- Middle Layout ---
    QGridLayout *middleLayout = new QGridLayout();

    QLabel *labelPenColour = new QLabel("Pen colour:");
    comboBoxPenColor = new QComboBox();
    comboBoxPenColor->addItems({"Black", "Dark Blue", "Dark Green", "Dark Brown"});

    QLabel *labelFillColour = new QLabel("Fill colour:");
    comboBoxFillColor = new QComboBox();
    comboBoxFillColor->addItems({"Red", "Orange", "Yellow", "Green", "Blue", "Indigo", "Violet"});

    pushButtonCreateShape = new QPushButton("Create Shape");
    pushButtonCreateShape->setDefault(true);

    middleLayout->addWidget(labelPenColour, 0, 0);
    middleLayout->addWidget(comboBoxPenColor, 0, 1);
    middleLayout->addWidget(pushButtonCreateShape, 0, 2, 2, 1);

    middleLayout->addWidget(labelFillColour, 1, 0);
    middleLayout->addWidget(comboBoxFillColor, 1, 1);

    // --- Bottom Layout ---
    QGridLayout *bottomLayout = new QGridLayout();
    graphicsShapeView = new QGraphicsView();
    graphicsShapeView->setScene(scene);

    // Initialize as class members and default to not active.
    pushButtonPreviousShape = new QPushButton("Previous");
    pushButtonPreviousShape->setEnabled(false);

    pushButtonNextShape = new QPushButton("Next");
    pushButtonNextShape->setEnabled(false);

    bottomLayout->addWidget(graphicsShapeView, 0, 0, 1, 2);
    bottomLayout->addWidget(pushButtonPreviousShape, 1, 0);
    bottomLayout->addWidget(pushButtonNextShape, 1, 1);

    // Add sub-layouts
    mainLayout->addLayout(topLayout, 0, 0);
    mainLayout->addLayout(middleLayout, 1, 0);
    mainLayout->addLayout(bottomLayout, 2, 0);

    // ==========================================
    // SIGNAL & SLOT CONNECTIONS
    // ==========================================
    connect(pushButtonCreateShape, &QPushButton::clicked, this, &MainWindow::onCreateShapeClicked);
    connect(comboBoxShape, &QComboBox::currentTextChanged, this, &MainWindow::onShapeSelectionChanged);

    // Connect Navigation Buttons
    connect(pushButtonPreviousShape, &QPushButton::clicked, this, &MainWindow::onPreviousShapeClicked);
    connect(pushButtonNextShape, &QPushButton::clicked, this, &MainWindow::onNextShapeClicked);

    onShapeSelectionChanged(comboBoxShape->currentText());
}

MainWindow::~MainWindow() = default;

void MainWindow::onShapeSelectionChanged(const QString &shape)
{
    if (shape == "Circle" || shape == "Square") {
        labelProperty2->setVisible(false);
        spinBoxProperty2->setVisible(false);

        if (shape == "Circle") labelProperty1->setText("Radius:");
        else labelProperty1->setText("Side length:");
    } else {
        labelProperty2->setVisible(true);
        spinBoxProperty2->setVisible(true);

        if (shape == "Rectangle") {
            labelProperty1->setText("Width:");
            labelProperty2->setText("Height:");
        } else {
            labelProperty1->setText("X-Radius:");
            labelProperty2->setText("Y-Radius:");
        }
    }
}

QColor MainWindow::getColorFromName(const QString &name)
{
    if (name == "Black") return Qt::black;
    if (name == "Dark Blue") return Qt::darkBlue;
    if (name == "Dark Green") return Qt::darkGreen;
    if (name == "Dark Brown") return QColor(101, 67, 33);
    if (name == "Red") return Qt::red;
    if (name == "Orange") return QColor(255, 165, 0);
    if (name == "Yellow") return Qt::yellow;
    if (name == "Green") return Qt::green;
    if (name == "Blue") return Qt::blue;
    if (name == "Indigo") return QColor(75, 0, 130);
    if (name == "Violet") return QColor(238, 130, 238);
    return Qt::black;
}

void MainWindow::onCreateShapeClicked()
{
    scene->clear();

    QString shapeType = comboBoxShape->currentText();
    int prop1 = spinBoxProperty1->value();
    int prop2 = spinBoxProperty2->value();
    int penWidth = spinBoxPenWidth->value();

    QColor penColor = getColorFromName(comboBoxPenColor->currentText());
    QColor fillColor = getColorFromName(comboBoxFillColor->currentText());

    Shape *shape = nullptr;

    if (shapeType == "Circle") {
        shape = new Circle(penWidth, penColor, fillColor, prop1);
    } else if (shapeType == "Square") {
        shape = new Square(penWidth, penColor, fillColor, prop1);
    } else if (shapeType == "Ellipse") {
        shape = new Ellipse(penWidth, penColor, fillColor, prop1, prop2);
    } else if (shapeType == "Rectangle") {
        shape = new Rectangle(penWidth, penColor, fillColor, prop1, prop2);
    }

    // Pass ownership of the shape pointer to the Singleton Manager
    if (shape) {
        ShapeManager::getInstance().addShape(shape);
        shape->draw(scene);
        updateNavigationButtons();
    }
}

void MainWindow::onPreviousShapeClicked()
{
    Shape* prevShape = ShapeManager::getInstance().getPrevious();
    if (prevShape) {
        scene->clear(); // This deletes the QGraphicsItems, NOT our Shape pointers
        prevShape->draw(scene);
        updateNavigationButtons();
    }
}

void MainWindow::onNextShapeClicked()
{
    Shape* nextShape = ShapeManager::getInstance().getNext();
    if (nextShape) {
        scene->clear();
        nextShape->draw(scene);
        updateNavigationButtons();
    }
}

void MainWindow::updateNavigationButtons()
{
    pushButtonPreviousShape->setEnabled(ShapeManager::getInstance().hasPrevious());
    pushButtonNextShape->setEnabled(ShapeManager::getInstance().hasNext());
}