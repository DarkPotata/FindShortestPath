#ifndef TEST_ALGORITMDEJCSTRA_H
#define TEST_ALGORITMDEJCSTRA_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QSet>
#include <QDebug>

class TestDijkstraAlgorithm : public QObject
{
    Q_OBJECT

private slots:
    void testMinimalGraph();
    void testMultiplePathsSelectShortest();
    void testNoPathExists();
    void testIsolatedStartCity();
    void testGraphWithCycles();
    void testLargeGraph();

private:
    // Только объявление, без определения
    QMap<char, QMap<char, int>> createTestMatrix(const QVector<QVector<int>>& data, const QVector<char>& cities);
};

#endif // TEST_ALGORITMDEJCSTRA_H
