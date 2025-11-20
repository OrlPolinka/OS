#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>

const std::string USERNAME = "User-c121f4dd";
const int ITERATIONS = 90;
const int DELAY_MS = 100;
const wchar_t* EVENT_NAME = L"Global\\Lab06dStartEvent"; // Именованное событие

void RunThread(const std::string& name, bool isChild) {
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    // Открытие события
    HANDLE hEvent = nullptr;
    while ((hEvent = OpenEventW(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, EVENT_NAME)) == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Дочерние процессы ждут сигнала
    if (isChild) {
        WaitForSingleObject(hEvent, INFINITE);
    }

    for (int i = 0; i < ITERATIONS; ++i) {
        // Родитель выполняет первые 15 итераций до сигнала
        if (!isChild && i + 1 == 16) {
            SetEvent(hEvent); // Сигнал дочерним процессам
            std::ostringstream oss;
            oss << "[" << name << "] Событие отправлено. Начинаем совместное выполнение.\n";
            std::cout << oss.str();
        }

        char letter = USERNAME[i % nameLen];
        std::ostringstream oss;
        oss << "[" << name << "] PID: " << pid << " – TID: " << tid << " – №" << (i + 1) << " – " << letter << "\n";
        std::cout << oss.str();

        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_MS));
    }

    CloseHandle(hEvent);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc == 2 && std::string(argv[1]) == "childA") {
        RunThread("Процесс A", true);
        return 0;
    }

    if (argc == 2 && std::string(argv[1]) == "childB") {
        RunThread("Процесс B", true);
        return 0;
    }

    // Родительский процесс
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    // Создание события (ручной сброс, начальное состояние — не установлен)
    HANDLE hEvent = CreateEventW(&sa, TRUE, FALSE, EVENT_NAME);
    if (!hEvent) {
        std::cerr << "Не удалось создать событие. Код ошибки: " << GetLastError() << "\n";
        return 1;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION piA = {}, piB = {};

    wchar_t cmdA[] = L"Lab-06d.exe childA";
    wchar_t cmdB[] = L"Lab-06d.exe childB";

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
        std::cerr << "Ошибка запуска процесса A. Код ошибки: " << GetLastError() << "\n";
        CloseHandle(hEvent);
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
        std::cerr << "Ошибка запуска процесса B. Код ошибки: " << GetLastError() << "\n";
        TerminateProcess(piA.hProcess, 1);
        CloseHandle(piA.hProcess);
        CloseHandle(piA.hThread);
        CloseHandle(hEvent);
        return 1;
    }


    RunThread("Main", false);

    // Ожидание завершения дочерних процессов
    WaitForSingleObject(piA.hProcess, INFINITE);
    WaitForSingleObject(piB.hProcess, INFINITE);

    // Очистка
    CloseHandle(piA.hProcess);
    CloseHandle(piA.hThread);
    CloseHandle(piB.hProcess);
    CloseHandle(piB.hThread);
    CloseHandle(hEvent);

    std::cout << "Все процессы завершили работу.\n";
    return 0;
}
