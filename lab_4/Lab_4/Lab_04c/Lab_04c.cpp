#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const std::string USERNAME = "User-c121f4dd";

// Структура для передачи параметров потока
struct ThreadArgs {
    int iterations;
};

// Функция потока
DWORD WINAPI Lab_04x(LPVOID lpParam) {
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    printf("Поток TID: %lu начал работу\n", tid);

    for (int i = 0; i < args->iterations; ++i) {
        char letter = USERNAME[i % nameLen];
        printf("PID: %lu – TID: %lu – №%d – %c\n", pid, tid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }

    printf("Поток TID: %lu завершил работу\n", tid);
    return 0;
}

int main() {
    setlocale(LC_ALL, "ru");

    const int mainIterations = 100;
    size_t nameLen = USERNAME.length();
    DWORD pid = GetCurrentProcessId();
    DWORD mainTid = GetCurrentThreadId();

    // Аргументы для дочерних потоков
    ThreadArgs args1{ 50 };
    ThreadArgs args2{ 125 };

    // Создание дочерних потоков
    HANDLE hThread1 = CreateThread(nullptr, 0, Lab_04x, &args1, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, Lab_04x, &args2, 0, nullptr);

    if (!hThread1 || !hThread2) {
        std::cerr << "Ошибка при создании потоков\n";
        return 1;
    }

    printf("Главный поток TID: %lu начал работу\n", mainTid);

    for (int i = 0; i < mainIterations; ++i) {
        // Завершение второго потока на 40-й итерации
        if (i == 40) {
            TerminateThread(hThread2, 0);
            printf("Главный поток завершил поток 2 на итерации %d\n", i + 1);
        }

        char letter = USERNAME[i % nameLen];
        printf("PID: %lu – TID: %lu – №%d – %c\n", pid, mainTid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }

    // Ожидание завершения первого потока
    WaitForSingleObject(hThread1, INFINITE);

    // Очистка ресурсов
    CloseHandle(hThread1);
    CloseHandle(hThread2);

    printf("Все потоки завершили работу. Программа завершена.\n");
    return 0;
}
