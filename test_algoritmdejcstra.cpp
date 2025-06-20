#include "test_algoritmdejcstra.h"
#include "main.h"
#include <QtTest>
#include <limits>

// Реализация функции createTestMatrix в .cpp файле
QMap<char, QMap<char, int>> TestDijkstraAlgorithm::createTestMatrix(const QVector<QVector<int>>& data, const QVector<char>& cities)
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

void TestDijkstraAlgorithm::testMinimalGraph()
{
    /* Тест 1: Минимальное количество городов */
    QVector<char> cities = {'A', 'B'};
    QVector<QVector<int>> matrixData = {
        {-1, 1},
        {-1, -1}
    };

    auto siegeMatrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'B'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    QCOMPARE(distance, 1);
    QCOMPARE(path, QVector<char>({'A', 'B'}));
}

void TestDijkstraAlgorithm::testMultiplePathsSelectShortest()
{
    /* Тест 3: Несколько путей, выбор любого кратчайшего */
    QVector<char> cities = {'A', 'B', 'C', 'D', 'F', 'Q'};
    QVector<QVector<int>> matrixData = {
        {-1, 5, 15, 6, 5, 10},
        {-1, -1, 2, -1, -1, -1},
        {-1, -1, -1, -1, -1, 3},
        {-1, -1, -1, -1, -1, 10},
        {-1, -1, -1, -1, -1, 5},
        {-1, -1, -1, -1, -1, -1}
    };

    auto siegeMatrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'Q'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    // Все возможные кратчайшие пути с длиной 10
    QVector<QVector<char>> validPaths = {
        {'A', 'B', 'C', 'Q'},  // 5 + 2 + 3 = 10
        {'A', 'F', 'Q'},         // 5 + 5 = 10
        {'A', 'Q'}               // 10
    };

    // Проверяем длину пути
    QCOMPARE(distance, 10);

    // Проверяем, что найденный путь есть среди допустимых
    bool pathIsValid = false;
    for (const auto& validPath : validPaths) {
        if (path == validPath) {
            pathIsValid = true;
            break;
        }
    }

    QVERIFY2(pathIsValid, "Найденный путь не является одним из кратчайших");


}

void TestDijkstraAlgorithm::testNoPathExists()
{
    /* Тест 4: Отсутствует путь между городами */
    QVector<char> cities = {'A', 'B', 'C', 'D'};
    QVector<QVector<int>> matrixData = {
        {-1, 1, -1, -1},
        {-1, -1, -1, -1},
        {-1, -1, -1, 1},
        {-1, -1, -1, -1}
    };

    auto siegeMatrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'D'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    QCOMPARE(distance, -1);
    QVERIFY(path.isEmpty());
}

void TestDijkstraAlgorithm::testIsolatedStartCity()
{
    /* Тест 5: Город отправления изолированный */
    QMap<char, QMap<char, int>> siegeMatrix; // Пустая матрица
    QVector<char> path = {'A', 'D'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    QCOMPARE(distance, -1);
    QVERIFY(path.isEmpty());
}

void TestDijkstraAlgorithm::testGraphWithCycles()
{
    /* Тест 6: В графе есть циклы */
    QVector<char> cities = {'A', 'B', 'C', 'D'};
    QVector<QVector<int>> matrixData = {
        {-1, 4, -1, -1},
        {-1, -1, 1, -1},
        {-1, -1, -1, 3},
        {3, -1, -1, -1}
    };

    auto siegeMatrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'D'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    QCOMPARE(distance, 8);
    QCOMPARE(path, QVector<char>({'A', 'B', 'C', 'D'}));
}

void TestDijkstraAlgorithm::testLargeGraph()
{
    /* Тест 2: Большой размер графа */
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

    auto siegeMatrix = createTestMatrix(matrixData, cities);
    QVector<char> path = {'A', 'M'};

    int distance = algoritmDejcstra(siegeMatrix, path);

    QCOMPARE(distance, 17);
    QCOMPARE(path, QVector<char>({'A', 'B', 'C', 'Q', 'U', 'I', 'O', 'M'}));
}


//QTEST_APPLESS_MAIN(TestDijkstraAlgorithm)
//#include "test_algoritmdejcstra.moc"
