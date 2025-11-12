#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const std::string USERNAME = "User-c121f4dd";
const int ITERATIONS = 90;
const int DELAY_MS = 100;

CRITICAL_SECTION cs; // Критическая секция

// Структура для передачи параметров потока
struct ThreadArgs {
    std::string name;
    HANDLE waitFor1;
    HANDLE waitFor2;
    bool isMain;
};

// Функция потока
DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    printf("[%s] TID: %lu начал работу\n", args->name.c_str(), tid);

    bool inCritical = false;

    for (int i = 0; i < ITERATIONS; ++i) {
        if (i + 1 == 30) {
            EnterCriticalSection(&cs);
            std::cout << "[" << args->name << "] ВХОД в критическую зону\n";
            inCritical = true;
        }

        char letter = USERNAME[i % nameLen];
        printf("[%s] PID: %lu – TID: %lu – №%d – %c\n", args->name.c_str(), pid, tid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));

        if (i + 1 == 61 && inCritical) {
            std::cout << "[" << args->name << "] ВЫХОД из критической зоны\n";
            LeaveCriticalSection(&cs);
            inCritical = false;
        }
    }

    if (args->isMain) {
        printf("[Main] TID: %lu завершил итерации, ждёт дочерние потоки...\n", tid);
        WaitForSingleObject(args->waitFor1, INFINITE);
        WaitForSingleObject(args->waitFor2, INFINITE);
    }

    printf("[%s] TID: %lu завершил работу\n", args->name.c_str(), tid);
    return 0;
}

int main() {
    setlocale(LC_ALL, "ru");

    InitializeCriticalSection(&cs);

    // Аргументы для дочерних потоков
    ThreadArgs argsA{ "Поток A", nullptr, nullptr, false };
    ThreadArgs argsB{ "Поток B", nullptr, nullptr, false };

    HANDLE hThreadA = CreateThread(nullptr, 0, ThreadFunc, &argsA, 0, nullptr);
    HANDLE hThreadB = CreateThread(nullptr, 0, ThreadFunc, &argsB, 0, nullptr);

    if (!hThreadA || !hThreadB) {
        std::cerr << "Ошибка при создании потоков A и B\n";
        return 1;
    }

    // Аргументы для главного потока
    ThreadArgs argsMain{ "Main", hThreadA, hThreadB, true };

    HANDLE hMainThread = CreateThread(nullptr, 0, ThreadFunc, &argsMain, 0, nullptr);
    if (!hMainThread) {
        std::cerr << "Ошибка при создании главного потока\n";
        CloseHandle(hThreadA);
        CloseHandle(hThreadB);
        return 1;
    }

    WaitForSingleObject(hMainThread, INFINITE);

    // Очистка
    CloseHandle(hThreadA);
    CloseHandle(hThreadB);
    CloseHandle(hMainThread);
    DeleteCriticalSection(&cs);

    printf("Все потоки завершили работу. Программа завершена.\n");
    return 0;
}
