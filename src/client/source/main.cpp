#include "client.hpp"
#include "ui.hpp"

int main() {

  UserInterface ui;

  system("stty raw");
  
  while (ui.input("Enter something: ") != "quit") {
 
  }

  system("stty cooked");
}