//
// Created by ChaosChen on 2021/5/10.
//

#include <functional>
#include <thread>
#include <mocker/util.h>

namespace mocker {
    size_t GetThreadId() {
        return std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    uint32_t GetFiberId() {
        return 0;
    }
}

