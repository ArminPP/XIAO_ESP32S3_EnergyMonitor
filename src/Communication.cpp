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
        return;
    }

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(EmonHost, httpPort))
    {
        LOG(LOG_ERROR, "EmonCMS connection failed: %s|%i", EmonHost, httpPort);
        return;
    }

    // https://emoncms.org/Modules/site/api.php
    // GET	input/post?node=emontx&fulljson={"power1":100,"power2":200,"power3":300}&apikey=APIKEY_WRITE
    /*
        // We now create a URI for the request
        String url = "GET /emoncms/input/post.json?apikey=" + emoncms_apikey + "&node=" + node + "&json={2:" + temperature + ",1:" + vcc + "";
        client.println(url);
        client.print("Host: ");
        client.println(emoncms_host);
        client.println("Connection: close");
        client.println();
        client.stop(); //Stopping client
        Serial.println("Post to emoncms : success");
    */



// INFO                                                .
THIS IS WORKING:
http://192.168.0.194/emoncms/input/post.json?node=Test&apikey=1ce596688fc9a1e40d25d855a1336dad&json={Power:21.4,Current:4.25,Energy:7.7}

BUT Current = 1   Power = 2    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


apikey is mandatory, otherwise only int (!) instead of float will be stored !!!


better split messages in seperat "print" instead of char[] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// {
//     client.print("GET /input/post.json?json={power:200");  // make sure there is a [space] between GET and /input
//     client.print(",realPower:");
//     client.print(realPower);
//     client.print(",apparentPower:");
//     client.print(apparentPower);
//     client.print(",PowerFActor:");
//     client.print(PowerFActor);  
//     client.print(",supplyVoltage:");
//     client.print(supplyVoltage);
//     client.print(",Irms:");
//     client.print(Irms);  
//     client.print("}&apikey="
//     client.print(MYAPIKEY)         //assuming MYAPIKEY is a char or string
//     client.println(" HTTP/1.1");   //make sure there is a [space] BEFORE the HTTP
//     client.println(F("Host: emoncms.org"));
//     client.print(F("User-Agent: Arduino-ethernet"));
//     client.println(F("Connection: close"));     //    Although not technically necessary, I found this helpful
//     client.println();
// }
 



    char msg[256] = {'\0'};
    snprintf(msg,
             sizeof(msg),
             "GET /emoncms/input/post.json?node=%s&json={\"Voltage:\"%f, \"Irms:\"%f, \"Power_245:\"%f, \"Energy:\"%f, \"Frequency:\"%f,  "
             "\"PowerFactor:\"%f}&apikey=%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             EmonNodeID,
             PZEM004data.Voltage,
             PZEM004data.Current,
             PZEM004data.Power,
             PZEM004data.Energy,
             PZEM004data.Frequency,
             PZEM004data.PowerFactor,
             EmonApiKey,
             EmonHost);

    // DEBUG                                                                                 .
    // DEBUG   To much information, floods the log file!                                     .
    //   LOG(LOG_INFO, "send to Emon:%s", msg);

    // This will send the request to the server
    client.print(msg);

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

    // Serial.println();
    LOG(LOG_INFO, "closing EmonCMS connection");
}