#pragma once

#include "blocking-queue.hpp"

#include <vector>
#include <thread>
#include <atomic>

class ThreadPool {
public:
  explicit ThreadPool(size_t num_workers);
  ~ThreadPool();
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  bool add_task(Task task);
  void join();
  void stop();

private:
  void worker_routine();

  std::atomic<bool> run{};
  bool synchronous = false;
  std::vector<std::thread> workers;
  BoundedBlockingMPMCQueue queue;
};