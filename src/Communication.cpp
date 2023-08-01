#include "Communication.h"

#include <WiFi.h>
#include "Log.h"
#include "Tools.h"

// **********************************************************************
// send values to EmonCMS
// **********************************************************************
void sendToEMON(PZEM_004T_Sensor_t &PZEM004data)

{
    if (!WIFIisConnected())
    {
        LOG(LOG_ERROR, "EmonCMS - no WiFi connection!");
        LOG(LOG_INFO, "WiFi IP-Address:%s", WiFi.localIP().toString().c_str());
        return;
    }

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(Credentials::EMONCMS_HOST, Credentials::EMONCMS_PORT, Credentials::EMONCMS_TIMEOUT))
    {
        LOG(LOG_ERROR, "EmonCMS connection failed: %s|%i", Credentials::EMONCMS_HOST, Credentials::EMONCMS_PORT);
        LOG(LOG_INFO, "WiFi IP-Address:%s", WiFi.localIP().toString().c_str());
        return;
    }

    // INFO                                                .
    // THIS IS WORKING:
    // http://192.168.0.194/emoncms/input/post.json?node=Test&apikey=1ce596688fc9a1e40d25d855a1336dad&json={Power:21.4,Current:4.25,Energy:7.7}

    client.print("GET /emoncms/input/post.json"); // make sure there is a [space] between GET and /input
    client.print("?node=");
    client.print(Credentials::EMONCMS_NODE_ID);
    client.print("&apikey=");                   // apikey is mandatory, otherwise only
    client.print(Credentials::EMONCMS_API_KEY); // integer (!) instead of float will be stored !!!
    client.print("&json={");
    client.print("1"); // tagname of Current in old EmonCMS
    client.print(PZEM004data.Current);
    client.print(",2"); // tagname of Power in old EmonCMS
    client.print(PZEM004data.Power);
    client.print(",Energy");
    client.print(PZEM004data.Energy);
    client.print(",Frequency");
    client.print(PZEM004data.Frequency);
    client.print(",Voltage");
    client.print(PZEM004data.Voltage);
    client.print(",PowerFactor");
    client.print(PZEM004data.PowerFactor);
    client.print("}");
    client.println(" HTTP/1.1"); // make sure there is a [space] BEFORE the HTTP
    client.print(F("Host: "));
    client.println(Credentials::EMONCMS_HOST);
    // client.print(F("User-Agent: ESP32-Wifi"));
    client.println(F("Connection: close")); //    Although not technically necessary, I found this helpful
    client.println();

    unsigned long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > EmonCmsWiFiTimeout)
        {
            LOG(LOG_ERROR, ">>> EmonCMS client timeout !");
            client.stop();
            return;
        }
    }

    // DEBUG                                                              .
    // Read all the lines of the reply from server and print them to Serial
    while (client.available())
    {
        // String line = client.readStringUntil('\r');
        // LOG(LOG_INFO, "%s", line);
        char c = client.read();
        Serial.write(c);
    }

    // client.stop(); // Stopping client // INFO               is this needed?!
    // Serial.println();
    LOG(LOG_INFO, "closing EmonCMS connection");
}