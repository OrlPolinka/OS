#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const std::string USERNAME = "User-c121f4dd";  

// Структура для передачи параметров потоку
struct ThreadArgs {
    int iterations;
};

// Функция потока
DWORD WINAPI Lab_04x(LPVOID lpParam) {
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    int iterations = args->iterations;
    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    size_t nameLen = USERNAME.length();

    for (int i = 0; i < iterations; ++i) {
        char letter = USERNAME[i % nameLen];
        printf("PID: %lu – TID: %lu – №%d – %c\n", pid, tid, i + 1, letter);
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
    }


    return 0;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <кол-во_итераций>" << std::endl;
        return 1;
    }

    int iterations = std::stoi(argv[1]);
    if (iterations <= 0) {
        std::cerr << "Ошибка: количество итераций должно быть положительным числом." << std::endl;
        return 1;
    }

    ThreadArgs args{ iterations };

    // Создание потока
    HANDLE hThread = CreateThread(
        nullptr,            // Дескриптор безопасности
        0,                  // Размер стека
        Lab_04x,            // Функция потока
        &args,              // Аргументы
        0,                  // Флаги
        nullptr             // ID потока
    );

    if (!hThread) {
        std::cerr << "Ошибка при создании потока: " << GetLastError() << std::endl;
        return 1;
    }

    // Ожидание завершения потока
    WaitForSingleObject(hThread, INFINITE);

    // Очистка ресурсов
    CloseHandle(hThread);

    return 0;
}
