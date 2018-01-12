// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#include "common.h"
//#include <stdbool.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

char *create_app_dir_from_home(const char *home, const bool create) {
	char *dir = malloc(strlen(home) + strlen(APP_DIR_PATH));
	strcpy(dir, home);
	strcpy(dir + strlen(dir), APP_DIR_PATH);
	struct stat st = {0};
	if (create && stat(dir, &st) == -1) {
		mkdir(dir, 0755);
	}
	return dir;
}

FILE *open_file(const char *base_path, const char *file_path, const bool rw) {
	FILE *file;
	char *path = malloc(strlen(base_path) + strlen(file_path));

	strcat(strcpy(path, base_path), file_path);
	file = fopen(path, rw ? "w" : "r");
	free(path);
	return file;
}