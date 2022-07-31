#include <gtest/gtest.h>
#include "logger.hpp"
#include <sstream>

TEST(logger, singleOstream) {
    std::stringstream ss;
    std::stringstream answer;
    
    Logger log = {&ss};
    log << "Hello world!";
    answer << "Hello world!";

    log << 5;
    answer << 5;

    log << 0.25;
    answer << 0.25;

    ASSERT_EQ(ss.str(), answer.str());
}

TEST(logger, multipleOstreams) {
    std::stringstream s1, s2, s3;

    std::stringstream answer;
    
    Logger log = {&s1, &s2, &s3};
    log << "Hello world!";
    answer << "Hello world!";

    log << 5;
    answer << 5;

    log << 0.25;
    answer << 0.25;

    ASSERT_EQ(s1.str(), answer.str());
    ASSERT_EQ(s2.str(), answer.str());
    ASSERT_EQ(s3.str(), answer.str());
}

TEST(logger, endlHandling) {
  std::stringstream s1, s2, s3;

    std::stringstream answer;
    
    Logger log = {&s1, &s2, &s3};
    
    log << std::endl;
    answer << std::endl;

    ASSERT_EQ(s1.str(), answer.str());
    ASSERT_EQ(s2.str(), answer.str());
    ASSERT_EQ(s3.str(), answer.str());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}