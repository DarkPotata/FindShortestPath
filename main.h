#include <QtCore/QCoreApplication>
#include <iostream>
#include <QString>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>
#include <limits>
#include <algorithm>
#include <QtTest/QtTest>


enum type_error {
    emptyFile,
    noGraph,
    noCityes,
    manyCityes,
    nameOfCityes,
    distanceCityes,
    emptyGraph,
    graphLinks,
    loopInGraph,
    ArcWithoutDistance,
    TableContentsError
};

struct Error
{
    enum type_error type;
    QString stringValue;
    int position_error;

    QString toString() const {
        QString errorType;
        switch(type) {
        case emptyFile: errorType = "Файл пустой"; break;
        case noGraph: errorType = "Во входном файле нет графа"; break;
        case noCityes: errorType = "Отсутствуют города (от и до)"; break;
        case manyCityes: errorType = "Слишком много городов (более 10)"; break;
        case nameOfCityes: errorType = "Ошибка в названии городов"; break;
        case distanceCityes: errorType = "Ошибка в расстоянии"; break;
        case emptyGraph: errorType = "Граф не содержит городов"; break;
        case graphLinks: errorType = "Ошибка связей графа"; break;
        case loopInGraph: errorType = "Петля в графе"; break;
        case ArcWithoutDistance: errorType = "Дуга без расстояния"; break;
        case TableContentsError: errorType = "Ошибка оглавления (digraph G и {})"; break;
        }
        return QString("Ошибка: %1\nПозиция: %2\nДоп. информация: %3")
            .arg(errorType)
            .arg(position_error)
            .arg(stringValue);
    }

    // Перегрузка операции ==
    bool operator==(const Error& other) const {
        return (type == other.type) &&
               (position_error == other.position_error) &&
               (stringValue == other.stringValue);
    }
};


bool definingGraphBysiegeMatrix(QMap<char, QMap<char, int>>& siegeMatrix, QStringList& dotRecord);
int algoritmDejcstra(const QMap<char, QMap<char, int>>& siegeMatrix, QVector<char>& path);
QString createDotWithPath(QMap<char, QMap<char, int>>& siegeMatrix, int value, QVector<char> & path);

