CC=gcc
CFLAGS=-fPIC -DPIC -shared -rdynamic
IFLAGS=-IOAuth2/include
LDFLAGS=$(IFLAGS) `pkg-config --cflags --libs libcurl json-c libssl libcrypto`
INSTALL_PATH=/lib/security/$(BIN)

BIN=pam_android.so
BINTEST=module

all: clean setup module

.PHONY: clean setup module test

clean:
	rm -f setup $(BIN) $(BINTEST)

setup: setup.c OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c util/server.c
	$(CC) -o $@ $+ $(LDFLAGS)

module: module.c OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c util/module_lib.c /lib/x86_64-linux-gnu/libpam.so.0
	$(CC) $(CFLAGS) -o $(BIN) $+ $(LDFLAGS)

test: clean setup
	make module CFLAGS="-Wall -g -DTEST" BIN=$(BINTEST)


install: all
	cp $(BIN) $(INSTALL_PATH)
	chown root:root $(INSTALL_PATH)
	chmod 644 $(INSTALL_PATH)

uninstall:
	rm -f $(INSTALL_PATH)
