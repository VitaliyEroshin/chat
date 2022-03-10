#include "ui.hpp"

std::string UserInterface::input(std::string invite, bool saveInvite) {
  std::cout << invite;
  char inputChar;
  while (inputChar != 13) {
    inputChar = getchar();

    if (inputChar == 127) { 
      // Backspace
      printBackspace();
      continue;
    } else if (inputChar == 27) {
      getchar();
      switch(getchar()) {
        case 'A':
          // code for arrow up
          break;
        case 'B':
          // code for arrow down
          break;
        case 67:
          std::cout << char(27) << char(91) << char(67);
          break;
        case 68:
          std::cout << char(27) << char(91) << char(68);
          break;
      }
    }

    if (inputChar != 13 && !std::isalpha(inputChar) && !std::iswalpha(inputChar) && !std::isdigit(inputChar)) {
      continue;
    }
    std::cout << inputChar;
    inputBuffer.push_back(inputChar);
  }
  inputBuffer.pop_back();
  
  std::string value = (saveInvite ? invite : "") + inputBuffer;
  clearPreviousLine(inputBuffer.length() + invite.length());
  inputBuffer = "";
  return value;
}

void UserInterface::printBackspace() {
  if (!inputBuffer.empty()) {
    inputBuffer.pop_back();
    std::cout << "\b \b";
    std::flush(std::cout);
  }
}

void UserInterface::clearPreviousLine(size_t lineLength) {
  std::cout << "\r" << std::string(lineLength, ' ') << "\r";
  std::flush(std::cout);
}

void UserInterface::updateWindowSize() {
  struct winsize size;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
}