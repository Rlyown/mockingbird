//
// Created by ChaosChen on 2021/7/31.
//

#ifndef MOCKER_MACRO_H
#define MOCKER_MACRO_H

#include <cstring>
#include <cassert>

#include <mocker/log.h>
#include <mocker/util.h>

#ifndef NO_MOCKER_DEBUG

#define MOCKER_ASSERT(cond) \
    if (!(cond)) {             \
        MOCKER_LOG_ERROR(MOCKER_LOG_SYSTEM()) << "ASSERTION: " #cond \
            << "\nbacktrace:\n" \
            << mocker::BacktraceToString(100, 2, "    ");            \
        assert(cond);       \
    }

#define MOCKER_ASSERT2(cond, msg) \
    if (!(cond)) {             \
        MOCKER_LOG_ERROR(MOCKER_LOG_SYSTEM()) << "ASSERTION: " #cond \
            << "\n" << msg               \
            << "\nbacktrace:\n" \
            << mocker::BacktraceToString(100, 2, "    ");            \
        assert(cond);       \
    }

#else

#define MOCKER_ASSERT(cond) {}
#define MOCKER_ASSERT2(cond, msg) {}

#endif //NO_MOCKER_DEBUG

#endif //MOCKER_MACRO_H
