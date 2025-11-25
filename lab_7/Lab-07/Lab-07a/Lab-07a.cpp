#include <windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>

int main() {
    setlocale(LC_ALL, "ru");

    // Получаем системное время (UTC)
    SYSTEMTIME utcTime;
    GetSystemTime(&utcTime);

    // Получаем локальное время
    SYSTEMTIME localTime;
    GetLocalTime(&localTime);

    // Переводим SYSTEMTIME в FILETIME для вычисления смещения
    FILETIME ftUtc, ftLocal;
    SystemTimeToFileTime(&utcTime, &ftUtc);
    SystemTimeToFileTime(&localTime, &ftLocal);

    // Преобразуем FILETIME в 64-битное значение
    ULARGE_INTEGER uUtc, uLocal;
    uUtc.LowPart = ftUtc.dwLowDateTime;
    uUtc.HighPart = ftUtc.dwHighDateTime;
    uLocal.LowPart = ftLocal.dwLowDateTime;
    uLocal.HighPart = ftLocal.dwHighDateTime;

    // Разница в 100‑наносекундных интервалах
    LONGLONG diff = (LONGLONG)(uLocal.QuadPart - uUtc.QuadPart);

    // Переводим в часы (1 час = 3600 секунд = 3600 * 10^7 * 100нс)
    int offsetHours = (int)(diff / (3600LL * 10000000LL));

    // Формируем строку в формате ISO 8601
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(4) << localTime.wYear << "-"
        << std::setw(2) << localTime.wMonth << "-"
        << std::setw(2) << localTime.wDay << "T"
        << std::setw(2) << localTime.wHour << ":"
        << std::setw(2) << localTime.wMinute << ":"
        << std::setw(2) << localTime.wSecond
        << (offsetHours >= 0 ? "+" : "-")
        << std::setw(2) << std::abs(offsetHours);

    std::cout << oss.str() << std::endl;

    return 0;
}
