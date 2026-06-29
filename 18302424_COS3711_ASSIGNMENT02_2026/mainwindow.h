#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCreateShapeClicked();
    void onShapeSelectionChanged(const QString &shape);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    // Helper function to map combobox string to QColor
    QColor getColorFromName(const QString &name);
};
#endif // MAINWINDOW_H