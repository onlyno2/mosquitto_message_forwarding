#include "topics.h"

struct topic_tokens init_topic_tokens(void)
{
  struct topic_tokens tt;
  tt.n_fields = 0;

  return tt;
}

struct topic_tokens extract_topic(const char* topic)
{
  struct topic_tokens tt = init_topic_tokens();
  char *p, *s, *save;
  save = s = strdup(topic);
  p = strsep(&s, TOPIC_SEPARATOR);

  while(p) {
    tt.n_fields += 1;
    tt.tokens = realloc(tt.tokens, sizeof(char *) * (tt.n_fields));

    if(tt.tokens == NULL) {
      tt.n_fields = UNHANDLED_ERROR;
      return tt;
    }

    tt.tokens[tt.n_fields - 1] = strdup(p);
    p = strsep(&s, TOPIC_SEPARATOR);
  }

  free(save);
  return tt;
}

int api_version(struct topic_tokens tt)
{
  if(strcmp(tt.tokens[0], "v1") == 0)
    return V1;
  else return VERSION_UNDEFINED;
}

int message_handler(const char* topic, const char* client_id, const char* payload)
{
  int rc = UNHANDLED_ERROR;
  struct topic_tokens tt = extract_topic(topic);
  if(tt.n_fields == UNHANDLED_ERROR)
    return rc;
  
  int current_api_version = api_version(tt);
  if(current_api_version == VERSION_UNDEFINED)
    return UNHANDLED_ERROR;
  
  switch (current_api_version)
  {
  case V1:
    mosquitto_log_printf(MOSQ_LOG_INFO, "Before entering v1 handle");

    rc = v1_message_handler(tt, client_id, payload);
    break;
  }

  free(tt.tokens);
  return rc;
}

int v1_topic_type(struct topic_tokens tt)
{
  mosquitto_log_printf(MOSQ_LOG_INFO, "Topic type: %s", tt.tokens[1]);

  if(strcmp(tt.tokens[1], "evt") == 0)
    return EVENT_TOPIC;
  else if(strcmp(tt.tokens[1], "cmd") == 0)
    return COMMAND_TOPIC;
  else if(strcmp(tt.tokens[1], "manage") == 0)
    return MANAGE_TOPIC;
  else return UNDEFINED_TOPIC;
}

char* topic_last_token(struct topic_tokens tt)
{
  return tt.tokens[tt.n_fields - 1];
}

char* v1_device_event_id(struct topic_tokens tt)
{
  return topic_last_token(tt);
}

char* v1_device_command_id(struct topic_tokens tt)
{
  return topic_last_token(tt);
}

int v1_message_handler(struct topic_tokens tt, const char* client_id, const char* payload)
{
  int rc = UNHANDLED_ERROR;
  int topic_type = v1_topic_type(tt);
  char *event_id = NULL, *command_id = NULL;

  if(topic_type == UNDEFINED_TOPIC)
    return UNHANDLED_ERROR;
  
  switch (topic_type)
  {
  case EVENT_TOPIC:
    event_id = v1_device_event_id(tt);
    rc = v1_device_event_message_handler(client_id, payload, event_id);
    break;

  case COMMAND_TOPIC:
    mosquitto_log_printf(MOSQ_LOG_INFO, "Before entering handle");

    command_id = v1_device_command_id(tt);
    rc = v1_device_command_message_handler(tt, payload, command_id);
    break;

  case MANAGE_TOPIC:
    rc = v1_device_manage_message_handler(client_id, payload);
    break;  
  }

  return rc;
}

size_t device_client_size(device_client dc)
{
  return strlen(dc.project_id) + strlen(dc.class_id) + strlen(dc.type_id) + strlen(dc.device_id);
}

int v1_device_event_message_handler(const char* client_id, const char* payload, const char* event_id)
{
  device_client client;
  int rc = extract_client_id(client_id, &client);
  if(rc == UNHANDLED_ERROR)
    return rc;

  char *new_payload = NULL;
  rc = v1_build_client_event_payload(client, payload, event_id, &new_payload);

  /* Broker send new message for external apps to subcribe at topic v1/evt/project_id/class_id/types/type_id/devices/device_id */
  char *external_app_topic = (char*)malloc(sizeof(char) * (33 + device_client_size(client) + strlen(event_id) + 1));
  if(!sprintf(external_app_topic, "v1/evt-2/%s/%s/types/%s/devices/%s/event/%s", client.project_id, client.class_id, client.type_id, client.device_id, event_id))
    return rc;
  /* Broker send new message for platform core at topic v1/core/evt */
  char *platform_core_topic = (char*)malloc(sizeof(char) * 12);
  if(!sprintf(platform_core_topic, "v1/core/evt"))
    return rc;

  mosquitto_log_printf(MOSQ_LOG_INFO, "Forwarding event to topics: \n\t" RED "core" RESET ":" BLU " %s" RESET "\n\t" RED "ext" RESET ":" BLU " %s" RESET, platform_core_topic, external_app_topic);
  rc = mosquitto_broker_publish_copy(NULL, external_app_topic, strlen(new_payload), new_payload, 0, false, NULL);
  if(rc == MOSQ_ERR_INVAL)
    return rc;

  /* Send this message to all core platform instance */
  rc = mosquitto_broker_publish_copy(NULL, platform_core_topic, strlen(new_payload), new_payload, 0, false, NULL);
  if(rc == MOSQ_ERR_INVAL)
    return rc;

  free(external_app_topic);
  free(platform_core_topic);
  free(new_payload);
  return rc;
}

void client_to_command(struct topic_tokens tt, device_client* result)
{
  result->project_id = tt.tokens[2];
  result->class_id = tt.tokens[3];
  result->type_id = tt.tokens[5];
  result->device_id = tt.tokens[7];
}

int v1_device_command_message_handler(struct topic_tokens tt, const char* payload, const char* command_id)
{
  int rc = UNHANDLED_ERROR;
  device_client client;
  client_to_command(tt, &client);
    mosquitto_log_printf(MOSQ_LOG_INFO, "In entering handle");


  /* client's id which receive command */
  char *client_id = (char*)malloc(sizeof(char) * (device_client_size(client) + 5));
  if(!sprintf(client_id, "%s:%s:%s:%s", client.project_id, client.class_id, client.type_id, client.device_id))
    return rc;

  /* Broker send new message to specified client at topic v1/cmd/command_id */
  char *topic = (char*)malloc(sizeof(char) * (7 + strlen(command_id) + 1));
  if(!sprintf(topic, "v1/cmd/%s", command_id))
    return rc;

  mosquitto_log_printf(MOSQ_LOG_INFO, "Forwarding event to: \n\t" RED "topic" RESET ":" BLU " %s" RESET "\n\t" RED "client_id" RESET ":" BLU " %s" RESET, topic, client_id);


  char *new_payload = NULL;
  rc = v1_build_client_command_payload(payload, &new_payload);
  if(rc == UNHANDLED_ERROR)
    goto out;

  rc = mosquitto_broker_publish_copy(client_id, topic, strlen(new_payload), new_payload, 0, false, NULL);

out:
  free(client_id);
  free(topic);
  return rc;
}

int v1_device_manage_message_handler(const char* client_id, const char* payload)
{
  device_client client;
  int rc = extract_client_id(client_id, &client);
  if(rc == UNHANDLED_ERROR)
    return rc;

  char *new_payload = NULL;
  rc = v1_build_client_manage_payload(client, payload, &new_payload);
  if(rc == UNHANDLED_ERROR) 
    return rc;

  /* Broker send new message to all core platform instance at topic v1/core/manage */
  char *topic = (char*)malloc(sizeof(char) * 15);
  if(!sprintf(topic, "v1/core/manage"))
    return rc;

  rc = mosquitto_broker_publish_copy(NULL, topic, strlen(new_payload), new_payload, 0, false, NULL);

  free(new_payload);
  free(topic);
  return rc;
}