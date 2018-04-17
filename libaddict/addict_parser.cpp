// #include <iostream>
// #include <cassert>
// #include <vector>
// #include <sstream>
// #include <fstream>
// #include <stack>
// using namespace std;

#include "addict_parser.h"
#include "tools.h"
#include <sstream>
#include <fstream>
#include <libgen.h>
using namespace std;

#if defined(WIN32) || defined(_WIN32) 
#define PATH_SEP "\\" 
#else 
#define PATH_SEP "/" 
#endif 

/*

# dict := (char | '$' '$' | '$' fun)+
# char := ascii ranges 32-126, except 36 ('$')

# fun := [A-Za-z0-9]+ '(' arglist ')'
# arglist := expr | expr ',' arglist
# expr := fun | token
# token := char | '$' '$' | '$' ','  | '$' ')'

dict := (char | expr)*
expr := '[' (char)+ (',' dict)+ ']'
char := ascii ranges 32-126, except "[]\,", or \[ \] \\ \,

*/

// TODO: Add some context (current file, current line, current index) so that
// we can generate errors. This would also replace base_path.

AddictParser::AddictParser(AddictDAG *dag) {
    this->dag = dag;
}

pair<size_t,size_t> AddictParser::next_expr(Reader *s, const string &base_path) {
    assert(s->has_next() && s->peek() == '[');
    s->next();

    stringstream ss;
    while (s->has_next() && s->peek() != ']' && s->peek() != ',') {
        ss << s->next_char();
    }

    size_t start = this->dag->add_vertex();

    vector<pair<size_t, size_t> > args;
    while (s->has_next() && s->peek() != ']') {
        assert(s->peek() == ',');
        s->next();
        args.push_back(this->next_dict(s, base_path));
    }
    assert(s->has_next() && s->peek() == ']');
    s->next();

    string name = ss.str();
    // TODO: Check which function name corresponds to

    // if (name == "name") {
    //
    // } else if (name == "ext") {
    //
    // } else if (endswith(name, ".add")) {
    //     assert(args.size() == 0);
    //
    // } else {
    //     // file
    //     assert(args.size() == 0);
    //
    // }

    // TODO: Handle stuff other than just including other addict files
    assert(args.size() == 0);
    pair<size_t,size_t> res = this->parse_file(name, base_path);

    size_t end = this->dag->add_vertex();

    this->dag->add_basic_edge(start, res.first);
    this->dag->add_basic_edge(res.second, end);

    // TODO: name()
    // TODO: ext()
    // TODO: date()
    // TODO: num()
    // TODO: oneof()
    // TODO: include other .add

    // XXX: This should be last resort

    // return new file_expr(name);
    return make_pair(start, end);
}

pair<size_t,size_t> AddictParser::next_dict(Reader *s, const string &base_path) {
    // concat_expr *res = new concat_expr();

    size_t start = this->dag->add_vertex(),
           last = start;

    while (true) {
        if (s->has_next() && s->peek() == '[') {
            pair<size_t,size_t> cur = this->next_expr(s, base_path);
            this->dag->add_basic_edge(last, cur.first);
            last = cur.second;
        } else if (s->has_char()) {
            stringstream ss;
            while (s->has_char()) {
                ss << s->next_char();
            }
            size_t nxt = this->dag->add_vertex();
            this->dag->add_append_edge(last, ss.str(), nxt);
            last = nxt;
        } else {
            break;
        }
    }

    return make_pair(start, last);
}

pair<size_t,size_t> AddictParser::parse_string(const string &s, const string &base_path) {
    Reader *r = new Reader(s);
    pair<int,int> res = this->next_dict(r, base_path);
    assert(!r->has_next());
    delete r;
    return res;
}

pair<size_t,size_t> AddictParser::parse_file(string path, string base_path) {
    path = base_path + PATH_SEP + path;

    char *tmp = new char[path.size() + 1];
    for (size_t i = 0; i < path.size(); i++) tmp[i] = path[i];
    tmp[path.size()] = 0;
    base_path = dirname(tmp);
    delete[] tmp;

    // TODO: Error handling
    ifstream ifs(path.c_str());
    // if (!ifs.is_open()) {
    //     cerr << "error opening " << path << endl;
    // }
    assert(ifs.is_open());

    size_t start = this->dag->add_vertex();
    vector<pair<size_t,size_t> > res;
    string line;
    while (getline(ifs, line)) {
        // TODO: Do we want to clean up lines? Trailing '\r', leading '/'?

        // Preprocessor: remove comments
        stringstream ss;
        for (size_t i = 0; i < line.size(); ) {
            if (line[i] == '\\') {
                ss << line[i++];
                assert(i < line.size());
                ss << line[i++];
            } else if (line[i] == '#') {
                break;
            } else {
                ss << line[i++];
            }
        }

        line = ss.str();
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back(); // Strip DOS line endings
        if (line.empty()) continue;
        res.push_back(this->parse_string(line, base_path));
    }
    size_t end = this->dag->add_vertex();
    for (size_t i = 0; i < res.size(); i++) {
        this->dag->add_basic_edge(start, res[i].first);
        this->dag->add_basic_edge(res[i].second, end);
    }

    return make_pair(start, end);
}

