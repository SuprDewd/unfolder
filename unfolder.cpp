#include "lib/server.h"
#include "libaddict/addict_dag.h"
#include "libaddict/addict_parser.h"
#include "lib/connection.h"
#include <cstring>
using namespace std;

class RemoteUnfolderDAG : public AddictDAG {
private:
    Server *server;

public:
    RemoteUnfolderDAG(Server *server) : server(server) {
    }

    virtual size_t add_vertex() {
        return this->server->add_vertex();
    }

    virtual void add_append_edge(size_t from, const std::string &append, size_t to) {
        return this->server->add_edge_AppendPath(from, to, append);
    }

    virtual void add_basic_edge(size_t from, size_t to) {
        return this->server->add_edge(from, to);
    }
};

int main(int argc, char *argv[]) {
    Server *server = new Server();
    string clients;
    vector<string> dicts, base_urls;

    for (int at = 1; at < argc; ) {
        if (strcmp(argv[at], "-c") == 0 || strcmp(argv[at], "--clients") == 0) {
            at++;
            assert(at < argc);
            clients = clients + argv[at++] + ",";
        } else if (strcmp(argv[at], "-d") == 0 || strcmp(argv[at], "--dict") == 0) {
            at++;
            assert(at < argc);
            dicts.push_back(argv[at++]);
        } else if (strcmp(argv[at], "-u") == 0 || strcmp(argv[at], "--url") == 0) {
            at++;
            assert(at < argc);
            base_urls.push_back(argv[at++]);
        } else {
            cout << argv[at] << endl;
            assert(false); // TODO: Print help
        }
    }

    if (clients == "") {
        server->add_client(":"); // Default to local machine
    } else {
        assert(clients.back() == ',');
        stringstream ss;
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i] == ',') {
                server->add_client(ss.str());
                ss.str("");
            } else {
                ss << clients[i];
            }
        }
    }

    server->connect_to_clients();

    size_t hostname_start = server->add_vertex(),
           hostname_end = server->add_vertex();
    for (size_t i = 0; i < base_urls.size(); i++) {
        string url = base_urls[i], path;
        bool https = false;
        if (url.find("http://") == 0) {
            url = url.substr(7);
        } else if (url.find("https://") == 0) {
            https = true;
            url = url.substr(8);
        } else {
            assert(false); // TODO: Error message
        }

        if (url.empty() || url.back() != '/') {
            url.push_back('/');
        }

        string host = url.substr(0, url.find("/"));
        url = url.substr(url.find("/") + 1);
        server->add_edge_SetBaseUrl(hostname_start, hostname_end, https, host, url);
    }

    RemoteUnfolderDAG *rdag = new RemoteUnfolderDAG(server);
    AddictParser *add = new AddictParser(rdag);
    vector<pair<size_t,size_t> > dict_vs;
    for (size_t i = 0; i < dicts.size(); i++) {
        dict_vs.push_back(add->parse_file(dicts[i]));
    }

    size_t request_start = server->add_vertex(),
           request_end = server->add_vertex();
    server->add_edge_PerformRequest(request_start, request_end);

    for (size_t i = 0; i < dict_vs.size(); i++) {
        server->add_edge(hostname_end, dict_vs[i].first);
        server->add_edge(dict_vs[i].second, request_start);
    }

    server->set_start(hostname_start);
    server->start();

    while (true) {
        string cmd;
        getline(cin, cmd);
        if (cmd == "quit") {
            server->stop();
            break;
        } else {
            assert(false);
        }
    }

    return 0;
}
