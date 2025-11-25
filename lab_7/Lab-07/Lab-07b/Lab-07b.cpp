#include <windows.h>
#include <iostream>

int main() {
    setlocale(LC_ALL, "ru");

    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);   // частота таймера
    QueryPerformanceCounter(&start);    // стартовое время

    unsigned long long counter = 0;     // счётчик итераций
    bool printed5 = false, printed10 = false;

    while (true) {
        counter++;

        QueryPerformanceCounter(&now);
        double elapsed = double(now.QuadPart - start.QuadPart) / freq.QuadPart;

        if (!printed5 && elapsed >= 5.0) {
            std::cout << "Итераций за 5 секунд: " << counter << std::endl;
            printed5 = true;
        }

        if (!printed10 && elapsed >= 10.0) {
            std::cout << "Итераций за 10 секунд: " << counter << std::endl;
            printed10 = true;
        }

        if (elapsed >= 15.0) {
            std::cout << "Итераций за 15 секунд (итог): " << counter << std::endl;
            break;
        }
    }

    return 0;
}
