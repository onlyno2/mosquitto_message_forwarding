#ifndef __TOPICS_H__
#define __TOPICS_H__

#include <stdio.h>

#include "plugin.h"
#include "client.h"
#include "mosquitto_broker.h"
#define TOPIC_SEPARATOR "/"

struct topic_tokens {
  char **tokens;
  unsigned int n_fields;
} topic_tokens;

enum topic_type {
  EVENT_TOPIC,
  COMMAND_TOPIC,
  MANAGE_TOPIC,
  UNDEFINED_TOPIC = -1
};

enum api_version {
  V1 = 861,
  VERSION_UNDEFINED = 860
};

/**
 * @brief topic_tokens init
 * 
 * @param void
 * 
 * @return New topic_tokens instance
 * */
struct topic_tokens init_topic_tokens(void);

/**
 * @brief Convert a topic into an array of fields
 * 
 * @param topic
 * 
 * @return Topic's tokens
 * */
struct topic_tokens extract_topic(const char* topic);

/**
 * @brief Wrapper handler to call in Mosquitto Plugin callback
 * 
 * @param topic
 * @param client_id
 * @param payload
 * 
 * @return return_code
 * */
int message_handler(const char* topic, const char* client_id, const char* payload);

/**
 * @brief Get api version of current topic
 * 
 * @param tt Topic's tokens
 * 
 * @return api_version
 * */
int topic_api_version(struct topic_tokens tt);

/**
 * @brief Check topic_type
 * 
 * @param topic_token Array of topic's fields
 * 
 * @return topic_type
 * */
int v1_topic_type(struct topic_tokens tt);

/**
 * @brief V1 Message handler
 * 
 * @param tt Topic's tokens
 * @param client_id
 * @param payload
 * 
 * @return return_code
 * */
int v1_message_handler(struct topic_tokens tt, const char* client_id, const char* payload);

/**
 * @brief API V1 Get device's event_id
 * 
 * @param tt Topic's tokens
 * 
 * @return event_id
 * */
char* v1_device_event_id(struct topic_tokens tt);

/**
 * @brief API V1 Get device's command_id
 * 
 * @param tt Topic's tokens
 * 
 * @return command_id
 * */
char* v1_device_command_id(struct topic_tokens tt);

/**
 * @brief V1 Handler for device event message
 * 
 * @param client_id
 * @param payload
 * @param event_id
 * 
 * @return return_code
 * */
int v1_device_event_message_handler(const char* client_id, const char* payload, const char* event_id);

/**
 * @brief Find client who will receive command from topic
 * 
 * @param topic
 * @param result device_client
 * 
 * @return
 * */
int command_client(const char* topic, device_client* result);

/**
 * @brief V1 Handler for device event message
 * 
 * @param topic_tokens
 * @param payload Message payload
 * @param command_id
 * 
 * @return return_code
 * */
int v1_device_command_message_handler(struct topic_tokens tt, const char* payload, const char* command_id);

/**
 * @brief V1 Handler for device manage message
 * 
 * @param client_id
 * @param payload Message payload
 * 
 * @return return_code
 * */
int v1_device_manage_message_handler(const char* client_id, const char* payload);


#endif