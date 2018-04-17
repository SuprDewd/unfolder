#include "connection.h"
#include <sstream>
#include <cassert>
#include <unistd.h>
#include <iostream> // TODO: Remove
using namespace std;

HttpConnection::HttpConnection() {
}

void HttpConnection::open(const std::string &domain) {

    struct hostent *res = gethostbyname(domain.c_str());

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = *(unsigned long*)res->h_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        assert(false);
        // fatal("%s\n", strerror(errno));
    }
}

void HttpConnection::disconnect() {
    close(sock);
    sock = -1;
}

void HttpConnection::do_write(const char *s, size_t len) {
    // assert(this->sock != -1);
    write(this->sock, s, len);
}

void HttpConnection::do_read(char *s, size_t len) {
    // assert(this->sock != -1);
    read(this->sock, s, len);
}


HttpsConnection::HttpsConnection() {
}

void HttpsConnection::open(const std::string &domain) {
    SSL_library_init();
    /* SSL_load_error_std::strings(); */

    const SSL_METHOD* method = SSLv23_method();

    ctx = SSL_CTX_new(method);
    SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);

    web = BIO_new_ssl_connect(ctx);
    std::string conn_str = domain + ":443";

    BIO_set_conn_hostname(web, conn_str.c_str());

    BIO_get_ssl(web, &ssl);
    SSL_set_cipher_list(ssl, "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS");
    SSL_set_tlsext_host_name(ssl, domain.c_str());
    BIO_do_connect(web);
    BIO_do_handshake(web);

    /* X509* cert = SSL_get_peer_certificate(ssl); */
    /* if(cert) { X509_free(cert); } #<{(| Free immediately |)}># */

    // BIO_puts(web, "GET " HOST_RESOURCE " HTTP/1.1\r\nHost: " HOST_NAME "\r\nConnection: close\r\n\r\n");
    //
    // char buff[1536] = {};
    //
    // while (1) {
    //     int len = BIO_read(web, buff, sizeof(buff));
    //     if (len <= 0) break;
    //     int i;
    //     for (i = 0; i < len; i++) {
    //         printf("%c", buff[i]);
    //     }
    // }
    
}

void HttpsConnection::disconnect() {
    if(web != NULL) BIO_free_all(web), web = NULL;
    if (ctx != NULL) SSL_CTX_free(ctx), ctx = NULL;
}

void HttpsConnection::do_write(const char *s, size_t len) {
    BIO_write(this->web, s, len);
}

void HttpsConnection::do_read(char *s, size_t len) {
    BIO_read(this->web, s, len);
}

int get_status_code(bool https, const std::string &domain, const std::string &path) {
    Connection *conn;
    if (https) conn = new HttpsConnection();
    else conn = new HttpConnection();
    conn->open(domain);

    std::stringstream ss;
    ss << "GET " << path << " HTTP/1.1\r\n";
    ss << "Host: " << domain << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";

    std::string msg = ss.str();
    conn->do_write(msg.c_str(), msg.size());

    const int len = 15;
    char tmp[len];
    conn->do_read(tmp, len);

    int code = 0;
    size_t at = 0;
    while (at < len && tmp[at] != ' ') at++;
    at++;
    for (size_t i = 0; i < 3; i++) {
        assert(at < len);
        code = code * 10 + tmp[at++] - '0';
    }

    conn->disconnect();

    return code;
}
