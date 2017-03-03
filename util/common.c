// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#include "common.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

char *create_app_dir_from_home(const char *home, bool create) {
  char *dir = malloc(strlen(home) + 12);
  strcpy(dir, home);
  strcpy(dir + strlen(dir), "/.androidpam");
  struct stat st = {0};
  if (create && stat(dir, &st) == -1) {
    mkdir(dir, 0755);
  }
  return dir;
}