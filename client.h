#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <math.h>
#include "plugin.h"
#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <time.h>

#define CLIENT_ID_SEPARATOR ":"

typedef struct device_client {
  char *project_id;
  char *class_id;
  char *type_id;
  char *device_id;
} device_client;

/**
 * @brief Extract project_id, client_type, type_id, instance_id from mosquitto's client's client_id
 * 
 * @param client_id
 * @param result device_client info result
 * 
 * @return device_client
 * */
int extract_client_id(const char* client_id, device_client* result);

/**
 * @brief Get system's current timestamp
 * 
 * @return system's current timestamp in miliseconds
 * */
long current_timestamp();

/**
 * @brief Build new client event payload from old payload and client id for API V1
 * 
 * @param client_id
 * @param old_payload_string
 * @param result
 * 
 * @return return_code
 * */
int v1_build_client_event_payload(device_client dc, const char* old_payload_string, const char* event_id, char** result);

/**
 * @brief Build new client manage payload from old payload and client id for API V1
 * 
 * @param client_id
 * @param old_payload_string
 * @param result
 * 
 * @return return_code
 * */
int v1_build_client_manage_payload(device_client dc, const char* old_payload_string, char **result);

/**
 * @brief Build new client command payload for API V1
 * 
 * @param old_payload_string
 * @param result
 * 
 * @return return_code
 * */
int v1_build_client_command_payload(const char* old_payload_string, char **result);

#endif