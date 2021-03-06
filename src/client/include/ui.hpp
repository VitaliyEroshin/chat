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

    ~Cursor() = default;

    void move(Direction direction);

    static Direction getDirection(output_char_t c);

    void moveTo(const Position& pos);
  };

  struct Keyboard {
    static bool isTab(char x) { return x == 9; }
    static bool isEnter(char x) { return x == 13; }
    static bool isBackspace(char x) { return x == 127; }
    static bool isArrow(char x) { return x == 27; }
  };

  struct Input {
    struct Buffer {
      std::deque<output_char_t> right;
      std::deque<int> rightWidth;
      output_t left;
      std::deque<int> leftWidth;

      size_t rightSize = 0;
      size_t leftSize = 0;
      size_t leftWidthSum = 0;
      size_t rightWidthSum = 0;

      void pushLeft(char c, int sz);
      void pushRight(char c, int sz);

      void popLeft(bool popWidth = false);
      void popRight(bool popWidth = false);

      int moveLeft();
      int moveRight();
      void moveLeftAll();
      output_t getRight();
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

  void allocateSpace(size_t n);

  [[maybe_unused]] void testWindowCorners();

  void print(const output_t& s);
  int print(output_char_t c, bool move = true);
  void print(Cursor::Position pivot, Cursor::Position size, const output_t& text);
  void print(Cursor::Position pivot, const output_t& text);
  void printLines(Cursor::Position pivot, const std::vector<std::string>& lines);
  void clearSpace(Cursor::Position pivot, Cursor::Position size);
  
  static int characterSize(char c);
  static size_t getTextRealSize(const std::string& s);
  output_t input(Cursor::Position pivot, Cursor::Position size, bool dynamic = false);
  output_t askForm(Cursor::Position pivot, Cursor::Position size, const output_t& text);

private:
  void processInputTab(Cursor::Position pivot, Cursor::Position end);
  void processInputArrow(Cursor::Position pivot, Cursor::Position end);
  void processInputBackspace(Cursor::Position& pivot, Cursor::Position end, Cursor::Position& size, bool dynamic);
  void refreshInputBuffer(Cursor::Position pivot, Cursor::Position size);
  void log(size_t cell, output_t s);

  void scrollChatUp();
  void scrollChatDown();

  void allocateChatSpace(Cursor::Position& pivot, Cursor::Position& size);
  void deallocateChatSpace(Cursor::Position& pivot, Cursor::Position& size);

public:
  void clearWindow();

  size_t getWindowHeight() const { return out.window.height; };
  size_t getWindowWidth() const { return out.window.width; }
};