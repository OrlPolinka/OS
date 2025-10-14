#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono> // Для измерения времени

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "ru");

    if (argc != 4) {
        std::cerr << "Usage: Lab-03d-server <num_processes> <lower> <upper>" << std::endl;
        return 1;
    }

    int numProcs, lower, upper;
    try {
        numProcs = std::stoi(argv[1]);
        lower = std::stoi(argv[2]);
        upper = std::stoi(argv[3]);
    }
    catch (...) {
        std::cerr << "Invalid arguments. Must be integers." << std::endl;
        return 1;
    }

    if (numProcs <= 0 || lower > upper) {
        std::cerr << "Invalid input values." << std::endl;
        return 1;
    }

    int rangeSize = (upper - lower + 1) / numProcs;
    int remainder = (upper - lower + 1) % numProcs;

    std::vector<HANDLE> hReadPipes;
    std::vector<PROCESS_INFORMATION> procInfos;

    //  Начало замера времени
    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numProcs; ++i) {
        int start = lower + i * rangeSize;
        int end = start + rangeSize - 1;
        if (i == numProcs - 1) end += remainder;

        HANDLE hRead, hWrite;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            std::cerr << "Failed to create pipe. Error: " << GetLastError() << std::endl;
            return 1;
        }

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        PROCESS_INFORMATION pi = {};
        std::ostringstream cmd;
        cmd << "Lab_03d-client.exe " << start << " " << end;

        BOOL success = CreateProcessA(
            NULL,
            (LPSTR)cmd.str().c_str(),
            NULL, NULL, TRUE, 0, NULL, NULL,
            &si, &pi
        );

        if (!success) {
            std::cerr << "Failed to create process. Error: " << GetLastError() << std::endl;
            CloseHandle(hRead);
            CloseHandle(hWrite);
            continue;
        }

        CloseHandle(hWrite);
        hReadPipes.push_back(hRead);
        procInfos.push_back(pi);
    }

    for (size_t i = 0; i < hReadPipes.size(); ++i) {
        char buffer[4096];
        DWORD bytesRead;
        std::cout << "Process #" << i + 1 << " returned: ";
        while (ReadFile(hReadPipes[i], buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << buffer;
        }
        std::cout << std::endl;
        CloseHandle(hReadPipes[i]);
    }

    for (const auto& pi : procInfos) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // Конец замера времени
    auto endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(endTime - startTime).count();
    std::cout << "Время выполнения: " << duration << " секунд\n";

    std::cout << "All child processes are completed." << std::endl;
    return 0;
}
