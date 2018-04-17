#include "tools.h"

bool endswith(const std::string &s, const std::string &t) {
    if (s.size() < t.size()) {
        return false;
    }
    for (size_t i = 0; i < t.size(); i++) {
        if (t[i] != s[s.size() - t.size() + i]) {
            return false;
        }
    }
    return true;
}

