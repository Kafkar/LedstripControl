#ifndef CONFIG_H
#define CONFIG_H

#define SENSORNAME "TemperatuurMonitorBeneden" //change this to whatever you want to call your device

const uint32_t connectTimeoutMs = 5000;

const char* mqtt_serverName = "192.168.2.231";
const char* mqtt_username = "mqtt_user";
const char* mqtt_password = "mqtT_HoMe";

// InfluxDB server url, e.g. http://192.168.1.48:8086 (don't use localhost, always server name or ip address)
#define INFLUXDB_URL "http://192.168.2.231:8086"
// InfluxDB database name
#define INFLUXDB_DB_NAME "Temperatures"

#endif //CONFIG_H



