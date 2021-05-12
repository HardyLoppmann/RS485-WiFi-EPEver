
#include <ESP8266WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

void influxCloudPostData() {

  Serial.println("init Influx Cloud");
  Serial.print("Influx Cloud Url: ");
  Serial.println(myConfig.influxcloud_url);
  Serial.print("Influx Cloud Org: ");
  Serial.println(myConfig.influxcloud_org);
  Serial.print("Influx Cloud Bucket: ");
  Serial.println(myConfig.influxcloud_bucket);
  Serial.print("Influx Cloud Token: ");
  Serial.println(myConfig.influxcloud_token);

  // InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(myConfig.influxcloud_url, myConfig.influxcloud_org, myConfig.influxcloud_bucket, myConfig.influxcloud_token, InfluxDbCloud2CACert);

  // Setup wifi
  ESP8266WiFiMulti wifiMulti;

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Data point
  Point sensor("wifi_status");

  // Add tags
  sensor.addTag("device", "DEVICE");
  sensor.addTag("SSID", WiFi.SSID());

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();

  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("rssi", WiFi.RSSI());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // If no Wifi signal, try to reconnect it
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Wait 100ms");
  delay(100);
}
