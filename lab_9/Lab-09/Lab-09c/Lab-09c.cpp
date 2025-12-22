#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <locale.h>

void PrintDirectoryContents(LPCWSTR path) {
    WIN32_FIND_DATAW ffd;
    WCHAR searchPath[MAX_PATH];
    HANDLE hFind;

    wsprintfW(searchPath, L"%s\\*", path);
    hFind = FindFirstFileW(searchPath, &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        wprintf(L"Ошибка: не удалось прочитать каталог %s\n", path);
        return;
    }

    wprintf(L"Содержимое каталога %s:\n", path);
    do {
        if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0)
            continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            wprintf(L"[DIR]  %s\n", ffd.cFileName);
        else
            wprintf(L"[FILE] %s\n", ffd.cFileName);
    } while (FindNextFileW(hFind, &ffd));

    FindClose(hFind);
}

void WatchDirectory(LPCWSTR path) {
    HANDLE hDir = CreateFileW(
        path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        wprintf(L"Ошибка: не удалось открыть каталог %s\n", path);
        return;
    }

    BYTE buffer[1024];
    DWORD bytesReturned;
    FILE_NOTIFY_INFORMATION* fni;
    WCHAR fileName[MAX_PATH];

    wprintf(L"\nОтслеживание изменений в каталоге %s...\n", path);

    while (1) {
        if (ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            FALSE, // только первый уровень
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_CREATION |
            FILE_NOTIFY_CHANGE_SECURITY,
            &bytesReturned,
            NULL,
            NULL
        )) {
            fni = (FILE_NOTIFY_INFORMATION*)buffer;
            int len = fni->FileNameLength / sizeof(WCHAR);
            wcsncpy(fileName, fni->FileName, len);
            fileName[len] = L'\0';

            switch (fni->Action) {
            case FILE_ACTION_ADDED:
                wprintf(L"Добавлен: %s\n", fileName); break;
            case FILE_ACTION_REMOVED:
                wprintf(L"Удалён: %s\n", fileName); break;
            case FILE_ACTION_MODIFIED:
                wprintf(L"Изменён: %s\n", fileName); break;
            case FILE_ACTION_RENAMED_OLD_NAME:
                wprintf(L"Переименован (старое имя): %s\n", fileName); break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                wprintf(L"Переименован (новое имя): %s\n", fileName); break;
            default:
                wprintf(L"Событие %d для %s\n", fni->Action, fileName);
            }
        }
        else {
            wprintf(L"Ошибка ReadDirectoryChangesW: %lu\n", GetLastError());
            break;
        }
    }

    CloseHandle(hDir);
}

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc < 2) {
        wprintf(L"Использование: %s <путь_к_каталогу>\n", argv[0]);
        return 1;
    }

    DWORD attrs = GetFileAttributesW(argv[1]);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        wprintf(L"Ошибка: каталог %s не существует.\n", argv[1]);
        return 1;
    }

    PrintDirectoryContents(argv[1]);
    WatchDirectory(argv[1]);

    return 0;
}
