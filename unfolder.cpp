#include "lib/server.h"
#include <cstring>
using namespace std;

int main(int argc, char *argv[]) {
    Server *server = new Server();
    string clients;

    for (int at = 1; at < argc; ) {
        if (strcmp(argv[at], "-c") == 0 || strcmp(argv[at], "--clients") == 0) {
            at++;
            assert(at < argc);
            clients = clients + argv[at++] + ",";
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

    /*                         ___________
     *                index   /           \
     *   a          b/about\c/      d      \e       f
     *   o---conn---o-------o-- . --o--php--o--req--o
     *               \_hax_/         \_txt_/
     *
     */

    size_t a = server->add_vertex();
    size_t b = server->add_vertex();
    server->add_edge_SetBaseUrl(a, b, false, "unsec.algo.is", "test/");

    size_t c = server->add_vertex();
    server->add_edge_AppendPath(b, c, "index");
    server->add_edge_AppendPath(b, c, "about");
    server->add_edge_AppendPath(b, c, "hax");

    size_t d = server->add_vertex();
    server->add_edge_AppendPath(c, d, ".");

    size_t e = server->add_vertex();
    server->add_edge(c, e);
    server->add_edge_AppendPath(d, e, "php");
    server->add_edge_AppendPath(d, e, "txt");

    size_t f = server->add_vertex();
    server->add_edge_PerformRequest(e, f);

    server->set_start(a);

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
