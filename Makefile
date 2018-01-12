# Copyright (C) 2017  Maxr1998
# For more information, view the LICENSE at the root of this project

CC=gcc
CFLAGS=-Wall -Wextra -std=gnu99 -pedantic -pedantic-errors
IFLAGS=-IOAuth2/include
LDLIBS=`pkg-config --cflags --libs libcurl json-c libssl libcrypto`

COMMON_SOURCES=OAuth2/src/oauth2.c OAuth2/src/curl_request.c util/common.c

NAME=pam_android.so
NAME_TEST=module

INSTALL_PATH=/lib/security/$(NAME)
CONFIG_PATH=/usr/share/pam-configs/androidpam

all: clean setup module

.PHONY: clean setup module debug

clean:
	rm -f setup $(NAME) $(NAME_TEST)

setup: setup.c $(COMMON_SOURCES) util/server.c
	$(CC) $(CFLAGS) -o $@ $+ $(IFLAGS) $(LDLIBS)

module: module.c $(COMMON_SOURCES) util/module_lib.c /usr/lib/libpam.so.0
	$(CC) -fPIC -DPIC -shared -rdynamic $(CFLAGS) -o $(NAME) $+ $(IFLAGS) $(LDLIBS)

debug: clean setup
	make module CFLAGS="-g -DTEST $(CFLAGS)" BIN=$(NAME_TEST)


install: all
	# Module
	cp $(NAME) $(INSTALL_PATH)
	chown root:root $(INSTALL_PATH)
	chmod 644 $(INSTALL_PATH)
	# Config
	cp config/androidpam $(CONFIG_PATH)
	chown root:root $(CONFIG_PATH)
	chmod 644 $(CONFIG_PATH)
	pam-auth-update

uninstall:
	rm -f $(INSTALL_PATH)
