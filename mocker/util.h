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

namespace mocker {
    pid_t getThreadId();
    uint32_t getFiberId();

}

#endif //MOCKER_UTIL_H
