#ifndef ADDICT_READER_H
#define ADDICT_READER_H

#include <string>

class Reader {
private:
    const std::string &s;
    size_t at;

public:
    Reader(const std::string &s);
    bool has_next();
    char peek();
    char next();
    bool has_char();
    char next_char();
};

#endif
