#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);


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
        return; // Exit if we lack permissions to make the folder
    }

    // --- 4. Write the XML to the File ---
    QFile file(filePath);

    // Open the file in WriteOnly mode, formatting as Text
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() <<  "Finished to open file for writing";
        return;

    } else {
        QTextStream stream (&file);
        stream << document.toString();
        file.close();
        qDebug() << "Finished";
    }

    // create the model
    Filename = filePath;
    model = new QStandardItemModel(0,1,this);

    ReadFile();

    ui->treeView->setModel(model);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::ReadFile()
{

}

void Dialog::WriteFile()
{

}

void Dialog::on_pushButton_clicked()
{
    // save the document
}

