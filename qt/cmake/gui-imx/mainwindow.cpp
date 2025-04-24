#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "WinString.h"

#include <QTextCodec>
#include <QByteArray>
#include <QFile>

#include <cassert>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QTextCodec *wincodec = QTextCodec::codecForName("cp1251");
    assert(wincodec);
    QTextCodec::setCodecForLocale(wincodec);

    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->setupUi(this);
    setWindowState(Qt::WindowFullScreen);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
/*
    "windows-1251"
    "cp1251"
    "ANSI1251"
    "windows-1251"
    "cp1251"
    "ANSI1251"
*/

    QList<QByteArray> codecs = QTextCodec::availableCodecs();
    foreach (auto codec, codecs) {
        if(codec.contains("1251"))
            qDebug() << codec;
    }

    QFile f("/mnt/nfs/1251.txt" );
    if( f.open(QIODevice::ReadOnly|QIODevice::Text) ) {
        QByteArray wintext = f.readAll();
        qDebug() << "read" << wintext.size() << "bytes";
        QString txt = wincodec->toUnicode(wintext);
        ui->labelWin->setText(txt);
        QString txtOrig = QString::fromLocal8Bit(wintext);
        ui->labelWinOrig->setText(txtOrig);
        ui->labelWinString->setText(WinString::fromLocal8Bit(wintext));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    close();
}

