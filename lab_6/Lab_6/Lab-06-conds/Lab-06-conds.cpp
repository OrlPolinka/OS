#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <ctime>

using namespace std;
using namespace chrono;

CRITICAL_SECTION cs;
CONDITION_VARIABLE notFull;       // условная переменная: буфер не полон
CONDITION_VARIABLE notEmpty;      // условная переменная: буфер не пуст

vector<int> buffer; // кольцевой буфер
int maxBufferSize = 0;

int producedTotal = 0;            // всего произведено
int consumedTotal = 0;            // всего потреблено
int producedLast = 0;             // произведено за последние 5 сек
int consumedLast = 0;             // потреблено за последние 5 сек
int bufferSize = 0;               

double producerTimeTotal = 0;     // общее время работы производителя
double consumerTimeTotal = 0;     // общее время работы потребителя
steady_clock::time_point startTime;

// диапазоны задержек
int prodDelayMin = 0, prodDelayMax = 0;
int consDelayMin = 0, consDelayMax = 0;

bool running = true;

// Генерация случайной задержки в заданном диапазоне
int getRandomDelay(int minMs, int maxMs) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(minMs, maxMs);
    return dis(gen);
}

// Поток производителя
void producer() {
    while (running) {
        int delay = getRandomDelay(prodDelayMin, prodDelayMax);
        this_thread::sleep_for(milliseconds(delay));
        auto t1 = steady_clock::now();

        EnterCriticalSection(&cs);
        while (bufferSize == maxBufferSize) {
            cout << "[Producer] Буфер полон. Ожидание...\n";
            SleepConditionVariableCS(&notFull, &cs, INFINITE);
            cout << "[Producer] Пробуждение.\n";
        }

        buffer.push_back(++producedTotal);
        bufferSize++;
        producedLast++;
        cout << "[Producer] Добавлен элемент. Размер буфера: " << bufferSize << "\n";

        WakeConditionVariable(&notEmpty);   // будим потребителя
        LeaveCriticalSection(&cs);          // выход из критической зоны

        auto t2 = steady_clock::now();
        producerTimeTotal += duration<double>(t2 - t1).count();
    }
}

// Поток потребителя
void consumer() {
    while (running) {
        int delay = getRandomDelay(consDelayMin, consDelayMax);
        this_thread::sleep_for(milliseconds(delay));
        auto t1 = steady_clock::now();

        EnterCriticalSection(&cs);
        while (bufferSize == 0) {
            cout << "[Consumer] Буфер пуст. Ожидание...\n";
            SleepConditionVariableCS(&notEmpty, &cs, INFINITE);
            cout << "[Consumer] Пробуждение.\n";
        }

        int item = buffer.back();
        buffer.pop_back();
        bufferSize--;
        consumedTotal++;
        consumedLast++;
        cout << "[Consumer] Извлечён элемент. Размер буфера: " << bufferSize << "\n";

        WakeConditionVariable(&notFull);
        LeaveCriticalSection(&cs);

        auto t2 = steady_clock::now();
        consumerTimeTotal += duration<double>(t2 - t1).count();
    }
}

void statistics() {
    while (running) {
        this_thread::sleep_for(seconds(5));
        auto now = steady_clock::now();
        double elapsed = duration<double>(now - startTime).count();

        EnterCriticalSection(&cs);

        // вычисление метрик
        double prodSpeed = producedLast / 5.0;
        double consSpeed = consumedLast / 5.0;
        double prodAvg = producedLast ? producerTimeTotal / producedLast : 0;
        double consAvg = consumedLast ? consumerTimeTotal / consumedLast : 0;
        double fillPercent = (double)bufferSize / maxBufferSize * 100.0;

        cout << "\n===== Статистика =====\n";
        cout << "Время работы: " << elapsed << " сек\n";
        cout << "Произведено всего: " << producedTotal << "\n";
        cout << "Потреблено всего: " << consumedTotal << "\n";
        cout << "Текущий размер буфера: " << bufferSize << "\n";
        cout << "Заполнение буфера: " << fillPercent << "%\n";
        cout << "Произведено за период: " << producedLast << "\n";
        cout << "Среднее время производства: " << prodAvg << " сек\n";
        cout << "Скорость производства: " << prodSpeed << " эл/сек\n";
        cout << "Общее время работы производителя: " << producerTimeTotal << " сек\n";
        cout << "Потреблено за период: " << consumedLast << "\n";
        cout << "Среднее время потребления: " << consAvg << " сек\n";
        cout << "Скорость потребления: " << consSpeed << " эл/сек\n";
        cout << "Общее время работы потребителя: " << consumerTimeTotal << " сек\n";
        cout << "=======================\n\n";

        // сброс счетчиков
        producedLast = 0;
        consumedLast = 0;
        LeaveCriticalSection(&cs);
    }
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "Russian");

    if (argc != 6) {
        cout << "Использование: Lab-06-conds [размер буфера] [minProd] [maxProd] [minCons] [maxCons]\n";
        return 1;
    }

    maxBufferSize = atoi(argv[1]);
    prodDelayMin = atoi(argv[2]);
    prodDelayMax = atoi(argv[3]);
    consDelayMin = atoi(argv[4]);
    consDelayMax = atoi(argv[5]);

    char username[256];
    DWORD size = 256;
    GetUserNameA(username, &size);

    cout << "Пользователь: " << username << "\n";
    cout << "Буфер инициализирован. Размер: 0 / " << maxBufferSize << "\n";

    // инициализация синхронизации
    InitializeCriticalSection(&cs);
    InitializeConditionVariable(&notFull);
    InitializeConditionVariable(&notEmpty);

    startTime = steady_clock::now();

    thread t1(producer);
    thread t2(consumer);
    thread t3(statistics);

    cout << "Нажмите Enter для завершения...\n";
    cin.get();
    running = false;

    // пробуждение всех потоков
    WakeAllConditionVariable(&notFull);
    WakeAllConditionVariable(&notEmpty);

    t1.join();
    t2.join();
    t3.join();

    DeleteCriticalSection(&cs);
    return 0;
}
