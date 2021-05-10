//
// Created by ChaosChen on 2021/5/11.
//

#ifndef MOCKER_SINGLETON_H
#define MOCKER_SINGLETON_H

#include <memory>

namespace mocker {
    template<typename T, typename X = void, int N = 0>
    class Singleton {
    public:
        static T* GetInstance() {
            static T v;
            return &v;
        }

    private:
    };

    template<typename T, typename X = void, int N = 0>
    class SingletonPtr {
    public:
        static std::shared_ptr<T> GetInstance() {
            static std::shared_ptr<T> v(new T);
            return v;
        }

    private:
    };
}

#endif //MOCKER_SINGLETON_H
