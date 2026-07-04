#include "mainwindow.h"

// ==========================================
// SINGLETON: SHAPE MANAGER
// ==========================================
class ShapeManager {
private:
    QList<Shape*> shapes;
    int currentIndex;

    ShapeManager() : currentIndex(-1) {}

    ~ShapeManager() {
        qDeleteAll(shapes);
        shapes.clear();
    }

    ShapeManager(const ShapeManager&) = delete;
    ShapeManager& operator=(const ShapeManager&) = delete;

public:
    static ShapeManager& getInstance() {
        static ShapeManager instance;
        return instance;
    }

    void addShape(Shape* shape) {
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

    // --- NEW FEATURES ADDED FOR XML NAVIGATION ---

    // Returns the total number of shapes currently in the list
    int getCount() const { return shapes.size(); }

    // Allows us to jump to a specific shape in the history (used after XML load)
    void setCurrentIndex(int index) {
        if (index >= 0 && index < shapes.size()) {
            currentIndex = index;
        }
    }
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

    pushButtonPreviousShape = new QPushButton("Previous Shape");
    pushButtonPreviousShape->setEnabled(false);

    pushButtonNextShape = new QPushButton("Next Shape");
    pushButtonNextShape->setEnabled(false);

    bottomLayout->addWidget(graphicsShapeView, 0, 0, 1, 2);
    bottomLayout->addWidget(pushButtonPreviousShape, 1, 0);
    bottomLayout->addWidget(pushButtonNextShape, 1, 1);

    mainLayout->addLayout(topLayout, 0, 0);
    mainLayout->addLayout(middleLayout, 1, 0);
    mainLayout->addLayout(bottomLayout, 2, 0);

    // ==========================================
    // SIGNAL & SLOT CONNECTIONS
    // ==========================================
    connect(pushButtonCreateShape, &QPushButton::clicked, this, &MainWindow::onCreateShapeClicked);
    connect(comboBoxShape, &QComboBox::currentTextChanged, this, &MainWindow::onShapeSelectionChanged);
    connect(pushButtonPreviousShape, &QPushButton::clicked, this, &MainWindow::onPreviousShapeClicked);
    connect(pushButtonNextShape, &QPushButton::clicked, this, &MainWindow::onNextShapeClicked);

    // Set initial UI state
    onShapeSelectionChanged(comboBoxShape->currentText());

    // --- NEW LOGIC: Load XML Shapes Immediately on Startup ---
    loadShapesOnStartup();
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
        scene->clear();
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

// ==========================================
// XML HANDLING
// ==========================================

void MainWindow::addShape(QDomDocument &document, QDomElement &root, const QString &type, const QString &pw, const QString &pc, const QString &fc, const QString &p1, const QString &p2)
{
    QDomElement shape = document.createElement("shape");
    shape.setAttribute("type", type);
    shape.setAttribute("pw", pw);
    shape.setAttribute("pc", pc);
    shape.setAttribute("fc", fc);
    shape.setAttribute("p1", p1);
    shape.setAttribute("p2", p2);
    root.appendChild(shape);
}

// MODIFIED: Now parses strings into objects and loads them into ShapeManager
void MainWindow::ListElements(const QDomElement &root)
{
    QDomElement shapeElement = root.firstChildElement("shape");

    // Record the current size of the list so we know where the new shapes begin
    int startIndex = ShapeManager::getInstance().getCount();
    int addedCount = 0;

    while (!shapeElement.isNull()) {
        // Extract attributes
        QString type = shapeElement.attribute("type");
        int pw = shapeElement.attribute("pw").toInt();
        QColor pc = getColorFromName(shapeElement.attribute("pc"));
        QColor fc = getColorFromName(shapeElement.attribute("fc"));
        int p1 = shapeElement.attribute("p1").toInt();
        int p2 = shapeElement.attribute("p2").toInt(); // If empty, toInt() resolves to 0

        Shape *shape = nullptr;

        // Instantiate the correct object dynamically
        if (type == "Circle") {
            shape = new Circle(pw, pc, fc, p1);
        } else if (type == "Square") {
            shape = new Square(pw, pc, fc, p1);
        } else if (type == "Ellipse") {
            shape = new Ellipse(pw, pc, fc, p1, p2);
        } else if (type == "Rectangle") {
            shape = new Rectangle(pw, pc, fc, p1, p2);
        }

        // Add to the ShapeManager history
        if (shape) {
            ShapeManager::getInstance().addShape(shape);
            addedCount++;
        }

        qDebug().noquote() << "Shape Added from XML:" << type
                           << "\n  pw:" << pw
                           << "| pc:" << shapeElement.attribute("pc")
                           << "| fc:" << shapeElement.attribute("fc")
                           << "| p1:" << p1
                           << "| p2:" << (shapeElement.attribute("p2").isEmpty() ? "N/A" : QString::number(p2)) << "\n";

        shapeElement = shapeElement.nextSiblingElement("shape");
    }

    // NEW LOGIC: Display the first newly loaded shape in the view
    if (addedCount > 0) {
        // Shift the index back to the first shape we just added
        ShapeManager::getInstance().setCurrentIndex(startIndex);

        // Fetch and draw it
        Shape* firstLoaded = ShapeManager::getInstance().getCurrent();
        if (firstLoaded) {
            scene->clear();
            firstLoaded->draw(scene);
            updateNavigationButtons();
        }
    }
}

// Renamed from WriteAndReadXMLShapeFile and handles the automated startup logic
void MainWindow::loadShapesOnStartup()
{
    QDomDocument document;
    QDomElement rootWrite = document.createElement("shapeList");
    document.appendChild(rootWrite);

    addShape(document, rootWrite, "Square", "1", "Red", "Black", "110", "");
    addShape(document, rootWrite, "Circle", "2", "Green", "Blue", "75", "");
    addShape(document, rootWrite, "Ellipse", "3", "Black", "Red", "140", "55");
    addShape(document, rootWrite, "Rectangle", "4", "Blue", "Green", "75", "120");

    QString fileName = "shapes.xml";
    QString filePath = QDir(QApplication::applicationDirPath()).filePath(fileName);

    QFile fileWriteXML(filePath);
    if (!fileWriteXML.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << fileWriteXML.errorString();
        return;
    }
    else
    {
        QTextStream out(&fileWriteXML);
        out << document.toString(4);
        fileWriteXML.close();
        qDebug() << "Success: Shapes XML file written to" << filePath;
    }

    QDomDocument readDocument;
    QFile fileReadXML(filePath);

    if(!fileReadXML.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file.";
        return;
    }
    else
    {
        if(!readDocument.setContent(&fileReadXML))
        {
            qDebug() << "Failed to load document";
            fileReadXML.close();
            return;
        }
        fileReadXML.close();
    }

    QDomElement rootRead = readDocument.firstChildElement("shapeList");

    qDebug() << "--- Loading Shapes from XML into List ---";
    ListElements(rootRead);
}

void MainWindow::updateNavigationButtons()
{
    pushButtonPreviousShape->setEnabled(ShapeManager::getInstance().hasPrevious());
    pushButtonNextShape->setEnabled(ShapeManager::getInstance().hasNext());
}