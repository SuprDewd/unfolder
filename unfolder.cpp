#include "config.h"

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdarg>
#include <fstream>
#include <set>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <map>
#include <sstream>
#include <cstring>
#include <cassert>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#if HAVE_MACH_ERROR_H
#include <mach/error.h>
#else
#include <error.h>
#endif

using namespace std;

void usage(int argc, char *argv[]) {
    fprintf(stderr, "usage: %s [OPTION]...\n", argv[0]);
}

void fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "error: ");
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

class UrlList {
private:
    unordered_set<string> url_set;
    vector<string> urls;
    mutex mut;
    size_t at = 0;

public:
    void add(string url) {
        lock_guard<mutex> lock(mut);
        static vector<string> protocols = {"http://", "https://"};
        bool protocol_ok = false;
        for (size_t i = 0; i < protocols.size(); i++) {
            if (url.substr(0, protocols[i].size()) == protocols[i]) {
                protocol_ok = true;
                break;
            }
        }
        if (!protocol_ok) {
            fatal("unsupported protocol for url \"%s\"", url);
        }
        if (url.back() != '/') {
            url += "/";
        }
        if (url_set.find(url) != url_set.end()) {
            return;
        }
        url_set.insert(url);
        urls.push_back(url);
    }

    bool get(string &res) {
        lock_guard<mutex> lock(mut);
        if (at >= urls.size()) return false;
        res = urls[at++];
        return true;
    }
};

class PathDict {
private:
    struct Node {
        bool end = false,
             left = false;
        int below = 0;
        map<char, Node*> down;
        void reset() {
            below = 0;
            if (end) left = true, below++;
            for (map<char, Node*>::iterator it = down.begin(); it != down.end(); ++it) {
                it->second->reset();
                below += it->second->below;
            }
        }
    };

    mutex mut;
    Node *root = new Node();

public:
    void add(string path) {
        size_t start = 0;
        while (start < path.size() && path[start] == '/') {
            start++;
        }
        path = path.substr(start);

        lock_guard<mutex> lock(mut);
        Node *cur = root;
        for (size_t i = 0; i < path.size(); i++) {
            char c = path[i];
            if (cur->down.find(c) == cur->down.end()) {
                cur->down[c] = new Node();
            }
            cur = cur->down[c];
        }
        if (cur->end) return;
        cur = root;
        for (size_t i = 0; i < path.size(); i++) {
            char c = path[i];
            cur->below++;
            cur = cur->down[c];
        }
        cur->below++;
        cur->end = true;
        cur->left = true;
    }

    void import_file(const char *fpath) {
        ifstream ifs(fpath);
        string path;
        while (getline(ifs, path)) {
            add(path);
        }
    }

    void reset() {
        lock_guard<mutex> lock(mut);
        root->reset();
    }

    bool get(string &res) {
        lock_guard<mutex> lock(mut);
        stringstream ss;
        if (root->below == 0) {
            return false;
        }
        Node *cur = root;
        while (!cur->left) {
            cur->below--;
            for (map<char, Node*>::const_iterator it = cur->down.begin(); it != cur->down.end(); ++it) {
                if (it->second->below > 0) {
                    ss << it->first;
                    cur = it->second;
                    break;
                }
            }
        }
        cur->below--;
        cur->left = false;
        res = ss.str();
        return true;
    }
};

mutex print_mut;
void thread_runner(int id, string base_url, PathDict *paths) {
    assert(base_url.substr(0, string("http://").size()) == "http://");
    base_url = base_url.substr(string("http://").size());
    string domain = base_url.substr(0, base_url.find("/"));
    base_url = base_url.substr(base_url.find("/"));

    struct hostent *res = gethostbyname(domain.c_str());
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned long*)res->h_addr;

    char buf[BUFSIZ];
    int bufat = 0,
        bufcnt = 0,
        sock = -1;

#define ASSURE (bufat < bufcnt ? 0 : (bufcnt = read(sock, buf, BUFSIZ), bufat = 0))
#define NEXT() (ASSURE, buf[bufat++])
#define PEEK() (ASSURE, buf[bufat])

    string path;
    while (paths->get(path)) {
        bufat = bufcnt = 0;

        if (sock < 0) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                fatal("%s\n", strerror(errno));
            }
        }

        stringstream ss;
        ss << "GET " << base_url;
        for (size_t i = 0; i < path.size(); i++) {
            if (path[i] == ' ') {
                ss << "+";
            } else {
                ss << path[i];
            }
        }
        ss << " HTTP/1.1\r\nHost: " << domain << "\r\nConnection: keep-alive\r\n\r\n";

        string msg = ss.str();
        write(sock, msg.c_str(), msg.size());

        while (NEXT() != ' ');
        int code = 0;
        while ('0' <= PEEK() && PEEK() <= '9') {
            code = code * 10 + NEXT() - '0';
        }
        while (NEXT() != '\n');

        if (code != 404) {
            lock_guard<mutex> lock(print_mut);
            cout << code << " http://" << domain << base_url << path << endl;
        }

        int content_length = -1;
        bool do_close = false;
        while (true) {

            if (PEEK() == '\r' || PEEK() == '\n') {
                while (NEXT() != '\n');
                break;
            }

            stringstream key, value;
            while (PEEK() != ':') {
                key << NEXT();
            }
            NEXT();
            if (PEEK() == ' ') NEXT();
            while (PEEK() != '\n' && PEEK() != '\r') {
                value << NEXT();
            }
            while (NEXT() != '\n');

            if (key.str() == "Content-Length") {
                content_length = atoi(value.str().c_str());
            } else if (key.str() == "Connection" && value.str() == "close") {
                do_close = true;
                break;
            }
        }

        if (content_length >= 0 && !do_close) {
            while (content_length > 0) NEXT(), content_length--;
        } else {
            close(sock);
            sock = -1;
        }
    }

    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
}

int main(int argc, char *argv[]) {

    UrlList *urls = new UrlList();
    PathDict *paths = new PathDict();
    int thread_count = 4;

    for (int i = 1; i < argc; ) {
        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--url") == 0) {
            if (++i >= argc) usage(argc, argv), exit(1);
            urls->add(argv[i++]);
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dict") == 0) {
            if (++i >= argc) usage(argc, argv), exit(1);
            paths->import_file(argv[i++]);
        } else if (strcmp(argv[i], "-j") == 0) {
            if (++i >= argc) usage(argc, argv), exit(1);
            thread_count = atoi(argv[i++]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argc, argv);
            exit(0);
        } else {
            usage(argc, argv);
            exit(1);
        }
    }

    string url;
    while (urls->get(url)) {
        cout << "Entering directory: " << url << endl;
        paths->reset();
        vector<thread*> threads;
        for (int i = 0; i < thread_count; i++) {
            threads.push_back(new thread(thread_runner, i, url, paths));
        }
        for (int i = 0; i < thread_count; i++) {
            threads[i]->join();
            delete threads[i];
        }
    }

    return 0;
}
