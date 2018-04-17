
AC_INIT([unfolder], [0.2], [suprdewd@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_PROG_CXX
AC_CONFIG_FILES([
    Makefile
])

AM_CONFIG_HEADER([config.h])

AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([crypto], [BIO_new])
AC_CHECK_LIB([ssl], [SSL_CTX_new])
AC_CHECK_HEADERS([openssl/ssl.h])
AC_CHECK_HEADERS([openssl/bio.h])

AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strerror])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([error.h])
AC_CHECK_HEADERS([mach/error.h])
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

AC_OUTPUT

