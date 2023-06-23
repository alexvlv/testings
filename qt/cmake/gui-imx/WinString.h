#pragma once

#include <QTextCodec>
#include <QString>

class WinString {

    static QTextCodec* codec()
    {
        static QTextCodec *codec = QTextCodec::codecForName("cp1251");
        return codec;
    }

public:
    static QString fromLocal8Bit(const char *str, int size = -1)
    {
        return codec()->toUnicode(str,size);
    }
    static QString fromLocal8Bit(const QByteArray& ba)
    {
        return codec()->toUnicode(ba);
    }
};
