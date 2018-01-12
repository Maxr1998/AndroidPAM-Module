// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <stdio.h>

#define APP_DIR_PATH "/.androidpam"

#define CLIENT_ID "515535597677-0tm85eapguqml0cjde8m791ag547c458.apps.googleusercontent.com"
#define CLIENT_SECRET "PKJTpTwo6-nG-GYFYdHLS--v"
#define REDIRECT_URI "http://127.0.0.1:8567"
#define GOOGLE_OAUTH_SERVER "https://accounts.google.com/o/oauth2/v2/auth"
#define AUTH_SCOPE "email+profile"

#define GOOGLE_TOKEN_SERVER "https://accounts.google.com/o/oauth2/token"

char *create_app_dir_from_home(const char *home, const bool create);
FILE *open_file(const char *base_path, const char *file_path, const bool rw);

#endif