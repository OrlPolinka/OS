#include <windows.h>
#include <iostream>
#include <bitset>

int main() {
    setlocale(LC_ALL, "ru");
    // Идентификаторы
    DWORD processId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();

    // Дескрипторы
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    // Класс приоритета процесса
    DWORD priorityClass = GetPriorityClass(hProcess);

    // Приоритет потока
    int threadPriority = GetThreadPriority(hThread);

    // Маска процессорной родственности
    DWORD_PTR processAffinityMask = 0;
    DWORD_PTR systemAffinityMask = 0;
    if (!GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
        std::cerr << "Ошибка получения маски процессоров.\n";
        return 1;
    }

    // Количество доступных процессоров
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD processorCount = sysInfo.dwNumberOfProcessors;

    // Номер текущего процессора
    DWORD currentProcessor = GetCurrentProcessorNumber();

    // Вывод
    std::cout << "Идентификатор процесса: " << processId << "\n";
    std::cout << "Идентификатор потока: " << threadId << "\n";
    std::cout << "Класс приоритета процесса: " << priorityClass << "\n";
    std::cout << "Приоритет потока: " << threadPriority << "\n";
    std::cout << "Маска родственности процесса: " << std::bitset<64>(processAffinityMask) << "\n";
    std::cout << "Системная маска родственности: " << std::bitset<64>(systemAffinityMask) << "\n";
    std::cout << "Доступно процессоров: " << processorCount << "\n";
    std::cout << "Номер текущего процессора: " << currentProcessor << "\n";

    return 0;
}
