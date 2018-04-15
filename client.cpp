#include "lib/dag.h"
#include "lib/client.h"
#include "libddag/ddag.h"
#include "lib/io.h"
using namespace std;

int main() {

    Client *client = new Client(
        new IO(STDIN_FILENO, STDOUT_FILENO),
        new dDAG<State>()
    );

    client->run();

    return 0;
}
