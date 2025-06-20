#include "test_defininggraphbysiegematrix.h"
#include "main.h"
#include <QtTest>

void TestGraphParser::testEmptyGraph()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {"digraph G {", "}"};

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 2);
    QCOMPARE(errors[0].type, emptyGraph);
    QCOMPARE(errors[1].type, noCityes);
}

void TestGraphParser::testNoCitiesDeclared()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 7]",
        "A -> U [label = 9]",
        "U -> D [label = 8]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(result); // Должен парситься нормально, города берутся из связей
    QVERIFY(errors.empty());
}

void TestGraphParser::testInvalidCityNames()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph d{",
        "Agd2 -> D [label = 7]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, nameOfCityes);
    QVERIFY(errors[0].stringValue.contains("Название города должно быть одним символом"));
}

void TestGraphParser::testInvalidDistanceFormat()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;

    // Тест 1: Отрицательное расстояние
    QStringList dotRecord1 = {
        "digraph d{",
        "A -> B [label = -10]",
        "}"
    };
    bool result1 = definingGraphBysiegeMatrix(matrix, dotRecord1, errors);
    QVERIFY(!result1);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, distanceCityes);


    // Тест 2: Буквы вместо числа
    errors.clear();
    QStringList dotRecord2 = {
        "digraph d{",
        "A -> B [label = aaaaaa]",
        "}"
    };
    bool result2 = definingGraphBysiegeMatrix(matrix, dotRecord2, errors);
    QVERIFY(!result2);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, distanceCityes);
    int i = 0;
    i++;
}

void TestGraphParser::testMissingDistance()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph d{",
        "A -> B",
        "B -> C [label=5]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, ArcWithoutDistance);
}

void TestGraphParser::testNoGraphDeclaration()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "A -> B [label=1]",
        "B -> C [label=2]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, noGraph);
}

void TestGraphParser::testGraphLoop()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> A [label=5]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, loopInGraph);
}

void TestGraphParser::testTableContentsError()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "dgaph {",
        "A -> B",
        "B -> C [label=5]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1); // Ошибка в объявлении графа и отсутствие связей
    QCOMPARE(errors[0].type, noGraph);
}

void TestGraphParser::testTooManyCities()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 1]",
        "B -> C [label = 1]",
        "A -> C [label = 15]",
        "A -> D [label = 6]",
        "A -> F [label = 10]",
        "A -> Q [label = 10]",
        "D -> Q [label = 10]",
        "F -> Q [label = 10]",
        "C -> Q [label = 3]",
        "Q -> U [label = 3]",
        "U -> I [label = 3]",
        "I -> O [label = 3]",
        "O -> M [label = 3]",
        "P -> G [label = 3]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(!result);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors[0].type, manyCityes);
}

void TestGraphParser::testMinimalGraph()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 1]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(result);
    QVERIFY(errors.empty());
    QCOMPARE(matrix.size(), 1);
    QVERIFY(matrix.contains('A'));
    QCOMPARE(matrix['A']['B'], 1);
}

void TestGraphParser::testDisconnectedGraph()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 1]",
        "C -> D [label = 1]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(result);
    QVERIFY(errors.empty());
    QCOMPARE(matrix.size(), 2);
    QVERIFY(matrix.contains('A'));
    QVERIFY(matrix.contains('C'));
    QCOMPARE(matrix['A']['B'], 1);
    QCOMPARE(matrix['C']['D'], 1);
}

void TestGraphParser::testCyclicGraph()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 1]",
        "B -> C [label = 2]",
        "C -> A [label = 5]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(result);
    QVERIFY(errors.empty());
    QCOMPARE(matrix.size(), 3);
    QCOMPARE(matrix['A']['B'], 1);
    QCOMPARE(matrix['B']['C'], 2);
    QCOMPARE(matrix['C']['A'], 5);
}

void TestGraphParser::testMaxCitiesGraph()
{
    QMap<char, QMap<char, int>> matrix;
    QVector<Error> errors;
    QStringList dotRecord = {
        "digraph G {",
        "A -> B [label = 1]",
        "B -> C [label = 1]",
        "A -> C [label = 15]",
        "A -> D [label = 6]",
        "A -> F [label = 10]",
        "A -> Q [label = 10]",
        "D -> Q [label = 10]",
        "F -> Q [label = 10]",
        "C -> Q [label = 3]",
        "Q -> U [label = 3]",
        "U -> I [label = 3]",
        "I -> O [label = 3]",
        "O -> M [label = 3]",
        "}"
    };

    bool result = definingGraphBysiegeMatrix(matrix, dotRecord, errors);

    QVERIFY(result);
    QVERIFY(errors.empty());
    QCOMPARE(matrix.size(), 9); // A, B, C, D, F, Q, U, I, O (M только как цель)
}
