#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    DWORD currentPID = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Не удалось получить снимок потоков." << std::endl;
        return 1;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(snapshot, &te32)) {
        std::cerr << "Ошибка при получении первого потока." << std::endl;
        CloseHandle(snapshot);
        return 1;
    }

    std::cout << "Список потоков текущего процесса (PID: " << currentPID << "):" << std::endl;

    do {
        if (te32.th32OwnerProcessID == currentPID) {
            std::cout << "TID: " << te32.th32ThreadID << std::endl;
        }
    } while (Thread32Next(snapshot, &te32));

    CloseHandle(snapshot);
    return 0;
}
