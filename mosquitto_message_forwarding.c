#include "config.h"

#include <stdio.h>
#include <time.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

#include <cjson/cJSON.h>
#include "plugin.h"
#include "topics.h"

static mosquitto_plugin_id_t *mosq_pid = NULL;

static int callback_message(int event, void *event_data, void *userdata)
{
  struct mosquitto_evt_message *ed = event_data;

  UNUSED(event);
  UNUSED(userdata);

  const char *topic = ed->topic;
  const char *client_id = mosquitto_client_id(ed->client);
  const char *payload = ed->payload;

  message_handler(topic, client_id, payload);
  
  /* Other error code broke client connection :< */
  return MOSQ_ERR_SUCCESS;
}

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
  int i;

  for (i = 0; i < supported_version_count; i++)
  {
    if (supported_versions[i] == 5)
    {
      return 5;
    }
  }
  return -1;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
  UNUSED(user_data);
  UNUSED(opts);
  UNUSED(opt_count);

  mosq_pid = identifier;
  return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, NULL);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
  UNUSED(user_data);
  UNUSED(opts);
  UNUSED(opt_count);

  return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL);
}
