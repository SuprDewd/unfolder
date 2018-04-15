#include "client.h"

Client::Client(IO *server_io, dDAG<State> *dag) {
    this->server_io = server_io;
    this->dag = dag;
    this->worker_threads = -1;
}

void Client::add_vertex() {
    this->dag->add_vertex();
}

void Client::add_edge() {
    size_t vertex_from = this->server_io->read_int(),
           vertex_to = this->server_io->read_int();

    switch (this->server_io->read_int()) {
        case -1:
            {
                this->dag->add_edge(vertex_from, vertex_to);
            }
            break;
        case SetBaseUrl::ID:
            {
                bool https = this->server_io->read_int() == 1;
                std::string host = this->server_io->read_string(),
                       path = this->server_io->read_string();
                this->dag->add_edge(vertex_from, new SetBaseUrl(https, host, path), vertex_to);
            }
            break;
        case AppendPath::ID:
            {
                std::string append = this->server_io->read_string();
                this->dag->add_edge(vertex_from, new AppendPath(append), vertex_to);
            }
            break;
        case PerformRequest::ID:
            {
                this->dag->add_edge(vertex_from, new PerformRequest(this->server_io, worker_threads), vertex_to);
            }
            break;
        default:
            assert(false);
    }
}

void Client::set_start() {
    size_t vertex = this->server_io->read_int();
    this->dag->set_start(vertex);
}

void Client::next_phase() {
    this->dag->next_phase();
}

void Client::work() {
    long long start = this->server_io->read_long(),
              count = this->server_io->read_long();

    this->dag->execute(start, count);

    {
        auto lock = this->server_io->get_write_lock();
        this->server_io->write_string("available");
    }
}

void Client::run() {
    this->worker_threads = this->server_io->read_int();
    if (this->worker_threads == -1) {
        this->worker_threads = get_nprocs(); // Default to number of processors (probably suboptimal)
    }

    this->server_io->write_int(worker_threads);

    // TODO: Spawn worker threads?
    {
        auto lock = this->server_io->get_write_lock();
        this->server_io->write_string("available");
    }

    while (true) {
        std::string cmd = this->server_io->read_string();

        if (cmd == "quit") {
            auto lock = this->server_io->get_write_lock();
            this->server_io->write_string("bye");
            this->server_io->close_fds();
            break;
        }

        if (cmd == "add_vertex") this->add_vertex();
        else if (cmd == "add_edge") this->add_edge();
        else if (cmd == "set_start") this->set_start();
        else if (cmd == "next_phase") this->next_phase();
        else if (cmd == "work") this->work();
        else assert(false);
    }
}
