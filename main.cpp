#include "main.h"
#include "test_algoritmdejcstra.h"
#include "test_createdotwithpath.h"
#include "test_defininggraphbysiegematrix.h"


#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[]) {
// Настройка кодировки для Windows
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // Режим работы с файлами (если передано 2 аргумента)
    if (argc == 3) {
        QString inputPath = argv[1];
        QString outputPath = argv[2];
        QVector<Error> errors;

        // Чтение и обработка графа
        QStringList lines;
        QFile inputFile(inputPath);
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Ошибка: не удалось открыть файл" << inputPath;
            return 1;
        }

        QTextStream inputStream(&inputFile);
        while (!inputStream.atEnd()) {
            lines.append(inputStream.readLine());
        }
        inputFile.close();

        // Построение матрицы смежности
        QMap<char, QMap<char, int>> siegeMatrix;
        if (!definingGraphBysiegeMatrix(siegeMatrix, lines, errors)) {
            qDebug() << "Ошибки при построении графа:";
            for (const Error& error : errors) {
                qDebug() << error.stringValue;
            }
            return 1;
        }

        // Получаем начальную и конечную вершины из последней строки
        if (lines.isEmpty()) {
            qDebug() << "Файл пуст";
            return 1;
        }

        QString lastLine = lines.last().trimmed();
        QStringList vertices = lastLine.split(' ', QString::SplitBehavior::SkipEmptyParts);
        if (vertices.size() < 2) {
            qDebug() << "Не указаны начальная и конечная вершины";
            return 1;
        }

        QVector<char> path = {vertices[0].at(0).toUpper().toLatin1(),
                              vertices[1].at(0).toUpper().toLatin1()};

        // Поиск кратчайшего пути
        int distance = algoritmDejcstra(siegeMatrix, path);
        if (distance == -1) {
            qDebug() << "Путь между вершинами не найден";
            return 1;
        }

        // Сохранение результата
        QString dotCode = createDotWithPath(siegeMatrix, distance, path);
        QFile outputFile(outputPath);
        if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Ошибка: не удалось создать файл" << outputPath;
            return 1;
        }

        QTextStream outputStream(&outputFile);
        outputStream.setCodec("UTF-8");
        outputStream << dotCode;
        outputFile.close();

        qDebug() << "Результаты работы программы:";
        qDebug() << "Кратчайший путь сохранен в:" << outputPath;
        qDebug() << "Расстояние:" << distance;
        qDebug() << "Путь:" << path;

        // Подготовка аргументов для тестов
        int test_argc = 1;
        char* test_argv[] = { argv[0] };

        qDebug() << "\nЗапуск модульных тестов...";

        QVector<QObject*> tests = {
            new TestDijkstraAlgorithm,
            new TestCreateDotWithPath,
            new TestGraphParser
        };

        int failedTests = 0;
        for (QObject* test : tests) {
            qDebug() << "\n----- Запуск" << test->metaObject()->className() << "-----";
            failedTests += QTest::qExec(test, test_argc, test_argv);
            delete test;
        }

        qDebug() << "\nТесты завершены. Количество ошибок:" << failedTests;
        return failedTests > 0 ? 1 : 0;
    }

    // Режим запуска только тестов
    if (argc == 1) {
        return QTest::qExec(new TestDijkstraAlgorithm, argc, argv) |
               QTest::qExec(new TestCreateDotWithPath, argc, argv) |
               QTest::qExec(new TestGraphParser, argc, argv);
    }

    qDebug() << "Использование:";
    qDebug() << "Для работы с файлами:" << argv[0] << "input.dot output.dot";
    return 1;
}


bool definingGraphBysiegeMatrix(QMap<char, QMap<char, int>>& siegeMatrix,
                                const QStringList& dotRecord,
                                QVector<Error>& errors) {
    siegeMatrix.clear();
    QSet<char> cities;
    bool hasGraphDeclaration = false;
    bool hasEdges = false;
    int lineNumber = 0;

    // Проверка структуры файла
    for (const QString& line : dotRecord) {
        lineNumber++;
        QString trimmedLine = line.trimmed();

        if (trimmedLine.contains("digraph")) {
            hasGraphDeclaration = true;

            // Проверка открывающей скобки
            if (!trimmedLine.contains("{")) {
                int pos = line.indexOf("digraph");
                errors.append(Error(
                    TableContentsError,
                    QString("Отсутствует открывающая скобка '{' после объявления графа. Ожидается: 'digraph G {'"),
                    pos + 7  // Позиция после слова 'digraph'
                    ));
            }

            // Проверка закрывающей скобки (в последней строке)
            if (lineNumber == dotRecord.size() && !trimmedLine.contains("}")) {
                errors.append(Error(
                    TableContentsError,
                    "Отсутствует закрывающая скобка '}' в конце файла",
                    line.length() - 1
                    ));
            }
        }
    }

    if (!hasGraphDeclaration) {
        errors.append(Error(
            noGraph,
            "Файл не содержит объявления графа. Первая строка должна содержать 'digraph G {'",
            0
            ));
        return false;
    }

    // Обработка ребер графа
    lineNumber = 0;
    for (const QString& line : dotRecord) {
        lineNumber++;
        QString trimmedLine = line.trimmed();

        if (trimmedLine.isEmpty() || trimmedLine.startsWith("//") ||
            trimmedLine.startsWith("#")) {
            continue;
        }

        if (!trimmedLine.contains("->")) {
            // Проверяем, не является ли строка объявлением города без связей
            QRegExp cityRx("^[A-Za-z0-9]+;$");
            if (cityRx.exactMatch(trimmedLine)) {
                char city = trimmedLine[0].toUpper().toLatin1();
                cities.insert(city);
            }
            continue;
        }

        hasEdges = true;
        QRegExp rx("([A-Za-z0-9]+)\\s*->\\s*([A-Za-z0-9]+)\\s*(?:\\[label\\s*=\\s*([^\\]]+)\\])?;?");
        int pos = 0;

        while ((pos = rx.indexIn(line, pos)) != -1) {
            QString fromStr = rx.cap(1);
            QString toStr = rx.cap(2);
            QString weightStr = rx.cap(3);
            int arrowPos = line.indexOf("->", pos);
            int labelPos = line.indexOf("[label", pos);

            // Проверка формата строки
            if (fromStr.isEmpty() || toStr.isEmpty()) {
                int errPos = fromStr.isEmpty() ? pos : pos + fromStr.length() + 1;
                errors.append(Error(
                    graphLinks,
                    QString("Некорректный формат связи. Ожидается: 'Город1 -> Город2 [label=расстояние]'. Ошибка в %1")
                        .arg(fromStr.isEmpty() ? "начальном городе" : "конечном городе"),
                    errPos
                    ));
                pos += rx.matchedLength();
                continue;
            }

            // Проверка длины названий городов
            if (fromStr.length() != 1 || toStr.length() != 1) {
                QString badCity = fromStr.length() != 1 ? fromStr : toStr;
                int errPos = fromStr.length() != 1 ? pos : pos + fromStr.length() + 4;
                errors.append(Error(
                    nameOfCityes,
                    QString("Название города должно быть одним символом. Некорректное имя: '%1'").arg(badCity),
                    errPos
                    ));
                pos += rx.matchedLength();
                //continue;
            }

            char from = fromStr[0].toUpper().toLatin1();
            char to = toStr[0].toUpper().toLatin1();
            cities.insert(from);
            cities.insert(to);

            // Проверка на петлю
            if (from == to) {
                errors.append(Error(
                    loopInGraph,
                    QString("Обнаружена петля: город '%1' ссылается сам на себя").arg(from),
                    arrowPos
                    ));
            }

            // Проверка наличия веса
            if (weightStr.isEmpty()) {
                errors.append(Error(
                    ArcWithoutDistance,
                    QString("Отсутствует расстояние для связи '%1 -> %2'. Ожидается: '[label=расстояние]'")
                        .arg(from).arg(to),
                    labelPos != -1 ? labelPos : line.length() - 1
                    ));
                pos += rx.matchedLength();
                continue;
            }

            // Проверка корректности веса
            bool ok;
            int weight = weightStr.toInt(&ok);
            if (!ok || weight <= 0) {
                errors.append(Error(
                    distanceCityes,
                    QString("Некорректное расстояние '%1' для связи '%2 -> %3'. Должно быть положительным целым числом")
                        .arg(weightStr).arg(from).arg(to),
                    line.indexOf(weightStr, labelPos)
                    ));
                pos += rx.matchedLength();
                continue;
            }

            // Проверка на дублирование связей
            if (siegeMatrix.contains(from) && siegeMatrix[from].contains(to)) {
                errors.append(Error(
                    graphLinks,
                    QString("Дублирование связи: '%1 -> %2' уже существует с расстоянием %3")
                        .arg(from).arg(to).arg(siegeMatrix[from][to]),
                    arrowPos
                    ));
            }

            // Добавление в матрицу смежности
            if (!siegeMatrix.contains(from)) {
                siegeMatrix.insert(from, QMap<char, int>());
            }
            siegeMatrix[from].insert(to, weight);
            pos += rx.matchedLength();
        }
    }

    // Дополнительные проверки после обработки всех строк
    if (cities.size() > 10) {
        QStringList cityList;
        for (char city : cities) cityList << QString("'%1'").arg(city);

        errors.append(Error(
            manyCityes,
            QString("Превышено максимальное количество городов (%1). Обнаружены: %2")
                .arg(cities.size()).arg(cityList.join(", ")),
            -1  // Для ошибок файла в целом
            ));
    }

    if (!hasEdges) {
        errors.append(Error(
            emptyGraph,
            "Граф не содержит ни одной связи между городами",
            -1
            ));
    }

    if (cities.isEmpty()) {
        errors.append(Error(
            noCityes,
            "Граф не содержит объявленных городов",
            -1
            ));
    }

    return errors.empty();
}

int algoritmDejcstra(const QMap<char, QMap<char, int>>& siegeMatrix, QVector<char>& path) {

    char start = path[0];
    char end = path[1];
    path.clear();

    // Инициализация структур данных
    QMap<char, int> distances;
    QMap<char, char> previous;
    QSet<char> visited;
    QSet<char> allVertices;  // Используем QSet для автоматического избегания дубликатов

    // Собираем все вершины графа
    for (auto it = siegeMatrix.constBegin(); it != siegeMatrix.constEnd(); ++it) {
        allVertices.insert(it.key());
        for (auto neighbor = it.value().constBegin(); neighbor != it.value().constEnd(); ++neighbor) {
            allVertices.insert(neighbor.key());
        }
    }

    // Проверка существования вершин в графе
    if (!allVertices.contains(start) || !allVertices.contains(end)) {
        qDebug() << "Один из городов отсутствует в графе";
        return -1;
    }

    // Инициализация расстояний
    for (char vertex : allVertices) {
        distances[vertex] = std::numeric_limits<int>::max();
    }
    distances[start] = 0;

    // Основной цикл алгоритма
    while (true) {
        char current = '\0';
        int minDistance = std::numeric_limits<int>::max();

        // Поиск непосещенной вершины с минимальным расстоянием
        for (char vertex : allVertices) {
            if (!visited.contains(vertex) && distances[vertex] < minDistance) {
                minDistance = distances[vertex];
                current = vertex;
            }
        }

        // Условия выхода из цикла
        if (current == '\0') {
            qDebug() << "Невозможно достичь города" << end << "из города" << start;
            return -1;
        }
        if (current == end) {
            break;
        }

        visited.insert(current);

        // Обновление расстояний до соседей
        if (siegeMatrix.contains(current)) {
            for (auto neighbor = siegeMatrix[current].constBegin(); neighbor != siegeMatrix[current].constEnd(); ++neighbor) {
                char next = neighbor.key();
                if (!visited.contains(next)) {
                    int newDistance = distances[current] + neighbor.value();
                    if (newDistance < distances[next]) {
                        distances[next] = newDistance;
                        previous[next] = current;
                    }
                }
            }
        }
    }

    // Восстановление пути
    char current = end;
    while (current != start) {
        if (!previous.contains(current)) {
            qDebug() << "Путь прерван - отсутствует связь между городами";
            return -1;
        }
        path.prepend(current);
        current = previous[current];
    }
    path.prepend(start);

    return distances[end];
}

QString createDotWithPath(QMap<char, QMap<char, int>>& siegeMatrix, int value, QVector<char>& path) {
    QString dotString = "digraph G {\n";

    // Добавляем только рёбра из кратчайшего пути
    for (int i = 0; i < path.size() - 1; ++i) {
        char from = path[i];
        char to = path[i + 1];

        if (siegeMatrix.contains(from) && siegeMatrix[from].contains(to)) {
            int weight = siegeMatrix[from][to];
            dotString += QString("    %1 -> %2 [label = %3]\n")
                             .arg(from).arg(to).arg(weight);
        }
    }

    // Добавляем подпись с длиной пути
    dotString += QString("    label = \"Кратчайшее расстояние: %1\"\n")
                     .arg(value);

    dotString += "}";

    return dotString;
}

void printAdjacencyMatrix(const QMap<char, QMap<char, int>> &siegeMatrix) {
    if (siegeMatrix.isEmpty()) {
        qDebug() << "Матрица смежности пуста!";
        return;
    }

    // Собираем все уникальные вершины (источники и цели)
    QSet<char> allVertices;
    for (auto it = siegeMatrix.begin(); it != siegeMatrix.end(); ++it) {
        allVertices.insert(it.key());
        const QMap<char, int> &targets = it.value();
        for (auto jt = targets.begin(); jt != targets.end(); ++jt) {
            allVertices.insert(jt.key());
        }
    }

    // Преобразуем в список и сортируем для красивого вывода
    QList<char> sortedVertices = allVertices.values();
    std::sort(sortedVertices.begin(), sortedVertices.end());

    // Выводим заголовок (вершины в столбцах)
    qDebug() << "Матрица смежности:";
    QString header = "      ";
    for (char vertex : sortedVertices) {
        header += QString("%1     ").arg(vertex);
    }
    qDebug().noquote() << header;

    // Выводим разделительную строку
    QString separator = "   +";
    for (int i = 0; i < sortedVertices.size(); ++i) {
        separator += "-----+";
    }
    qDebug().noquote() << separator;

    // Выводим содержимое матрицы
    for (char from : sortedVertices) {
        QString row = QString("%1 |").arg(from);

        for (char to : sortedVertices) {
            if (siegeMatrix.contains(from) && siegeMatrix[from].contains(to)) {
                row += QString(" %1  |").arg(siegeMatrix[from][to], 3);
            } else {
                row += "  -  |";
            }
        }
        qDebug().noquote() << row;
        qDebug().noquote() << separator;
    }
}


