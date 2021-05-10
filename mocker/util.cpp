//
// Created by ChaosChen on 2021/5/10.
//

#include "util.h"
#include <functional>
#include <thread>

namespace mocker {
    size_t GetThreadId() {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    uint32_t GetFiberId() {
        return 0;
    }
}

