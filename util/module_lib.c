#include "module_lib.h"
#include <stdlib.h>      // malloc
#include <string.h>      // strlen
#include <openssl/ssl.h> // PEM_read_PUBKEY
#include <openssl/evp.h> // EVP_DigestVerify*
#include <openssl/err.h>

#include "curl_request.h"

#define FIREBASE_URL "https://androidpam-979c7.firebaseio.com/users/"
#define FIREBASE_AUTH_ARG ".json?auth="

void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

char *firebase_get(char *path, char *local_id, char *id_token) {
  char *response;
  char *url = malloc(46 + strlen(local_id) + strlen(path) + 11 + strlen(id_token));
  strcpy(url, FIREBASE_URL);
  strcpy(url + 46, local_id);
  strcpy(url + strlen(url), path);
  strcpy(url + strlen(url), FIREBASE_AUTH_ARG);
  strcpy(url + strlen(url), id_token);
  response = curl_make_request(url,  NULL);
  free(url);
  return response;
}

int firebase_set(char *path, char *local_id, char *id_token, char *value) {
  char *url = malloc(46 + strlen(local_id) + strlen(path) + 11 + strlen(id_token));
  strcpy(url, FIREBASE_URL);
  strcpy(url + 46, local_id);
  strcpy(url + strlen(url), path);
  strcpy(url + strlen(url), FIREBASE_AUTH_ARG);
  strcpy(url + strlen(url), id_token);
  free(curl_request(url, NULL, METHOD_PUT, value));
  free(url);
  return 0;
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

int verify(unsigned char *base, char *challenge, char *keyPemPath) {
  int retVal = 0;
  base64string bases = base64decode(base, strlen(base));
  unsigned char *sig = bases.pointer;
  int i = 0, msglen = strlen(challenge), siglen = bases.size;
  if (siglen == 0) {
    printf("Error, too short\n\n");
    goto end;
  }

  // Context object
  EVP_MD_CTX *mdctx = NULL;
  // Key
  EVP_PKEY *public = NULL;
  do {
    FILE *publicKeyFile = fopen(keyPemPath, "r");
    if (publicKeyFile == NULL) {
      printf("Public key not found!\n");
      break;
    }
    PEM_read_PUBKEY(publicKeyFile, &public, NULL, NULL);
    fclose(publicKeyFile);

    if (!(mdctx = EVP_MD_CTX_create())) break;
    if (1 != EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, public)) break;
    if (1 != EVP_DigestVerifyUpdate(mdctx, challenge, msglen)) break;
    i = EVP_DigestVerifyFinal(mdctx, sig, siglen);
    if (i > 0) {
      printf("Verified OK\n");
      retVal = 1;
      break;
    } else if (i == 0) {
      printf("Verification Failure\n");
      break;
    } else {
      printf("Error\n");
      ERR_print_errors_fp(stdout);
      break;
    }
  } while (0);

  // Cleanup
  end:
  free(bases.pointer);
  if (mdctx) EVP_MD_CTX_destroy(mdctx);
  return retVal;
}