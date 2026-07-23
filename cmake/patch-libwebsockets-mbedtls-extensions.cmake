# libwebsockets v4.3.3's tls/mbedtls/mbedtls-extensions.c (SAN rfc822Name
# parsing) calls mbedtls_x509_get_name() without declaring it. In mbedtls 3.x
# that declaration only lives in the library-private library/x509_internal.h,
# so the call fails to compile under -Werror (implicit-function-declaration).
#
# That header can't just be included: it drags in library/pk_internal.h,
# which assumes it's compiled as part of mbedtls itself with
# MBEDTLS_ALLOW_PRIVATE_ACCESS defined project-wide, and fails on the
# MBEDTLS_PRIVATE-mangled struct fields otherwise. mbedtls_x509_get_name()
# itself only needs mbedtls_x509_name, which mbedtls/x509.h already exposes
# publicly -- so forward-declare the one function instead of including the
# private header.
#
# Runs as a FetchContent PATCH_COMMAND, whose working directory is the root
# of the fetched libwebsockets source tree.

set(_file "lib/tls/mbedtls/mbedtls-extensions.c")
file(READ "${_file}" _contents)
if(NOT _contents MATCHES "mbedtls_x509_get_name\\(unsigned char")
    string(REPLACE
        "#include <mbedtls/x509.h>"
        "#include <mbedtls/x509.h>\n\n/* declared in mbedtls's private library/x509_internal.h; see comment in patch-libwebsockets-mbedtls-extensions.cmake */\nint mbedtls_x509_get_name(unsigned char **p, const unsigned char *end, mbedtls_x509_name *cur);"
        _contents "${_contents}")
    file(WRITE "${_file}" "${_contents}")
endif()
