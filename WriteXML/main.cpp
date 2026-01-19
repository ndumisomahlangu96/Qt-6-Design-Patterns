#include <QCoreApplication>
#include <QtXml>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Write XML
    QDomDocument document;

    // Make the root element
    QDomElement root = document.createElement("Books");

    // Add it to the document
    document.appendChild(root);

    // Add some elements
    for(int i = 0; i < 10; i++)
    {
        QDomElement node = document.createElement("Books");
        node.setAttribute("Name","My Book" + QString::number(i));
        node.setAttribute("ID",QString::number(i));
        root.appendChild(node);

        for(int i = 0; i < 10; i++)
        {
            QDomElement subnode = document.createElement("Chapter");
            subnode.setAttribute("Name","My Chapter" + QString::number(i));
            subnode.setAttribute("ID",QString::number(i));
            root.appendChild(subnode);
        }
    }

    // Write to file
    // CHANGED: Using a relative path saves the file in the application's working directory
    QFile file("WriteXML.xml");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing.";
        return -1;
    }
    else
    {
        QTextStream stream(&file);
        stream << document.toString();
        file.close();
        qDebug() << "Finished";
    }

    return a.exec();
}
