#include <windows.h>
#include <stdio.h>
#include <locale.h>

BOOL IsTextFile(LPCSTR FileName) {
    HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;

    BYTE buffer[512];
    DWORD bytesRead;
    BOOL isText = TRUE;

    if (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL)) {
        for (DWORD i = 0; i < bytesRead; i++) {
            if (buffer[i] < 9 || (buffer[i] > 13 && buffer[i] < 32)) {
                isText = FALSE;
                break;
            }
        }
    }
    CloseHandle(hFile);
    return isText;
}

void PrintInfo(LPSTR FileName) {
    HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: не удалось открыть файл %s\n", FileName);
        return;
    }

    BY_HANDLE_FILE_INFORMATION fi;
    if (!GetFileInformationByHandle(hFile, &fi)) {
        printf("Ошибка: не удалось получить информацию о файле\n");
        CloseHandle(hFile);
        return;
    }

    LARGE_INTEGER size;
    size.HighPart = fi.nFileSizeHigh;
    size.LowPart = fi.nFileSizeLow;

    printf("Имя файла: %s\n", FileName);
    printf("Размер: %lld Б (%.2f КиБ, %.2f МиБ)\n",
        size.QuadPart,
        (double)size.QuadPart / 1024.0,
        (double)size.QuadPart / (1024.0 * 1024.0));

    if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        printf("Тип: каталог\n");
    else
        printf("Тип: обычный файл\n");

    SYSTEMTIME st;
    FileTimeToSystemTime(&fi.ftCreationTime, &st);
    printf("Время создания: %02d.%02d.%04d %02d:%02d\n",
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);

    FileTimeToSystemTime(&fi.ftLastAccessTime, &st);
    printf("Последний доступ: %02d.%02d.%04d %02d:%02d\n",
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);

    FileTimeToSystemTime(&fi.ftLastWriteTime, &st);
    printf("Последнее изменение: %02d.%02d.%04d %02d:%02d\n",
        st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute);

    if (IsTextFile(FileName))
        printf("Файл является текстовым\n");
    else
        printf("Файл является бинарным\n");

    CloseHandle(hFile);
}

void PrintText(LPSTR FileName) {
    if (!IsTextFile(FileName)) {
        printf("Файл %s не является текстовым\n", FileName);
        return;
    }

    HANDLE hFile = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: не удалось открыть файл %s\n", FileName);
        return;
    }

    BYTE buffer[512];
    DWORD bytesRead;
    while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, stdout);
    }

    CloseHandle(hFile);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc < 2) {
        printf("Использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    PrintInfo(argv[1]);
    printf("\n--- Содержимое файла ---\n");
    PrintText(argv[1]);

    return 0;
}
