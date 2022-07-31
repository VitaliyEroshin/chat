#include "blocking-queue.hpp"
#include <iostream>
using Queue = BoundedBlockingMPMCQueue;

Task Queue::get() {
  std::unique_lock lock(mutex);
  while (q.empty() && !is_closed) {
    is_not_empty.wait(lock);
  }
  if (is_closed) {
    return [](){
      std::cout << "empty";
    };
  }

  return get_acquired();
}

Task Queue::get_acquired() {
  Task task(std::move(q.front()));
  q.pop();
  return task;
}

bool Queue::push(Task task) {
  std::unique_lock lock(mutex);
  if (q.size() == max_bound) {
    return false;
  }
  q.push(std::move(task));
  is_not_empty.notify_one();
  return true;
}

void Queue::clear() {
  std::unique_lock lock(mutex);
  while (!q.empty()) {
    q.pop();
  }
  is_not_empty.notify_all();
}

void Queue::set_bound(int bound) {
  std::unique_lock lock(mutex);
  while (q.size() > bound) {
    q.pop();
  }
  max_bound = bound;
}
Queue::~BoundedBlockingMPMCQueue() {
  stop();
}

void Queue::stop() {
  std::unique_lock lock(mutex);
  clear();
  is_closed = true;
  is_not_empty.notify_all();
}