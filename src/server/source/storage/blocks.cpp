#include "storage.hpp"

void Block::save(const std::string& path) {
  std::ofstream stream(path);
  for (auto &row : data) {
    stream << row << "\n";
  }
}

std::string& Block::operator[](size_t i) {
  return data[i];
}

Block::Block(const std::string& path, const std::string& block): block(block) {
  std::ifstream stream(path);
  for (size_t i = 0; i < block_size; ++i) {
    std::string s;
    std::getline(stream, s);
    data.push_back(s);
  }
}

Block::Block(const std::string& block)
    : block(block), 
      data(std::vector<std::string>(block_size)) 
{}

Block& LRUCache::operator[](const std::string& i) {
  if (iterators.count(i)) {
    auto it = iterators[i];
    cache.push_front(std::move(*it));
    cache.erase(it);
    iterators[i] = cache.begin();
  } else {
    if (cache.size() == maxCacheSize) {
      iterators.erase(cache.back().block);
      cache.pop_back();
    }
    cache.emplace_front(path + i, i);
    iterators[i] = cache.begin();
  }
  return cache.front();
}

Block& LRUCache::operator[](size_t i) {
  return operator[](std::to_string(i));
}

LRUCache::LRUCache(size_t size, const std::string& path)
    : maxCacheSize(size), 
      path(path)
{}