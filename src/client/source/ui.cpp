#include "ui.hpp"
#include "client.hpp"

void UserInterface::print(
        Cursor::Position pivot, Cursor::Position size, const output_t& text) {
  
  auto pos = cursor.position;
  cursor.moveTo(pivot);

  for (size_t i = 0; i < size.x * size.y; ++i) {
    print((i < text.size() ? text[i] : ' '));

    if (i != 0 && (i + 1) % size.y == 0) {
      if (cursor.position.x == pivot.x + size.x - 1) {
        continue;
      }
      cursor.moveTo({cursor.position.x + 1, pivot.y});
    }
  }
  cursor.moveTo(pos);
  out.flush();
}

void UserInterface::print(
        Cursor::Position pivot, const output_t& text) {
  
  print(pivot, {1, text.size()}, text);
}

void UserInterface::log(size_t cell, output_t s) {
  print({getWindowHeight() - 1, getWindowWidth() - 21 - 20 * cell}, {1, 20}, s);
  out.flush();
}

output_t UserInterface::input(
        Cursor::Position pivot, Cursor::Position size, size_t characterLimit) {

  output_char_t c = 0;
  auto pos = cursor.position;
  cursor.moveTo(pivot);
  out.flush();

  Cursor::Position end{pivot.x + size.x - 1, pivot.y + size.y - 1};
  while (!UserInterface::Keyboard::isEnter(c)) {
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
      processInputBackspace(pivot, end, size);
      continue;
    }

    if (in.buffer.right.size() + in.buffer.left.size() == size.x * size.y) {
      continue;
    }
    
    if (!std::isalnum(c) && !std::ispunct(c) && !std::isblank(c)) {
      continue;
    }

    in.buffer.left.push_back(c);
    print(c);
    out.flush();

    if (!in.buffer.right.empty()) {
      refreshInputBuffer(pivot, size);
    }
    
    if (cursor.position.y == end.y + 1 && cursor.position.x != end.x) {
      cursor.moveTo({cursor.position.x + 1, pivot.y});
    }
  }

  if (UserInterface::Keyboard::isEnter(in.buffer.left.back())) {
    in.buffer.left.pop_back();
  }

  in.buffer.moveLeftAll();
  output_t value = in.buffer.left;
  in.buffer.left.clear();
  cursor.moveTo(pos);
  out.flush();
  return value;
}

output_t UserInterface::askForm(Cursor::Position pivot, Cursor::Position size, const output_t& text) {
  print(pivot, text);
  pivot.y += text.size();
  return input(pivot, size);
}

void UserInterface::processInputTab(Cursor::Position pivot, Cursor::Position end) {
  
}

void UserInterface::processInputArrow(Cursor::Position pivot, Cursor::Position end) {
  getchar();
  auto direction = UserInterface::Cursor::getDirection(getchar());
  if (direction == Cursor::Direction::left) {
    if (cursor.position == pivot || in.buffer.left.empty())
      return;
      
    in.buffer.moveRight();

    if (cursor.position.y == pivot.y) {
      cursor.moveTo({cursor.position.x - 1, end.y});
    } else {
      cursor.move(direction);
    }
  }
  if (direction == Cursor::Direction::right) {
    if (cursor.position == Cursor::Position{end.x, end.y + 1} || in.buffer.right.empty())
      return;

    in.buffer.moveLeft();

    if (cursor.position.y == end.y && cursor.position.x != end.x) {
      cursor.moveTo({cursor.position.x + 1, pivot.y});
    } else {
      cursor.move(direction);
    }
  }
  if (direction == Cursor::Direction::up) {
    if (cursor.position.x == pivot.x) {
      scrollChatUp();
      return;
    }
  }
  if (direction == Cursor::Direction::down) {
    if (cursor.position.x == end.x) {
      scrollChatDown();
      return;
    }
  }
}

void UserInterface::refreshInputBuffer(Cursor::Position pivot, Cursor::Position size) {
  output_t buffer = in.buffer.left + in.buffer.getRight();
  print(pivot, size, buffer);
}

void UserInterface::processInputBackspace(Cursor::Position pivot, Cursor::Position end, Cursor::Position size) {
  if (cursor.position == pivot) {
    return;
  }
  
  in.buffer.left.pop_back();
  if (cursor.position.y == pivot.y) {
    cursor.moveTo({cursor.position.x - 1, end.y});
  } else {
    cursor.move(Cursor::Direction::left);
  }
  
  refreshInputBuffer(pivot, size);
  out.flush();
}

void UserInterface::clearWindow() {
  print({0, 0}, {getWindowHeight(), getWindowWidth() - 1}, "");
}

void UserInterface::Input::Buffer::moveLeft() {
  left.push_back(right.front());
  right.pop_front();
}

void UserInterface::Input::Buffer::moveRight() {
  right.push_front(left.back());
  left.pop_back();
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
  for (size_t i = right.size(); i > 0; --i) {
    s.push_back(right[i - 1]);
  }
  return s;
}

UserInterface::UserInterface(Client& client)
  : out(Output()), cursor(out), in(Input(out, keyboard)), client(client) {
  system("stty raw -echo");
  allocateSpace(getWindowHeight() - 1);
}

UserInterface::~UserInterface() {
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
  out.out << s;
  cursor.position.y += s.size();
}

void UserInterface::print(output_char_t c) {
  out.out << c;
  ++cursor.position.y;
}

void UserInterface::scrollChatDown() {
  client.scrolldown();
}

void UserInterface::scrollChatUp() {
  client.scrollup();
}