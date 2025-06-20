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

    
}

int algoritmDejcstra(const QMap<char, QMap<char, int>>& siegeMatrix, QVector<char>& path) {
    
}

QString createDotWithPath(QMap<char, QMap<char, int>>& siegeMatrix, int value, QVector<char>& path) {

}


