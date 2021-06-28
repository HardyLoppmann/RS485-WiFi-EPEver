
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
#define NTP_SERVER1  "pool.ntp.org"
#define NTP_SERVER2  "time.nis.gov"
#define WRITE_PRECISION WritePrecision::S
#define MAX_BATCH_SIZE 23
#define WRITE_BUFFER_SIZE 100

InfluxDBClient createClient() {
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

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);

  client.setWriteOptions(WriteOptions().writePrecision(WRITE_PRECISION).batchSize(MAX_BATCH_SIZE).bufferSize(WRITE_BUFFER_SIZE));
  return client;  
}

// void writePoint(Point &sensorNetworks, InfluxDBClient *client, String field, float fieldValue) {
//   // // Data point
//   // Point sensor("epever");

//   // // Add tags
//   // sensor.addTag("Panel", "Voltage");
  
//   // // // Clear fields for reusing the point. Tags will remain untouched
//   // // sensor.clearFields();

//   // // Store measured value into point
//   // // Report RSSI of currently connected network
//   // sensor.addField(field, fieldValue);

//   // // Print what are we exactly writing
//   // Serial.print("Writing: ");
//   // Serial.println(sensor.toLineProtocol());

//   // // // If no Wifi signal, try to reconnect it
//   // // if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED)) {
//   // //   Serial.println("Wifi connection lost");
//   // // }

//   // // Write point
//   // if (!client.writePoint(sensor)) {
//   //   Serial.print("InfluxDB write failed: ");
//   //   Serial.println(client.getLastErrorMessage());
//   // }
//   // Serial.println("Wait 100ms");
//   // delay(100);
//     // Point sensorNetworks("wurst");
//     sensorNetworks.addField(field, fieldValue);
//     Serial.print("Writing: ");
//     Serial.println(client->pointToLineProtocol(sensorNetworks));
//     client->writePoint(sensorNetworks);
// }


void influxCloudPostData() {
  InfluxDBClient client = createClient();

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    return;
  }
  
  // Point sensor("epever");
  Point sensorNetworks("wurst");
  
  // Report networks (low priority data) just in case we successfully wrote the previous batch
  if (client.isBufferEmpty()) {

    sensorNetworks.addField( "panel-voltage", live.l.pV /100.f);
    sensorNetworks.addField( "panel-amp", live.l.pI /100.f);
    sensorNetworks.addField( "panel-watt", live.l.pP /100.f);

    sensorNetworks.addField( "battery-voltage", live.l.bV /100.f);
    sensorNetworks.addField( "battery-amp", live.l.bI /100.f);
    sensorNetworks.addField( "battery-watt", live.l.bP /100.f);

    sensorNetworks.addField( "load-voltage", live.l.lV /100.f);
    sensorNetworks.addField( "load-amp", live.l.lI /100.f);
    sensorNetworks.addField( "load-watt", live.l.lP /100.f);

    sensorNetworks.addField( "battery-current", batteryCurrent/100.f);
    sensorNetworks.addField( "battery-soc", batterySOC/1.0f);
    sensorNetworks.addField( "load-state", loadState==1?"1":"0");

    sensorNetworks.addField( "battery-min-voltage", stats.s.bVmin /100.f);
    sensorNetworks.addField( "battery-max-voltage", stats.s.bVmax/100.f);
    sensorNetworks.addField( "panel-min-voltage", stats.s.pVmin/100.f);
    sensorNetworks.addField( "panel-max-voltage", stats.s.pVmax/100.f);
    sensorNetworks.addField( "consumed-day", stats.s.consEnerDay/100.f);
    sensorNetworks.addField( "consumed-all", stats.s.consEnerTotal/100.f);

    sensorNetworks.addField( "gen-day", stats.s.genEnerDay/100.f);
    sensorNetworks.addField( "gen-all", stats.s.genEnerTotal/100.f);
    // sensorNetworks.addField( "battery-voltage-status", batt_volt_status[status_batt.volt]);
    // sensorNetworks.addField( "battery-temp", batt_temp_status[status_batt.temp]);
    // sensorNetworks.addField( "charger-mode", charger_charging_status[charger_mode]);

    Serial.print("Writing: ");
    Serial.println(client.pointToLineProtocol(sensorNetworks));
    client.writePoint(sensorNetworks);

    // // Report all the detected wifi networks
    // int networks = WiFi.scanNetworks();
    // // Set identical time for the whole network scan
    // time_t tnow = time(nullptr);
    // for (int i = 0; i < networks; i++) {
    //   Point sensorNetworks("wifi_networks");
    //   sensorNetworks.addTag("device", DEVICE);
    //   sensorNetworks.addTag("SSID", WiFi.SSID(i));
    //   sensorNetworks.addTag("channel", String(WiFi.channel(i)));
    //   sensorNetworks.addTag("open", String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN));
    //   sensorNetworks.addField("rssi", WiFi.RSSI(i));
    //   sensorNetworks.setTime(tnow);  //set the time

    //   // Print what are we exactly writing
    //   Serial.print("Writing: ");
    //   Serial.println(client.pointToLineProtocol(sensorNetworks));

    //   // Write point into buffer - low priority measures
    //   client.writePoint(sensorNetworks);
    // }
  } else {
    Serial.println("Wifi networks reporting skipped due to communication issues");
  }    

  Serial.println("Flushing data into InfluxDB");
  if (!client.flushBuffer()) {
    Serial.print("InfluxDB flush failed: ");
    Serial.println(client.getLastErrorMessage());
    Serial.print("Full buffer: ");
    Serial.println(client.isBufferFull() ? "Yes" : "No");
  }

  // Serial.println("Wait 100ms");
  delay(1000);
}


