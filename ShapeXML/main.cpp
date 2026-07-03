#include <QCoreApplication>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>

// Extracted regular void function
// We pass document and root by reference (&) so the function can modify them
void addShape(QDomDocument& document, QDomElement& root, const QString& type, const QString& pw, const QString& pc, const QString& fc, const QString& p1, const QString& p2)
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

// DOM method of reading xml
void ListElements(const QDomElement& root)
{
    // Traverse the child elements looking for "shape" tags
    QDomElement shapeElement = root.firstChildElement("shape");

    while (!shapeElement.isNull()) {
        // Extract attributes
        QString type = shapeElement.attribute("type");
        QString pw = shapeElement.attribute("pw");
        QString pc = shapeElement.attribute("pc");
        QString fc = shapeElement.attribute("fc");
        QString p1 = shapeElement.attribute("p1");
        QString p2 = shapeElement.attribute("p2");

        // Format and output the data to the console
        qDebug().noquote() << "Shape:" << type
                           << "\n  pw:" << pw
                           << "| pc:" << pc
                           << "| fc:" << fc
                           << "| p1:" << p1
                           << "| p2:" << (p2.isEmpty() ? "N/A" : p2) << "\n";

        // Move to the next "shape" sibling
        shapeElement = shapeElement.nextSiblingElement("shape");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. Initialize the XML Document
    QDomDocument document;

    // 2. Create and append the root element <shapeList>
    QDomElement rootWrite = document.createElement("shapeList");
    document.appendChild(rootWrite);

    // 3. Populate the XML with data exactly as formatted in the image
    // Note: document and root are now passed as the first two arguments
    addShape(document, rootWrite, "Square", "1", "Red", "Black", "110", "");
    addShape(document, rootWrite, "Circle", "2", "Green", "Blue", "75", "");
    addShape(document, rootWrite, "Ellipse", "3", "Black", "Red", "140", "55");
    addShape(document, rootWrite, "Rectangle", "4", "Blue", "Green", "75", "120");

    // 4. Define the output file path (same directory as the executable)
    QString fileName = "shapes.xml";
    QString filePath = QDir(QCoreApplication::applicationDirPath()).filePath(fileName);

    // 5. Open the file and write the XML content
    QFile fileWriteXML(filePath);
    if (!fileWriteXML.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << fileWriteXML.errorString();
        return -1;
    }
    else
    {
        QTextStream out(&fileWriteXML);
        // The '4' parameter sets the indentation level to 4 spaces for a clean structure
        out << document.toString(4);
        fileWriteXML.close();

        qDebug() << "Success: Shapes XML file written to" << filePath;
    }

    // 6. Load the file
    // --- Read the XML to File ---
    QDomDocument readDocument;
    QFile fileReadXML(filePath);

    if(!fileReadXML.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file.";
        return -1;
    }
    else
    {
        if(!readDocument.setContent(&fileReadXML))
        {
            qDebug() << "Failed to load document";
            fileReadXML.close();
            return -1;
        }
        fileReadXML.close();
    }

    // Get the root element
    QDomElement rootRead = readDocument.firstChildElement("shapeList");

    // List the shapes.
    qDebug() << "--- Listing Shapes ---";
    ListElements(rootRead);

    // Return 0 directly since we don't need to start the event loop for a one-off script
    return 0;
}