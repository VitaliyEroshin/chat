#pragma once

#include <queue>
#include <mutex>
#include <functional>
#include <condition_variable>

using Task = std::function<void()>;

class BoundedBlockingMPMCQueue {
public:
  BoundedBlockingMPMCQueue() = default;
  ~BoundedBlockingMPMCQueue();
  BoundedBlockingMPMCQueue(const BoundedBlockingMPMCQueue&) = delete;
  BoundedBlockingMPMCQueue& operator=(const BoundedBlockingMPMCQueue&) = delete;

  Task get();
  bool push(Task task);

  void clear();
  void set_bound(int bound);
  void stop();
private:
  int max_bound = 1024;
  Task get_acquired();

  bool is_closed = false;
  std::queue<Task> q; // Guarded by mutex
  std::mutex mutex;
  std::condition_variable is_not_empty;
};