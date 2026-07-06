#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scene(new QGraphicsScene(this))
{
    this->setWindowTitle("Shapes");
    this->resize(458, 406);

    // ==========================================
    // 1. PURE C++ UI GENERATION
    // ==========================================
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    // --- Top Layout (Shape selection and properties) ---
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

    // --- Middle Layout (Colours and Create Button) ---
    QGridLayout *middleLayout = new QGridLayout();

    QLabel *labelPenColour = new QLabel("Pen colour:");
    comboBoxPenColor = new QComboBox();
    comboBoxPenColor->addItems({"Black", "Dark Blue", "Dark Green", "Dark Brown"});

    QLabel *labelFillColour = new QLabel("Fill colour:");
    comboBoxFillColor = new QComboBox();
    comboBoxFillColor->addItems({"Red", "Orange", "Yellow", "Green", "Blue", "Indigo", "Violet"});

    QPushButton *pushButtonCreateShape = new QPushButton("Create Shape");
    pushButtonCreateShape->setDefault(true);

    middleLayout->addWidget(labelPenColour, 0, 0);
    middleLayout->addWidget(comboBoxPenColor, 0, 1);
    middleLayout->addWidget(pushButtonCreateShape, 0, 2, 2, 1); // Row span 2

    middleLayout->addWidget(labelFillColour, 1, 0);
    middleLayout->addWidget(comboBoxFillColor, 1, 1);

    // --- Bottom Layout (Graphics View and Next/Prev) ---
    QGridLayout *bottomLayout = new QGridLayout();
    graphicsShapeView = new QGraphicsView();
    graphicsShapeView->setScene(scene);

    QPushButton *pushButtonPreviousShape = new QPushButton("Previous Shape");
    pushButtonPreviousShape->setEnabled(false);

    QPushButton *pushButtonNextShape = new QPushButton("Next Shape");
    pushButtonNextShape->setEnabled(false);

    bottomLayout->addWidget(graphicsShapeView, 0, 0, 1, 2); // Column span 2
    bottomLayout->addWidget(pushButtonPreviousShape, 1, 0);
    bottomLayout->addWidget(pushButtonNextShape, 1, 1);

    // Add sub-layouts to the main layout
    mainLayout->addLayout(topLayout, 0, 0);
    mainLayout->addLayout(middleLayout, 1, 0);
    mainLayout->addLayout(bottomLayout, 2, 0);

    // ==========================================
    // 2. SIGNAL & SLOT CONNECTIONS
    // ==========================================
    connect(pushButtonCreateShape, &QPushButton::clicked, this, &MainWindow::onCreateShapeClicked);
    connect(comboBoxShape, &QComboBox::currentTextChanged, this, &MainWindow::onShapeSelectionChanged);

    // Trigger initial UI setup to correctly label the properties based on default combobox value
    onShapeSelectionChanged(comboBoxShape->currentText());
}

MainWindow::~MainWindow() = default;


// ==========================================
// 3. IMPLEMENTATION OF SHAPE DRAWING LOGIC
// ==========================================

void MainWindow::onShapeSelectionChanged(const QString &shape)
{
    // Toggle UI controls depending on whether 1 or 2 properties are required
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
    QString shapeType = comboBoxShape->currentText();
    int prop1 = spinBoxProperty1->value();
    int prop2 = spinBoxProperty2->value();
    int penWidth = spinBoxPenWidth->value();

    // ==========================================
    // NEW: INPUT VALIDATION
    // ==========================================

    // 1. Validate Pen Width and Property 1 (used by ALL shapes)
    if (penWidth == 0 || prop1 == 0) {
        QMessageBox::warning(this, "Invalid Input",
                             "Please enter a non-zero value for both the Pen width and Property 1.");
        return; // Abort creation
    }

    // 2. Validate Property 2 ONLY if the shape is an Ellipse or Rectangle
    if ((shapeType == "Ellipse" || shapeType == "Rectangle") && prop2 == 0) {
        QMessageBox::warning(this, "Invalid Input",
                             "Please enter a non-zero value for Property 2.");
        return; // Abort creation
    }

    // ==========================================
    // SHAPE CREATION (Proceeds only if valid)
    // ==========================================

    scene->clear();

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

    if (shape) {
        // Draw the shape to the QGraphicsScene
        shape->draw(scene);

        // FIX: Removed calls to undefined ShapeManager and updateNavigationButtons()
        // Prevent memory leak by deleting the shape pointer after it's drawn
        delete shape;
    }
}