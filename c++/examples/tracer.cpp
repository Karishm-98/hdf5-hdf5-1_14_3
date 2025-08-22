#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <errno.h>
#include <vector>

std::string readStringFromChild(pid_t child, long addr) {
    std::string result;
    long word;
    char *p;
    while (true) {
        errno = 0;
        word = ptrace(PTRACE_PEEKDATA, child, addr, nullptr);
        if (errno != 0) break;

        p = reinterpret_cast<char*>(&word);
        for (int i = 0; i < sizeof(word); ++i) {
            if (p[i] == '\0') return result;
            result += p[i];
        }
        addr += sizeof(word);
    }
    return result;
}

std::string getFdPath(pid_t pid, int fd) {
    std::ostringstream path;
    path << "/proc/" << pid << "/fd/" << fd;
    char buf[4096];
    ssize_t len = readlink(path.str().c_str(), buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(buf);
    }
    return "unknown";
}

std::string getFdOffset(pid_t pid, int fd) {
    std::ostringstream path;
    path << "/proc/" << pid << "/fdinfo/" << fd;
    std::ifstream fin(path.str());
    std::string line;
    while (std::getline(fin, line)) {
        if (line.rfind("pos:", 0) == 0) {
            return line.substr(5);
        }
    }
    return "unknown";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./tracer <program-to-trace> [args...]\n";
        return 1;
    }

    pid_t child = fork();
    if (child == 0) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        return 1;
    } else {
        int status;
        waitpid(child, &status, 0);
        ptrace(PTRACE_SYSCALL, child, nullptr, nullptr);

        std::unordered_map<int, std::string> fdToPath;

        while (true) {
            waitpid(child, &status, 0);
            if (WIFEXITED(status)) break;

            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, child, nullptr, &regs);

#ifdef __x86_64__
            long syscall = regs.orig_rax;
            long arg0 = regs.rdi;
            long arg1 = regs.rsi;
            long arg2 = regs.rdx;
#elif __aarch64__
            long syscall = regs.regs[8];
            long arg0 = regs.regs[0];
            long arg1 = regs.regs[1];
            long arg2 = regs.regs[2];
#endif

            static bool inSyscall = false;
            static long currentFd = -1;

            if (!inSyscall) {
                // Syscall entry
                if (syscall == SYS_read) {
                    currentFd = arg0;
                } else if (syscall == SYS_open || syscall == SYS_openat) {
                    std::string path = readStringFromChild(child, arg1);
                    std::cout << "[OPEN SYSCALL] path: " << path << std::endl;
                }
                inSyscall = true;
            } else {
                // Syscall exit
                if (syscall == SYS_read && currentFd >= 0) {
                    std::string filePath = getFdPath(child, currentFd);
                    std::string offset = getFdOffset(child, currentFd);
                    std::cout << "[READ] fd=" << currentFd
                              << ", path=" << filePath
                              << ", offset=" << offset << std::endl;
                    currentFd = -1;
                }
                inSyscall = false;
            }

            ptrace(PTRACE_SYSCALL, child, nullptr, nullptr);
        }
    }

    return 0;
}
