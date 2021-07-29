//
// Created by ChaosChen on 2021/7/18.
//

#ifndef MOCKER_THREAD_H
#define MOCKER_THREAD_H

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>

namespace mocker {

    class Semaphore {
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        void wait();
        void notify();
    public:

        Semaphore(const Semaphore&) = delete;
        Semaphore(const Semaphore&&) = delete;
        Semaphore& operator= (const Semaphore&) = delete;
    private:
        sem_t m_semaphore;

    };


    template<class T>
    class ScopedLockImple {
    public:
        ScopedLockImple(T& mutex) : m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
        }

        ~ScopedLockImple() {
            unlock();
        }

        void lock() {
            if (!m_locked) {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock() {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked;
    };


    template<class T>
    class ReadScopedLockImple {
    public:
        ReadScopedLockImple(T& mutex) : m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
        }

        ~ReadScopedLockImple() {
            unlock();
        }

        void lock() {
            if (!m_locked) {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock() {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked;
    };


    template<class T>
    class WriteScopedLockImple {
    public:
        WriteScopedLockImple(T& mutex) : m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
        }

        ~WriteScopedLockImple() {
            unlock();
        }

        void lock() {
            if (!m_locked) {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock() {
            if (m_locked) {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked;
    };


    class RWMutex {
    public:
        typedef ReadScopedLockImple<RWMutex> ReadLock;
        typedef WriteScopedLockImple<RWMutex> WriteLock;
        RWMutex() {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex() {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock() {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock() {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock() {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };


    class Thread {
    public:
        typedef std::shared_ptr<Thread> ptr;
        using task = std::function<void()>;

        Thread(Thread::task cb, const std::string& name = "");
        ~Thread();

        pid_t getId() const { return m_id; }
        const std::string& getName() const { return m_name; }

        void join();

        static Thread* getCurrent();
        static const std::string& getCurrentName();
        static void setCurrentName(const std::string& name);

    public:
        Thread(const Thread&) = delete;
        Thread(const Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;

    private:
        static void* run(void* arg);
    private:
        pid_t m_id = -1;
        pthread_t m_thread = 0;
        Thread::task m_cb;
        std::string m_name;

        Semaphore m_semaphore;
    };
}

#endif //MOCKER_THREAD_H
