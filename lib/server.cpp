#include "server.h"
#include <mutex>
using namespace std;

const long long BATCH = 20; // Number of jobs to send to each client at a time. TODO: Make this dynamic.

const int PROGRESS_WIDTH = 60;

mutex print_mut;

void clear_progress() {
    lock_guard<mutex> lock(print_mut);
    cerr << "\x1B[1K\x1B[1F\x1B[1E";
}

void print_progress(long long done, long long total) {
    lock_guard<mutex> lock(print_mut);

    cerr << "[";
    for (int i = 0; i < PROGRESS_WIDTH; i++) {
        if (i < PROGRESS_WIDTH * done / total) {
            cerr << "#";
        } else {
            cerr << " ";
        }
    }
    cerr << "] " << done << "/" << total;
}

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

        this->current_count = count;
        this->current_done = 0;
        print_progress(this->current_done, this->current_count);

        while (current < count) {
            // TODO: Wait for machine to become available (some kind of scheduler, with list of available machines, using a semaphore)
            Machine *m = this->available_machines->get();

            // TODO: Send fixed (or dynamic) number of jobs to this machine, and mark it as unavailable
            long long send = min(BATCH, count - current);

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

        clear_progress();
    }
}

Server::Server() {
    this->dag = new dDAG<State>();
    this->available_machines = new MachineList();
    this->start_vertex = 0;
    this->vertices = 1;
    this->scheduler_thread = NULL;
}

void Server::add_client(string connection_string) {
    this->machines.push_back(new Machine(connection_string, this->dag, this));
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
void Server::add_edge_SetBaseUrl(size_t vertex_from, size_t vertex_to, bool https, string host, string path) {
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
void Server::add_edge_AppendPath(size_t vertex_from, size_t vertex_to, string append) {
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
    this->scheduler_thread = new thread(&Server::scheduler, this);
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

void Server::response_available(Machine *machine) {
    this->available_machines->make_available(machine);
}

void Server::response_found(Machine *machine, const string &response) {
    clear_progress();
    {
        lock_guard<mutex> lock(print_mut);
        cout << response << endl;
    }
    print_progress(this->current_done, this->current_count);
}

void Server::response_work_done(Machine *machine, long long amount) {
    clear_progress();
    this->current_done += amount;
    print_progress(this->current_done, this->current_count);
}

