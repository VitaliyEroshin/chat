#include "ui.hpp"

std::string UserInterface::input(std::string invite, bool saveInvite) {
  inputInvite = invite;
  lineLength.push_front(invite.length());
  std::cout << invite;
  char input;

  while (!keyboard.isEnter(input)) {
    input = getchar();

    if (keyboard.isBackspace(input)) { 
      printBackspace();
      moveTailLeft(1);
      continue;
    }

    if (keyboard.isArrow(input)) {
      getchar();
      switch(getchar()) {
        case 67:
          if (buffer.right.empty())
            continue;

          buffer.moveLeft();
          coursor.moveRight();
          continue;
          
        case 68:
          if (buffer.left.empty())
            continue;
          
          buffer.moveRight();
          coursor.moveLeft();
          continue;
      }
    }
    
    std::cout << input;
    
    buffer.left.push_back(input);
    updateTail();
  }

  buffer.left.pop_back();
  buffer.moveLeftAll();

  std::string value = (saveInvite ? invite : "") + buffer.left;

  lineLength.push_front(buffer.left.length() + invite.length());

  if (lineLength.size() >= 64) {
    lineLength.pop_back();
  }

  std::cout << "\n\r";
  std::flush(std::cout);
  buffer.left = "";
  return value;
}

void UserInterface::printBackspace() {
  if (buffer.left.empty())
    return;

  buffer.left.pop_back();
  std::cout << "\b";
  std::flush(std::cout);
}

void UserInterface::clearPreviousLine() {
  size_t len = lineLength.front();
  coursor.moveUp();
  std::cout << "\r" << std::string(len, ' ') << "\r";
  std::flush(std::cout);
  lineLength.pop_front();
}

void UserInterface::updateWindowSize() {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
  windowHeight = size.ws_row;
  windowWidth = size.ws_col;
}

void UserInterface::updateTail() {
  for (auto &x : buffer.right) {
    std::cout << x;
  }

  for (auto &x : buffer.right) {
    coursor.moveLeft();
  }
}

void UserInterface::moveTailLeft(size_t n) {
  for (auto &x : buffer.right) {
    std::cout << x;
  }
  std::cout << std::string(n, ' ');
  for (int i = 0; i < n + buffer.right.size(); ++i) {
    coursor.moveLeft();
  }
}

void UserInterface::InputBuffer::moveLeft() {
  left.push_back(right.front());
  right.pop_front();
}

void UserInterface::InputBuffer::moveRight() {
  right.push_front(left.back());
  left.pop_back();
}

void UserInterface::InputBuffer::moveLeftAll() {
  while (!right.empty()) {
    moveLeft();
  }
}

void UserInterface::initWindow() {
  updateWindowSize();
  for (int i = 0; i < windowHeight; ++i) {
    printLine("");
  }
  std::flush(std::cout);
  for (int i = 0; i < windowHeight; ++i) {
    coursor.moveUp();
    lineLength.pop_back();
  }
}

void UserInterface::print(std::string s) {
  std::cout << s;
}

void UserInterface::printLine(std::string s) {
  lineLength.push_back(s.size());
  std::cout << s << "\n\r";
}

void UserInterface::hideBuffer() {
  
}

void UserInterface::showBuffer() {
  
}