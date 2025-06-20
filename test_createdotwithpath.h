// test_createdotwithpath.h
#ifndef TEST_CREATEDOTWITHPATH_H
#define TEST_CREATEDOTWITHPATH_H

#include <QObject>
#include <QMap>
#include <QVector>

class TestCreateDotWithPath : public QObject
{
    Q_OBJECT
private slots:
    void testMinimalGraph();
    void testMultiplePaths();
    void testNoPath();
    void testLargeGraph();

private:
    QMap<char, QMap<char, int>> createTestMatrix(const QVector<QVector<int>>& data, const QVector<char>& cities);
};
#endif
