#include "reader.h"
#include <cassert>

Reader::Reader(const std::string &s) : s(s) {
    this->at = 0;
}

bool Reader::has_next() {
    return this->at < this->s.size();
}

char Reader::peek() {
    assert(this->has_next());
    return this->s[at];
}

char Reader::next() {
    assert(this->has_next());
    return this->s[this->at++];
}

bool Reader::has_char() {
    if (!this->has_next()) return false;
    if (this->peek() == '[' || this->peek() == ']' || this->peek() == ',') return false;
    return 32 <= this->peek() && this->peek() <= 126;
}

char Reader::next_char() {
    assert(this->has_char());
    if (this->peek() == '\\') {
        this->next();
        assert(this->has_next());
        assert(this->peek() == '\\' || this->peek() == '[' || this->peek() == ']' || this->peek() == ',');
        return this->next();
    }
    return this->next();
}

