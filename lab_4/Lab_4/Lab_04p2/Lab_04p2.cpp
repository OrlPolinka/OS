#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>

const int MAX_GLOBAL = 1000000;
int globalPrimes[MAX_GLOBAL];
int globalIndex = 0;

// Thread-local буфер
__declspec(thread) std::vector<int> localBuffer;

struct PrimeArgs {
    int start;
    int end;
};

// Проверка на простое число
bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i <= sqrt(n); ++i)
        if (n % i == 0) return false;
    return true;
}

// Потоковая функция
DWORD WINAPI FindPrimes(LPVOID lpParam) {
    PrimeArgs* args = static_cast<PrimeArgs*>(lpParam);
    for (int i = args->start; i <= args->end; ++i) {
        if (isPrime(i)) {
            localBuffer.push_back(i);
        }
    }

    // Копирование в глобальный массив ( без синхронизации)
    for (int prime : localBuffer) {
        if (globalIndex < MAX_GLOBAL) {
            globalPrimes[globalIndex++] = prime;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc != 4) {
        std::cerr << "Использование: Lab-04p2 <кол-во потоков> <нижняя граница> <верхняя граница>\n";
        return 1;
    }

    int threadCount = atoi(argv[1]);
    int lower = atoi(argv[2]);
    int upper = atoi(argv[3]);

    if (threadCount <= 0 || lower >= upper) {
        std::cerr << "Неверные аргументы.\n";
        return 1;
    }

    int range = upper - lower + 1;
    int chunk = range / threadCount;
    int remainder = range % threadCount;

    std::vector<HANDLE> handles;
    std::vector<PrimeArgs> args(threadCount);

    int currentStart = lower;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < threadCount; ++i) {
        int currentEnd = currentStart + chunk - 1;
        if (i == threadCount - 1) currentEnd += remainder;

        args[i] = { currentStart, currentEnd };
        HANDLE hThread = CreateThread(nullptr, 0, FindPrimes, &args[i], 0, nullptr);
        if (!hThread) {
            std::cerr << "Ошибка при создании потока " << i << "\n";
            return 1;
        }
        handles.push_back(hThread);
        currentStart = currentEnd + 1;
    }

    WaitForMultipleObjects(handles.size(), handles.data(), TRUE, INFINITE);
    for (HANDLE h : handles) CloseHandle(h);

    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(endTime - startTime).count();

    std::sort(globalPrimes, globalPrimes + globalIndex);

    std::cout << "Найдено простых чисел: " << globalIndex << "\n";
    std::cout << "Время выполнения: " << duration << " секунд\n";

    return 0;
}
