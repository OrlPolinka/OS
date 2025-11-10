#include <windows.h>
#include <iostream>
#include <thread>
#include <string>

volatile LONG counter1 = 0;
volatile LONG counter2 = 0;
const int maxIterations = 1'000'000;

DWORD WINAPI ThreadFunction(LPVOID param) {
    int threadId = reinterpret_cast<int>(param);
    for (int i = 0; i < maxIterations; ++i) {
        if (i % 1000 == 0) {
            std::wcout << L"[Поток " << threadId << L"] Итерация: " << i << L"\n";
        }
        if (threadId == 1) InterlockedIncrement(&counter1);
        else InterlockedIncrement(&counter2);
    }
    return 0;
}

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "ru");
    if (argc != 5) {
        std::wcerr << L"Использование: Lab-05c <P1: affinity mask> <P2: process priority> <P3: thread1 priority> <P4: thread2 priority>\n";
        return 1;
    }

    DWORD_PTR affinityMask = _wtoi64(argv[1]);
    DWORD processPriority = _wtoi(argv[2]);
    int threadPriority1 = _wtoi(argv[3]);
    int threadPriority2 = _wtoi(argv[4]);

    std::wcout << L"Маска процессоров (P1): " << affinityMask << "\n";
    std::wcout << L"Приоритет процесса (P2): " << processPriority << "\n";
    std::wcout << L"Приоритет потока 1 (P3): " << threadPriority1 << "\n";
    std::wcout << L"Приоритет потока 2 (P4): " << threadPriority2 << "\n";

    // Установка приоритета процесса и маски
    SetPriorityClass(GetCurrentProcess(), processPriority);
    SetProcessAffinityMask(GetCurrentProcess(), affinityMask);

    // Создание потоков
    HANDLE hThread1 = CreateThread(nullptr, 0, ThreadFunction, (LPVOID)1, 0, nullptr);
    HANDLE hThread2 = CreateThread(nullptr, 0, ThreadFunction, (LPVOID)2, 0, nullptr);

    SetThreadPriority(hThread1, threadPriority1);
    SetThreadPriority(hThread2, threadPriority2);

    std::wcout << L"Ожидание 15 секунд для анализа в Process Explorer...\n";
    Sleep(15'000);

    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);

    std::wcout << L"Поток 1 завершён. Итераций: " << counter1 << "\n";
    std::wcout << L"Поток 2 завершён. Итераций: " << counter2 << "\n";

    CloseHandle(hThread1);
    CloseHandle(hThread2);

    return 0;
}
