#include "main.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>



int main(int argc, char *argv[]) {
    //QCoreApplication app(argc, argv);

    // Вывод русского языка в консоль
    QTextStream outStream(stdout);
    outStream.setCodec(QTextCodec::codecForName("cp866"));


    QVector<Error> errors;
    //C:/Users/pc/Desktop/FindShortestPath/test.dot
    //C:/Users/pc/Desktop/FindShortestPath/output.dot


    // Чтение входного файла
    outStream << QString("Введите путь к входному файлу:\n") << flush;
    QString inputPath;
    QTextStream in(stdin);
    inputPath = in.readLine();

    QStringList lines;
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        outStream << QString("Ошибка: не удалось открыть файл ") << inputPath << "\n";
        return 1;
    }

    // Чтение графа
    QTextStream inputStream(&inputFile);
    while (!inputStream.atEnd()) {
        lines.append(inputStream.readLine());
    }
    inputFile.close();

    // Построение матрицы смежности
    QMap<char, QMap<char, int>> siegeMatrix;
    if (!definingGraphBysiegeMatrix(siegeMatrix, lines, errors)) {
        outStream << QString("Ошибка при построении матрицы смежности\n");
        return 1;
    }

    // Ввод вершин для поиска пути
    outStream << QString("Введите начальную и конечную вершины (через пробел): ") << flush;
    QStringList vertices = in.readLine().split(' ');
    if (vertices.size() < 2) {
        outStream << QString("Необходимо указать две вершины\n");
        return 1;
    }

    QVector<char> path = {vertices[0].at(0).toLatin1(), vertices[1].at(0).toLatin1()};
    int distance = algoritmDejcstra(siegeMatrix, path);

    if (distance == -1) {
        outStream << QString("Путь между вершинами не найден\n");
        return 1;
    }

    // Генерация DOT-кода с путём
    QString dotCode = createDotWithPath(siegeMatrix, distance, path);

    // Сохранение в выходной файл
    outStream << QString("Введите путь для сохранения выходного DOT-файла: ") << flush;
    QString outputPath = in.readLine();

    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        outStream << QString("Ошибка: не удалось создать файл ") << outputPath << "\n";
        return 1;
    }

    QTextStream outputStream(&outputFile);
    outputStream << dotCode;
    outputFile.close();

    outStream << QString("Граф с выделенным путём успешно сохранён в ") << outputPath << "\n";
    outStream << QString("Кратчайшее расстояние: ") << distance << "\n";
    outStream << QString("Путь: ");
    for (char vertex : path) {
        outStream << vertex << " ";
    }
    outStream << "\n";

    return 0;


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
        QRegExp rx("([A-Za-z0-9]+)\\s*->\\s*([A-Za-z0-9]+)\\s*(?:\\[label\\s*=\\s*(\\d+)\\])?;?");
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
                continue;
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
    // Проверяем, что в path есть начальная и конечная вершины
    if (path.size() < 2) {
        qDebug() << "Ошибка: в path должны быть начальный и конечный город";
        return -1;
    }

    char start = path[0];  // Начальный город
    char end = path[1];    // Конечный город
    path.clear();          // Очищаем для заполнения новым путём

    // Инициализация структур данных
    QMap<char, int> distances;          // Кратчайшие расстояния до вершин
    QMap<char, char> previous;          // Предыдущие вершины для восстановления пути
    QSet<char> visited;                 // Посещённые вершины
    QList<char> allVertices;            // Все вершины графа

    // Собираем все вершины графа
    for (auto it = siegeMatrix.begin(); it != siegeMatrix.end(); ++it) {
        allVertices.append(it.key());
        for (auto neighbor = it.value().begin(); neighbor != it.value().end(); ++neighbor) {
            if (!allVertices.contains(neighbor.key())) {
                allVertices.append(neighbor.key());
            }
        }
    }

    // Инициализируем расстояния (бесконечность для всех, кроме старта)
    for (char vertex : allVertices) {
        distances[vertex] = std::numeric_limits<int>::max();
    }
    distances[start] = 0;

    // Основной цикл алгоритма
    while (true) {
        // Находим непосещённую вершину с минимальным расстоянием
        char current = '\0';
        int minDistance = std::numeric_limits<int>::max();

        for (char vertex : allVertices) {
            if (!visited.contains(vertex) && distances[vertex] < minDistance) {
                minDistance = distances[vertex];
                current = vertex;
            }
        }

        // Если все вершины посещены или недостижимы
        if (current == '\0' || current == end) {
            break;
        }

        visited.insert(current);

        // Обновляем расстояния до соседей
        if (siegeMatrix.contains(current)) {
            for (auto neighbor = siegeMatrix[current].begin(); neighbor != siegeMatrix[current].end(); ++neighbor) {
                char next = neighbor.key();
                int edgeWeight = neighbor.value();

                if (!visited.contains(next)) {
                    int newDistance = distances[current] + edgeWeight;
                    if (newDistance < distances[next]) {
                        distances[next] = newDistance;
                        previous[next] = current;
                    }
                }
            }
        }
    }

    // Восстанавливаем путь, если он существует
    if (distances[end] == std::numeric_limits<int>::max()) {
        qDebug() << "Пути между" << start << "и" << end << "не существует";
        return -1;
    }

    // Восстановление пути от конца к началу
    char current = end;
    while (current != start) {
        path.prepend(current);
        current = previous[current];
    }
    path.prepend(start);

    // Возвращаем длину пути (сумму весов рёбер)
    return distances[end];
}

QString createDotWithPath(QMap<char, QMap<char, int>>& siegeMatrix, int value, QVector<char>& path) {
    QString dotString = "digraph G {\n";

    // 1. Добавляем все рёбра графа (серым цветом)
    for (auto it = siegeMatrix.begin(); it != siegeMatrix.end(); ++it) {
        char from = it.key();
        for (auto edge = it.value().begin(); edge != it.value().end(); ++edge) {
            char to = edge.key();
            int weight = edge.value();

            // Проверяем, является ли это ребро частью кратчайшего пути
            bool isPathEdge = false;
            for (int i = 0; i < path.size() - 1; i++) {
                if (path[i] == from && path[i+1] == to) {
                    isPathEdge = true;
                    break;
                }
            }

            if (isPathEdge) {
                // 2. Выделяем рёбра кратчайшего пути (красным цветом)
                dotString += QString("    %1 -> %2 [label=\"%3\", color=\"red\", penwidth=2.0];\n")
                                 .arg(from).arg(to).arg(weight);
            } else {
                // Обычные рёбра (серым цветом)
                dotString += QString("    %1 -> %2 [label=\"%3\", color=\"gray\"];\n")
                                 .arg(from).arg(to).arg(weight);
            }
        }
    }

    // 3. Добавляем подпись с длиной кратчайшего пути
    if (path.size() >= 2) {
        dotString += QString("    label=\"Кратчайшее расстояние между %1 и %2 равно %3\";\n")
                         .arg(path.first()).arg(path.last()).arg(value);
    }

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


