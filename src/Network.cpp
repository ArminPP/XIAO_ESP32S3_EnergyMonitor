#include "Network.h"
#include <WiFi.h>

#include "Tools.h"
#include "Credentials.h"
#include "Log.h"
#include "Globals.h"

WiFiServer APserver(80); // special server only for access point @ Ethernet! (REWORK after tests are over!!!)
WiFiServer WIFIserver(80);

// INFO       DEBUG only -> move it to Webserver.h/cpp!
void doWebserver(Client &client, PZEM_004T_Sensor_t &PZEM004data)
{
    // WiFiClient client = server.available();   // listen for incoming clients
    if (client)
    {                                      // if you get a client,
        LOG(LOG_DEBUG, "new WiFi client"); // print a message out the serial port
        String currentLine = "";           // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();
                        // the content of the HTTP response follows the header:
                        client.print("<center><h1 style=\"font-size:4vw;\"><br>Welcome to EnergyMonitor V0.2 BETA</h1></center>");
                        client.println("<br>");
                        client.println("<br>");
                        client.printf("<center><h2 style=\"font-size:2vw;\">Power: %5.2f&deg;C</h2></center>", PZEM004data.Power);
                        client.printf("<center><h2 style=\"font-size:2vw;\">Current: %5.2f&deg;C</h2></center>", PZEM004data.Current);
                        client.printf("<center><h2 style=\"font-size:2vw;\">Energy: %5.2f&deg;C</h2></center>", PZEM004data.Energy);
                        client.printf("<center><h2 style=\"font-size:2vw;\">Frequency: %5.2f&deg;C</h2></center>", PZEM004data.Frequency);
                        client.printf("<center><h2 style=\"font-size:2vw;\">Voltage: %5.2f&deg;C</h2></center>", PZEM004data.Voltage);
                        client.printf("<center><h2 style=\"font-size:2vw;\">PowerFactor: %5.2f&deg;C</h2></center>", PZEM004data.PowerFactor);
                        client.printf("<center><h2 style=\"font-size:2vw;\">rssi: %d</h2></center>", WiFi.RSSI());

                        uint16_t dd = 0;
                        byte hh     = 0;
                        byte ss     = 0;
                        byte mm     = 0;
                        uint16_t ms = 0;
                        ElapsedRuntime(dd, hh, mm, ss, ms);

                        client.printf("<center><h2 style=\"font-size:2vw;\">Runtime: %05d|%02i:%02i:%02i:%03i </h2></center>", dd, hh, mm, ss, ms);

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // close the connection:
        client.stop();
        LOG(LOG_DEBUG, "WiFI client disconnected");
    }
}

// Callbacks for WiFi
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
// **********************************************************************
// **********************************************************************
{
    // INFO    AFTER UPDATING ESP-ARDUINO TO 4.1.0 SOME WiFi Events are gone?!
    LOG(LOG_WARNING, "Disconnected from WiFi, reason: %i - trying to reconnect...", info.wifi_sta_connected); // info.disconnected.reason
    WiFi.reconnect();
}

// Callbacks for WiFi
void WiFiConnected(WiFiEvent_t event, WiFiEventInfo_t info)
// **********************************************************************
// **********************************************************************
{
    // INFO    AFTER UPDATING ESP-ARDUINO TO 4.1.0 SOME WiFi Events are gone?!
    // LOG(LOG_INFO, "Connected to WiFi. SSID Length:%i  SSID:", info.connected.ssid_len)
    LOG(LOG_INFO, "Connected to WiFi. SSID Length:%i  SSID:", info.wifi_sta_connected.ssid_len);
    // INFO    AFTER UPDATING ESP-ARDUINO TO 4.1.0 SOME WiFi Events are gone?!
    // for (int i = 0; i < info.connected.ssid_len; i++)
    for (int i = 0; i < info.wifi_sta_connected.ssid_len; i++)
    {
        LOG_W((char)info.wifi_sta_connected.ssid[i]);
    }

    LOG(LOG_INFO, "BSSID: ");
    for (int i = 0; i < 6; i++)
    {
        // INFO    AFTER UPDATING ESP-ARDUINO TO 4.1.0 SOME WiFi Events are gone?!
        // LOG(LOG_INFO, "%02X", info.connected.bssid[i]);
        LOG(LOG_INFO, "%02X", info.wifi_sta_connected.bssid[i]);

        if (i < 5)
        {
            LOG_W((int)(':')); // won't work with log ?!
        }
    }
    // INFO    AFTER UPDATING ESP-ARDUINO TO 4.1.0 SOME WiFi Events are gone?!
    LOG(LOG_INFO, "Channel:%i Auth mode:%i", info.wifi_sta_connected.channel, info.wifi_sta_connected.authmode);
    // LOG(LOG_INFO, "Channel:%i Auth mode:%i", info.connected.channel, info.connected.authmode);

    // to early -> timeout at NTP fetch, delay needed?
    // getNtpTime(WiFiudp);
}

// There are two methods, to establish a AP
// 1) use credentials from settings file
// 2) use default credentials from settings file
// 2a add unique chars to default hardcoded SSDI
bool openAccessPoint(bool useDefaultValues)
// **********************************************************************
// **********************************************************************
{
    if (Credentials::AP_ENABLED || useDefaultValues) // NEW  if no settings file is detected, open AP!
    {
        LOG(LOG_DEBUG, "-------------------> AP");
        WiFi.begin(); // start WiFi with no credentials
        delay(500);

        APserver.begin(); // INFO    FOR AWOT <-------------------------

        // setting hostname in ESP32 always before setting the mode!
        // https://github.com/matthias-bs/BresserWeatherSensorReceiver/issues/19
        WiFi.setHostname(Credentials::ESP_HOSTNAME); // TODO doesn't work?

        WiFi.mode(WIFI_MODE_AP);

        char AP_SSID_ext[32];
        strcpy(AP_SSID_ext, Credentials::AP_SSID);

        if (useDefaultValues)
        {
            // if TRUE use hardcoded AP credentials from Credentials.cpp!
            // only add a unique Hostname/SSID

            // generate a random ssid (last 4 Bytes of MAC)
            // if AP is forced by user button
            strcat(AP_SSID_ext, GetMACquadruple());
        }

        WiFi.softAP(AP_SSID_ext, Credentials::AP_PASSWORD);

        delay(100);

        IPAddress AP_IP; // convert char[] to IPaddress
        AP_IP.fromString(Credentials::AP_STATIC_IP);
        IPAddress AP_SUB;
        AP_SUB.fromString(Credentials::AP_STATIC_SN);

        WiFi.softAPConfig(AP_IP, AP_IP, AP_SUB); // Gateway = IP

        LOG(LOG_DEBUG, "Access Point SSID from File: %s", Credentials::AP_SSID);
        LOG(LOG_DEBUG, "Access Point PW from File: %s", Credentials::AP_PASSWORD); // DEBUG dangerous due to security issues ?!

        LOG(LOG_INFO, "Access Point SSID: %s", AP_SSID_ext);
        LOG(LOG_INFO, "AP IP-Address:     %s", WiFi.softAPIP().toString().c_str());
        LOG(LOG_INFO, "AP MAC:            %s", WiFi.softAPmacAddress().c_str());

        if (Credentials::ENABLE_LOG_DEBUG)
        {
            WiFi.printDiag(Serial);
        }

        return true;
    }
    else
    {
        LOG(LOG_INFO, "Access Point mode not enabled");
        return false;
    }
}

bool setupWIFI()
// **********************************************************************
// **********************************************************************
{
    /*
    int16_t n = WiFi.scanNetworks();                      // scan for available Networks ...
    LOG(LOG_INFO, "Scan available AP's: %i", n);          //
    LOG(LOG_INFO, "   network(s) found");                 //
    for (int i = 0; i < n; i++)                           //
    {                                                     //
        LOG(LOG_INFO, "     %s", (WiFi.SSID(i).c_str())); //
    }                                                     //
    if (n == 0)                                           //
    {                                                     //
        return false;                                     // if no AP's available exit
    }                                                     // scanning for AP's  <----<<
    */

    // Beep(); // INFO function doesn't work with core2!

    // setting hostname in ESP32 always before setting the mode!
    // https://github.com/matthias-bs/BresserWeatherSensorReceiver/issues/19
    WiFi.setHostname(Credentials::ESP_HOSTNAME); // TODO doesn't work?

    WiFi.mode(WIFI_MODE_STA); // WIFI_MODE_APSTA    // INFO!

    WiFi.onEvent(WiFiDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(WiFiConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    // WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);  // TODO    to much messages?!

    if (Credentials::WIFI_DHCP) // WIFI_DHCP is selected
    {
        LOG(LOG_INFO, "WiFi DHCP is selected.");
    }
    else
    {
        IPAddress ip; // convert char[] to IPaddress
        ip.fromString(Credentials::WIFI_STATIC_IP);
        LOG(LOG_INFO, "static IP from Settings.ini file:    %s", ip.toString().c_str());

        IPAddress dns;
        dns.fromString(Credentials::WIFI_STATIC_DNS);
        LOG(LOG_INFO, "DNS from from Settings.ini file:     %s", dns.toString().c_str());

        IPAddress gw;
        gw.fromString(Credentials::WIFI_STATIC_GW);
        LOG(LOG_INFO, "gateway from from Settings.ini file: %s", gw.toString().c_str());

        IPAddress sn;
        sn.fromString(Credentials::WIFI_STATIC_SN);
        LOG(LOG_INFO, "subnet mask from Settings.ini file:  %s", sn.toString().c_str());

        if (ip && gw && sn && dns)
        {
            WiFi.config(ip, gw, sn, dns); //
            LOG(LOG_DEBUG, "5: WiFi.config(ip, gw, sn, dns)");
        }
        else if (ip && gw && sn)
        {
            WiFi.config(ip, gw, sn); // this is the working one!
            LOG(LOG_DEBUG, "4: WiFi.config(ip, gw, sn)");
        }
        else
        {
            LOG(LOG_ERROR, "No valid static IP settings!");
            return false;
        }
    }

    if (WiFi.status() == WL_NO_SHIELD)
    {
        LOG(LOG_ERROR, "WiFi shield was not found.");
        return false;
    }

    WiFi.begin(Credentials::WIFI_SSID, Credentials::WIFI_PASSWORD); // , Credentials::ESP_NOW_CHANNEL
    LOG(LOG_DEBUG, "WiFi SSID:  %s", Credentials::WIFI_SSID);
    LOG(LOG_DEBUG, "WiFi PASSW: %s", Credentials::WIFI_PASSWORD);

    if (Credentials::ENABLE_LOG_DEBUG)
    {
        Serial.println(F("----------------> after begin"));
        WiFi.printDiag(Serial);
    }

    LOG(LOG_INFO, "connecting to WiFi");

    // WiFi timeout
    int16_t timeout = 20; // 20 * 0.5 sec to connect to a WiFi, otherwise create an AP

    while (WiFi.status() != WL_CONNECTED && timeout > 0) // better timeout handling!
    {
        delay(500);
        LOG_W((int)('.'));
        timeout--;
    }
    LOG_W((int)('\n')); // add a new line
    if (timeout == 0)
    {
        LOG(LOG_DEBUG, "WiFI timeout!"); // detect if WiFi is connected or run into timeout...
    }

    if (WIFIisConnected())
    {
        LOG(LOG_INFO, "WiFi IP-Address:%s", WiFi.localIP().toString().c_str());
        LOG(LOG_INFO, "WiFi rssid:     %i", WiFi.RSSI());
        LOG(LOG_INFO, "WiFi MAC:       %s", WiFi.macAddress().c_str());
        LOG(LOG_INFO, "WiFi Channel:   %i", WiFi.channel());

        WIFIserver.begin(); // INFO    FOR AWOT <-------------------------

        return true; // exit setup, because WiFi is connected
    }
    else
    {
        LOG(LOG_ERROR, "WiFI timeout!");
        return false;
    }
}

// If AP and WiFi are enabled, then WiFI is prior to AP Mode
bool setupNetwork()
// **********************************************************************
// **********************************************************************
{
    if (Credentials::WIFI_ENABLED)
    {
        LOG(LOG_DEBUG, "connect to WiFi ...");
        delay(500);
        if (setupWIFI())
        {
            return true; // if successful, then exit
        }
    }
    else
    {
        LOG(LOG_INFO, "WiFi mode not enabled");
    }

    if (Credentials::AP_ENABLED) // open an WiFi AP
    {
        LOG(LOG_DEBUG, "establish access point...");
        delay(500);
        if (openAccessPoint(false))
        {
            return true; // if successful, then exit
        }
    }
    else
    {
        LOG(LOG_INFO, "Access point mode not enabled");
    }
    return false; // if this will be reached, then no Network connection was available!
}

// https://github.com/lasselukkari/aWOT/issues/126#issuecomment-807431074
void doNetwork(PZEM_004T_Sensor_t &PZEM004data)
// **********************************************************************
// **********************************************************************
{
    // beware of the correct order!
    // 1st AP, 2nd WiFI
    if (WiFi.getMode() == WIFI_MODE_AP) // mode AP
    {
        WiFiClient APclient = APserver.available();
        doWebserver(APclient, PZEM004data);
    }
    else if (((WiFi.getMode() == WIFI_MODE_APSTA) || (WiFi.getMode() == WIFI_MODE_STA)) && (WiFi.status() == WL_CONNECTED))
    {
        WiFiClient WIFIclient = WIFIserver.available();
        doWebserver(WIFIclient, PZEM004data);
    }
}
