#ifndef UNFOLDER_MACHINE_H
#define UNFOLDER_MACHINE_H

#include "dag.h"
#include "../libddag/ddag.h"
#include "io.h"
#include "client.h"
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

class Machine;
#include "machine_list.h"

class Machine {
private:
    std::string host;
    int threads;
    std::thread *listener_thread,
                *client_thread;
    pid_t child;

    dDAG<State> *local_dag;
    bool _is_local;

    MachineList *available_machines;

    void listen();

public:
    IO *io;

    Machine(std::string connection_string, dDAG<State> *local_dag, MachineList *available_machines);
    std::string get_host();
    bool is_local();
    void run_client(int stdin_fd, int stdout_fd);
    void connect();
    void join();
};

#endif
