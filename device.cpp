#include <FS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include "factory_config.h"
#include "device.h"

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
    Serial.begin( 115200 );
    Serial.println();
    Serial.println( "Starting up" );
#ifndef ADC_NOVCC
    Serial.print( "Input Voltage: " );
    Serial.println( ESP.getVcc() );
#endif

    // Intialise SPIFFS
    bool factoryResetFlag = false;
    if ( ! SPIFFS.begin() ) {
        factoryReset();
        factoryResetFlag = true;
    } else {
        load();
        if ( m_version != FACTORY_VERSION ) {
            Serial.print( "Version mismatch. Expected VERSION: " );
            Serial.println( FACTORY_VERSION );
            factoryReset();
            factoryResetFlag = true;
        }
    }

    Serial.println("done");

    return factoryResetFlag;
}

void Device::initWiFi()
{
    WiFi.disconnect();

    wiFiStartUp:
        if ( m_mode == WIFI_AP ) {
            // WiFi.mode( WIFI_AP );
            Serial.println( "Configuring Access Point" );
            if( m_password[0] == '\0' ) {
                WiFi.softAP( m_ssid.c_str() );
            } else {
                WiFi.softAP( m_ssid.c_str(), m_password.c_str() );
            }
        } else if( m_mode == WIFI_STA ) {
            // WiFi.mode( WIFI_STA );
            Serial.println( "Connecting to WiFi Network" );
            if( m_password[0] == '\0' ) {
                WiFi.begin( m_ssid.c_str() );
            } else {
                WiFi.begin( m_ssid.c_str(), m_password.c_str() );
            }
        }

        // Wait for connection
        while ( WiFi.status() != WL_CONNECTED ) {
            // If after 20 seconds the module has not been able to connect to a WiFi
            if( m_mode == WIFI_STA && millis() >= 30000 ) {
                // Fallback to AP mode with the current settings
                Serial.println("Unable to connect to network.");
                m_ssid = m_name;
                WiFi.disconnect();
                m_mode = WIFI_AP;
                goto wiFiStartUp;
            } else if( millis() >= 60000 ) { // Stop waiting even for the fallback mode
                Serial.println("Emergency escape. Attempting to continue!");
                break;
            }

            delay ( 500 );
            Serial.print('.');
        }

        // Report IP Address
        if ( m_mode == WIFI_AP ) {
            m_ip = WiFi.softAPIP();
        } else if( m_mode == WIFI_STA ) {
            m_ip = WiFi.localIP();
        }

        Serial.print("WiFi SSID: ");
        Serial.println(m_ssid);
        Serial.print("IP address: ");
        Serial.println(m_ip);
}

void Device::load() {
    Serial.println( "Loading device configuration" );

    char data[64];

    File f = SPIFFS.open( DEVICE_FILE, "r" );
    if ( !f ) {
        Serial.println( "CRITICAL ERROR: Cannot open device configuration file" );
        Serial.println( "Recovery procedure initiated" );
        factoryReset();
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

    printConfig();

    f.close();
}

void Device::factoryReset()
{
    Serial.println( "Factory reset" );

    m_version = FACTORY_VERSION;

    if( FACTORY_MODE == "OFF" ) m_mode = WIFI_OFF;
    else if( FACTORY_MODE == "AP" ) m_mode = WIFI_AP;
    else if( FACTORY_MODE == "STA" ) m_mode = WIFI_STA;
    else if( FACTORY_MODE == "AP_STA" ) m_mode = WIFI_AP_STA;

    m_name = FACTORY_NAME;
    m_ssid = FACTORY_SSID;
    m_password = FACTORY_PASS;

    store();

    printConfig();
}

void Device::store()
{
    Serial.println( "Storing device settings" );
    SPIFFS.remove( DEVICE_FILE );
    File f = SPIFFS.open( DEVICE_FILE, "w" );

    f.print( "VERSION=" );
    f.println( m_version );

    f.print( "MODE=" );
    if( m_mode == WIFI_AP ) f.println( "AP" );
    else if( m_mode == WIFI_OFF ) f.println( "OFF" );
    else if( m_mode == WIFI_STA ) f.println( "STA" );
    else if( m_mode == WIFI_AP_STA ) f.println( "AP_STA" );

    f.print( "NAME=" );
    f.println( m_name );

    f.print( "SSID=" );
    f.println(m_ssid);

    f.print( "PASS=" );
    f.println( m_password );

    f.close();
}

void Device::printConfig()
{
    Serial.print( "VERSION: " );
    Serial.println( m_version );

    Serial.print( "MODE: " );
    if( m_mode == WIFI_AP ) Serial.println( "AP" );
    else if( m_mode == WIFI_OFF ) Serial.println( "OFF" );
    else if( m_mode == WIFI_STA ) Serial.println( "STA" );
    else if( m_mode == WIFI_AP_STA ) Serial.println( "AP_STA" );

    Serial.print( "NAME: " );
    Serial.println( m_name );

    Serial.print( "SSID: " );
    Serial.println( m_ssid );

    Serial.print( "PASSWORD: " );
    for(int i = 0; i < m_password.length(); ++i) {
        Serial.print( "*" );
    }
    Serial.println();
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
