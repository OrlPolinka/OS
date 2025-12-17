#include <windows.h>
#include <stdio.h>
#include <locale.h>

struct RegionSummary {
    SIZE_T busyBytes;
    SIZE_T freeBytes;
    SIZE_T overheadBytes;
};

// Печать размера в КиБ/МиБ
static void fmtBytes(SIZE_T bytes, char* buf, size_t bufSize) {
    double kib = bytes / 1024.0;
    double mib = kib / 1024.0;
    if (mib >= 1.0)
        snprintf(buf, bufSize, "%.2f MiB", mib);
    else
        snprintf(buf, bufSize, "%.2f KiB", kib);
}

// Информация о куче
void HeapInfo(HANDLE heap) {
    PROCESS_HEAP_ENTRY entry;
    RegionSummary sum = { 0,0,0 };
    SIZE_T blockCountBusy = 0, blockCountFree = 0, blockCountOverhead = 0;

    printf("\n================ HeapInfo ================\n");

    if (!HeapValidate(heap, 0, NULL)) {
        printf("HeapValidate: куча НЕвалидна (или ошибка). GetLastError=%lu\n", GetLastError());
    }
    else {
        printf("HeapValidate: куча валидна.\n");
    }

    entry.lpData = NULL;
    while (HeapWalk(heap, &entry)) {
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {
            sum.busyBytes += entry.cbData;
            blockCountBusy++;
        }
        else if (entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) {
            sum.overheadBytes += entry.cbData;
            blockCountOverhead++;
        }
        else {
            sum.freeBytes += entry.cbData;
            blockCountFree++;
        }

        if (entry.wFlags & (PROCESS_HEAP_REGION | PROCESS_HEAP_UNCOMMITTED_RANGE)) {
            sum.overheadBytes += entry.cbData;
            blockCountOverhead++;
        }
    }

    DWORD err = GetLastError();
    if (err != ERROR_NO_MORE_ITEMS) {
        printf("HeapWalk завершился с ошибкой: %lu\n", err);
    }

    SIZE_T totalAccounted = sum.busyBytes + sum.freeBytes + sum.overheadBytes;

    char buf[64];
    fmtBytes(sum.busyBytes, buf, sizeof(buf));
    printf("  Занято (busy):      %s  (%llu байт, блоков: %llu)\n", buf,
        (unsigned long long)sum.busyBytes, (unsigned long long)blockCountBusy);

    fmtBytes(sum.freeBytes, buf, sizeof(buf));
    printf("  Свободно (free):    %s  (%llu байт, блоков: %llu)\n", buf,
        (unsigned long long)sum.freeBytes, (unsigned long long)blockCountFree);

    fmtBytes(sum.overheadBytes, buf, sizeof(buf));
    printf("  Оверхед (overhead): %s  (%llu байт, записей: %llu)\n", buf,
        (unsigned long long)sum.overheadBytes, (unsigned long long)blockCountOverhead);

    fmtBytes(totalAccounted, buf, sizeof(buf));
    printf("  Учтено суммарно:    %s  (%llu байт)\n", buf,
        (unsigned long long)totalAccounted);

    printf("\nРегиональный срез (VirtualQuery на диапазоне кучи):\n");
    printf("==========================================\n");
}

int main() {
    setlocale(LC_ALL, "ru");

    const SIZE_T initialSize = 1ull * 1024 * 1024; // 1 МиБ
    const SIZE_T maxSize = 8ull * 1024 * 1024;     // 8 МиБ

    HANDLE heap = HeapCreate(0, initialSize, maxSize);
    if (!heap) {
        printf("HeapCreate: ошибка. GetLastError=%lu\n", GetLastError());
        return 1;
    }

    char buf[64];
    fmtBytes(initialSize, buf, sizeof(buf));
    char buf2[64];
    fmtBytes(maxSize, buf2, sizeof(buf2));
    printf("Куча создана. Initial=%s, Max=%s\n", buf, buf2);

    HeapInfo(heap); system("pause"); system("cls");

    const SIZE_T blockSize = 512ull * 1024; // 512 КиБ
    const int blockCount = 10;
    void* blocks[blockCount];

    for (int i = 0; i < blockCount; ++i) {
        void* p = HeapAlloc(heap, 0, blockSize);
        if (!p) {
            fmtBytes(blockSize, buf, sizeof(buf));
            printf("HeapAlloc: не удалось выделить блок %d размером %s. GetLastError=%lu\n",
                i, buf, GetLastError());
            break;
        }
        blocks[i] = p;
        fmtBytes(blockSize, buf, sizeof(buf));
        printf("Выделен блок #%d addr=%p size=%s\n", i, p, buf);
        HeapInfo(heap); system("pause"); system("cls");
    }

    for (int i = 0; i < blockCount; ++i) {
        int* arr = (int*)blocks[i];
        SIZE_T count = blockSize / sizeof(int);
        for (SIZE_T j = 0; j < count; ++j) {
            arr[j] = (int)j;
        }
    }
    printf("Этап 3: заполнение всех блоков завершено (%d блоков).\n", blockCount);
    system("pause"); system("cls");

    for (int i = 0; i < blockCount; ++i) {
        if (!HeapFree(heap, 0, blocks[i])) {
            printf("HeapFree: ошибка при освобождении блока #%d. GetLastError=%lu\n", i, GetLastError());
        }
    }
    printf("Этап 4: освобождение всех блоков завершено.\n");
    HeapInfo(heap); system("pause"); system("cls");

    if (!HeapDestroy(heap)) {
        printf("HeapDestroy: ошибка. GetLastError=%lu\n", GetLastError());
    }
    else {
        printf("Куча уничтожена.\n");
    }
    system("pause");

    return 0;
}
