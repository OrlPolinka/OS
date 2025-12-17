#include <windows.h>
#include <stdio.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "ru");

    // 1. Определяем размер страницы
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD pageSize = si.dwPageSize;
    printf("Размер страницы: %lu байт\n", (unsigned long)pageSize);

    // 2. Резервируем 256 страниц виртуальной памяти
    SIZE_T reserveSize = 256 * pageSize;
    LPVOID baseAddr = VirtualAlloc(NULL, reserveSize, MEM_RESERVE, PAGE_NOACCESS);
    if (!baseAddr) {
        printf("Ошибка резервирования памяти\n");
        return 1;
    }
    printf("Резервировано 256 страниц по адресу: %p\n", baseAddr);
    system("pause"); // Этап 1: смотрим VMMap и RAMMap

    // 3. Выделяем физическую память для второй половины (128 страниц)
    LPVOID commitAddr = (LPBYTE)baseAddr + (128 * pageSize);
    LPVOID committed = VirtualAlloc(commitAddr, 128 * pageSize, MEM_COMMIT, PAGE_READWRITE);
    if (!committed) {
        printf("Ошибка коммита памяти\n");
        VirtualFree(baseAddr, 0, MEM_RELEASE);
        return 1;
    }
    printf("Выделена физическая память для второй половины: %p\n", committed);
    system("pause"); // Этап 2: смотрим VMMap и RAMMap

    // 4. Заполняем вторую половину последовательностью чисел
    int* arr = (int*)committed;
    SIZE_T count = (128 * pageSize) / sizeof(int);
    for (SIZE_T i = 0; i < count; i++) {
        arr[i] = (int)i;
    }
    printf("Заполнено %llu целых чисел.\n", (unsigned long long)count);
    system("pause"); // Этап 3: смотрим VMMap и RAMMap

    // 5. Меняем защиту на "только чтение"
    DWORD oldProtect;
    if (!VirtualProtect(committed, 128 * pageSize, PAGE_READONLY, &oldProtect)) {
        printf("Ошибка изменения защиты\n");
    }
    else {
        printf("Защита изменена на READONLY.\n");
    }
    system("pause"); // Этап 4: смотрим VMMap

    // 6. Освобождаем физическую память (128 страниц)
    if (!VirtualFree(committed, 128 * pageSize, MEM_DECOMMIT)) {
        printf("Ошибка освобождения физической памяти\n");
    }
    else {
        printf("Физическая память освобождена.\n");
    }
    system("pause"); // Этап 5: смотрим VMMap и RAMMap

    // 7. Освобождаем всю область (256 страниц)
    if (!VirtualFree(baseAddr, 0, MEM_RELEASE)) {
        printf("Ошибка освобождения виртуальной памяти\n");
    }
    else {
        printf("Виртуальная память освобождена.\n");
    }
    system("pause"); // Этап 6: смотрим VMMap

    return 0;
}
