//
// Created by ChaosChen on 2021/5/10.
//

#include <execinfo.h>

#include <mocker/log.h>
#include <mocker/util.h>
#include <mocker/coroutine.h>

namespace mocker {
    static Logger::ptr g_logger = MOCKER_LOG_SYSTEM();

    pid_t GetThreadId() {
        // https://man7.org/linux/man-pages/man2/gettid.2.html
        // return syscall(SYS_gettid);
        return gettid();
    }

    uint32_t GetCoroutineId() {
        return Coroutine::GetCoroutineId();
    }


    void Backtrace(std::vector<std::string>& bt, int size, int skip) {
        void** array = (void **) malloc(sizeof (void *) * size);
        int s = backtrace(array, size);

        char** symbols = backtrace_symbols(array, s);
        if (symbols == nullptr) {
            MOCKER_LOG_ERROR(g_logger) << "backtrace_symbols error";
            return;
        }

        for (int i = skip; i < s; ++i) {
            bt.emplace_back(symbols[i]);
        }

        free(symbols);
        free(array);
    }

    std::string BacktraceToString(int size, int skip, const std::string& prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);

        std::stringstream ss;
        for (auto& frame : bt) {
            ss << prefix << frame << std::endl;
        }
        return ss.str();
    }
}

