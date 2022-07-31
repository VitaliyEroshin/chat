#pragma once
#include <iostream>
#include <string>
#include <deque>
#include <mutex>
#include <vector>

#include <sys/ioctl.h>
#include <unistd.h>
#include "types.hpp"

class Client;

class UserInterface {
public:
  struct Output {
    struct Window {
      size_t height;
      size_t width;
    };

    Output();
    ~Output() = default;

    Window window{};
    std::ostream& out;

    void flush() { std::flush(out); }
  };

  struct Cursor {
    struct Position {
      size_t x = 0;
      size_t y = 0;

      bool operator==(const Position& other) const
      { return x == other.x && y == other.y; }
    };

    enum Direction {left, right, up, down};

    Position position;
    Output& output;

    explicit Cursor(Output& output)
      : position({0, 0}), output(output) {};

    ~Cursor() = default;

    void move(Direction direction);

    static Direction get_direction(output_char_t c);

    void move_to(const Position& pos);
  };

  struct Keyboard {
    static bool is_tab(char x) { return x == 9; }
    static bool is_enter(char x) { return x == 13; }
    static bool is_backspace(char x) { return x == 127; }
    static bool is_arrow(char x) { return x == 27; }
  };

  struct Input {
    struct Buffer {
      std::deque<output_char_t> right;
      std::deque<int> right_width;
      std::deque<int> left_width;
      output_t left;

      size_t right_size = 0;
      size_t left_size = 0;
      size_t left_width_sum = 0;
      size_t right_width_sum = 0;

      void push_left(char c, int sz);
      void push_right(char c, int sz);

      void pop_left(bool popWidth = false);
      void pop_right(bool popWidth = false);

      int move_left();
      int move_right();
      void move_left_all();
      output_t get_right();
    };

    Output& output;
    Keyboard& keyboard;

    Buffer buffer;

    Input(Output& output, Keyboard& keyboard)
      : output(output), keyboard(keyboard) {};
    ~Input() = default;
  };
  
  Output out;
  Cursor cursor;
  Input in;
  Keyboard keyboard;
  Client& client;
  std::mutex printing;

  UserInterface(Client& client);

  ~UserInterface();

  void alloc_cli_window_space(size_t n);

  [[maybe_unused]] void test_cli_window_corners();

  void print(const output_t& s);
  int print(output_char_t c, bool move = true);
  void print(Cursor::Position pivot, Cursor::Position size, const output_t& text);
  void print(Cursor::Position pivot, const output_t& text);
  void print_lines(Cursor::Position pivot, const std::vector<std::string>& lines);
  void clear_space(Cursor::Position pivot, Cursor::Position size);
  
  static int char_size(char c);
  static size_t get_text_width(const std::string& s);
  output_t input(Cursor::Position pivot, Cursor::Position size, bool dynamic = false);
  output_t ask_form(Cursor::Position pivot, Cursor::Position size, const output_t& text);

private:
  void proc_input_tab(Cursor::Position pivot, Cursor::Position end);
  void proc_input_arrow(Cursor::Position pivot, Cursor::Position end);
  void proc_input_backspace(Cursor::Position& pivot, Cursor::Position end, Cursor::Position& size, bool dynamic);
  void proc_input_buffer(Cursor::Position pivot, Cursor::Position size);
  void log(size_t cell, output_t s);

  void scroll_chat_up();
  void scroll_chat_down();

  void alloc_input_space(Cursor::Position& pivot, Cursor::Position& size);
  void dealloc_input_space(Cursor::Position& pivot, Cursor::Position& size);

public:
  void clear_cli_window();

  size_t get_cli_window_height() const { return out.window.height; };
  size_t get_cli_window_width() const { return out.window.width; }
};