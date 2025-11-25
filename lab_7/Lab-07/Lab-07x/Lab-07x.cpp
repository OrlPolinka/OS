#include <windows.h>
#include <iostream>

bool isPrime(unsigned long long n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (unsigned long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    int runSeconds = 30; // по умолчанию 30 секунд
    if (argc == 2) {
        runSeconds = atoi(argv[1]);
    }

    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    unsigned long long num = 2;
    unsigned long long index = 1;

    while (true) {
        if (isPrime(num)) {
            std::cout << index << ": " << num << std::endl;
            index++;
        }
        num++;

        QueryPerformanceCounter(&now);
        double elapsed = double(now.QuadPart - start.QuadPart) / freq.QuadPart;
        if (elapsed >= runSeconds) {
            break;
        }
    }

    std::cout << "Программа работала " << runSeconds << " секунд." << std::endl;
    return 0;
}
