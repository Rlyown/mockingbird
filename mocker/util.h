//
// Created by ChaosChen on 2021/5/10.
//

#ifndef MOCKER_UTIL_H
#define MOCKER_UTIL_H

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdint>

namespace mocker {
    size_t GetThreadId();
    uint32_t GetFiberId();
}

#endif //MOCKER_UTIL_H
