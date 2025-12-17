#include <windows.h>
#include <stdio.h>
#include <locale.h>


// -------------- Глобальные переменные --------------

// Глобальная инициализированная переменная (ожидаемо: секция .data)
int g_init = 42;

// Глобальная неинициализированная переменная (ожидаемо: секция .bss)
int g_uninit;

// Глобальная статическая инициализированная переменная (ожидаемо: .data)
static int g_static_init = 7;

// Глобальная статическая неинициализированная переменная (ожидаемо: .bss)
static int g_static_uninit;

// Глобальная константа (часто попадает в .rdata)
const char* g_const_str = "Hello, sections!";

// -------------- Любая функция --------------
int add(int a, int b) {
    return a + b;
}

// Функция для вывода адреса и метки
void print_addr(const char* label, void* ptr) {
    printf("%-40s | адрес: %p\n", label, ptr);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    // Выводим базовый адрес модуля процесса 
    HMODULE hModule = GetModuleHandleA(NULL);
    printf("Базовый адрес образа (Lab-08a.exe): %p\n\n", hModule);

    // Локальные переменные
    int local_init = 123;
    int local_uninit;
    local_uninit = 0;

    // Локальные статические переменные
    static int local_static_init = 555; // лок. статическая инициализированная (.data/.rdata)
    static int local_static_uninit;     // лок. статическая неинициализированная (.bss)

    // Адрес произвольной функции (код в секции .text)
    void* fn_add_addr = (void*)&add;

    printf("Адреса объявленных сущностей:\n");
    print_addr("Глобальная инициализированная (g_init)", &g_init);
    print_addr("Глобальная неинициализированная (g_uninit)", &g_uninit);
    print_addr("Глобальная статическая инициализированная (g_static_init)", &g_static_init);
    print_addr("Глобальная статическая неинициализированная (g_static_uninit)", &g_static_uninit);
    print_addr("Глобальная константа-указатель (g_const_str)", (void*)&g_const_str);

    print_addr("Локальная инициализированная (local_init)", &local_init);
    print_addr("Локальная неинициализированная (local_uninit)", &local_uninit);
    print_addr("Локальная статическая инициализированная (local_static_init)", &local_static_init);
    print_addr("Локальная статическая неинициализированная (local_static_uninit)", &local_static_uninit);

    print_addr("Аргумент argc (на стеке)", &argc);
    print_addr("Аргумент argv (указатель на массив указателей)", &argv);
    if (argc > 0 && argv) {
        print_addr("argv[0] (указатель на строку пути исполняемого файла)", argv[0]);
    }

    printf("%-40s | адрес: %p\n", "Функция add (секция .text)", fn_add_addr);
    printf("%-40s | адрес: %p\n", "Дереференс g_const_str (адрес первой буквы)", (const void*)g_const_str);

    printf("\nНажмите Enter, чтобы завершить...\n");
    getchar();

    return 0;
}
