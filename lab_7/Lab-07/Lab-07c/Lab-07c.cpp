#include <windows.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    unsigned long long counter = 0;       // счётчик итераций
    unsigned long long totalCounter = 0;  // накопленная сумма

    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    // Создаём ожидающий таймер
    HANDLE hTimer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (!hTimer) {
        std::cerr << "Не удалось создать таймер\n";
        return 1;
    }

    // Каждые 3 секунды будем ждать таймер
    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart = -30000000LL; // 3 секунды в 100-нс интервалах

    int elapsedPeriods = 0;

    while (true) {
        // Считаем итерации до следующего срабатывания таймера
        while (true) {
            counter++;
            QueryPerformanceCounter(&now);
            double elapsed = double(now.QuadPart - start.QuadPart) / freq.QuadPart;

            if (elapsed >= (elapsedPeriods + 1) * 3.0) {
                break;
            }
        }

        elapsedPeriods++;

        std::cout << "Прошло " << (elapsedPeriods * 3)
            << " секунд, значение счётчика: " << counter << std::endl;

        totalCounter += counter;

        if (elapsedPeriods * 3 >= 15) {
            std::cout << "Итоговое значение счётчика (сумма всех периодов): "
                << totalCounter << std::endl;
            break;
        }

        // Запускаем таймер на следующие 3 секунды
        SetWaitableTimer(hTimer, &liDueTime, 0, nullptr, nullptr, FALSE);
        WaitForSingleObject(hTimer, INFINITE);
    }

    CloseHandle(hTimer);
    return 0;
}
