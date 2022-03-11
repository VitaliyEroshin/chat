#include "client.hpp"
#include "ui.hpp"

int main() {

  UserInterface ui;

  system("stty raw");
  
  // while (ui.input("Enter something: ") != "quit") {
  //   ui.clearPreviousLine();
  // }
  ui.input("Enter something: ");
  ui.input("Enter something again: ");
  ui.clearPreviousLine();
  ui.clearPreviousLine();

  system("stty cooked");
}