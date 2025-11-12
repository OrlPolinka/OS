#include <windows.h>      
#include <iostream>       
#include <vector>         
#include <thread>         
#include <random>         
#include <chrono>         

using namespace std;

vector<vector<vector<int>>> matrices;       // 3D-вектор: матрицы[N][M][M]
vector<vector<LONG>> resultMatrix;          // итоговая сумма матриц
int numThreads = 0, numMatrices = 0, matrixSize = 0;

SYNCHRONIZATION_BARRIER barrier;            // барьер для синхронизации этапов


int getRandomValue() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(-2000, 2000);
    return dis(gen);
}

// Потоковая функция
void worker(int threadId) {
    // Вычисляем диапазон матриц, которые обрабатывает этот поток
    int perThread = numMatrices / numThreads;
    int start = threadId * perThread;
    int end = (threadId == numThreads - 1) ? numMatrices : start + perThread;

    // Локальная матрица для частичной суммы
    vector<vector<int>> localSum(matrixSize, vector<int>(matrixSize, 0));

    //  расчёт частичных сумм
    for (int k = start; k < end; ++k) {
        for (int i = 0; i < matrixSize; ++i) {
            for (int j = 0; j < matrixSize; ++j) {
                localSum[i][j] += matrices[k][i][j];
            }
        }
    }

    // Барьер: ждём завершения всех потоков
    EnterSynchronizationBarrier(&barrier, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);

    // атомарное добавление в общую матрицу
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            InterlockedAdd(&resultMatrix[i][j], localSum[i][j]);
        }
    }

    // Барьер: ждём завершения свёртки
    EnterSynchronizationBarrier(&barrier, SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    if (argc != 4) {
        cout << "Использование: Lab-06-bar [кол-во потоков] [кол-во матриц] [размер матриц]\n";
        return 1;
    }

    numThreads = atoi(argv[1]);
    numMatrices = atoi(argv[2]);
    matrixSize = atoi(argv[3]);

    // Инициализация барьера
    InitializeSynchronizationBarrier(&barrier, numThreads, -1);

    // Генерация матриц
    matrices.resize(numMatrices, vector<vector<int>>(matrixSize, vector<int>(matrixSize)));
    for (int k = 0; k < numMatrices; ++k)
        for (int i = 0; i < matrixSize; ++i)
            for (int j = 0; j < matrixSize; ++j)
                matrices[k][i][j] = getRandomValue();

    // Инициализация итоговой матрицы
    resultMatrix.resize(matrixSize, vector<LONG>(matrixSize, 0));

    // Запуск потоков
    vector<thread> threads;
    for (int i = 0; i < numThreads; ++i)
        threads.emplace_back(worker, i);

    // Ожидание завершения
    for (auto& t : threads)
        t.join();

    cout << "\nСумма всех матриц:\n";
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            cout << resultMatrix[i][j] << "\t";
        }
        cout << "\n";
    }

    // Очистка барьера
    DeleteSynchronizationBarrier(&barrier);
    return 0;
}
