#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const std::string USERNAME = "User-c121f4dd";

// Структура для передачи параметров потока
struct ThreadArgs {
    int iterations;
    HANDLE selfHandle;
    HANDLE resumeEvent;
    bool waitForMain;
};

// Функция потока
DWORD WINAPI Lab_04x(LPVOID lpParam) {
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    printf("Поток TID: %lu начал работу\n", tid);

    for (int i = 0; i < args->iterations; ++i) {
        // Поток 2 ждёт сигнала от главного потока
        if (args->waitForMain && i == 40) {
            printf("Поток TID: %lu приостановлен на итерации %d, ждёт сигнал от главного потока\n", tid, i + 1);
            WaitForSingleObject(args->resumeEvent, INFINITE);
            printf("Поток TID: %lu возобновлён после сигнала\n", tid);
        }

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

    // Событие для возобновления второго потока
    HANDLE resumeEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    // Аргументы и создание первого потока
    ThreadArgs args1{ 50, nullptr, nullptr, false };
    HANDLE hThread1 = CreateThread(nullptr, 0, Lab_04x, &args1, CREATE_SUSPENDED, nullptr);
    args1.selfHandle = hThread1;
    ResumeThread(hThread1);

    // Аргументы и создание второго потока
    ThreadArgs args2{ 125, nullptr, resumeEvent, true };
    HANDLE hThread2 = CreateThread(nullptr, 0, Lab_04x, &args2, 0, nullptr);
    args2.selfHandle = hThread2;

    printf("Главный поток TID: %lu начал работу\n", mainTid);

    for (int i = 0; i < mainIterations; ++i) {
        // Приостановка первого потока на 20-й итерации
        if (i == 20) {
            SuspendThread(hThread1);
            printf("Главный поток приостановил поток 1 на итерации %d\n", i + 1);
        }

        // Возобновление первого потока на 60-й итерации
        if (i == 60) {
            ResumeThread(hThread1);
            printf("Главный поток возобновил поток 1 на итерации %d\n", i + 1);
        }

        // Завершение цикла — сигнал второму потоку
        if (i == mainIterations - 1) {
            SetEvent(resumeEvent);
            printf("Главный поток завершил цикл и дал сигнал потоку 2\n");
        }

        char letter = USERNAME[i % nameLen];
        printf("PID: %lu – TID: %lu – №%d – %c\n", pid, mainTid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }

    // Ожидание завершения дочерних потоков
    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    // Очистка ресурсов
    CloseHandle(hThread1);
    CloseHandle(hThread2);
    CloseHandle(resumeEvent);

    printf("Все потоки завершили работу. Программа завершена.\n");
    return 0;
}
