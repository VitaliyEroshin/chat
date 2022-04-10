#include "storage.hpp"
#include <filesystem>

void Block::save(const std::string& path) {
  savePath = path;
  std::ofstream stream(path);
  stream << block_size << "\n";
  for (auto &row : data) {
    stream << row << "\n";
  }
}

void Block::save() {
  save(savePath);
}

size_t Block::size() {
  return block_size;
}

std::string& Block::operator[](size_t i) {
  if (i >= data.size()) {
    data.resize(i + 1);
    block_size = data.size();
  }
  return data[i];
}

void Block::add(const std::string& s) {
  data.push_back(s);
  ++block_size;
}

Block::Block(const std::string& path, const std::string& block)
    : block(block),
      savePath(path)
{
  if (!std::filesystem::exists(path)) {
    block_size = 0;
    return;
  }
  std::ifstream stream(path);
  std::string s;
  std::getline(stream, s);
  block_size = std::stoi(s);
  for (size_t i = 0; i < block_size; ++i) {
    std::getline(stream, s);
    data.push_back(s);
  }
}

Block::Block(const std::string& block)
    : block(block), 
      data() 
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