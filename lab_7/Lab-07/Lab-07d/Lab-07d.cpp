#include <windows.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi1 = {}, pi2 = {};

    wchar_t cmd1[] = L"Lab-07x.exe 60";
    wchar_t cmd2[] = L"Lab-07x.exe 120";

    if (!CreateProcessW(nullptr, cmd1, nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi1)) {
        std::cerr << "Ошибка запуска процесса 1. Код ошибки: " << GetLastError() << "\n";
        return 1;
    }

    if (!CreateProcessW(nullptr, cmd2, nullptr, nullptr, FALSE,
        CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi2)) {
        std::cerr << "Ошибка запуска процесса 2. Код ошибки: " << GetLastError() << "\n";
        return 1;
    }

    std::cout << "Родительский процесс ожидает завершения дочерних...\n";

    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    std::cout << "Оба дочерних процесса завершили работу.\n";

    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    return 0;
}
