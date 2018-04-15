#include "server.h"

void Server::scheduler() {

    // this->run_clients();

    while (true) {
        // XXX: wait until there are jobs available

        // Start next phase
        this->dag->next_phase();
        this->next_phase(true); // Skip local DAG update

        long long count = this->dag->get_current_count(),
                  current = 0;

        if (count == 0) {
            break; // TODO: Don't do this in interactive mode
        }

        while (current < count) {
            // TODO: Wait for machine to become available (some kind of scheduler, with list of available machines, using a semaphore)
            Machine *m = this->available_machines->get();

            // TODO: Send fixed (or dynamic) number of jobs to this machine, and mark it as unavailable
            long long send = std::min(1LL, count - current);

            {
                auto lock = m->io->get_write_lock();
                m->io->write_string("work");
                m->io->write_long(current);
                m->io->write_long(send);
            }

            current += send;
        }

        // wait until all machines are available
        for (size_t i = 0; i < this->machines.size(); i++) this->available_machines->get();
        for (size_t i = 0; i < this->machines.size(); i++) this->available_machines->make_available(this->machines[i]);
    }
}

Server::Server() {
    this->dag = new dDAG<State>();
    this->available_machines = new MachineList();
    this->start_vertex = 0;
    this->vertices = 1;
    this->scheduler_thread = NULL;
}

void Server::add_client(std::string connection_string) {
    this->machines.push_back(new Machine(connection_string, this->dag, this->available_machines));
}

void Server::connect_to_clients() {
    for (size_t i = 0; i < this->machines.size(); i++) {
        this->machines[i]->connect();
    }
}

size_t Server::add_vertex() {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("add_vertex");
    }
    return this->vertices++;
}

void Server::set_start(size_t vertex) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("set_start");
        this->machines[i]->io->write_int(vertex);
    }
}

void Server::next_phase(bool skip_local) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        if (skip_local && this->machines[i]->is_local()) {
            continue;
        }
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("next_phase");
    }
}

void Server::add_edge(size_t vertex_from, size_t vertex_to) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("add_edge");
        this->machines[i]->io->write_int(vertex_from);
        this->machines[i]->io->write_int(vertex_to);
        this->machines[i]->io->write_int(-1);
    }
}
void Server::add_edge_SetBaseUrl(size_t vertex_from, size_t vertex_to, bool https, std::string host, std::string path) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("add_edge");
        this->machines[i]->io->write_int(vertex_from);
        this->machines[i]->io->write_int(vertex_to);
        this->machines[i]->io->write_int(SetBaseUrl::ID);
        this->machines[i]->io->write_int(https ? 1 : 0);
        this->machines[i]->io->write_string(host);
        this->machines[i]->io->write_string(path);
    }
}
void Server::add_edge_AppendPath(size_t vertex_from, size_t vertex_to, std::string append) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("add_edge");
        this->machines[i]->io->write_int(vertex_from);
        this->machines[i]->io->write_int(vertex_to);
        this->machines[i]->io->write_int(AppendPath::ID);
        this->machines[i]->io->write_string(append);
    }
}
void Server::add_edge_PerformRequest(size_t vertex_from, size_t vertex_to) {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("add_edge");
        this->machines[i]->io->write_int(vertex_from);
        this->machines[i]->io->write_int(vertex_to);
        this->machines[i]->io->write_int(PerformRequest::ID);
    }
}

void Server::run_clients() {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("run");
        this->machines[i]->io->write_long(0);
    }
}

void Server::start() {
    this->scheduler_thread = new std::thread(&Server::scheduler, this);
}

void Server::stop() {
    for (size_t i = 0; i < this->machines.size(); i++) {
        auto lock = this->machines[i]->io->get_write_lock();
        this->machines[i]->io->write_string("quit");
    }

    this->wait_for_clients();
    // TODO: stop scheduler thread
}

void Server::wait_for_clients() {
    for (size_t i = 0; i < this->machines.size(); i++) {
        this->machines[i]->join();
    }
}

