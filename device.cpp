#include <FS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "factory_config.h"
#include "device.h"

#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#ifndef FACTORY_RESET_PIN
    #define FACTORY_RESET_PIN 9
#endif

#ifndef FACTORY_VERSION
    #define FACTORY_VERSION "ESP05"
#endif

#ifndef FACTORY_MODE
    #define FACTORY_MODE "AP"
#endif

#ifndef FACTORY_NAME
    #define FACTORY_NAME "ESP Web Device"
#endif

#ifndef FACTORY_SSID
    #define FACTORY_SSID "ESP Web Device"
#endif

#ifndef FACTORY_PASS
    #define FACTORY_PASS "ESP Web Device"
#endif

#ifndef DEVICE_FILE
    #define DEVICE_FILE "/device.txt"
#endif

#ifndef ADC_NOVCC
    ADC_MODE(ADC_VCC);
#endif

namespace Base {

Device::Device()
{
}

bool Device::boot()
{
    Serial.begin( 57600 );
    Serial.println();
    Serial.println( "Starting up" );
#ifndef ADC_NOVCC
    Serial.print( "Input Voltage: " );
    Serial.println( ESP.getVcc() );
#endif

//    // Intialise SPIFFS
//    bool factoryResetFlag = false;
//    if ( ! SPIFFS.begin() ) {
//        factoryResetFlag = true;
//    } else {
//        load();
//        if ( m_version != FACTORY_VERSION ) {
//            Serial.print( "Version mismatch. Expected VERSION: " );
//            Serial.println( FACTORY_VERSION );
//            factoryResetFlag = true;
//        }
//    }

    Serial.println("done");
    

    return false;
}

void Device::initWiFi()
{   
    WiFiManager wifiManager;
    wifiManager.autoConnect(FACTORY_SSID);
    Serial.println("connected...yeey :)");
}

void Device::load() {
    Serial.println( "Loading device configuration" );

    char data[64];

    File f = SPIFFS.open( DEVICE_FILE, "r" );
    if ( !f ) {
        Serial.println( "CRITICAL ERROR: Cannot open device configuration file" );
        return;
    }

    // Parse config file
    while( f.available() ) {
        String var, value;
        char chr = f.read();

        // Parsing lines
        while( f.available() && ( chr >= 'A' && chr <= 'Z' || chr == '_' || chr >= '0' && chr <= '9' ) ) {
            var += chr;
            chr = f.read();
        }
        if( chr ==  '=' && f.available() ) {
            chr = f.read();
            while( f.available() && chr != '\n' && chr != '\r' ) {
                value += chr;
                chr = f.read();
            }

            // Store the setting
            if( var == "VERSION" ) {
                m_version = value;
            } else if( var == "MODE" ) {
                if( value == "OFF" ) m_mode = WIFI_OFF;
                else if( value == "AP" ) m_mode = WIFI_AP;
                else if( value == "STA" ) m_mode = WIFI_STA;
                else if( value == "AP_STA" ) m_mode = WIFI_AP_STA;
            } else if( var == "NAME" ) {
                m_name = value;
            } else if( var == "SSID" ) {
                m_ssid = value;
            } else if( var == "PASS" ) {
                m_password = value;
            }
        }
    }

    f.close();
}



void Device::reboot()
{
    Serial.println("Rebooting device");

    // Terminate the Serial connection
    Serial.flush();
    Serial.swap();

    delay(1000); // Make sure all actions are executed

    ESP.restart();
    delay(1000); // Hold until chip restarts
}

IPAddress Device::ip()
{
    return m_ip;
}

String Device::name()
{
    return m_name;
}

void Device::setName(String name)
{
    m_name = name;
}

WiFiMode Device::mode()
{
    return m_mode;
}

void Device::setMode(WiFiMode mode)
{
    m_mode = mode;
}

String Device::ssid()
{
    return m_ssid;
}

void Device::setSSID(String ssid)
{
    m_ssid = ssid;
}

String Device::password()
{
    return m_password;
}

void Device::setPassword(String password)
{
    m_password = password;
}

} // Base::
