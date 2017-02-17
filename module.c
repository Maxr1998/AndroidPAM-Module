/* Define which PAM interfaces we provide */
#define PAM_SM_AUTH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include "oauth2.h"
#include "curl_request.h"
#include <json-c/json.h>
#include "util/common.h"
#include "util/module_lib.h"

#define GOOGLE_ID_SERVER "https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyAssertion?key=AIzaSyADECXIeWAOJidQ-IrmKuxAp3zmUmz6btY"

void debug_print(char *out) {
  #ifdef TEST
  printf("%s", out);
  #endif
}

int main() {
  #ifdef TEST
  pam_sm_authenticate(NULL, 0, 0, NULL);
  #else
  #endif
}

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  int retVal = PAM_IGNORE;

  #ifndef TEST
  struct passwd *pw = NULL, pw_s;
  const char *user = NULL;
  char buffer[1024];
  int pgu_ret, gpn_ret;

  pgu_ret = pam_get_user(pamh, &user, NULL);
  if (pgu_ret != PAM_SUCCESS || user == NULL) {
    return(PAM_IGNORE);
  }

  gpn_ret = getpwnam_r(user, &pw_s, buffer, sizeof(buffer), &pw);
  if (gpn_ret != 0 || pw == NULL || pw->pw_dir == NULL || pw->pw_dir[0] != '/') {
    return(PAM_IGNORE);
  }
  char *home_dir = pw->pw_dir;
  #else
  const char *home_dir = "/home/max";
  #endif

  // VARIABLES
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *handle = curl_easy_init();
  char *app_dir_path = create_app_dir_from_home(home_dir, false);
  char *output = NULL;
  struct json_object *info_jo = NULL;
  char *local_id = NULL;
  char *id_token = NULL;
  int clength = 32;
  char challenge[clength];

  // MAIN LOGIC
  debug_print("Logging in...\n");
  // Read refresh-token
  char *token_path = malloc(strlen(app_dir_path) + 14);
  strcat(strcpy(token_path, app_dir_path), "/refresh-token");
  FILE *refresh_token_file = fopen(token_path, "r");
  free(token_path);
  if (refresh_token_file == NULL) {
    goto CLEANUP;
  }
  char token[128];
  fgets(token, 128, refresh_token_file);
  fclose(refresh_token_file);

  // Get access_token
  oauth2_config *config = oauth2_init(CLIENT_ID, CLIENT_SECRET);
  oauth2_set_redirect_uri(config, REDIRECT_URI);
  oauth2_tokens tokens = oauth2_access_tokens(config, GOOGLE_TOKEN_SERVER, token, true);
  oauth2_cleanup(config);
  if (tokens.access_token == NULL) {
    debug_print("Error: Access token not avaiable!\n");
    oauth2_tokens_cleanup(tokens);
    goto CLEANUP;
  }

  // Get idToken
  struct curl_slist *slist1 = NULL;
  slist1 = curl_slist_append(slist1, "Content-Type: application/json");
  char *post_data = malloc(26 + strlen(tokens.access_token) + 76 + 100);
  strcpy(post_data, "{\"postBody\":\"access_token=");
  strcpy(post_data + 26, tokens.access_token);
  oauth2_tokens_cleanup(tokens);
  strcpy(post_data + strlen(post_data), "&providerId=google.com\",\"requestUri\":\"http://localhost\",\"returnSecureToken\":true}");
  output = curl_request(handle, GOOGLE_ID_SERVER, slist1, METHOD_POST, post_data);
  curl_slist_free_all(slist1);
  free(post_data);

  // Parse JSON and get local_id and id_token
  info_jo = json_tokener_parse(output);
  struct json_object *local_id_jo, *id_token_jo;
  if (!json_object_object_get_ex(info_jo, "localId", &local_id_jo) || !json_object_object_get_ex(info_jo, "idToken", &id_token_jo)) {
    debug_print("Error: idToken not in verification response!\n");
    goto CLEANUP;
  }
  local_id = strdup(json_object_get_string(local_id_jo));
  id_token = strdup(json_object_get_string(id_token_jo));
  free(output);
  json_object_put(info_jo);

  // Create and send challenge
  gen_random(challenge, clength);
  debug_print("Sending challenge... ");
  fflush(stdout);
  char *tmp_challenge = malloc(clength + 2);
  strcpy(tmp_challenge, "\"");
  strcpy(tmp_challenge + 1, challenge);
  strcpy(tmp_challenge + 1 + clength, "\"");
  firebase_set(handle, "/request/challenge", local_id, id_token, tmp_challenge);
  free(tmp_challenge);
  firebase_set(handle, "/request/state", local_id, id_token, "\"request\"");

  debug_print("Done.\nWaiting for response and signature...\n");
  while (strcmp(output = firebase_get(handle, "/request/state", local_id, id_token), "\"signed\"") != 0) {
    free(output);
  }
  free(output);

  // Get signature and verify it
  output = firebase_get(handle, "/request/response", local_id, id_token);
  debug_print("Received response!\n");
  unsigned char *p = strdup(output);
  p++[strlen(p)] = 0;
  char *public_key_path = malloc(strlen(app_dir_path) + 11);
  strcat(strcpy(public_key_path, app_dir_path), "/public.pem");
  if (verify(p, challenge, public_key_path)) {
    retVal = PAM_SUCCESS;
  }
  free(--p);
  free(public_key_path);

  firebase_set(handle, "/request/response", local_id, id_token, "\"\"");
  firebase_set(handle, "/request/state", local_id, id_token, "\"idle\"");

  CLEANUP:
  curl_easy_cleanup(handle);
  curl_global_cleanup();
  free(app_dir_path);
  free(output);
  json_object_put(info_jo);
  free(local_id);
  free(id_token);
  return retVal;
}

/*
PAM entry point for setting user credentials (that is, to actually
establish the authenticated user's credentials to the service provider)
*/
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_IGNORE);
}