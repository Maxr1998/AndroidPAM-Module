/* AndroidPAM, a pam module to unlock your computer with your phone's fingerprint reader
 * Copyright (C) 2017  Maxr1998
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pwd.h>

#include "oauth2.h"
#include "util/common.h"
#include "util/server.h"

int main() {
  // VARIABLES
  char *token_path;
  FILE *refresh_token_file;
  oauth2_config *config = oauth2_init(CLIENT_ID, CLIENT_SECRET);
  oauth2_set_redirect_uri(config, REDIRECT_URI);
  oauth2_tokens tokens;
  char *auth_code = NULL;

  // Create application directory
  const char *home_dir;
  if ((home_dir = getenv("HOME")) == NULL) {
    home_dir = getpwuid(getuid())->pw_dir;
  }
  char *app_dir_path = create_app_dir_from_home(home_dir, true);
  token_path = malloc(strlen(app_dir_path) + 14);
  strcat(strcpy(token_path, app_dir_path), "/refresh-token");
  free(app_dir_path);
  refresh_token_file = fopen(token_path, "r");

  // MAIN LOGIC
  printf("Checking for existing OAuth2.0 token... ");
  // Check for token file
  if (refresh_token_file != NULL) {
    printf("Success!\nValidating token... ");
    char token[128];
    fgets(token, 128, refresh_token_file);
    tokens = oauth2_access_tokens(config, GOOGLE_TOKEN_SERVER, token, true);
    if (tokens.access_token != NULL) {
      printf("Token valid!\nExiting here.\n");
      goto CLEANUP;
    }
    oauth2_tokens_cleanup(tokens);
    printf("Invalid, re-authenticating…\n");
    fclose(refresh_token_file);
  } else {
    printf("Failed.\nNo token set up: You will now be redirected to your browser, please login with the Google account you also use on your phone.\n");
  }
  refresh_token_file = fopen(token_path, "w");

  // Countdown to let the user read the above message(s)
  for (int i = 3; i > 0; i--) {
    printf("\r %d ", i);
    fflush(stdout);
    sleep(1);
  }
  printf("\n");

  // Build URL and build call
  char *url = oauth2_request_auth_code(config, GOOGLE_OAUTH_SERVER, AUTH_SCOPE, "");
  char *call = malloc(10 + strlen(url) + 1);
  strcpy(call, "xdg-open \"");
  strcpy(call + 10, url);
  strcpy(call + strlen(call), "\"");
  free(url);
  system(call);
  free(call);

  auth_code = start_server();
  if (auth_code == NULL) {
    printf("Error acquiring code, try again.\n");
    goto CLEANUP;
  }

  tokens = oauth2_access_tokens(config, GOOGLE_TOKEN_SERVER, auth_code, false);
  char *refresh_token = tokens.refresh_token;
  printf("Success, saving request token: %s\n", refresh_token);
  fputs(refresh_token, refresh_token_file);

  CLEANUP:
  free(token_path);
  fclose(refresh_token_file);
  oauth2_cleanup(config);
  oauth2_tokens_cleanup(tokens);
  free(auth_code);
  return 0;
}