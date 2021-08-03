//
// Created by ChaosChen on 2021/5/10.
//

#ifndef MOCKER_UTIL_H
#define MOCKER_UTIL_H

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdint>
#include <vector>
#include <string>

namespace mocker {
    pid_t GetThreadId();
    uint32_t GetCoroutineId();

    void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "\t");
}

#endif //MOCKER_UTIL_H
