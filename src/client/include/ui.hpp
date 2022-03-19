#pragma once
#include <iostream>
#include <string>
#include <deque>

#include <sys/ioctl.h>
#include <unistd.h>
#include "types.hpp"

class UserInterface {
public:
  struct Output {
    struct Window {
      size_t height;
      size_t width;
    };

    Output();

    Window window;
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

    void move(Direction direction);

    static Direction getDirection(output_char_t c);

    void moveTo(const Position& pos);
  };

  struct Keyboard {
    static bool isEnter(char x) { return x == 13; }
    static bool isBackspace(char x) { return x == 127; }
    static bool isArrow(char x) { return x == 27; }
  };

  struct Input {
    struct Buffer {
      std::deque<output_char_t> right;
      output_t left;

      void moveLeft();
      void moveRight();
      void moveLeftAll();
      output_t getRight();
    };

    Output& output;
    Keyboard& keyboard;

    Buffer buffer;

    Input(Output& output, Keyboard& keyboard)
      : output(output), keyboard(keyboard) {};
  };
  
  Output out;
  Cursor cursor;
  Input in;
  Keyboard keyboard;

  UserInterface();

  ~UserInterface();

  void allocateSpace(size_t n);

  [[maybe_unused]] void testWindowCorners();

  void print(const output_t& s);
  void print(output_char_t c);
  void print(Cursor::Position pivot, Cursor::Position size, const output_t& text);
  void print(Cursor::Position pivot, const output_t& text);

  output_t input(Cursor::Position pivot, Cursor::Position size, size_t characterLimit = 256);
  output_t askForm(Cursor::Position pivot, Cursor::Position size, const output_t& text);

private:
  void processInputArrow(Cursor::Position pivot, Cursor::Position end);
  void processInputBackspace(Cursor::Position pivot, Cursor::Position end, Cursor::Position size);
  void refreshInputBuffer(Cursor::Position pivot, Cursor::Position size);
  void log(size_t cell, output_t s);

public:
  void clearWindow();

  size_t getWindowHeight() const { return out.window.height; };
  size_t getWindowWidth() const { return out.window.width; }

  void scrollChatUp() {
    // TODO
  }

  void scrollChatDown() {
    // TODO
  }
};