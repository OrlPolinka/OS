#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define BLOCK_SIZE (64*1024) 
#define MAX_CONCURRENT 5

typedef struct {
    HANDLE hSrc;
    HANDLE hDst;
    OVERLAPPED ovlRead;
    OVERLAPPED ovlWrite;
    BYTE* buffer;
    DWORD fileSize;
    DWORD copied;
    CHAR srcName[MAX_PATH];
    CHAR dstName[MAX_PATH];
} COPYCTX;

static volatile BOOL g_stop = FALSE;

BOOL WINAPI CtrlHandler(DWORD type) {
    if (type == CTRL_C_EVENT) {
        printf("\nОстановка по Ctrl+C...\n");
        g_stop = TRUE;
        return TRUE;
    }
    return FALSE;
}

void PrintProgress(const char* name, DWORD copied, DWORD total) {
    int percent = (int)((copied * 100.0) / total);
    printf("\r[%s] %d%% (%lu/%lu байт)", name, percent, copied, total);
    fflush(stdout);
}

DWORD WINAPI CopyThread(LPVOID param) {
    COPYCTX* ctx = (COPYCTX*)param;
    DWORD read = 0, written = 0;
    LARGE_INTEGER pos;
    pos.QuadPart = 0;

    while (!g_stop && ctx->copied < ctx->fileSize) {
        DWORD toRead = BLOCK_SIZE;
        if (ctx->fileSize - ctx->copied < BLOCK_SIZE)
            toRead = ctx->fileSize - ctx->copied;

        ctx->ovlRead.Offset = pos.LowPart;
        ctx->ovlRead.OffsetHigh = pos.HighPart;

        if (!ReadFile(ctx->hSrc, ctx->buffer, toRead, NULL, &ctx->ovlRead)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                printf("\nОшибка чтения %s: %lu\n", ctx->srcName, GetLastError());
                break;
            }
        }
        GetOverlappedResult(ctx->hSrc, &ctx->ovlRead, &read, TRUE);

        ctx->ovlWrite.Offset = pos.LowPart;
        ctx->ovlWrite.OffsetHigh = pos.HighPart;

        if (!WriteFile(ctx->hDst, ctx->buffer, read, NULL, &ctx->ovlWrite)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                printf("\nОшибка записи %s: %lu\n", ctx->dstName, GetLastError());
                break;
            }
        }
        GetOverlappedResult(ctx->hDst, &ctx->ovlWrite, &written, TRUE);

        ctx->copied += written;
        pos.QuadPart += written;

        PrintProgress(ctx->srcName, ctx->copied, ctx->fileSize);
    }

    printf("\nФайл %s скопирован.\n", ctx->srcName);

    CloseHandle(ctx->hSrc);
    CloseHandle(ctx->hDst);
    free(ctx->buffer);
    free(ctx);
    return 0;
}

BOOL EnsureDirs(LPCSTR src, LPCSTR dst) {
    DWORD attrs = GetFileAttributesA(src);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        printf("Ошибка: каталог источника %s не существует.\n", src);
        return FALSE;
    }
    attrs = GetFileAttributesA(dst);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectoryA(dst, NULL)) {
            printf("Ошибка: не удалось создать каталог %s\n", dst);
            return FALSE;
        }
    }
    return TRUE;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc < 3) {
        printf("Использование: %s <каталог_источник> <каталог_приемник>\n", argv[0]);
        return 1;
    }

    if (!EnsureDirs(argv[1], argv[2])) return 1;

    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    WIN32_FIND_DATAA ffd;
    CHAR searchPath[MAX_PATH];
    sprintf(searchPath, "%s\\*", argv[1]);
    HANDLE hFind = FindFirstFileA(searchPath, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Ошибка поиска файлов.\n");
        return 1;
    }

    int activeThreads = 0;
    HANDLE threads[MAX_CONCURRENT];

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        // Формируем пути
        CHAR srcPath[MAX_PATH], dstPath[MAX_PATH];
        sprintf(srcPath, "%s\\%s", argv[1], ffd.cFileName);
        sprintf(dstPath, "%s\\%s", argv[2], ffd.cFileName);

        HANDLE hSrc = CreateFileA(srcPath, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (hSrc == INVALID_HANDLE_VALUE) {
            printf("Ошибка открытия %s\n", srcPath);
            continue;
        }

        HANDLE hDst = CreateFileA(dstPath, GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
        if (hDst == INVALID_HANDLE_VALUE) {
            printf("Ошибка создания %s\n", dstPath);
            CloseHandle(hSrc);
            continue;
        }

        LARGE_INTEGER sz;
        GetFileSizeEx(hSrc, &sz);

        COPYCTX* ctx = (COPYCTX*)malloc(sizeof(COPYCTX));
        ZeroMemory(ctx, sizeof(COPYCTX));
        ctx->hSrc = hSrc;
        ctx->hDst = hDst;
        ctx->buffer = (BYTE*)malloc(BLOCK_SIZE);
        ctx->fileSize = (DWORD)sz.QuadPart;
        ctx->copied = 0;
        strcpy(ctx->srcName, ffd.cFileName);
        strcpy(ctx->dstName, dstPath);

        HANDLE th = CreateThread(NULL, 0, CopyThread, ctx, 0, NULL);
        threads[activeThreads++] = th;

        if (activeThreads >= MAX_CONCURRENT) {
            WaitForMultipleObjects(activeThreads, threads, TRUE, INFINITE);
            activeThreads = 0;
        }

    } while (FindNextFileA(hFind, &ffd));

    FindClose(hFind);

    if (activeThreads > 0) {
        WaitForMultipleObjects(activeThreads, threads, TRUE, INFINITE);
    }

    printf("Все файлы скопированы.\n");
    return 0;
}
