#include "constants.hpp"

std::map<std::string, std::any> ClientConfig::get_default_values() {
  // Net constants
  int port = 8888;

  // Duration constants
  int loadingBackgroundSpeed = 300;
  int refreshOutputDelay = 0;

  int connectionBackgroundDuration = 2000;
  int connectionFailedMessageDuration = 1000;

  int connectionSucceedMessageDuration = 300;

  std::map<std::string, std::any> default_values;
  default_values.insert({"port", port});
  default_values.insert({"loadingBackgroundSpeed", loadingBackgroundSpeed});
  default_values.insert({"refreshOutputDelay", refreshOutputDelay});
  default_values.insert({"connectionBackgroundDuration", connectionBackgroundDuration});
  default_values.insert({"connectionFailedMessageDuration", connectionFailedMessageDuration});
  default_values.insert({"connectionSucceedMessageDuration", connectionSucceedMessageDuration});

  return default_values;
}