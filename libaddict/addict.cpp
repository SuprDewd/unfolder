#include "addict_dag.h"
#include "addict_parser.h"
#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <list>
// TODO: Use <filesystem> ?
using namespace std;

class WordDAG : public AddictDAG {
private:
    vector<vector<pair<string, size_t> > > adj;

    void output(ostream &outs, size_t cur, size_t end, list<string*> &s) {
        if (cur == end) {
            for (list<string*>::const_iterator it = s.begin(); it != s.end(); ++it) {
                outs << **it;
            }
            outs << endl;
            return;
        }

        for (size_t i = 0; i < this->adj[cur].size(); i++) {
            size_t nxt = this->adj[cur][i].second;
            s.push_back(&this->adj[cur][i].first);
            this->output(outs, nxt, end, s);
            s.pop_back();
        }
    }

public:
    virtual size_t add_vertex() {
        size_t vertex = this->adj.size();
        this->adj.push_back(vector<pair<string, size_t> >());
        return vertex;
    }

    virtual void add_append_edge(size_t from, const std::string &append, size_t to) {
        assert(from < to);
        this->adj[from].push_back(make_pair(append, to));
    }

    virtual void add_basic_edge(size_t from, size_t to) {
        assert(from < to);
        return this->add_append_edge(from, "", to);
    }

    void output(ostream &outs, size_t start, size_t end) {
        list<string*> cur;
        this->output(outs, start, end, cur);
    }
};

int main(int argc, char *argv[]) {
    WordDAG *dag = new WordDAG();
    AddictParser *parser = new AddictParser(dag);

    assert(argc == 2);
    pair<size_t, size_t> res = parser->parse_file(argv[1]);

    dag->output(cout, res.first, res.second);
}

