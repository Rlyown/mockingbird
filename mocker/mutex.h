//
// Created by ChaosChen on 2021/7/30.
//

#ifndef MOCKER_MUTEX_H
#define MOCKER_MUTEX_H

#include <pthread.h>
#include <semaphore.h>
#include <atomic>

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

    class Mutex {
    public:
        typedef ScopedLockImple<Mutex> Lock;
        Mutex() {
            pthread_mutex_init(&m_mutex, nullptr);
        }

        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock() {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }


    private:
        pthread_mutex_t m_mutex;
    };


    class Spinlock {
    public:
        typedef ScopedLockImple<Spinlock> Lock;
        Spinlock() {
            pthread_spin_init(&m_mutex, 0);
        }

        ~Spinlock() {
            pthread_spin_destroy(&m_mutex);
        }

        void lock() {
            pthread_spin_lock(&m_mutex);
        }

        void unlock() {
            pthread_spin_unlock(&m_mutex);
        }

    private:
        pthread_spinlock_t m_mutex;
    };


    class CASLock {
    public:
        typedef ScopedLockImple<CASLock> Lock;
        CASLock() {
            m_mutex.clear();
        }

        ~CASLock() {

        }

        void lock() {
            while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
        }

        void unlock() {
            std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
        }
    private:
        volatile std::atomic_flag m_mutex;
    };


    // Fake mutex
    class NullMutex {
    public:
        typedef ScopedLockImple<NullMutex> Lock;
        NullMutex() {}
        ~NullMutex() {}
        void lock() {}
        void unlock() {}
    };

    // Fake RWMutex
    class NullRWMutex {
    public:
        typedef ReadScopedLockImple<NullRWMutex> ReadLock;
        typedef WriteScopedLockImple<NullRWMutex> WriteLock;
        NullRWMutex() {}
        ~NullRWMutex() {}

        void wrlock() {}
        void rdlock() {}
        void unlock() {}
    };



}

#endif //MOCKER_MUTEX_H
