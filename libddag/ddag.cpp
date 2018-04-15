#include "ddag.h"

template <class T>
bool BasicEdge<T>::forward(T *state) const { return true; }

template <class T>
void BasicEdge<T>::backward(T *state) const { }

template <class State>
dDAG<State>::dEdge::dEdge(Edge<State> *edge, size_t vertex_to) {
    this->edge = edge;
    this->vertex_to = vertex_to;
    this->age = 2;
    assert(edge != NULL);
}

template <class State>
size_t dDAG<State>::dEdge::get_vertex_to() const {
    return this->vertex_to;
}

template <class State>
size_t dDAG<State>::dEdge::get_version() const {
    return this->version;
}

template <class State>
int dDAG<State>::dEdge::get_age() const {
    return this->age;
}

template <class State>
bool dDAG<State>::dEdge::forward(State *state) { return this->edge->forward(state); }

template <class State>
void dDAG<State>::dEdge::backward(State *state) { this->edge->backward(state); }

template <class State>
void dDAG<State>::execute(State *state, size_t vertex, long long &current, long long start, long long count, bool used_current) {
    if (this->adj[vertex].empty()) {
        current++;
    }

    for (size_t i = 0; i < this->adj[vertex].size(); i++) {
        size_t to = this->adj[vertex][i]->vertex_to;
        char age = this->adj[vertex][i]->age;
        if (age == 2) continue;

        bool now_used_current = used_current || age == 1;
        long long here = now_used_current ? this->count_total[to] : this->count_current[to];

        if (current > start + count - 1) {
            // We've passed the range of paths that we're interested in
            break;
        } else if (current + here - 1 < start) {
            // We've not reached the range of paths we're interested in
            current += here;
        } else if (this->adj[vertex][i]->forward(state)) {
            this->execute(state, to, current, start, count, now_used_current);
            this->adj[vertex][i]->backward(state);
        }
    }
}

template <class State>
dDAG<State>::dDAG() {
    size_t start = this->add_vertex();
    assert(start == START_VERTEX);
}

template <class State>
size_t dDAG<State>::add_vertex() {
    size_t idx = this->adj.size();
    adj.push_back(std::vector<dEdge*>());
    count_total.push_back(0);
    count_current.push_back(0);
    return idx;
}

template <class State>
void dDAG<State>::add_edge(size_t vertex_from, Edge<State> *edge, size_t vertex_to) {
    assert(START_VERTEX < vertex_from && vertex_from < vertex_to && vertex_to < this->adj.size()); // To make sure graph is a DAG
    this->adj[vertex_from].push_back(new dEdge(edge, vertex_to));
}

template <class State>
void dDAG<State>::add_edge(size_t vertex_from, size_t vertex_to) {
    this->add_edge(vertex_from, new BasicEdge<State>(), vertex_to);
}

template <class State>
size_t dDAG<State>::adj_count(size_t vertex) {
    assert(vertex < this->adj.size());
    return this->adj[vertex].size();
}

template <class State>
typename dDAG<State>::dEdge* dDAG<State>::get_adj(size_t vertex, size_t i) {
    assert(vertex < this->adj.size() && i < this->adj[vertex].size());
    return this->adj[vertex][i];
}

template <class State>
void dDAG<State>::set_start(size_t vertex) {
    this->adj[START_VERTEX].push_back(new dDAG<State>::dEdge(new BasicEdge<State>(), vertex));
}

template <class State>
void dDAG<State>::next_phase() {
    // XXX: No changes should be made to the DAG while this is running
    for (size_t vertex = this->adj.size(); vertex --> 0; ) {
        this->count_total[vertex] = this->count_current[vertex] = 0;

        if (this->adj[vertex].empty()) {
            this->count_total[vertex]++;
        }

        for (size_t i = 0; i < this->adj[vertex].size(); i++) {
            size_t to = this->adj[vertex][i]->vertex_to;
            char age = this->adj[vertex][i]->age;
            this->count_total[vertex] += this->count_total[to];
            this->count_current[vertex] += age == 2 ? this->count_total[to] : this->count_current[to];
            if (age > 0) {
                this->adj[vertex][i]->age--;
            }
        }
    }
}

template <class State>
long long dDAG<State>::get_current_count() {
    return this->count_current[START_VERTEX];
}

template <class State>
void dDAG<State>::execute(long long start, long long count) {
    long long current = 0;
    this->execute(new State(), START_VERTEX, current, start, count, false);
}

