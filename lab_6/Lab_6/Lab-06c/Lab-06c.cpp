#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>

const std::string USERNAME = "User-c121f4dd";
const int ITERATIONS = 90;
const int DELAY_MS = 100;
const wchar_t* SEMAPHORE_NAME = L"Global\\Lab06cSemaphore"; // Именованный бинарный семафор

void RunThread(const std::string& name) {
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    // Ожидание появления семафора
    HANDLE hSemaphore = nullptr;
    while ((hSemaphore = OpenSemaphoreW(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, SEMAPHORE_NAME)) == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    bool inCritical = false;

    for (int i = 0; i < ITERATIONS; ++i) {
        if (i + 1 == 30) {
            WaitForSingleObject(hSemaphore, INFINITE); // Захват семафора один раз
            std::ostringstream oss;
            oss << "[" << name << "] ВХОД в критическую зону\n";
            std::cout << oss.str();
            inCritical = true;
        }

        char letter = USERNAME[i % nameLen];
        std::ostringstream oss;
        oss << "[" << name << "] PID: " << pid << " – TID: " << tid << " – №" << (i + 1) << " – " << letter << "\n";
        std::cout << oss.str();

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));

        if (i + 1 == 61 && inCritical) {
            std::ostringstream oss;
            oss << "[" << name << "] ВЫХОД из критической зоны\n";
            std::cout << oss.str();
            ReleaseSemaphore(hSemaphore, 1, nullptr); // Освобождение семафора один раз
            inCritical = false;
        }
    }

    CloseHandle(hSemaphore);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc == 2 && std::string(argv[1]) == "childA") {
        RunThread("Процесс A");
        return 0;
    }

    if (argc == 2 && std::string(argv[1]) == "childB") {
        RunThread("Процесс B");
        return 0;
    }

    // Родительский процесс
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    // Создание бинарного семафора (начальное значение 1, максимум 1)
    HANDLE hSemaphore = CreateSemaphoreW(&sa, 1, 1, SEMAPHORE_NAME);
    if (!hSemaphore) {
        std::cerr << "Не удалось создать семафор\n";
        return 1;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION piA, piB;

    wchar_t cmdA[] = L"Lab-06c.exe childA";
    wchar_t cmdB[] = L"Lab-06c.exe childB";

    // Запуск дочернего процесса A
    if (!CreateProcessW(nullptr, cmdA, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &piA)) {
        std::cerr << "Ошибка запуска процесса A\n";
        return 1;
    }

    // Запуск дочернего процесса B
    if (!CreateProcessW(nullptr, cmdB, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &piB)) {
        std::cerr << "Ошибка запуска процесса B\n";
        return 1;
    }

    RunThread("Main");

    // Ожидание завершения дочерних процессов
    WaitForSingleObject(piA.hProcess, INFINITE);
    WaitForSingleObject(piB.hProcess, INFINITE);

    // Очистка
    CloseHandle(piA.hProcess);
    CloseHandle(piA.hThread);
    CloseHandle(piB.hProcess);
    CloseHandle(piB.hThread);
    CloseHandle(hSemaphore);

    std::cout << "Все процессы завершили работу.\n";
    return 0;
}
