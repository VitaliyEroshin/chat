#include "logger.hpp"

Logger::Logger(std::initializer_list<std::ostream*> streams): s(streams) {};