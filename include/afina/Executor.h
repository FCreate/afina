#ifndef AFINA_THREADPOOL_H
#define AFINA_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <set>
#include <iostream>
//using namespace std;

namespace Afina {

/**
 * # Thread pool
 */
class Executor {
    enum class State {
        // Threadpool is fully operational, tasks could be added and get executed
        kRun,

        // Threadpool is on the way to be shutdown, no ned task could be added, but existing will be
        // completed as requested
        kStopping,

        // Threadppol is stopped
        kStopped
    };
public:
    //Executor() construct{}
    Executor(size_t low_watermark=2,
             size_t high_watermark=5, size_t max_queue_size=10,
             std::chrono::milliseconds idle_time=std::chrono::milliseconds(500))
            :
            state(State::kRun), low_watermark(low_watermark),
            high_watermark(high_watermark), max_queue_size(max_queue_size),
            idle_threads(0), idle_time(idle_time) {

        std::lock_guard<std::mutex> lock(mutex);
        while (threads.size() < low_watermark) {
            pthread_t pthr;
            pthread_create(&pthr, NULL, perform, this);
            threads.insert(pthr);
        }
    }
    //~Executor();
    ~Executor() {
        Stop(true);
    }

    /**
     * Signal thread pool to stop, it will stop accepting new jobs and close threads just after each become
     * free. All enqueued jobs will be complete.
     *
     * In case if await flag is true, call won't return until all background jobs are done and all threads are stopped
     */
    void Stop(bool await = false) {
        std::cout << "executor debug: " << __PRETTY_FUNCTION__ << std::endl;
        std::unique_lock<std::mutex> lock(mutex);
        state = State::kStopping;
        empty_condition.notify_all();
        if (!await) {
            return;
        }
        lock.unlock();
        Join();
    }

    void Join(bool await = false) {
        std::cout << "executor debug: " << __PRETTY_FUNCTION__ << std::endl;

        std::unique_lock<std::mutex> lock(mutex);
        while (!threads.empty()) {
            finish_condition.wait(lock);
        }
    }

    /**
     * Add function to be executed on the threadpool. Method returns true in case if task has been placed
     * onto execution queue, i.e scheduled for execution and false otherwise.
     *
     * That function doesn't wait for function result. Function could always be written in a way to notify caller about
     * execution finished by itself
     */
    template <typename F, typename... Types> bool Execute(F &&func, Types... args) {
        // Prepare "task"
        std::cout << "executor debug: " << __PRETTY_FUNCTION__ << std::endl;
        auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);

        std::lock_guard<std::mutex> lock(mutex);
        if (state != State::kRun) {
            return false;
        }
        //add new thread
        if (idle_threads == 0) {
            std::cout << "Idle_threads == 0" << std::endl;
            if (threads.size() < high_watermark) {
                std::cout << "New thread is creating" << std::endl;
                pthread_t thr;
                pthread_create(&thr, NULL, perform, this);
                threads.insert(thr);

            } else if (tasks.size() == max_queue_size) {
                return false;
            }
        }
        // Enqueue new task
        std::cout << "Enqueue new task" << std::endl;
        tasks.push_back(exec);
        empty_condition.notify_one();
        return true;
    }

private:
    // No copy/move/assign allowed
    Executor(const Executor &);            // = delete;
    Executor(Executor &&);                 // = delete;
    Executor &operator=(const Executor &); // = delete;
    Executor &operator=(Executor &&);      // = delete;

    /**
     * Main function that all pool threads are running. It polls internal task queue and execute tasks
     */
    static void* perform(void *p) {
        auto executor = reinterpret_cast<Executor *>(p);
        auto prev_time = std::chrono::system_clock::now();
        while (1) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(executor->mutex);

                while (executor->tasks.empty()) {
                    executor->idle_threads++;
                    executor->empty_condition.wait_for(lock, executor->idle_time);
                    executor->idle_threads--;
                    auto idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - prev_time);

                    if ((idle_time >= executor->idle_time && executor->threads.size() > executor->low_watermark ||
                         executor->state == Executor::State::kStopping) && executor->tasks.empty()) {

                        executor->threads.erase(pthread_self());
                        executor->finish_condition.notify_one();
                        return 0;
                    }
                }

                task = std::move(executor->tasks.front());
                executor->tasks.pop_front();
            }
            std::cout << "Executing" << std::endl;
            task();
            std::cout << "Executing is finished" << std::endl;
            prev_time = std::chrono::system_clock::now();
        }

    }

    /**
     * Mutex to protect state below from concurrent modification
     */
    std::mutex mutex;

    /**
     * Conditional variable to await new data in case of empty queue
     */
    std::condition_variable empty_condition;

    std::condition_variable finish_condition;
    /**
     * Vector of actual threads that perorm execution
     */
    //std::vector<std::thread> threads;
    std::set<pthread_t> threads;

    /**
     * Task queue
     */
    std::deque<std::function<void()>> tasks;

    /**
     * Flag to stop bg threads
     */
    State state;
    size_t low_watermark;
    size_t high_watermark;
    size_t idle_threads;
    size_t max_queue_size;
    std::chrono::milliseconds idle_time;
};

} // namespace Afina

#endif // AFINA_THREADPOOL_H
