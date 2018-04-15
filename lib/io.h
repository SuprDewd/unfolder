#ifndef UNFOLDER_IO_H
#define UNFOLDER_IO_H

#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <mutex>

class IO {
private:
    int read_fd, write_fd;
    std::mutex write_mut;

public:
    IO(int read_fd, int write_fd);
    std::unique_lock<std::mutex> get_write_lock();
    void write_char(char c, bool flush=true);
    void write_int(int x, bool flush=true);
    void write_long(long long x, bool flush=true);
    void write_string(const std::string &s, bool flush=true);
    char read_char();
    int read_int();
    long long read_long();
    std::string read_string();
    void close_fds();
};

#endif
