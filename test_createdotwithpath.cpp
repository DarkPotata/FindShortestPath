#include "test_createdotwithpath.h"
#include "main.h"
#include <QtTest>

QMap<char, QMap<char, int>> TestCreateDotWithPath::createTestMatrix(
    const QVector<QVector<int>>& data, const QVector<char>& cities)
{
    QMap<char, QMap<char, int>> matrix;
    for (int i = 0; i < data.size(); ++i) {
        for (int j = 0; j < data[i].size(); ++j) {
            if (data[i][j] != -1) {
                matrix[cities[i]][cities[j]] = data[i][j];
            }
        }
    }
    return matrix;
}

void TestCreateDotWithPath::testMinimalGraph()
{
    // Тест 1: Минимальное количество городов
    QVector<char> cities = {'A', 'B'};
    QVector<QVector<int>> matrixData = {{-1, 1}, {-1, -1}};
    auto matrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'B'};
    int distance = 1;

    QString result = createDotWithPath(matrix, distance, path);
    QString expected = R"(digraph G {
    A -> B [label = 1]
    label = "Кратчайшее расстояние: 1"
})";

    QCOMPARE(result, expected);
}

void TestCreateDotWithPath::testMultiplePaths()
{
    // Тест 2: Несколько путей, выбор кратчайшего
    QVector<char> cities = {'A', 'B', 'C', 'Q'};
    QVector<QVector<int>> matrixData = {
        {-1, 5, 15, 10},
        {-1, -1, 2, -1},
        {-1, -1, -1, 3},
        {-1, -1, -1, -1}
    };
    auto matrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'B', 'C', 'Q'};
    int distance = 10;

    QString result = createDotWithPath(matrix, distance, path);
    QString expected = R"(digraph G {
    A -> B [label = 5]
    B -> C [label = 2]
    C -> Q [label = 3]
    label = "Кратчайшее расстояние: 10"
})";

    QCOMPARE(result, expected);
}

void TestCreateDotWithPath::testNoPath()
{
    // Тест 3: Отсутствует путь между городами
    QMap<char, QMap<char, int>> matrix;
    QVector<char> path;
    int distance = -1;

    QString result = createDotWithPath(matrix, distance, path);
    QString expected = R"(digraph G {
    label = "Кратчайшее расстояние: -1"
})";

    QCOMPARE(result, expected);
}

void TestCreateDotWithPath::testLargeGraph()
{
    // Тест 4: Большой граф (10 городов)
    QVector<char> cities = {'A', 'B', 'C', 'D', 'F', 'Q', 'U', 'I', 'O', 'M'};
    QVector<QVector<int>> matrixData = {
        {0, 1, 15, 6, 10, 10, -1, -1, -1, -1},
        {-1, 0, 1, -1, -1, -1, -1, -1, -1, -1},
        {-1, -1, 0, -1, -1, 3, -1, -1, -1, -1},
        {-1, -1, -1, 0, -1, 10, -1, -1, -1, -1},
        {-1, -1, -1, -1, 0, 10, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1, 0, 3, -1, -1, -1},
        {-1, -1, -1, -1, -1, -1, 0, 3, -1, -1},
        {-1, -1, -1, -1, -1, -1, -1, 0, 3, -1},
        {-1, -1, -1, -1, -1, -1, -1, -1, 0, 3},
        {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0}
    };
    auto matrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'B', 'C', 'Q', 'U', 'I', 'O', 'M'};
    int distance = 17;

    QString result = createDotWithPath(matrix, distance, path);
    QString expected = R"(digraph G {
    A -> B [label = 1]
    B -> C [label = 1]
    C -> Q [label = 3]
    Q -> U [label = 3]
    U -> I [label = 3]
    I -> O [label = 3]
    O -> M [label = 3]
    label = "Кратчайшее расстояние: 17"
})";

    QCOMPARE(result, expected);
}
