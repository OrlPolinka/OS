#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

int main() {
    setlocale(LC_ALL, "ru");
    // Засекаем время начала
    auto start = std::chrono::high_resolution_clock::now();

    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    DWORD priorityClass = GetPriorityClass(hProcess);
    int threadPriority = GetThreadPriority(hThread);

    const int totalIterations = 1'000'000;
    const int delayInterval = 1000;

    for (int i = 1; i <= totalIterations; ++i) {
        if (i % delayInterval == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            std::cout << "Итерация: " << i << "\n"
                << "  PID: " << processId << "\n"
                << "  TID: " << threadId << "\n"
                << "  Класс приоритета процесса: " << priorityClass << "\n"
                << "  Приоритет потока: " << threadPriority << "\n"
                << "  Назначенный процессор: " << GetCurrentProcessorNumber() << "\n"
                << "-----------------------------\n";
        }
    }

    // Засекаем время окончания
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double elapsedSeconds = elapsedMs / 1000.0;
    int minutes = static_cast<int>(elapsedSeconds) / 60;
    double seconds = elapsedSeconds - (minutes * 60);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Время выполнения: " << minutes << " мин " << seconds << " сек\n";

    return 0;
}
