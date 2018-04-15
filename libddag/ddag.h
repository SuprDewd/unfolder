#ifndef DDAG_H
#define DDAG_H
#include <vector>
#include <cassert>
#include <unordered_map>

template <class T>
class Edge {
public:
    virtual bool forward(T *state) const;
    virtual void backward(T *state) const;
};

template <class T>
class BasicEdge : public Edge<T> {
public:
    virtual bool forward(T *state) const;
    virtual void backward(T *state) const;
};

// XXX: Are vectors thread-safe? In particular, can multiple threads access
// a vector while another thread appends to the vector? What if a resize occurs?

template <class State>
class dDAG {
public:

    class dEdge {
    private:
        Edge<State> *edge;
        size_t vertex_to;
        char age;

    public:

        dEdge(Edge<State> *edge, size_t vertex_to);
        size_t get_vertex_to() const;
        size_t get_version() const;
        int get_age() const;
        bool forward(State *state);
        void backward(State *state);

        friend class dDAG<State>;
    };

private:

    std::vector<std::vector<dEdge*> > adj;
    std::vector<long long> count_total, count_current;

    void execute(State *state, size_t vertex, long long &current, long long start, long long count, bool used_current);

public:

    static const size_t START_VERTEX = 0;

    dDAG();
    size_t add_vertex();
    void add_edge(size_t vertex_from, Edge<State> *edge, size_t vertex_to);
    void add_edge(size_t vertex_from, size_t vertex_to);
    size_t adj_count(size_t vertex);
    dEdge* get_adj(size_t vertex, size_t i);
    void set_start(size_t vertex);
    void next_phase();
    long long get_current_count();
    void execute(long long start, long long count);
};

#endif
