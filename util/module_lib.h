// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#ifndef __MODLIB_H__
#define __MODLIB_H__

#include <curl/curl.h>

#define GOOGLE_ID_SERVER "https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyAssertion?key=AIzaSyADECXIeWAOJidQ-IrmKuxAp3zmUmz6btY"
#define FIREBASE_URL "https://androidpam-979c7.firebaseio.com/users/"
#define FIREBASE_AUTH_ARG ".json?auth="

void gen_random(char *s, const int len);
char *firebase_get(CURL *handle, char *path, char *local_id, char *id_token);
void firebase_set(CURL *handle, char *path, char *local_id, char *id_token, char *value);

typedef struct {
	unsigned char *pointer;
	int size;
} base64string;

base64string base64decode(const void *b64_decode_this, int decode_this_many_bytes);
int verify(const char *base, const char *challenge, FILE *public_key_file);

#endif