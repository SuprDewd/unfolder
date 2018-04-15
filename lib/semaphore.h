#ifndef UNFOLDER_SEMAPHORE_H
#define UNFOLDER_SEMAPHORE_H

// https://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

#include <mutex>
#include <condition_variable>

class semaphore
{
private:
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_; // Initialized as locked.

public:
    semaphore();
    void notify();
    void wait();
    bool try_wait();
};

#endif
