#include "objects.hpp"

bool bit(char x, int i) {
    return (x & (1<<i)) != 0;
}

void set(char& x, int i, bool value) {
    if (value) 
        x |= (1<<i);
    else if (bit(x, i)) 
        x ^= (1<<i);
}

void Object::setId(int id_) {
    id = id_;
    set(attributes, 0, true);
}

void Object::setAuthor(int author_) {
    author = author_;
    set(attributes, 1, true);
}

void Object::setTimestamp(int timestamp_) {
    timestamp = timestamp_;
    set(attributes, 2, true);
}

void Object::setThread(int thread_) {
    thread = thread_;
    set(attributes, 3, true);
}

void Object::setReply(int reply_) {
    reply = reply_;
    set(attributes, 4, true);
}

void Object::setPrev(int prev_) {
    prev = prev_;
    set(attributes, 5, true);
}

void Object::setNext(int next_) {
    next = next_;
    set(attributes, 6, true);
}

void Object::setReturnCode(int code_) {
    code = code_;
    set(attributes, 7, true);
}

bool Object::hasId() const
{ return bit(attributes, 0); }

bool Object::hasAuthor() const
{ return bit(attributes, 1); }

bool Object::hasTimestamp() const
{ return bit(attributes, 2); }

bool Object::hasThread() const
{ return bit(attributes, 3); }

bool Object::hasReply() const
{ return bit(attributes, 4); }

bool Object::hasPrev() const
{ return bit(attributes, 5); }

bool Object::hasNext() const
{ return bit(attributes, 6); }

bool Object::hasReturnCode() const 
{ return bit(attributes, 7); }