#ifndef COMMON_H_
#define COMMON_H_

#include <thread>
#include <mutex>
#include <vector>
#include <map>
#include <queue>
#include <condition_variable>

template <typename T>
class RequestQueue {
    private:
        std::queue<T*> q_;
        std::mutex q_mutex_;
        std::condition_variable q_cond_;

    public:
        void enqueue(T *request){
            std::unique_lock<std::mutex> lock(q_mutex_);
            q_.push(request);
            q_cond_.notify_one();
        }

        T* dequeue(){
            std::unique_lock<std::mutex> lock(q_mutex_);
            q_cond_.wait(lock, 
                                [this]() {return !q_.empty();});
            T *request = q_.front();
            q_.pop();

            return request;
        }
};

#endif