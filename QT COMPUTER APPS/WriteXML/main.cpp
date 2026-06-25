#include <QCoreApplication>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Write XML
    QDomDocument document;


    //Make the root element
    QDomElement root = document.createElement("Books");

    //Add it to the document
    document.appendChild(root);

    // (Optional) Add a child element so the file isn't completely empty
    QDomElement book = document.createElement("Book");
    book.setAttribute("title", "Intro to Modern Qt");
    root.appendChild(book);

    // --- 2. Setup the Dynamic File Path ---
    // Safely find the user's standard 'Documents' folder on any Windows machine
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    // Define the custom folder structure
    QString customFolderPath = documentsPath + "/Test/MyXML";

    // Define the exact file path
    QString filePath = customFolderPath + "/MyXML.xml";

    // --- 3. Create the Custom Directories ---
    QDir dir;
    // mkpath() will safely create 'Test', then 'MyXML' inside it.
    // It returns true if successful or if the folders already exist.
    if (!dir.mkpath(customFolderPath)) {
        qDebug() << "Failed to create directory path:" << customFolderPath;
        return -1; // Exit if we lack permissions to make the folder
    }

    // --- 4. Write the XML to the File ---
    QFile file(filePath);

    // Open the file in WriteOnly mode, formatting as Text
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        // Write the document to the stream. The '4' dictates standard 4-space indentation.
        stream << document.toString(4);

        file.close();
        qDebug() << "Success! XML file written to:" << filePath;
    } else {
        qDebug() << "Error opening file for writing:" << file.errorString();
    }


    return QCoreApplication::exec();
}
