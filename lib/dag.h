#ifndef UNFOLDER_DAG_H
#define UNFOLDER_DAG_H

#include "dag_state.h"
#include "io.h"
#include "../libddag/ddag.h"
#include "../libddag/ddag.cpp"
#include <vector>
#include <string>
#include <sstream>

class SetBaseUrl : public Edge<State> {
private:
    bool https;
    std::string host, path;

public:
    static const int ID = 0;

    SetBaseUrl(bool https, std::string host, std::string path);
    virtual bool forward(State *state) const;
    virtual void backward(State *state) const;
};

class AppendPath : public Edge<State> {
private:
    std::string append;

public:
    static const int ID = 1;

    AppendPath(std::string append);
    virtual bool forward(State *state) const;
    virtual void backward(State *state) const;
};

class PerformRequest : public Edge<State> {
private:
    IO *io;

public:
    static const int ID = 2;
    int w;

    PerformRequest(IO *io, int w);
    virtual bool forward(State *state) const;
    virtual void backward(State *state) const;
};

#endif
