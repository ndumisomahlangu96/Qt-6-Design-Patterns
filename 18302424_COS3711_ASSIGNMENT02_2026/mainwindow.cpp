#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "shape.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    // Set the scene for the QGraphicsView defined in the UI
    ui->graphicsShapeView->setScene(scene);

    // Connect interactions
    connect(ui->pushButtonCreateShape, &QPushButton::clicked, this, &MainWindow::onCreateShapeClicked);
    connect(ui->comboBox_3, &QComboBox::currentTextChanged, this, &MainWindow::onShapeSelectionChanged);

    // Trigger initial UI setup to correctly label the properties based on default combobox value
    onShapeSelectionChanged(ui->comboBox_3->currentText());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onShapeSelectionChanged(const QString &shape)
{
    // Toggle UI controls depending on whether 1 or 2 properties are required
    if (shape == "Circle" || shape == "Square") {
        ui->labelProperty2->setVisible(false);
        // Updated from spinBox_2 to spinBoxProperty2
        ui->spinBoxProperty2->setVisible(false);

        if (shape == "Circle") ui->labelProperty1->setText("Radius:");
        else ui->labelProperty1->setText("Side length:");
    } else {
        ui->labelProperty2->setVisible(true);
        // Updated from spinBox_2 to spinBoxProperty2
        ui->spinBoxProperty2->setVisible(true);

        if (shape == "Rectangle") {
            ui->labelProperty1->setText("Width:");
            ui->labelProperty2->setText("Height:");
        } else {
            ui->labelProperty1->setText("X-Radius:");
            ui->labelProperty2->setText("Y-Radius:");
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
    return Qt::black; // Default fallback
}

void MainWindow::onCreateShapeClicked()
{
    // Clear previous shapes from the scene
    scene->clear();

    // Extract UI values using the updated descriptive spinbox names
    QString shapeType = ui->comboBox_3->currentText();
    int prop1 = ui->spinBoxProperty1->value();
    int prop2 = ui->spinBoxProperty2->value();
    int penWidth = ui->spinBoxPenWidth->value();

    QColor penColor = getColorFromName(ui->comboBox->currentText());
    QColor fillColor = getColorFromName(ui->comboBox_2->currentText());

    Shape *shape = nullptr;

    // Instantiate appropriate concrete shape via polymorphism
    if (shapeType == "Circle") {
        shape = new Circle(penWidth, penColor, fillColor, prop1);
    } else if (shapeType == "Square") {
        shape = new Square(penWidth, penColor, fillColor, prop1);
    } else if (shapeType == "Ellipse") {
        shape = new Ellipse(penWidth, penColor, fillColor, prop1, prop2);
    } else if (shapeType == "Rectangle") {
        shape = new Rectangle(penWidth, penColor, fillColor, prop1, prop2);
    }

    // Render shape and clean up dynamic memory
    if (shape) {
        shape->draw(scene);
        delete shape; // We pass drawing responsibility to scene; the object can now be safely destroyed
    }
}