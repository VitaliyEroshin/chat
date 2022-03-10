#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

class UserInterface {
public:
  int windowHeight;
  int windowWidth;
  void updateWindowSize();
  void clearPreviousLine(size_t lineLength);
  void printBackspace();
  
  std::string inputBuffer = "";
  std::string input(std::string invite, bool saveInvite = false);
};