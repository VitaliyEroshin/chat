#include "ui.hpp"
#include "client.hpp"

void UserInterface::print(
        Cursor::Position pivot, Cursor::Position size, const output_t& text) {

  std::lock_guard<std::mutex> lock(printing);
  auto pos = cursor.position;
  cursor.move_to(pivot);
  auto it = text.begin();

  static int digit = 0;
  ++digit;
  digit %= 10;

  int printed_len = 0;
  for (size_t i = 0; i < size.x * size.y;) {
    if (it == text.end()) {
      print(' ');
      ++i;
      ++printed_len;
    } else {
      size_t length = char_size(*it);
      
      while (length--) {
        int width = print(*it, false);
        if (width != -1) {
          i += width;
          printed_len += width;
        }
        ++it;
      }
    }

    if (printed_len >= size.y) {
      if (cursor.position.x >= pivot.x + size.x - 1) {
        continue;
      }
      cursor.move_to({cursor.position.x + 1, pivot.y});
      printed_len = 0;
    }
  }
  cursor.move_to(pos);
  out.flush();
}

size_t UserInterface::get_text_width(const std::string& s) {
  wchar_t cstr[1024];
  mbstowcs(cstr, s.c_str(), 1024);

  return wcswidth(cstr, 1024);
}

void UserInterface::print(
        Cursor::Position pivot, const output_t& text) {
  
  print(pivot, {1, get_text_width(text)}, text);
}

void UserInterface::print_lines(
  Cursor::Position pivot, const std::vector<std::string>& lines) {

  for (const auto &x : lines) {
    print(pivot, x);
    ++pivot.x;
  }
}

void UserInterface::clear_space(Cursor::Position pivot, Cursor::Position size) {
  print(pivot, size, "");
}

void UserInterface::log(size_t cell, output_t s) {
  print({get_cli_window_height() - 1, get_cli_window_width() - 21 - 20 * cell}, {1, 20}, s);
  out.flush();
}

int UserInterface::char_size(char c) {
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
  cursor.move_to(pivot);
  out.flush();
  Cursor::Position end{pivot.x + size.x - 1, pivot.y + size.y - 1};
  while (!UserInterface::Keyboard::is_enter(c)) {
    c = getchar();
    
    if (UserInterface::Keyboard::is_tab(c)) {
        proc_input_tab(pivot, end);
      continue;
    }

    if (UserInterface::Keyboard::is_arrow(c)) {
        proc_input_arrow(pivot, end);
      continue;
    }

    if (UserInterface::Keyboard::is_backspace(c)) {
        proc_input_backspace(pivot, end, size, dynamic);
      continue;
    }

    const int maxLength = 768;
    if (in.buffer.right_size + in.buffer.left_size >= maxLength) {
      continue;
    }

    int bufferSize = in.buffer.right_width_sum + in.buffer.left_width_sum;
    if (dynamic && bufferSize >= size.x * size.y - 2) {
        alloc_input_space(pivot, size);
    } else if (bufferSize >= size.x * size.y) {
      continue;
    }

    if (UserInterface::Keyboard::is_enter(c)) {
      continue;
    }
    {
      std::lock_guard<std::mutex> lock(printing);
        in.buffer.push_left(c, print(c));
      int sz = char_size(c);
      for (int i = 0; i < sz - 1; ++i) {
        c = getchar();
          in.buffer.push_left(c, print(c));
        
      }
      out.flush();
    }

    if (!in.buffer.right.empty()) {
        proc_input_buffer(pivot, size);
    }
    
    if (cursor.position.y >= end.y + 1 && cursor.position.x != end.x) {
      cursor.move_to({cursor.position.x + 1, pivot.y});
    }
  }
  
  
  if (UserInterface::Keyboard::is_enter(in.buffer.left.back())) {
      in.buffer.pop_left();
  }


    in.buffer.move_left_all();
  output_t value = in.buffer.left;
  in.buffer.left.clear();
  in.buffer.left_size = 0;
  in.buffer.left_width_sum = 0;
  in.buffer.left_width.clear();
  cursor.move_to(pos);
  out.flush();
  return value;
}

output_t UserInterface::ask_form(Cursor::Position pivot, Cursor::Position size, const output_t& text) {
  print(pivot, text);
  pivot.y += get_text_width(text);
  return input(pivot, size);
}

void UserInterface::proc_input_tab(Cursor::Position pivot, Cursor::Position end) {
  // TODO
}

void UserInterface::proc_input_arrow(Cursor::Position pivot, Cursor::Position end) {
  getchar();
  auto direction = UserInterface::Cursor::get_direction(getchar());
  std::lock_guard<std::mutex> lock(printing);
  int times = 0;
  if (direction == Cursor::Direction::left) {
    if (cursor.position == pivot || in.buffer.left.empty())
      return;
      
    times = in.buffer.move_right();

    if (cursor.position.y == pivot.y) {
      cursor.move_to({cursor.position.x - 1, end.y});
      return;
    }
  } else if (direction == Cursor::Direction::right) {
    if (cursor.position == Cursor::Position{end.x, end.y + 1} || in.buffer.right.empty())
      return;

    times = in.buffer.move_left();

    if (cursor.position.y >= end.y && cursor.position.x != end.x) {
      cursor.move_to({cursor.position.x + 1, pivot.y});
      return;
    }
  } else if (direction == Cursor::Direction::up) {
    if (in.buffer.left_width_sum < end.y - pivot.y) {
        scroll_chat_up();
      return;
    }

    for (times = 0; times <= end.y - pivot.y;) {
      times += in.buffer.move_right();
    }

    direction = Cursor::Direction::left;
  } else {
    if (in.buffer.right_width_sum < end.y - pivot.y) {
      scroll_chat_down();
      return;
    }

    for (times = 0; times <= end.y - pivot.y;) {
      times += in.buffer.move_left();
    }
    direction = Cursor::Direction::right;
  }

  for (int i = 0; i < times; ++i) {
    if (direction == Cursor::Direction::left && cursor.position.y == pivot.y) {
      cursor.move_to({cursor.position.x - 1, end.y});
      continue;
    } else if (direction == Cursor::Direction::right && cursor.position.y >= end.y && cursor.position.x != end.x) {
      cursor.move_to({cursor.position.x + 1, pivot.y});
      continue;
    }
    cursor.move(direction);
  }
}

void UserInterface::proc_input_buffer(Cursor::Position pivot, Cursor::Position size) {
  output_t buffer = in.buffer.left + in.buffer.get_right();
  print(pivot, size, buffer);
}

void UserInterface::proc_input_backspace(Cursor::Position& pivot, Cursor::Position end, Cursor::Position& size, bool dynamic) {
  if (cursor.position == pivot) {
    return;
  }

  while (!char_size(in.buffer.left.back())) {
      in.buffer.pop_left();
  }
  int times = in.buffer.left_width.back();
    in.buffer.pop_left(true);

  if (cursor.position.y == pivot.y) {
    cursor.move_to({cursor.position.x - 1, end.y + 1});
  } 
  for (int i = 0; i < times; ++i)
    cursor.move(Cursor::Direction::left);
  

  if (dynamic && in.buffer.left_size + in.buffer.right_size < (size.x - 1) * size.y) {
      dealloc_input_space(pivot, size);
  }

    proc_input_buffer(pivot, size);
  out.flush();
}

void UserInterface::clear_cli_window() {
  print({0, 0}, {get_cli_window_height(), get_cli_window_width() - 1}, "");
}

void UserInterface::Input::Buffer::push_left(char c, int sz) {
  left.push_back(c);
  if (UserInterface::char_size(c))  {
      left_size += UserInterface::char_size(c);
  }
  if (sz != -1) {
    left_width.push_back(sz);
      left_width_sum += sz;
  }
}

void UserInterface::Input::Buffer::push_right(char c, int sz) {
  right.push_front(c);
  if (UserInterface::char_size(c))  {
      right_size += UserInterface::char_size(c);
  }
  if (sz != -1) {
    right_width.push_front(sz);
      right_width_sum += sz;
  }
}

void UserInterface::Input::Buffer::pop_left(bool popWidth) {
  if (left.empty()) {
    return;
  }
  if (UserInterface::char_size(left.back())) {
      left_size -= UserInterface::char_size(left.back());
  }
  left.pop_back();
  if (popWidth) {
      left_width_sum -= left_width.back();
    left_width.pop_back();
  }
}

void UserInterface::Input::Buffer::pop_right(bool popWidth) {
  if (right.empty()) {
    return;
  }
  if (UserInterface::char_size(right.front())) {
      right_size -= UserInterface::char_size(right.front());
  }
  right.pop_front();
  if (popWidth) {
      right_width_sum -= right_width.front();
    right_width.pop_front();
  }
}

int UserInterface::Input::Buffer::move_left() {
    push_left(right.front(), right_width.front());
    pop_right(true);

  while (!right.empty() && !UserInterface::char_size(right.front())) {
      push_left(right.front(), -1);
      pop_right();
  }

  return left_width.back();
}

int UserInterface::Input::Buffer::move_right() {
  while (!left.empty() && !UserInterface::char_size(left.back())) {
      push_right(left.back(), -1);
      pop_left();
  }

    push_right(left.back(), left_width.back());
    pop_left(true);

  return right_width.front();
}

void UserInterface::Input::Buffer::move_left_all() {
  while (!right.empty()) {
      move_left();
  }
}

UserInterface::Output::Output(): out(std::cout) {
  winsize size{};
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
  get_direction(output_char_t c) {
  if (c == 65)
    return Direction::up;

  if (c == 66)
    return Direction::down;

  if (c == 67)
    return Direction::right;

  return Direction::left;
}

void UserInterface::Cursor::move_to(const Position& pos) {
  while (position.x < pos.x)
    move(Direction::down);

  while (position.x > pos.x)
    move(Direction::up);

  while (position.y < pos.y)
    move(Direction::right);

  while (position.y > pos.y)
    move(Direction::left);
}

output_t UserInterface::Input::Buffer::get_right() {
  output_t s;
  for (auto &x : right) {
    s.push_back(x);
  }
  return s;
}

UserInterface::UserInterface(Client& client)
  : out(Output()), cursor(out), in(Input(out, keyboard)), client(client) {
  system("stty raw -echo");
    alloc_cli_window_space(get_cli_window_height() - 1);
}

UserInterface::~UserInterface() {
  cursor.move_to({0, 0});
  system("stty cooked echo");
}

void UserInterface::alloc_cli_window_space(size_t n) {
  auto pos = cursor.position;
  cursor.move_to({0, 0});
  for (size_t i = 0; i < n; ++i) {
    out.out << "\n\r";
    ++cursor.position.x;
  }
  out.flush();
  cursor.move_to(pos);
  out.flush();
}

[[maybe_unused]] void UserInterface::test_cli_window_corners() {
  auto pos = cursor.position;

  cursor.move_to({0, 0});
  print("1");
  cursor.move_to({get_cli_window_height() - 1, 0});
  print("2");
  cursor.move_to({0, get_cli_window_width() - 1});
  print("3");
  cursor.move_to({get_cli_window_height() - 1, get_cli_window_width() - 1});
  print("4");

  cursor.move_to(pos);
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

  if (char_size(c)) {
    sz = char_size(c);
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

void UserInterface::scroll_chat_down() {
  client.scrolldown();
}

void UserInterface::scroll_chat_up() {
  client.scrollup();
}

void UserInterface::alloc_input_space(Cursor::Position& pivot, Cursor::Position& size) {
  ++client.chatspace;
  --pivot.x;
  ++size.x;
  auto pos = cursor.position;
  cursor.move(Cursor::Direction::up);
    client.draw_chat_ptr();
    proc_input_buffer(pivot, size);
  client.update.store(true);
}

void UserInterface::dealloc_input_space(Cursor::Position& pivot, Cursor::Position& size) {
  --client.chatspace;
  ++pivot.x;
  --size.x;
  cursor.move(Cursor::Direction::down);
    client.draw_chat_ptr();
    proc_input_buffer(pivot, size);
  client.update.store(true);
}