#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <mosquitto_broker.h>
#include <cjson/cJSON.h>

enum return_code {
  INVALID_FORMAT = 501,
  UNHANDLED_ERROR = 500,
  SUCCESS = 200
};

#endif
