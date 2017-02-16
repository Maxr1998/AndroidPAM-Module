#ifndef __MODLIB_H__
#define __MODLIB_H__

void gen_random(char *s, const int len);
char *firebase_get(char *path, char *local_id, char *id_token);
int firebase_set(char *path, char *local_id, char *id_token, char *value);

typedef struct {
  unsigned char *pointer;
  int size;
} base64string;
base64string base64decode(const void *b64_decode_this, int decode_this_many_bytes);
int verify(unsigned char *base, char *challenge, char *keyPemPath);

#endif