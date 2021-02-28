#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <time.h>

#define SEPARATOR ":"
#define TOPIC_SEPARATOR "/"

/**
 * Section: extract info
 * */

/**
 * @brief Extract project_id, client_type, type_id, instance_id from mosquitto's client's client_id
 * 
 * @param char* client_id
 * @param char** project_id
 * @param char** client_type
 * @param char** type_id
 * @param char** instance_id
 * 
 * @return Return Code
 * */
int extract_client_id(const char* client_id, char **project_id, char** client_type, char** type_id, char** instance_id);

/**
 * Section: Create JSON payload
 * */
int create_new_payload(char* project_id, char* client_type, char* type_id, char* instance_id, char* old_payload, char** result);

void current_timestamp(char** timestamp);

int extract_topic(const char* topic, char, char** token);
