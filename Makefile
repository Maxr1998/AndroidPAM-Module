CC=gcc
CFLAGS=-fPIC -DPIC -shared -rdynamic
IFLAGS=-IOAuth2/include
LDLIBS=`pkg-config --cflags --libs libcurl json-c libssl libcrypto`

COMMON_SOURCES=OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c

NAME=pam_android.so
NAME_TEST=module

INSTALL_PATH=/lib/security/$(NAME)

all: clean setup module

.PHONY: clean setup module debug

clean:
	rm -f setup $(NAME) $(NAME_TEST)

setup: setup.c $(COMMON_SOURCES) util/server.c
	$(CC) -o $@ $+ $(IFLAGS) $(LDLIBS)

module: module.c $(COMMON_SOURCES) util/module_lib.c /lib/x86_64-linux-gnu/libpam.so.0
	$(CC) $(CFLAGS) -o $(NAME) $+ $(IFLAGS) $(LDLIBS)

debug: clean setup
	make module CFLAGS="-Wall -g -DTEST" BIN=$(NAME_TEST)


install: all
	cp $(NAME) $(INSTALL_PATH)
	chown root:root $(INSTALL_PATH)
	chmod 644 $(INSTALL_PATH)

uninstall:
	rm -f $(INSTALL_PATH)
