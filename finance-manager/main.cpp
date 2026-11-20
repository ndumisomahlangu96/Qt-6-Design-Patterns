#include "mainwindow.h"

#include <QApplication>

/**
 * @brief Application entry point.
 *
 * Initializes the Qt application environment, constructs and displays the
 * MainWindow, and enters the main Qt event loop to handle user interactions.
 *
 * @param argc Command-line argument count.
 * @param argv Command-line argument values.
 * @return Exit status code returned by the Qt event loop.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}