#ifndef UNFOLDER_CLIENT_H
#define UNFOLDER_CLIENT_H

#include "io.h"
#include "../libddag/ddag.h"
#include "dag.h"
#include <string>
#include <sys/sysinfo.h>
#include <cassert>

class Client {
private:
    IO *server_io;
    dDAG<State> *dag;
    int worker_threads;
public:
    Client(IO *server_io, dDAG<State> *dag);
    void add_vertex();
    void add_edge();
    void set_start();
    void next_phase();
    void work();
    void run();
};

#endif
