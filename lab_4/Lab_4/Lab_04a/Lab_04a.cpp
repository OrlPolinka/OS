#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const std::string USERNAME = "User-c121f4dd";

// Структура для передачи параметров потока
struct ThreadArgs {
    int iterations;
    HANDLE waitFor1;
    HANDLE waitFor2;
    bool isMain;
};

// Функция потока
DWORD WINAPI Lab_04x(LPVOID lpParam) {
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    int iterations = args->iterations;
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    printf("Поток TID: %lu начал работу\n", tid);

    for (int i = 0; i < iterations; ++i) {
        char letter = USERNAME[i % nameLen];
        printf("PID: %lu – TID: %lu – №%d – %c\n", pid, tid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }

    // Если это главный поток — ждём завершения дочерних
    if (args->isMain) {
        printf("Главный поток TID: %lu завершил итерации, ждёт дочерние потоки...\n", tid);
        WaitForSingleObject(args->waitFor1, INFINITE);
        WaitForSingleObject(args->waitFor2, INFINITE);
    }

    printf("Поток TID: %lu завершил работу\n", tid);
    return 0;
}

int main() {
    setlocale(LC_ALL, "ru");

    // Аргументы для дочерних потоков
    ThreadArgs args1{ 50, nullptr, nullptr, false };
    ThreadArgs args2{ 125, nullptr, nullptr, false };

    // Создание дочерних потоков
    HANDLE hThread1 = CreateThread(nullptr, 0, Lab_04x, &args1, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, Lab_04x, &args2, 0, nullptr);

    if (!hThread1 || !hThread2) {
        std::cerr << "Ошибка при создании дочерних потоков\n";
        return 1;
    }

    // Аргументы для главного потока
    ThreadArgs argsMain{ 100, hThread1, hThread2, true };

    // Создание главного потока
    HANDLE hMainThread = CreateThread(nullptr, 0, Lab_04x, &argsMain, 0, nullptr);
    if (!hMainThread) {
        std::cerr << "Ошибка при создании главного потока\n";
        CloseHandle(hThread1);
        CloseHandle(hThread2);
        return 1;
    }

    // Ожидание завершения главного потока
    WaitForSingleObject(hMainThread, INFINITE);

    // Очистка ресурсов
    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(hMainThread);

    printf("Все потоки завершили работу. Программа завершена.\n");
    return 0;
}
