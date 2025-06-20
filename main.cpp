#include "main.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QTextCodec>



int main(int argc, char *argv[]) {
    //QCoreApplication app(argc, argv);

    // Вывод русского языка в консоль
    QTextStream outStream(stdout);
    outStream.setCodec(QTextCodec::codecForName("cp866"));



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
    if (!definingGraphBysiegeMatrix(siegeMatrix, lines)) {
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


bool definingGraphBysiegeMatrix(QMap<char, QMap<char, int>> & siegeMatrix, QStringList & dotRecord) {

    // Очищаем матрицу смежности, чтобы начать с чистого листа.
        siegeMatrix.clear();

        // Обрабатываем строки dotRecord.
        for (const QString & line : dotRecord) {
            // Игнорируем пустые строки и строки, не содержащие ребер.
            if (line.isEmpty() || !line.contains("->")) {
                continue;
            }

            // Извлекаем информацию о ребре с помощью регулярных выражений.
            QRegExp rx("([A-Za-z0-9]+)\\s*->\\s*([A-Za-z0-9]+)\\s*\\[label\\s*=\\s*(\\d+)\\]");
            int pos = 0;

            while ((pos = rx.indexIn(line, pos)) != -1) {
                QString fromStr = rx.cap(1);
                QString toStr = rx.cap(2);
                QString weightStr = rx.cap(3);

                // Проверяем, что извлеченные строки не пусты.
                if (fromStr.isEmpty() || toStr.isEmpty() || weightStr.isEmpty()) {
                    qDebug() << "Ошибка: Неверный формат строки: " << line;
                    return false; // Или обработайте ошибку по-другому.
                }

                // Преобразуем строки в соответствующие типы данных.
                char from = fromStr.at(0).toLatin1();
                char to = toStr.at(0).toLatin1();
                int weight = weightStr.toInt();

                // Проверяем, что преобразование прошло успешно.
                if (weight == 0 && weightStr != "0") {  // Проверка на ошибку int.
                    qDebug() << "Ошибка: Неверный формат веса в строке: " << line;
                    return false; // Или обработайте ошибку по-другому.
                }

                // Заполняем матрицу смежности.
                if (!siegeMatrix.contains(from)) {
                    siegeMatrix.insert(from, QMap<char, int>());
                }
                siegeMatrix[from].insert(to, weight);
                pos += rx.matchedLength();
            }
        }

        return true;
}

int algoritmDejcstra(const QMap<char, QMap<char, int>>& siegeMatrix, QVector<char>& path) {
    
}

QString createDotWithPath(QMap<char, QMap<char, int>>& siegeMatrix, int value, QVector<char>& path) {

}


