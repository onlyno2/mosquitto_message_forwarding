#include "helpers.h"
#include "plugin.h"

int extract_client_id(const char* client_id, char **project_id, char** class_id, char** type_id, char **device_id)
{
  char *p, *s, *save;
  int rc = INVALID_FORMAT;

  save = s = strdup(client_id);
  if ((p = strsep(&s, SEPARATOR)) == NULL)
		goto out;
	*project_id = strdup(p);

  if ((p = strsep(&s, SEPARATOR)) == NULL)
		goto out;
	*class_id = strdup(p);

  if ((p = strsep(&s, SEPARATOR)) == NULL)
		goto out;
	*type_id = strdup(p);

  if ((p = strsep(&s, SEPARATOR)) == NULL)
		goto out;
	*device_id = strdup(p);

  rc = SUCCESS;

out: 
  free(save);
  return rc;
}

int create_new_payload(char* project_id, char* client_type, char* type_id, char* instance_id, char* old_payload_string, char** result)
{
  int rc = UNHANDLED_ERROR;
  cJSON *new_payload_json = cJSON_CreateObject();
  cJSON *old_payload_json = cJSON_Parse(old_payload_string);

  if(old_payload_json == NULL) {
    if(cJSON_AddStringToObject(new_payload_json, "payload", old_payload_string) == NULL)
      goto out;
  } else {
    if(!cJSON_AddItemToObject(new_payload_json, "payload", old_payload_json))
      goto out;
  }

  if(cJSON_AddStringToObject(new_payload_json, "project_id", project_id) == NULL)
    goto out;

  if(cJSON_AddStringToObject(new_payload_json, "client_type", client_type) == NULL)
    goto out;

  if(cJSON_AddStringToObject(new_payload_json, "type_id", type_id) == NULL)
    goto out;

  if(cJSON_AddStringToObject(new_payload_json, "instance_id", instance_id) == NULL)
    goto out;
  
  char *timestamp = NULL;
  current_timestamp(&timestamp);
  if(cJSON_AddStringToObject(new_payload_json, "timestamp", timestamp) == NULL)
    goto out;

  *result = cJSON_PrintUnformatted(new_payload_json);

  cJSON_Delete(new_payload_json);
  free(timestamp);
  rc = SUCCESS;
out:
  return rc;
}

void current_timestamp(char** timestamp)
{
  struct timespec ts;
	struct tm *ti;
	// char* time_buf = malloc(50);
  char time_buf[25];

  clock_gettime(CLOCK_REALTIME, &ts);
	ti = gmtime(&ts.tv_sec);
	strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", ti);

  *timestamp = strdup(time_buf);
}