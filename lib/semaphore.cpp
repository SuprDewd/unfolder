#include "semaphore.h"

semaphore::semaphore() {
    count_ = 0;
}

void semaphore::notify() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    ++count_;
    condition_.notify_one();
}

void semaphore::wait() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    while(!count_) // Handle spurious wake-ups.
        condition_.wait(lock);
    --count_;
}

bool semaphore::try_wait() {
    std::unique_lock<decltype(mutex_)> lock(mutex_);
    if(count_) {
        --count_;
        return true;
    }
    return false;
}
