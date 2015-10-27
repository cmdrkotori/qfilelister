#include <QFileDialog>
#include <QMimeData>
#include <QMimeDatabase>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    dw = new DrawnWidget(this);
    ui->pichost->layout()->addWidget(dw);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *evt)
{
    if (evt->mimeData()->hasUrls()) {
        evt->accept();
    } else {
        evt->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *evt)
{
    auto urls = evt->mimeData()->urls();
    foreach(QUrl url, urls) {
        qsl.append(url.toLocalFile());
        ui->listWidget->addItem(url.toLocalFile());
    }
}

void MainWindow::on_listWidget_currentTextChanged(const QString &currentText)
{
    pm.load(currentText);
    ui->width->setText(QString::number(pm.width()));
    ui->height->setText(QString::number(pm.height()));
    dw->setPixmap(pm);
    dw->update();
}

DrawnWidget::DrawnWidget(QWidget *parent) : QWidget(parent) {
    this->setMinimumSize(128,128);
}

void DrawnWidget::setPixmap(QPixmap &pm)
{
    this->pm = pm;
}

void DrawnWidget::paintEvent(QPaintEvent *evt)
{
    Q_UNUSED(evt);
    QPainter painter(this);
    int pw, ph;

    // draw background
    QPalette p = this->palette();
    painter.fillRect(QRect(0,0,width(),height()), QColor(0xffffff));
    painter.setPen(p.light().color());
    painter.setBrush(p.base());
    painter.drawRect(QRect(0,0,width()-1,height()-1));

    // do nothing if there's no pixmap loaded
    if (pm.isNull()) {
        return;
    }

    // calculate image area
    pw = pm.width();
    ph = pm.height();

    int aw = width() - 2;       // minus the 1px border
    int ah = height() - 2;

    int x, y, w, h;
    float ratio = (float)pw / (float)ph;

    // fit along most maximal dimension first,
    // then fallback to minimal fit if cropped
    auto fitWidth = [&]() {
        w = aw;
        h = w / ratio;
        x = 0;
        y = (ah - h)/2;
    };
    auto fitHeight = [&]() {
        h = ah;
        w = h * ratio;
        x = (aw - w)/2;
        y = 0;
    };
    if (ratio <= 1.0) {         // height is greater than width
        fitHeight();
        if (x < 0)
            fitWidth();
    } else {                    // width is greater than height
        fitWidth();
        if (y < 0)
            fitHeight();
    }

    // draw pixmap
    QRect r(QPoint(x+1,y+1),QSize(w,h));
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(r,pm);
}



void MainWindow::on_importButton_clicked()
{
    QFileDialog *qfd = new QFileDialog(this);
    qfd->setAttribute(Qt::WA_DeleteOnClose);
    qfd->setWindowTitle(tr("Import File"));
    connect(qfd, &QFileDialog::filesSelected, [this](QStringList items) {
        this->ui->listWidget->clear();
        this->qsl.clear();
        // foreach file, read all its items and add to working lists
        foreach (QString fname, items) {
            QFile f(fname);
            if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;
            else {
                QTextStream in(&f);
                QString line;
                while (true) {
                    line = in.readLine();
                    if (line.isNull())
                        break;
                    this->ui->listWidget->addItem(QString(line));
                    this->qsl.append(QString(line));
                }
            }
        }
    });
    qfd->show();
}

void MainWindow::on_exportButton_clicked()
{
    QFileDialog *qfd = new QFileDialog(this);
    qfd->setAttribute(Qt::WA_DeleteOnClose);
    qfd->setWindowTitle("Export File");
    connect(qfd, &QFileDialog::fileSelected, [this](QString item) {
        if (item.isEmpty())
            return;
        QFile f(item);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream out(&f);
        foreach(QString s, qsl)
            out << s << '\n';
    });
    qfd->show();
}

void MainWindow::on_listWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu m(this);
    QAction *a = new QAction(&m);
    a->setText("Remove");
    connect(a, &QAction::triggered, [this]() {
        int index = this->ui->listWidget->currentRow();
        if (index < 0)
            return;
        this->ui->listWidget->takeItem(index);
        this->qsl.takeAt(index);
    });
    qDebug() << ui->listWidget->mapToGlobal(pos);
    m.addAction(a);

    QMenu *openwith = m.addMenu("Open With");
    QString file = qsl.value(this->ui->listWidget->currentRow());
    QList<MimeTypeHandler *> apps = mimeTypeDb.appsFor(QMimeDatabase().mimeTypeForFile(file).name());
    foreach (MimeTypeHandler *app, apps) {
        a = new QAction(openwith);
        a->setText(app->name);
        connect(a, &QAction::triggered, [this, app, file]() {
            if (file.isNull())
                return;
            QString cmd = app->exec;
            if (cmd.contains(QRegExp("%[Uu]"))) {
                QString url = QString::fromLatin1(QUrl::fromLocalFile(file).toEncoded());
                cmd.replace(QRegExp("%[Uu]"), QString("\"%1\"").arg(url));
            }
            if (cmd.contains(QRegExp("%[Ff]")))
                cmd.replace(QRegExp("%[Ff]"), QString("\"%1\"").arg(file));

            QProcess *p = new QProcess();
            p->start(cmd);
            connect(p, static_cast<void (QProcess::*)(int)>(&QProcess::finished),
                    [p](int){ p->deleteLater(); });
        });
        openwith->addAction(a);
    }

    m.exec(ui->listWidget->mapToGlobal(pos));
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About File Lister",
       "<h2>File Lister</h2>"
       "<p>A simple way to make file lists"
       "<p>Based on Qt " QT_VERSION_STR
       "<p>Built on " __DATE__ " at " __TIME__
       "<h3>LICENSE</h3>"
       "<p>   Copyright (C) 2015"
       "<p>"
       "This program is free software; you can redistribute it and/or modify "
       "it under the terms of the GNU General Public License as published by "
       "the Free Software Foundation; either version 2 of the License, or "
       "(at your option) any later version."
       "<p>"
       "This program is distributed in the hope that it will be useful, "
       "but WITHOUT ANY WARRANTY; without even the implied warranty of "
       "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
       "GNU General Public License for more details."
       "<p>"
       "You should have received a copy of the GNU General Public License "
       "along with this program; if not, write to the Free Software "
       "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA "
       "02110-1301 USA.");
}
