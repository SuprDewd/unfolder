#include "io.h"

IO::IO(int read_fd, int write_fd) {
    this->read_fd = read_fd;
    this->write_fd = write_fd;
}

std::unique_lock<std::mutex> IO::get_write_lock() {
    return std::unique_lock<std::mutex>(this->write_mut);
}

void IO::write_char(char c, bool flush) {
    ssize_t cnt = write(this->write_fd, &c, sizeof(char));
    assert(cnt == sizeof(int));
    if (flush) fsync(this->write_fd);
}

void IO::write_int(int x, bool flush) {
    ssize_t cnt = write(this->write_fd, &x, sizeof(int));
    assert(cnt == sizeof(int));
    if (flush) fsync(this->write_fd);
}

void IO::write_long(long long x, bool flush) {
    ssize_t cnt = write(this->write_fd, &x, sizeof(long long));
    assert(cnt == sizeof(long long));
    if (flush) fsync(this->write_fd);
}

void IO::write_string(const std::string &s, bool flush) {
    this->write_int(s.size(), false);
    ssize_t cnt = write(this->write_fd, s.c_str(), s.size());
    assert(cnt == (ssize_t)s.size());
    if (flush) fsync(this->write_fd);
}

char IO::read_char() {
    char res;
    ssize_t cnt = read(this->read_fd, &res, sizeof(char));
    assert(cnt == sizeof(char));
    return res;
}

int IO::read_int() {
    int res;
    ssize_t cnt = read(this->read_fd, &res, sizeof(int));
    assert(cnt == sizeof(int));
    return res;
}

long long IO::read_long() {
    long long res;
    ssize_t cnt = read(this->read_fd, &res, sizeof(long long));
    assert(cnt == sizeof(long long));
    return res;
}

std::string IO::read_string() {
    int len = this->read_int();
    char *data = new char[len+1];
    ssize_t cnt = read(this->read_fd, data, len);
    assert(cnt == (ssize_t)len);
    data[len] = '\0';
    std::string res(data);
    delete data;
    return res;
}

void IO::close_fds() {
    close(this->read_fd);
    close(this->write_fd);
}

