#ifndef ADDICT_DAG_H
#define ADDICT_DAG_H

#include <string>

class AddictDAG {
public:
    virtual size_t add_vertex() = 0;
    virtual void add_basic_edge(size_t from, size_t to) = 0;
    virtual void add_append_edge(size_t from, const std::string &append, size_t to) = 0;
    // TODO: Handle function calls
};

#endif
