#include "Arduino.h"

#ifndef config_settings_H_
#define config_settings_H_

//Where in EEPROM do we store the configuration
#define EEPROM_storageSize 2048
#define EEPROM_WIFI_CHECKSUM_ADDRESS 0
#define EEPROM_WIFI_CONFIG_ADDRESS EEPROM_WIFI_CHECKSUM_ADDRESS+sizeof(uint32_t)

#define EEPROM_CHECKSUM_ADDRESS 512
#define EEPROM_CONFIG_ADDRESS EEPROM_CHECKSUM_ADDRESS+sizeof(uint32_t)

//We have allowed space for 2048-512 bytes of EEPROM for settings (1536 bytes)
struct eeprom_settings { 
  bool MQTT_Enable;
  int  mqtt_port;
  char mqtt_server[64 + 1];
  char mqtt_username[64 + 1];
  char mqtt_password[64 + 1];
  char mqtt_topic[64 + 1];
    
  bool influxdb_enabled;
  char influxdb_host[64 +1];
  int influxdb_httpPort;
  char influxdb_database[32 + 1];
  char influxdb_user[32 + 1];
  char influxdb_password[32 + 1];

  bool influxcloud_enabled;
  char influxcloud_url[64 + 1];
  char influxcloud_token[88 + 1];
  char influxcloud_org[32 + 1];
  char influxcloud_bucket[64 + 1];
};

extern eeprom_settings myConfig;

void WriteConfigToEEPROM();
bool LoadConfigFromEEPROM();
void WriteWIFIConfigToEEPROM();
bool LoadWIFIConfigFromEEPROM();
void FactoryResetSettings();

#endif
