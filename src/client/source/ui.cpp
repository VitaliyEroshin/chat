#include "ui.hpp"
#include "client.hpp"

void UserInterface::print(
        Cursor::Position pivot, Cursor::Position size, const output_t& text) {

  std::lock_guard<std::mutex> lock(printing);
  auto pos = cursor.position;
  cursor.moveTo(pivot);
  auto it = text.begin();

  static int digit = 0;
  ++digit;
  digit %= 10;

  int printedLength = 0;
  for (size_t i = 0; i < size.x * size.y;) {
    if (it == text.end()) {
      print(' ');
      ++i;
      ++printedLength;
    } else {
      size_t length = characterSize(*it);
      
      while (length--) {
        int width = print(*it, false);
        if (width != -1) {
          i += width;
          printedLength += width;
        }
        ++it;
      }
    }

    if (printedLength >= size.y) {
      if (cursor.position.x >= pivot.x + size.x - 1) {
        continue;
      }
      cursor.moveTo({cursor.position.x + 1, pivot.y});
      printedLength = 0;
    }
  }
  cursor.moveTo(pos);
  out.flush();
}

size_t UserInterface::getTextRealSize(const std::string& s) {
  wchar_t cstr[1024];
  mbstowcs(cstr, s.c_str(), 1024);

  return wcswidth(cstr, 1024);
}

void UserInterface::print(
        Cursor::Position pivot, const output_t& text) {
  
  print(pivot, {1, getTextRealSize(text)}, text);
}

void UserInterface::log(size_t cell, output_t s) {
  print({getWindowHeight() - 1, getWindowWidth() - 21 - 20 * cell}, {1, 20}, s);
  out.flush();
}

int UserInterface::characterSize(char c) {
  if ((c & 0x80) == 0)
    return 1;

  if ((c & 0xE0) == 0xC0)
    return 2;

  if ((c & 0xF0) == 0xE0)
    return 3;

  if ((c & 0xF8) == 0xF0)
    return 4;
  
  return 0;
}

output_t UserInterface::input(
        Cursor::Position pivot, Cursor::Position size, bool dynamic) {

  char c = 0;
  auto pos = cursor.position;
  cursor.moveTo(pivot);
  out.flush();
  Cursor::Position end{pivot.x + size.x - 1, pivot.y + size.y - 1};
  while (!UserInterface::Keyboard::isEnter(c)) {
    log(0, "R" + std::to_string(in.buffer.left.size()));
    log(1, "L" + std::to_string(in.buffer.right.size()));
    c = getchar();
    
    if (UserInterface::Keyboard::isTab(c)) {
      processInputTab(pivot, end);
      continue;
    }

    if (UserInterface::Keyboard::isArrow(c)) {
      processInputArrow(pivot, end);
      continue;
    }

    if (UserInterface::Keyboard::isBackspace(c)) {
      processInputBackspace(pivot, end, size, dynamic);
      continue;
    }

    const int maxLength = 768;
    if (in.buffer.rightSize + in.buffer.leftSize >= maxLength) {
      continue;
    }

    int bufferSize = in.buffer.rightWidthSum + in.buffer.leftWidthSum;
    if (dynamic && bufferSize >= size.x * size.y - 2) {
      allocateChatSpace(pivot, size);
    } else if (bufferSize >= size.x * size.y) {
      continue;
    }

    if (UserInterface::Keyboard::isEnter(c)) {
      continue;
    }
    {
      std::lock_guard<std::mutex> lock(printing);
      in.buffer.pushLeft(c, print(c));
      int sz = characterSize(c);
      for (int i = 0; i < sz - 1; ++i) {
        c = getchar();
        in.buffer.pushLeft(c, print(c));
        
      }
      out.flush();
    }

    if (!in.buffer.right.empty()) {
      refreshInputBuffer(pivot, size);
    }
    
    if (cursor.position.y >= end.y + 1 && cursor.position.x != end.x) {
      cursor.moveTo({cursor.position.x + 1, pivot.y});
    }
  }
  
  
  if (UserInterface::Keyboard::isEnter(in.buffer.left.back())) {
    in.buffer.popLeft();
  }

  
  in.buffer.moveLeftAll();
  output_t value = in.buffer.left;
  in.buffer.left.clear();
  in.buffer.leftSize = 0;
  in.buffer.leftWidthSum = 0;
  in.buffer.leftWidth.clear();
  cursor.moveTo(pos);
  out.flush();
  return value;
}

output_t UserInterface::askForm(Cursor::Position pivot, Cursor::Position size, const output_t& text) {
  print(pivot, text);
  pivot.y += getTextRealSize(text);
  return input(pivot, size);
}

void UserInterface::processInputTab(Cursor::Position pivot, Cursor::Position end) {
  
}

void UserInterface::processInputArrow(Cursor::Position pivot, Cursor::Position end) {
  getchar();
  auto direction = UserInterface::Cursor::getDirection(getchar());
  std::lock_guard<std::mutex> lock(printing);
  int times = 0;
  if (direction == Cursor::Direction::left) {
    if (cursor.position == pivot || in.buffer.left.empty())
      return;
      
    times = in.buffer.moveRight();

    if (cursor.position.y == pivot.y) {
      cursor.moveTo({cursor.position.x - 1, end.y});
      return;
    }
  } else if (direction == Cursor::Direction::right) {
    if (cursor.position == Cursor::Position{end.x, end.y + 1} || in.buffer.right.empty())
      return;

    times = in.buffer.moveLeft();

    if (cursor.position.y >= end.y && cursor.position.x != end.x) {
      cursor.moveTo({cursor.position.x + 1, pivot.y});
      return;
    }
  } else if (direction == Cursor::Direction::up) {
    if (cursor.position.x == pivot.x) {
      scrollChatUp();
      return;
    }

    for (size_t i = 0; i <= end.y - pivot.y; ++i) {
      in.buffer.moveRight();
    }
  } else if (direction == Cursor::Direction::down) {
    if (cursor.position.x == end.x) {
      scrollChatDown();
      return;
    }
  
    for (size_t i = 0; i <= end.y - pivot.y; ++i) {
      in.buffer.moveLeft();
    }
  }
  for (int i = 0; i < times; ++i)
    cursor.move(direction);
}

void UserInterface::refreshInputBuffer(Cursor::Position pivot, Cursor::Position size) {
  output_t buffer = in.buffer.left + in.buffer.getRight();
  print(pivot, size, buffer);
}

void UserInterface::processInputBackspace(Cursor::Position& pivot, Cursor::Position end, Cursor::Position& size, bool dynamic) {
  if (cursor.position == pivot) {
    return;
  }

  while (!characterSize(in.buffer.left.back())) {
    in.buffer.popLeft();
  }
  int times = in.buffer.leftWidth.back();
  in.buffer.popLeft(true);

  if (cursor.position.y == pivot.y) {
    cursor.moveTo({cursor.position.x - 1, end.y + 1});
  } 
  for (int i = 0; i < times; ++i)
    cursor.move(Cursor::Direction::left);
  

  if (dynamic && in.buffer.leftSize + in.buffer.rightSize < (size.x - 1) * size.y) {
    deallocateChatSpace(pivot, size);
  }
  
  refreshInputBuffer(pivot, size);
  out.flush();
}

void UserInterface::clearWindow() {
  print({0, 0}, {getWindowHeight(), getWindowWidth() - 1}, "");
}

void UserInterface::Input::Buffer::pushLeft(char c, int sz) {
  left.push_back(c);
  if (UserInterface::characterSize(c))  {
    leftSize += UserInterface::characterSize(c);
  }
  if (sz != -1) {
    leftWidth.push_back(sz);
    leftWidthSum += sz;
  }
}

void UserInterface::Input::Buffer::pushRight(char c, int sz) {
  right.push_front(c);
  if (UserInterface::characterSize(c))  {
    rightSize += UserInterface::characterSize(c);
  }
  if (sz != -1) {
    rightWidth.push_front(sz);
    rightWidthSum += sz;
  }
}

void UserInterface::Input::Buffer::popLeft(bool popWidth) {
  if (left.empty()) {
    return;
  }
  if (UserInterface::characterSize(left.back())) {
    leftSize -= UserInterface::characterSize(left.back());
  }
  left.pop_back();
  if (popWidth) {
    leftWidthSum -= leftWidth.back();
    leftWidth.pop_back();
  }
}

void UserInterface::Input::Buffer::popRight(bool popWidth) {
  if (right.empty()) {
    return;
  }
  if (UserInterface::characterSize(right.front())) {
    rightSize -= UserInterface::characterSize(right.front());
  }
  right.pop_front();
  if (popWidth) {
    rightWidthSum -= rightWidth.front();
    rightWidth.pop_front();
  }
}

int UserInterface::Input::Buffer::moveLeft() {
  pushLeft(right.front(), rightWidth.front());
  popRight(true);

  while (!right.empty() && !UserInterface::characterSize(right.front())) {
    pushLeft(right.front(), -1);
    popRight();
  }

  return leftWidth.back();
}

int UserInterface::Input::Buffer::moveRight() {
  while (!left.empty() && !UserInterface::characterSize(left.back())) {
    pushRight(left.back(), -1);
    popLeft();
  }

  pushRight(left.back(), leftWidth.back());
  popLeft(true);

  return rightWidth.front();
}

void UserInterface::Input::Buffer::moveLeftAll() {
  while (!right.empty()) {
    moveLeft();
  }
}

UserInterface::Output::Output(): out(std::cout) {
  winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  window.height = size.ws_row;
  window.width = size.ws_col;
};

void UserInterface::Cursor::move(Direction direction) {
  output.out << char(27) << char(91);
  if (direction == Direction::left) {
    --position.y;
    output.out << char(68);
  } else if (direction == Direction::right) {
    ++position.y;
    output.out << char(67);
  } else if (direction == Direction::up) {
    --position.x;
    output.out << char(65);
  } else if (direction == Direction::down) {
    ++position.x;
    output.out << char(66);
  }
}

UserInterface::Cursor::Direction UserInterface::Cursor::
  getDirection(output_char_t c) {
  if (c == 65)
    return Direction::up;

  if (c == 66)
    return Direction::down;

  if (c == 67)
    return Direction::right;

  return Direction::left;
}

void UserInterface::Cursor::moveTo(const Position& pos) {
  while (position.x < pos.x)
    move(Direction::down);

  while (position.x > pos.x)
    move(Direction::up);

  while (position.y < pos.y)
    move(Direction::right);

  while (position.y > pos.y)
    move(Direction::left);
}

output_t UserInterface::Input::Buffer::getRight() {
  output_t s;
  for (auto &x : right) {
    s.push_back(x);
  }
  return s;
}

UserInterface::UserInterface(Client& client)
  : out(Output()), cursor(out), in(Input(out, keyboard)), client(client) {
  system("stty raw -echo");
  allocateSpace(getWindowHeight() - 1);
}

UserInterface::~UserInterface() {
  cursor.moveTo({0, 0});
  system("stty cooked echo");
}

void UserInterface::allocateSpace(size_t n) {
  auto pos = cursor.position;
  cursor.moveTo({0, 0});
  for (size_t i = 0; i < n; ++i) {
    out.out << "\n\r";
    ++cursor.position.x;
  }
  out.flush();
  cursor.moveTo(pos);
  out.flush();
}

[[maybe_unused]] void UserInterface::testWindowCorners() {
  auto pos = cursor.position;

  cursor.moveTo({0, 0});
  print("1");
  cursor.moveTo({getWindowHeight() - 1, 0});
  print("2");
  cursor.moveTo({0, getWindowWidth() - 1});
  print("3");
  cursor.moveTo({getWindowHeight() - 1, getWindowWidth() - 1});
  print("4");

  cursor.moveTo(pos);
  out.flush();
}

void UserInterface::print(const output_t& s) {
  for (const auto &c : s) {
    print(c);
  }
}

int UserInterface::print(output_char_t c, bool move) {
  static char buffer[4];
  static wchar_t wcs[4];
  static size_t sz = 0;
  static size_t i = 0;

  if (characterSize(c)) {
    sz = characterSize(c);
    i = 0;
  } else {
    ++i;
  }
  
  buffer[i] = c;
  out.out << c;
  if (i + 1 == sz) {
    mbstowcs(wcs, buffer, 4);
    int width = wcswidth(wcs, 4);
    if (width != -1) {
      cursor.position.y += width;
      
      
    }

    for (size_t i = 0; i < 4; ++i) {
      wcs[i] = 0;
      buffer[i] = 0;
    }
    return width;
  }
  return -1;
}

void UserInterface::scrollChatDown() {
  client.scrolldown();
}

void UserInterface::scrollChatUp() {
  client.scrollup();
}

void UserInterface::allocateChatSpace(Cursor::Position& pivot, Cursor::Position& size) {
  ++client.chatspace;
  --pivot.x;
  ++size.x;
  auto pos = cursor.position;
  cursor.move(Cursor::Direction::up);
  client.drawChatPointer();
  refreshInputBuffer(pivot, size);
  client.update.store(true);
}

void UserInterface::deallocateChatSpace(Cursor::Position& pivot, Cursor::Position& size) {
  --client.chatspace;
  ++pivot.x;
  --size.x;
  cursor.move(Cursor::Direction::down);
  client.drawChatPointer();
  refreshInputBuffer(pivot, size);
  client.update.store(true);
}