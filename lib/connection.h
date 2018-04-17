#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <string>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>

class Connection {
public:
    virtual void open(const std::string &domain) = 0;
    virtual void disconnect() = 0;
    virtual void do_write(const char *s, size_t len) = 0;
    virtual void do_read(char *s, size_t len) = 0;
};

class HttpConnection : public Connection {
private:
    char buf[BUFSIZ];
    int bufat = 0,
        bufcnt = 0,
        sock = -1;

public:
    HttpConnection();
    virtual void open(const std::string &domain);
    virtual void disconnect();
    virtual void do_write(const char *s, size_t len);
    virtual void do_read(char *s, size_t len);
};

class HttpsConnection : public Connection {
private:
    SSL_CTX* ctx = NULL;
    BIO *web = NULL;
    SSL *ssl = NULL;

public:
    HttpsConnection();
    virtual void open(const std::string &domain);
    virtual void disconnect();
    virtual void do_write(const char *s, size_t len);
    virtual void do_read(char *s, size_t len);
};

int get_status_code(bool https, const std::string &domain, const std::string &path);

#endif
