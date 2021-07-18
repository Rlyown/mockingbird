//
// Created by ChaosChen on 2021/5/10.
//

#include <mocker/util.h>

namespace mocker {
    pid_t getThreadId() {
        // https://man7.org/linux/man-pages/man2/gettid.2.html
        // return syscall(SYS_gettid);
        return gettid();
    }

    uint32_t getFiberId() {
        return 0;
    }
}

