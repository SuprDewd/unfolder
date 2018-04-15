#ifndef DDAG_STATE_H
#define DDAG_STATE_H

#include <string>
#include <vector>

class State {
public:
    bool https;
    std::string host;
    std::vector<char> path; // TODO: Use char*?

    // TODO: Connection

    State();
};

#endif
