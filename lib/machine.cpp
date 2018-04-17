#include "machine.h"

void Machine::listen() {
    while (true) {
        std::string cmd = this->io->read_string();
        if (cmd == "bye") {
            std::cout << "bye from " << host << std::endl;
            break;
        } else if (cmd == "response") {
            this->server->response_found(this, this->io->read_string());
        } else if (cmd == "available") {
            this->server->response_available(this);
        } else if (cmd == "work_done") {
            this->server->response_work_done(this, this->io->read_long());
        } else {
            assert(false);
        }
    }

    this->io->close_fds();
}

Machine::Machine(std::string connection_string, dDAG<State> *local_dag, Server* server) {
    this->threads = -1;
    this->listener_thread = NULL;
    this->client_thread = NULL;
    this->child = -1;
    this->local_dag = local_dag;
    this->server = server;
    this->io = NULL;

    size_t at = 0;
    while (at < connection_string.size() && connection_string[at] != '/') at++;
    if (at != connection_string.size()) {
        at = 0;
        threads = 0;
        while (connection_string[at] != '/') {
            threads = threads * 10 + connection_string[at++] - '0';
        }
        at++;
        std::stringstream ss;
        while (at < connection_string.size()) ss << connection_string[at++];
        this->host = ss.str();
    } else {
        this->host = connection_string;
    }

    this->_is_local = this->host == ":";
}

std::string Machine::get_host() {
    return this->host;
}

bool Machine::is_local() {
    return this->_is_local;
}

void Machine::run_client(int stdin_fd, int stdout_fd) {
    Client *client = new Client(
        new IO(stdin_fd, stdout_fd),
        this->local_dag
    );
    client->run();
}

void Machine::connect() {
    // Connect to client over ssh (except host ':', which is special)
    int stdinpipe[2],
        stdoutpipe[2];

    pipe(stdinpipe);
    pipe(stdoutpipe);

    if (host == ":") {
        this->client_thread = new std::thread(&Machine::run_client, this, stdinpipe[0], stdoutpipe[1]);
    } else {
        this->child = fork();
        if (this->child == 0) {
            close(stdinpipe[1]);
            dup2(stdinpipe[0], STDIN_FILENO);

            close(stdoutpipe[0]);
            dup2(stdoutpipe[1], STDOUT_FILENO);

            execlp("ssh", "ssh", host.c_str(), "unfolder-client", NULL);
            return;
        }

        close(stdinpipe[0]);
        close(stdoutpipe[1]);
    }

    this->io = new IO(stdoutpipe[0], stdinpipe[1]);

    this->io->write_int(this->threads);
    this->threads = this->io->read_int();
    std::cout << "Client at host " << host << " with " << this->threads << " threads" << std::endl;

    // Start listener
    this->listener_thread = new std::thread(&Machine::listen, this);
}

void Machine::join() {
    if (this->listener_thread != NULL) this->listener_thread->join();
    if (this->client_thread != NULL) this->client_thread->join();
    if (this->child != -1) waitpid(this->child, NULL, 0);
}

