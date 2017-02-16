CC=gcc
CFLAGS=-fPIC -DPIC -shared -rdynamic
IFLAGS=-IOAuth2/include
LDFLAGS=$(IFLAGS) `pkg-config --cflags --libs libcurl json-c libssl libcrypto`
PAM_PATH=`locate pam_unix.so | sed 's/\(.*\/\).*/\1/'`

BIN=libpam.so
BINTEST=module

all: clean setup module

.PHONY: clean setup module test

clean:
	rm -f setup $(BIN) $(BINTEST)

setup: setup.c OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c util/server.c
	$(CC) -o $@ $+ $(LDFLAGS)

module: module.c OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c util/module_lib.c
	$(CC) $(CFLAGS) -o $(BIN) $+ $(LDFLAGS)

test: clean setup
	make module CFLAGS="-Wall -g -DTEST" BIN=$(BINTEST)


install: all
	cp libpam.so $(PAM_PATH)pam_android.so
	chown root:root $(PAM_PATH)pam_android.so
	chmod 755 $(PAM_PATH)pam_android.so