#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QGLWidget>
#include <QWidget>
#include "mimetypes/mimetypes.h"

namespace Ui {
class MainWindow;
class DrawnWidget;
}
class DrawnWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawnWidget(QWidget *parent = 0);
    ~DrawnWidget() {}

    void setPixmap(QPixmap &pm);

protected:
    void paintEvent(QPaintEvent *evt);

private:
    QPixmap pm;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void dragEnterEvent(QDragEnterEvent *evt);
    void dropEvent(QDropEvent *evt);

private slots:
    void on_listWidget_currentTextChanged(const QString &currentText);
    void on_importButton_clicked();
    void on_exportButton_clicked();
    void on_listWidget_customContextMenuRequested(const QPoint &pos);
    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    DrawnWidget* dw;        // drawn preview widget
    QPixmap pm;             // preview pixmap
    QStringList qsl;        // working list mirror of widget data
    MimeTypes mimeTypeDb;
};




#endif // MAINWINDOW_H
