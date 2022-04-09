#include <gtest/gtest.h>
#include "encoder.hpp"

using Enc = StrEncoder; // Put your encoder here for test.

TEST(encoder, simpleText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.message = "Hello world";
  sent.id = 15521;

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.message, received.message);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, emptyText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.message = "";
  sent.id = 0;

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.message, received.message);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, largeText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.message = std::string(10'000, 'x');
  sent.id = 0;

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.message, received.message);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, command) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::command;
  sent.message = "/command";

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.message, received.message);
  ASSERT_EQ(sent.type, received.type);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}