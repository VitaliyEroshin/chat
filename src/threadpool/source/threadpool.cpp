#include "threadpool.hpp"
#include <thread>
using namespace std::chrono_literals;

ThreadPool::ThreadPool(size_t num_workers) {
  run.store(true);
  if (num_workers == 0) {
    synchronous = true;
    return;
  }

  for (size_t i = 0; i < num_workers; ++i) {
    workers.emplace_back([this](){ worker_routine(); });
  }
}

void ThreadPool::worker_routine() {
  while (run.load()) {
    auto task = queue.get();
    task();
  }
}

void ThreadPool::join() {
  for (auto&& worker : workers) {
    worker.join();
  }
}

void ThreadPool::stop() {
  queue.stop();
  run.store(false);
}

ThreadPool::~ThreadPool() {
  stop();
}

bool ThreadPool::add_task(Task task) {
  if (synchronous) {
    task();
    return true;
  }

  return queue.push(std::move(task));
}