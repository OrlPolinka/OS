#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_LINE_LEN 1024

HANDLE g_hFile = INVALID_HANDLE_VALUE;
CHAR* g_fileBuf = NULL;      
DWORD  g_fileSize = 0;        
CHAR** g_lines = NULL;        
DWORD  g_lineCount = 0;       

BOOL   LoadFileToBuffer();
BOOL   ParseLines();
BOOL   SaveBufferToFile();
BOOL   RefreshFromDisk();
void   FreeGlobals();
BOOL   EnsureOpen();
BOOL   ValidatePosForInsert(INT pos);
BOOL   ValidatePosForAccess(INT pos, DWORD* outIndex);

BOOL OpenFile(LPSTR filePath) {
    if (g_hFile != INVALID_HANDLE_VALUE) {
        printf("Ошибка: файл уже открыт.\n");
        return FALSE;
    }

    g_hFile = CreateFileA(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,            
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (g_hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: файл не существует или не удалось открыть: %lu\n", GetLastError());
        return FALSE;
    }

    if (!LoadFileToBuffer()) {
        CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE;
        return FALSE;
    }
    if (!ParseLines()) {
        FreeGlobals();
        CloseHandle(g_hFile); g_hFile = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    printf("Файл открыт. Строк: %lu, Размер: %lu байт.\n", (unsigned long)g_lineCount, (unsigned long)g_fileSize);
    return TRUE;
}

BOOL CloseFile() {
    if (g_hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: файл не открыт.\n");
        return FALSE;
    }
    FreeGlobals();
    CloseHandle(g_hFile);
    g_hFile = INVALID_HANDLE_VALUE;
    printf("Файл закрыт, ресурсы освобождены.\n");
    return TRUE;
}

void FreeGlobals() {
    if (g_lines) { free(g_lines); g_lines = NULL; }
    if (g_fileBuf) { free(g_fileBuf); g_fileBuf = NULL; }
    g_fileSize = 0;
    g_lineCount = 0;
}

BOOL LoadFileToBuffer() {
    LARGE_INTEGER sz;
    if (!GetFileSizeEx(g_hFile, &sz)) {
        printf("Ошибка получения размера файла: %lu\n", GetLastError());
        return FALSE;
    }
    if (sz.QuadPart > 0x7FFFFFFF) {
        printf("Слишком большой файл для примера.\n");
        return FALSE;
    }
    g_fileSize = (DWORD)sz.QuadPart;

    g_fileBuf = (CHAR*)malloc(g_fileSize + 1);
    if (!g_fileBuf) {
        printf("Ошибка выделения памяти.\n");
        return FALSE;
    }

    DWORD read = 0;
    if (g_fileSize == 0) {
        g_fileBuf[0] = '\0';
        return TRUE;
    }

    SetFilePointer(g_hFile, 0, NULL, FILE_BEGIN);
    if (!ReadFile(g_hFile, g_fileBuf, g_fileSize, &read, NULL) || read != g_fileSize) {
        printf("Ошибка чтения файла: %lu\n", GetLastError());
        return FALSE;
    }
    g_fileBuf[g_fileSize] = '\0';
    return TRUE;
}

BOOL ParseLines() {
    DWORD count = 0;
    for (DWORD i = 0; i < g_fileSize; i++) {
        if (g_fileBuf[i] == '\n') count++;
    }
    if (g_fileSize > 0 && count == 0) count = 1;

    g_lines = (CHAR**)malloc(sizeof(CHAR*) * (count + 1));
    if (!g_lines) {
        printf("Ошибка выделения памяти для массива строк.\n");
        return FALSE;
    }

    DWORD idx = 0;
    CHAR* p = g_fileBuf;
    CHAR* start = p;

    while (*p) {
        if (*p == '\n') {
            g_lines[idx++] = start;
            if (p > start && *(p - 1) == '\r') *(p - 1) = '\0';
            *p = '\0';
            start = p + 1;
        }
        p++;
    }
    if (start < g_fileBuf + g_fileSize) {
        g_lines[idx++] = start;
    }

    g_lineCount = idx;
    return TRUE;
}

BOOL RefreshFromDisk() {
    if (!LoadFileToBuffer()) return FALSE;
    if (g_lines) { free(g_lines); g_lines = NULL; }
    return ParseLines();
}

BOOL SaveBufferToFile() {
    if (g_hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: файл не открыт.\n");
        return FALSE;
    }
    size_t total = 0;
    for (DWORD i = 0; i < g_lineCount; i++) {
        total += strlen(g_lines[i]) + 2; 
    }
    CHAR* out = (CHAR*)malloc(total + 1);
    if (!out) {
        printf("Ошибка выделения памяти.\n");
        return FALSE;
    }

    CHAR* w = out;
    for (DWORD i = 0; i < g_lineCount; i++) {
        size_t len = strlen(g_lines[i]);
        memcpy(w, g_lines[i], len);
        w += len;
        *w++ = '\r';
        *w++ = '\n';
    }
    *w = '\0';

    SetFilePointer(g_hFile, 0, NULL, FILE_BEGIN);
    DWORD written = 0;
    if (!WriteFile(g_hFile, out, (DWORD)(w - out), &written, NULL)) {
        printf("Ошибка записи: %lu\n", GetLastError());
        free(out);
        return FALSE;
    }
    SetEndOfFile(g_hFile);

    free(out);
    return RefreshFromDisk();
}

BOOL EnsureOpen() {
    if (g_hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: файл не открыт.\n");
        return FALSE;
    }
    return TRUE;
}

BOOL ValidatePosForInsert(INT pos) {
    if (pos == -1 || pos == 0) return TRUE;
    if (pos > 0 && (DWORD)pos <= g_lineCount + 1) return TRUE;
    return FALSE;
}

BOOL ValidatePosForAccess(INT pos, DWORD* outIndex) {
    if (pos == -1) {
        if (g_lineCount == 0) return FALSE;
        *outIndex = g_lineCount - 1;
        return TRUE;
    }
    if (pos == 0) {
        if (g_lineCount == 0) return FALSE;
        *outIndex = 0;
        return TRUE;
    }
    if (pos > 0) {
        if ((DWORD)pos == 0 || (DWORD)pos > g_lineCount) return FALSE;
        *outIndex = (DWORD)pos - 1;
        return TRUE;
    }
    return FALSE;
}

BOOL AddRow(LPSTR row, INT pos) {
    if (!EnsureOpen()) return FALSE;
    if (row == NULL) { printf("Ошибка: строка равна NULL.\n"); return FALSE; }
    if (!ValidatePosForInsert(pos)) { printf("Ошибка: недопустимая позиция вставки.\n"); return FALSE; }

    DWORD insertIdx = 0;
    if (pos == -1) insertIdx = g_lineCount;          
    else if (pos == 0) insertIdx = 0;               
    else insertIdx = (DWORD)pos - 1;                

    CHAR** newLines = (CHAR**)malloc(sizeof(CHAR*) * (g_lineCount + 1));
    if (!newLines) { printf("Ошибка выделения памяти.\n"); return FALSE; }

    for (DWORD i = 0; i < insertIdx; i++) newLines[i] = g_lines[i];

    size_t len = strlen(row);
    CHAR* copy = (CHAR*)malloc(len + 1);
    if (!copy) { free(newLines); printf("Ошибка выделения памяти.\n"); return FALSE; }
    memcpy(copy, row, len + 1);

    newLines[insertIdx] = copy;

    for (DWORD i = insertIdx; i < g_lineCount; i++) newLines[i + 1] = g_lines[i];

    free(g_lines);
    g_lines = newLines;
    g_lineCount++;

    if (!SaveBufferToFile()) return FALSE;

    printf("Строка добавлена на позицию %ld.\n", (long)pos);
    return TRUE;
}

BOOL RemRow(INT pos) {
    if (!EnsureOpen()) return FALSE;

    DWORD idx = 0;
    if (!ValidatePosForAccess(pos, &idx)) { printf("Ошибка: недопустимая позиция удаления.\n"); return FALSE; }

    CHAR** newLines = (CHAR**)malloc(sizeof(CHAR*) * (g_lineCount - 1));
    if (!newLines && g_lineCount > 1) { printf("Ошибка выделения памяти.\n"); return FALSE; }

    if (g_fileBuf == NULL || (g_lines[idx] < g_fileBuf || g_lines[idx] >= g_fileBuf + g_fileSize)) {
        free(g_lines[idx]);
    }

    for (DWORD i = 0; i < idx; i++) newLines[i] = g_lines[i];
    for (DWORD i = idx + 1; i < g_lineCount; i++) newLines[i - 1] = g_lines[i];

    free(g_lines);
    g_lines = newLines;
    g_lineCount--;

    if (!SaveBufferToFile()) return FALSE;

    printf("Строка удалена с позиции %ld.\n", (long)pos);
    return TRUE;
}

BOOL PrintRow(INT pos) {
    if (!EnsureOpen()) return FALSE;

    DWORD idx = 0;
    if (!ValidatePosForAccess(pos, &idx)) { printf("Ошибка: недопустимая позиция.\n"); return FALSE; }

    printf("Строка [%lu]: %s\n", (unsigned long)(idx + 1), g_lines[idx]);
    return TRUE;
}

BOOL PrintRows() {
    if (!EnsureOpen()) return FALSE;
    for (DWORD i = 0; i < g_lineCount; i++) {
        printf("%lu. %s\n", (unsigned long)(i + 1), g_lines[i]);
    }
    return TRUE;
}

static void FlushStdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main() {
    SetConsoleOutputCP(65001); 
    setlocale(LC_ALL, "ru");

    while (1) {
        printf("\nВыберите выполняемую операцию:\n");
        printf("1. Открыть файл.\n");
        printf("2. Вставить строку.\n");
        printf("3. Удалить строку.\n");
        printf("4. Вывести строку.\n");
        printf("5. Вывести файл.\n");
        printf("6. Закрыть файл.\n");
        printf("0. Выход.\n");
        printf("> ");

        int cmd = -1;
        if (scanf("%d", &cmd) != 1) { printf("Ошибка ввода команды.\n"); FlushStdin(); continue; }
        FlushStdin();

        if (cmd == 0) {
            if (g_hFile != INVALID_HANDLE_VALUE) CloseFile();
            break;
        }

        switch (cmd) {
        case 1: {
            char path[MAX_PATH];
            printf("Введите путь к файлу: ");
            if (!fgets(path, sizeof(path), stdin)) { printf("Ошибка ввода.\n"); break; }
            size_t L = strlen(path);
            if (L && (path[L - 1] == '\n' || path[L - 1] == '\r')) path[L - 1] = '\0';
            OpenFile(path);
            break;
        }
        case 2: {
            if (!EnsureOpen()) break;
            char row[MAX_LINE_LEN];
            int pos;
            printf("Введите позицию вставки (0 — начало, -1 — конец, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода числа.\n"); FlushStdin(); break; }
            FlushStdin();
            printf("Введите строку: ");
            if (!fgets(row, sizeof(row), stdin)) { printf("Ошибка ввода.\n"); break; }
            size_t L = strlen(row);
            if (L && (row[L - 1] == '\n' || row[L - 1] == '\r')) row[L - 1] = '\0';
            AddRow(row, pos);
            break;
        }
        case 3: {
            if (!EnsureOpen()) break;
            int pos;
            printf("Введите позицию удаления (0 — первая, -1 — последняя, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода числа.\n"); FlushStdin(); break; }
            FlushStdin();
            RemRow(pos);
            break;
        }
        case 4: {
            if (!EnsureOpen()) break;
            int pos;
            printf("Введите позицию вывода (0 — первая, -1 — последняя, >0 — номер): ");
            if (scanf("%d", &pos) != 1) { printf("Ошибка ввода числа.\n"); FlushStdin(); break; }
            FlushStdin();
            PrintRow(pos);
            break;
        }
        case 5: {
            PrintRows();
            break;
        }
        case 6: {
            CloseFile();
            break;
        }
        default:
            printf("Неизвестная команда.\n");
        }
    }

    return 0;
}
