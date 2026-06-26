#include <QCoreApplication>
#include <QDomDocument>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

// DOM method of reading xml
void ListElements(QDomElement root, QString tagname, QString attribute)
{
    // elementsByTagName dynamically finds only the subchildren within the provided root element
    QDomNodeList items = root.elementsByTagName(tagname);
    qDebug() << "Total items =" << items.count();

    for(int i = 0; i < items.count(); i++)
    {
        QDomNode itemnode = items.at(i);

        //convert to element
        if(itemnode.isElement())
        {
            QDomElement itemelement = itemnode.toElement();
            // trimmed() is optional but cleans up the leading space in " My Chapter"
            qDebug() << itemelement.attribute(attribute).trimmed();
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Write XML
    QDomDocument document;

    //Make the root element
    QDomElement rootWrite = document.createElement("Books");

    //Add it to the document
    document.appendChild(rootWrite);

    //Add some elements
    for(int i = 0; i < 10; i++)
    {
        QDomElement book = document.createElement("Book");
        book.setAttribute("Name", " My Book " + QString::number(i));
        book.setAttribute("ID", QString::number(i));
        rootWrite.appendChild(book);

        for(int h = 0; h < 10; h++)
        {
            QDomElement chapter = document.createElement("Chapter");
            chapter.setAttribute("Name", "My Chapter " + QString::number(h));
            chapter.setAttribute("ID", QString::number(i));

            // FIX: Append the chapter to the 'book' node so it becomes a subchild
            book.appendChild(chapter);
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
    QFile fileWrite(filePath);

    // Open the file in WriteOnly mode, formatting as Text
    if (!fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() <<  "Failed to open file for writing.";
        return -1;

    } else {
        QTextStream stream (&fileWrite);
        stream << document.toString();
        fileWrite.close();
        qDebug() << "Finished closing file for writing.";
    }

    // Load the file
    // --- 5. Read the XML to File ---
    QFile fileRead(filePath);
    if(!fileRead.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file.";
    }
    else
    {
        if(!document.setContent(&fileRead))
        {
            qDebug() << "Failed to load document";
            return -1;
        }
        fileRead.close();
    }

    //get the root element
    QDomElement rootRead = document.firstChildElement();

    // List the books.
    qDebug() << "--- Listing Root Books ---";
    ListElements(rootRead, "Book", "Name");

    qDebug() << "\r\n--- More Advanced: Reading Subchildren ---";

    // Get the chapters
    QDomNodeList books = rootRead.elementsByTagName("Book");
    for (int i = 0; i < books.count(); i++)
    {
        QDomNode booknode = books.at(i);
        //convert to an element
        if(booknode.isElement())
        {
            QDomElement book = booknode.toElement();
            qDebug() << "\nChapters in" << book.attribute("Name").trimmed() << ":";

            // Pass the specific book node in as the root to count its internal subchildren
            ListElements(book, "Chapter", "Name");
        }
    }
    qDebug() << "\nFinished Reading XML file.";

    return QCoreApplication::exec();
}