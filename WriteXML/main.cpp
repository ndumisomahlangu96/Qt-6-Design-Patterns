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

    //Add it to the document
    document.appendChild(root);

    //Add some elements
    for(int i = 0; i < 10; i++)
    {
        QDomElement book = document.createElement("Book");
        book.setAttribute("Name", " My Book " + QString::number(i));
        book.setAttribute("ID", QString::number(i));
        root.appendChild(book);

        for(int h = 0; h < 10; h++)
        {
            QDomElement chapter = document.createElement("Chapter");
            chapter.setAttribute("Name", " My Chapter " + QString::number(h));
            chapter.setAttribute("ID", QString::number(i));
            root.appendChild(chapter);
        }
    }


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
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() <<  "Finished to open file for writing";
        return -1;

    } else {
        QTextStream stream (&file);
        stream << document.toString();
        file.close();
        qDebug() << "Finished";
    }


    return QCoreApplication::exec();
}
