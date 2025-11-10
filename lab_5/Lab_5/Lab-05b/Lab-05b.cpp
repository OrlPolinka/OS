#include <windows.h>
#include <iostream>
#include <string>

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "ru");
    if (argc != 4) {
        std::wcerr << L"Использование: Lab-05b <P1: affinity mask> <P2: priority 1> <P3: priority 2>\n";
        return 1;
    }

    DWORD_PTR affinityMask = _wtoi64(argv[1]);
    DWORD priority1 = _wtoi(argv[2]);
    DWORD priority2 = _wtoi(argv[3]);

    std::wcout << L"Маска процессоров (P1): " << affinityMask << "\n";
    std::wcout << L"Приоритет дочернего процесса 1 (P2): " << priority1 << "\n";
    std::wcout << L"Приоритет дочернего процесса 2 (P3): " << priority2 << "\n";

    // Путь к Lab-05x.exe
    std::wstring childPath = L"Lab-05x.exe";

    // Структуры запуска
    STARTUPINFOW si1 = { sizeof(si1) };
    PROCESS_INFORMATION pi1;
    STARTUPINFOW si2 = { sizeof(si2) };
    PROCESS_INFORMATION pi2;

    // Первый процесс
    if (CreateProcessW(
        childPath.c_str(), nullptr, nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE, nullptr, nullptr, &si1, &pi1)) {

        SetProcessAffinityMask(pi1.hProcess, affinityMask);
        SetPriorityClass(pi1.hProcess, priority1);
        std::wcout << L"Процесс 1 запущен: PID = " << pi1.dwProcessId << "\n";
    }
    else {
        std::wcerr << L"Ошибка запуска процесса 1\n";
        return 1;
    }

    // Второй процесс
    if (CreateProcessW(
        childPath.c_str(), nullptr, nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE, nullptr, nullptr, &si2, &pi2)) {

        SetProcessAffinityMask(pi2.hProcess, affinityMask);
        SetPriorityClass(pi2.hProcess, priority2);
        std::wcout << L"Процесс 2 запущен: PID = " << pi2.dwProcessId << "\n";
    }
    else {
        std::wcerr << L"Ошибка запуска процесса 2\n";
        return 1;
    }


    // Ожидание завершения дочерних процессов
    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    // Очистка
    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    std::wcout << L"Оба дочерних процесса завершены.\n";
    return 0;
}
