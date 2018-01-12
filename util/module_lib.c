// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#include "module_lib.h"
#include <stdlib.h>      // malloc
#include <string.h>      // strlen
#include <openssl/ssl.h> // PEM_read_PUBKEY
#include <openssl/evp.h> // EVP_DigestVerify*
#include <openssl/err.h>
#include <time.h>

#include "curl_request.h"

void gen_random(char *s, const int len) {
	srand(time(NULL));

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	s[len] = 0;
}

char *firebase_build_url(char *path, char *local_id, char *id_token) {
	char *url = malloc(46 + strlen(local_id) + strlen(path) + 11 + strlen(id_token));
	if (!url) {
		return NULL;
	}
	strcpy(url, FIREBASE_URL);
	strcpy(url + 46, local_id);
	strcpy(url + strlen(url), path);
	strcpy(url + strlen(url), FIREBASE_AUTH_ARG);
	strcpy(url + strlen(url), id_token);
	return url;
}

char *firebase_get(CURL *handle, char *path, char *local_id, char *id_token) {
	char *url = firebase_build_url(path, local_id, id_token);
	if (!url) {
		return NULL;
	}
	char *curl_response = curl_request(handle, url, NULL, METHOD_POST, NULL);
	free(url);
	const int length = strlen(curl_response) - 2;
	memmove(curl_response, curl_response + 1, length);
	*(curl_response + length) = 0;
	char *curl_response_shortened = realloc(curl_response, length + 1);
	if (!curl_response_shortened) {
		return curl_response;
	}
	return curl_response_shortened;
}

void firebase_set(CURL *handle, char *path, char *local_id, char *id_token, char *value) {
	char *url = firebase_build_url(path, local_id, id_token);
	if (!url) {
		return;
	}
	int length = strlen(value) + 2;
	char *value_next = malloc(length + 1);
	if (!value_next) {
		return;
	}
	memcpy(value_next + 1, value, length - 2);
	*(value_next) = *(value_next + length - 1) = '\"';
	*(value_next + length) = 0;
	free(curl_request(handle, url, NULL, METHOD_PUT, value_next));
	free(url);
	free(value_next);
}

base64string base64decode(const void *b64_decode_this, int decode_this_many_bytes) {
	base64string base;
	BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
	unsigned char *base64_decoded = calloc((decode_this_many_bytes*3)/4+1, sizeof(unsigned char) ); //+1 = null.
	b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
	mem_bio = BIO_new(BIO_s_mem());                         //Initialize our memory source BIO.
	BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes); //Base64 data saved in source.
	BIO_push(b64_bio, mem_bio);          //Link the BIOs by creating a filter-source BIO chain.
	BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);          //Don't require trailing newlines.
	int decoded_byte_index = 0;   //Index where the next base64_decoded byte should be written.
	while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ) { //Read byte-by-byte.
		decoded_byte_index++; //Increment the index until read of BIO decoded data is complete.
	} //Once we're done reading decoded data, BIO_read returns -1 even though there's no error.
	base.size = decoded_byte_index;
	base.pointer = base64_decoded;
	BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
	return base;        //Returns base-64 decoded data with trailing null terminator.
}

int verify(const char *base, const char *challenge, FILE *public_key_file) {
	if (public_key_file == NULL) {
		printf("Public key not found, please move it to the right folder!\n");
		return 0;
	}
	base64string bases = base64decode(base, strlen(base));
	unsigned char *sig = bases.pointer;
	int verify_result = 0, msglen = strlen(challenge), siglen = bases.size;

	if (!msglen || !siglen) {
		printf("Challenge or response are empty!");
		free(sig);
		return 0;
	}

	// Context object
	EVP_MD_CTX *mdctx = NULL;
	EVP_PKEY *public_key = NULL;
	PEM_read_PUBKEY(public_key_file, &public_key, NULL, NULL);
	fclose(public_key_file);

	if (!(mdctx = EVP_MD_CTX_create()) || EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, public_key) != 1 ||
			EVP_DigestVerifyUpdate(mdctx, challenge, msglen) != 1 ||
			(verify_result = EVP_DigestVerifyFinal(mdctx, sig, siglen)) < 0) {
		printf("Error in the OpenSSL library!\n");
		verify_result = 0;
#ifdef TEST
		ERR_print_errors_fp(stdout);
#endif
	} else {
		printf("Verification %s\n", verify_result > 0 ? "Successful" : "Failure");
	}
	EVP_MD_CTX_destroy(mdctx);
	return verify_result;
}