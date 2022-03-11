#include <iostream>
#include <string>
#include <deque>
#include <sys/ioctl.h>
#include <unistd.h>

class UserInterface {
public:
  struct Coursor {
    void moveUp() { std::cout << char(27) << char(91) << char(65); }
    void moveDown() { std::cout << char(27) << char(91) << char(66); }
    void moveLeft() { std::cout << char(27) << char(91) << char(68); }
    void moveRight() { std::cout << char(27) << char(91) << char(67); }
  };

  struct Keyboard {
    bool isEnter(char x) { return x == 13; }
    bool isBackspace(char x) { return x == 127; }
    bool isArrow(char x) { return x == 27; } 
  };

  struct InputBuffer {
    std::deque<char> right;
    std::string left;

    void moveLeft();
    void moveRight();
    void moveLeftAll();
  };
  

  Coursor coursor;
  InputBuffer buffer;
  Keyboard keyboard;
  
  int windowHeight;
  int windowWidth;
  void updateWindowSize();
  void clearPreviousLine();
  void clearLine(size_t line);
  void printBackspace();
  void updateTail();
  void moveTailLeft(size_t n);

  std::deque<size_t> lineLength;
  
  std::string input(std::string invite, bool saveInvite = false);
};