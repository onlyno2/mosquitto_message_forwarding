#include "client.h"

int extract_client_id(const char* client_id, device_client* result)
{
  int rc = UNHANDLED_ERROR;
  char *p, *s, *save;
  save = s = strdup(client_id);

  if((p = strsep(&s, CLIENT_ID_SEPARATOR)) == NULL)
    goto out;
  result->project_id = strdup(p);

  if((p = strsep(&s, CLIENT_ID_SEPARATOR)) == NULL)
    goto out;
  result->class_id = strdup(p);

  if((p = strsep(&s, CLIENT_ID_SEPARATOR)) == NULL)
    goto out;
  result->type_id = strdup(p);
  
  if((p = strsep(&s, CLIENT_ID_SEPARATOR)) == NULL)
    goto out;
  result->device_id = strdup(p);

  rc = SUCCESS;
out:
  free(save);
  return rc;
}

long current_timestamp()
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec*1000 + lround(ts.tv_nsec/1.0e6);
}

int v1_add_client_info_to_message(device_client dc, cJSON** payload_json)
{
  int rc = UNHANDLED_ERROR;

  if(cJSON_AddStringToObject(*payload_json, "project_id", dc.project_id) == NULL)
    goto out;

  if(cJSON_AddStringToObject(*payload_json, "class_id", dc.class_id) == NULL)
    goto out;

  if(cJSON_AddStringToObject(*payload_json, "type_id", dc.type_id) == NULL)
    goto out;

  if(cJSON_AddStringToObject(*payload_json, "device_id", dc.device_id) == NULL)
    goto out;
out:
  return rc;
}

int v1_build_client_event_payload(device_client dc, const char* old_payload_string, const char* event_id, char** result)
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

  v1_add_client_info_to_message(dc, &new_payload_json);
  if(cJSON_AddStringToObject(new_payload_json, "event_id", event_id) == NULL)
    goto out;
  
  long timestamp = current_timestamp();
  if(cJSON_AddNumberToObject(new_payload_json, "timestamp", timestamp) == NULL)
    goto out;

  *result = cJSON_PrintUnformatted(new_payload_json);

  cJSON_Delete(new_payload_json);
  rc = SUCCESS;
out:
  return rc;
}

int v1_build_client_manage_payload(device_client dc, const char* old_payload_string, char **result)
{
  int rc = UNHANDLED_ERROR;
  cJSON *old_payload_json = cJSON_Parse(old_payload_string);
  /* Only accept JSON payload */
  if(old_payload_json == NULL)
    return rc;

  cJSON *new_payload_json = cJSON_CreateObject();
  cJSON *metadata = NULL, *device_info = NULL, *payload_field = NULL; 

  metadata = cJSON_GetObjectItemCaseSensitive(old_payload_json, "metadata");
  device_info = cJSON_GetObjectItemCaseSensitive(old_payload_json, "device_info");

  /* Manage payload only have metadata and device_info object */
  
  v1_add_client_info_to_message(dc, &new_payload_json);
  if((payload_field = cJSON_AddObjectToObject(new_payload_json, "payload")) == NULL)
    return rc;

  if(metadata != NULL) {
    if(!cJSON_AddItemReferenceToObject(payload_field, "metadata", metadata))
      goto out;
  } else {
    cJSON *empty_metadata_object = cJSON_CreateObject();
    if(!cJSON_AddItemToObject(payload_field, "metadata", empty_metadata_object))
      goto out;
  }

  if(device_info != NULL) {
    if(!cJSON_AddItemReferenceToObject(payload_field, "device_info", device_info))
      goto out;
  } else {
    cJSON *empty_device_info_object = cJSON_CreateObject();
    if(!cJSON_AddItemToObject(payload_field, "device_info", empty_device_info_object))
      goto out;
  }

  *result = cJSON_PrintUnformatted(new_payload_json);

out:
  cJSON_Delete(new_payload_json);
  cJSON_Delete(old_payload_json);
  rc = SUCCESS;
  return rc;
}

int v1_build_client_command_payload(const char* old_payload_string, char **result)
{
  int rc = UNHANDLED_ERROR;
  cJSON *old_payload_json = cJSON_Parse(old_payload_string);
  /* Only accept JSON payload */
  if(old_payload_json == NULL) {
    return rc;
  }

  *result = cJSON_PrintUnformatted(old_payload_json);
  free(old_payload_json);

  rc = SUCCESS;
  return rc;
}