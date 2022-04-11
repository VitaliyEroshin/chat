#include <gtest/gtest.h>
#include "encoder.hpp"

using Enc = StrEncoder; // Put your encoder here for test.

TEST(encoder, simpleText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.content = "Hello world";
  sent.setId(1555);

  ASSERT_EQ(sent.hasId(), true);
  ASSERT_EQ(sent.id, 1555);

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.content, received.content);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, emptyText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.content = "";
  sent.setId(1555);

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.content, received.content);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, largeText) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::text;
  sent.content = std::string(10'000, 'x');
  sent.setId(0);

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.content, received.content);
  ASSERT_EQ(sent.id, received.id);
  ASSERT_EQ(sent.type, received.type);
}

TEST(encoder, command) {
  Enc enc;
  Object sent;
  sent.type = Object::Type::command;
  sent.content = "/command";

  Object received = enc.decode(enc.encode(sent));

  ASSERT_EQ(sent.content, received.content);
  ASSERT_EQ(sent.type, received.type);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}