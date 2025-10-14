#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>

const int MAX_GLOBAL = 10000;
int globalPrimes[MAX_GLOBAL];
int globalIndex = 0;

DWORD tlsIndex;

// Структура аргументов
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
DWORD WINAPI L4Primes(LPVOID lpParam) {
    PrimeArgs* args = static_cast<PrimeArgs*>(lpParam);
    int start = args->start;
    int end = args->end;

    // Выделение TLS-буфера
    std::vector<int>* localBuffer = new std::vector<int>();
    TlsSetValue(tlsIndex, localBuffer);

    for (int i = start; i <= end; ++i) {
        if (isPrime(i)) {
            localBuffer->push_back(i);
        }
    }

    // Копирование в глобальный массив (без синхронизации!)
    for (int prime : *localBuffer) {
        if (globalIndex < MAX_GLOBAL) {
            globalPrimes[globalIndex++] = prime;
        }
    }

    delete localBuffer;
    return 0;
}

int main() {
    setlocale(LC_ALL, "ru");

    // Создание TLS индекса
    tlsIndex = TlsAlloc();
    if (tlsIndex == TLS_OUT_OF_INDEXES) {
        std::cerr << "Ошибка создания TLS индекса\n";
        return 1;
    }

    // Диапазоны для потоков
    PrimeArgs args1{ 2, 5000 };
    PrimeArgs args2{ 5001, 10000 };

    HANDLE hThread1 = CreateThread(nullptr, 0, L4Primes, &args1, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, L4Primes, &args2, 0, nullptr);

    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    TlsFree(tlsIndex);

    // Вывод результатов
    std::cout << "Найдено простых чисел: " << globalIndex << std::endl;
    for (int i = 0; i < globalIndex; ++i) {
        std::cout << globalPrimes[i] << " ";
        if ((i + 1) % 20 == 0) std::cout << std::endl;
    }

    return 0;
}
