#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>

const std::string USERNAME = "User-c121f4dd";
const int ITERATIONS = 90;
const int DELAY_MS = 100;
const wchar_t* MUTEX_NAME = L"Global\\Lab06bMutex"; // Именованный мьютекс для межпроцессной синхронизации

void RunThread(const std::string& name) {
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    // Ожидание появления мьютекса
    HANDLE hMutex = nullptr;
    while ((hMutex = OpenMutexW(SYNCHRONIZE, FALSE, MUTEX_NAME)) == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    bool inCritical = false;

    for (int i = 0; i < ITERATIONS; ++i) {
        if (i + 1 == 30) {
            WaitForSingleObject(hMutex, INFINITE); // Захват мьютекса один раз
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
            ReleaseMutex(hMutex); // Освобождение мьютекса один раз
            inCritical = false;
        }
    }

    CloseHandle(hMutex);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    // Обработка аргументов дочерних процессов
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

    HANDLE hMutex = CreateMutexW(&sa, FALSE, MUTEX_NAME);
    if (!hMutex) {
        std::cerr << "Не удалось создать мьютекс\n";
        return 1;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION piA, piB;

    wchar_t cmdA[] = L"Lab-06b.exe childA";
    wchar_t cmdB[] = L"Lab-06b.exe childB";

    // Запуск дочернего процесса A
    if (!CreateProcessW(
        nullptr,
        cmdA,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE, 
        nullptr,
        nullptr,
        &si,
        &piA))
    {
        std::cerr << "Ошибка запуска процесса A\n";
        return 1;
    }

    // Запуск дочернего процесса B
    if (!CreateProcessW(
        nullptr,
        cmdB,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE, 
        nullptr,
        nullptr,
        &si,
        &piB))
    {
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
    CloseHandle(hMutex);

    std::cout << "Все процессы завершили работу.\n";
    return 0;
}
