#ifndef TEST_GRAPH_PARSER_H
#define TEST_GRAPH_PARSER_H

#include <QObject>
#include <QMap>
#include <QVector>


class TestGraphParser : public QObject
{
    Q_OBJECT
private slots:
    void testEmptyGraph();
    void testNoCitiesDeclared();
    void testInvalidCityNames();
    void testInvalidDistanceFormat();
    void testMissingDistance();
    void testNoGraphDeclaration();
    void testGraphLoop();
    void testTableContentsError();
    void testTooManyCities();
    void testMinimalGraph();
    void testDisconnectedGraph();
    void testCyclicGraph();
    void testMaxCitiesGraph();
};

#endif // TEST_GRAPH_PARSER_H
