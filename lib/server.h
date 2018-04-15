#ifndef UNFOLDER_SERVER_H
#define UNFOLDER_SERVER_H

#include "dag.h"
#include "machine.h"
#include "machine_list.h"
#include "io.h"
#include <vector>
#include <string>
#include <thread>

class Server {
private:
    dDAG<State> *dag = new dDAG<State>();
    std::vector<Machine*> machines;
    MachineList *available_machines;

    size_t start_vertex,
           vertices;

    std::thread *scheduler_thread;

    void scheduler();

public:
    Server();
    void add_client(std::string connection_string);
    void connect_to_clients();
    size_t add_vertex();
    void set_start(size_t vertex);
    void next_phase(bool skip_local=false);
    void add_edge(size_t vertex_from, size_t vertex_to);
    void add_edge_SetBaseUrl(size_t vertex_from, size_t vertex_to, bool https, std::string host, std::string path);
    void add_edge_AppendPath(size_t vertex_from, size_t vertex_to, std::string append);
    void add_edge_PerformRequest(size_t vertex_from, size_t vertex_to);
    void run_clients();
    void start();
    void stop();
    void wait_for_clients();
};

#endif
