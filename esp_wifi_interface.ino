#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "front_end.h"
#include "device.h"

#define LOCALDOMAIN "veronicalamp"
#define FACTORY_RESET_PIN 5

const char json[] = "{\"a\": \"%d\", \"c\": [\"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\"], \"m\": \"%d\", \"n\": \"%s\", \"s\": \"%s\", \"u\": \"%d:%d:%d\", \"w\": \"%d\"}";

// Front End Initial colors
const char white[3]  = {255, 255, 255};
const char yellow[3] = {255, 255, 0};
const char green[3] = {0, 232, 66};
const char blue[3] = {0, 174, 255};
const char red[3] = {255, 0, 66};
const char teal[3] = {0, 128, 128};
const char lilac[3] = {128, 0, 128};
const char orange[3] = {255, 127, 0};

// Network Stack
MDNSResponder mdns;
ESP8266WebServer server(80);
void rootPath();
void statusPath();
void setPath();
void setColorHSLPath();
void setColorRGBPath();

// Physical Access Trigger
unsigned long lastPhysicalAccess = 0;

// Effect stages
int mode = 0, stage = -1, speed = 2;
byte color[3][3];

// LED colors
byte m_R, m_G, m_B;

// Helper functions
int hex2bin( const char * );
int urldecode( const char * );

// ESP Base Device LIbrary
Base::Device device;

void setup() {
    // Boot the module
    if ( device.boot() ) // Factory defaults restored during boot
    {
        EEPROM.begin(48);
        factoryEEPROM();
        device.reboot();
    }

    // Establish connectivity
    device.initWiFi();

    EEPROM.begin(48);

    // Initialise pins
    pinMode( FACTORY_RESET_PIN, INPUT );

    // Attach an interrupt to the Factory Reset Pin for Access Control
    attachInterrupt(FACTORY_RESET_PIN, physicalAccess, RISING);

    // Initialise MDNS Responder
    if ( mdns.begin ( LOCALDOMAIN ) ) {
        Serial.println ("MDNS responder started");
    }

    // Initialise the Web Server
    server.on( "/", rootPath );
    server.on( "/set", HTTP_POST, setPath );
    server.on( "/status", HTTP_GET, statusPath );
    server.on( "/color/hls", HTTP_POST, setColorHSLPath );
    server.on( "/color/rgb", HTTP_POST, setColorRGBPath );
    server.onNotFound(rootPath);
    server.begin();
    Serial.println("Initialised HTTP server");

    Serial.println("done");
}

void loop() {
    // Handle services
    mdns.update();
    server.handleClient();
}

void factoryEEPROM() {
    EEPROM.write( 0, 0 ); // Current mode = OFF

    EEPROM.put( 1 + 0 * 3, white ); // White
    EEPROM.put( 1 + 1 * 3, yellow ); // yellow
    EEPROM.put( 1 + 2 * 3, green ); // green
    EEPROM.put( 1 + 3 * 3, blue ); // blue
    EEPROM.put( 1 + 4 * 3, red ); // red
    EEPROM.put( 1 + 5 * 3, teal ); // teal
    EEPROM.put( 1 + 6 * 3, lilac ); // lilac
    EEPROM.put( 1 + 7 * 3, orange ); // orange

    EEPROM.write( 25, 0 ); // Currently active color is 0

    EEPROM.write( 26, 0 ); // Current speed setting is 0

    EEPROM.commit();
}

void rootPath() {
    server.send( 200, "text/html", html );
}

void statusPath() {
    char temp[sizeof(json) + 8 * 6 + 129];

    // Currently active color
    int a = EEPROM.read(25);

    // Stores the colors loaded from EEPROM
    byte c[8][3];

    // Get colors from EEPROM
    for(int i = 0; i < 8; ++i) {
        EEPROM.get( 1 + i * 3, c[i] );
    }

    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;

    snprintf(temp, sizeof(json) + 8 * 6 + 129, json, a, c[0][0], c[0][1], c[0][2], c[1][0], c[1][1], c[1][2], c[2][0], c[2][1], c[2][2], c[3][0], c[3][1], c[3][2], c[4][0], c[4][1], c[4][2], c[5][0], c[5][1], c[5][2], c[6][0], c[6][1], c[6][2], c[7][0], c[7][1], c[7][2], mode, device.name().c_str(), device.ssid().c_str(), hr, min, sec % 60, (int)(device.mode() == WIFI_STA) + 1);
    server.send( 200, "text/html", temp );
}

void setPath() {
    String password, temp, error="";
    char temp2[64];
    bool storeConfig = false;
    for(int i = 0; i < server.args(); ++i) {
        if ( server.argName(i) == "p") { // Network password
            temp = server.arg(i);
            temp.remove(63);
            urldecode(temp2, temp.c_str());
            password = temp2;
            if( password.length() == 0 ) break;
            error = "Passwords don't match!";
        } else if ( server.argName(i) == "c") { // Network password confirmation
            temp = server.arg(i);
            temp.remove(63);
            urldecode(temp2, temp.c_str());
            if( password == temp2 ) {
                error = "";
                device.setPassword(password);
                storeConfig = true;
            } else break;
        }
    }
    if( error.length() == 0 ) {
        for(int i = 0; i < server.args(); ++i) {
            if ( server.argName(i) == "a" ) { // Current color
                EEPROM.write( 25, server.arg(i).toInt() % 255 );
                char color[6];
                byte preset[3];
                EEPROM.get(1 + server.arg(i).toInt() % 8 * 3, preset);
//                snprintf(color, 6, "%02x%02x%02x", preset[0], preset[1], preset[2]);
//                Serial.print('b');
//                Serial.println(color);                
                Serial.print("c ");
                Serial.print(int(preset[0]));
                Serial.print(" ");
                Serial.print(int(preset[1]));
                Serial.print(" ");
                Serial.println(int(preset[2]));
            } else if ( server.argName(i) == "m") { // Current mode
                int wmode = server.arg(i).toInt() % 255;
                 // EEPROM.write( 0, wmode );
                mode = wmode;
                // Prepare the mode for initialization
                stage = -1;
                switch (mode) {
                  case 0: 
                    Serial.println("effect stop");
                    break;
                  case 1: 
                    Serial.println("effect HSV fade");
                    break;
                  case 2: 
                    Serial.println("effect duration up");
                    break;
                  case 3: 
                    Serial.println("effect brightness up");
                    break;
                  case 4: 
                    Serial.println("shutdown");
                    break;
                  case 5: 
                    Serial.println("effect random fade");
                    break;
                  case 6: 
                    Serial.println("effect duration down");
                    break;
                  case 7: 
                    Serial.println("effect brightness down");
                    break;
                  default:
                    Serial.println(mode);
                    break;
                }
            } else if ( server.argName(i) == "w") { // Current WiFi mode
                int wmode = (unsigned int)server.arg(i).toInt() % 255;
                if ( wmode == 1 ) {
                    device.setMode( WIFI_AP );
                } else if ( wmode == 2 ) {
                    device.setMode( WIFI_STA );
                } else if ( wmode == 3 ) {
                    device.setMode( WIFI_AP );
                    device.setPassword("");
                } else if ( wmode == 4 ) {
                    device.setMode( WIFI_STA );
                    device.setPassword("");
                }
                storeConfig = true;
            } else if ( server.argName(i) == "n") { // Device Human Readable name
                temp = server.arg(i);
                temp.remove(63);
                urldecode(temp2, temp.c_str());
                device.setName(temp2);
                storeConfig = true;
            } else if ( server.argName(i) == "s") { // Network SSID
                temp = server.arg(i);
                temp.remove(63);
                urldecode(temp2, temp.c_str());
                device.setSSID(temp2);
                storeConfig = true;
            } else if ( server.argName(i) == "d") { // Domain name
                temp = server.arg(i);
                temp.remove(63);
                urldecode(temp2, temp.c_str());
                // Ignore
            } else { // Color input
                for(int j = 0; j < 8; ++j) {
                    if( server.argName(i) == (String("c") + j) ) {

                      // Convert hex color to byte array
                      temp = server.arg(i);
                      temp.remove(6);
                      Serial.print('b');
                      Serial.println(temp);
                      const char *src = temp.c_str();
                      byte color[3];
                      for(int k = 0; k < 3; ++k) {
                          color[k] = hex2bin(src);
                          src += 2;
                      }

                      EEPROM.put( 1 + j * 3, color );
                    }
                }
            }
        }
    }
    if( error.length() == 0 && storeConfig &&
        lastPhysicalAccess != 0 &&
        millis() - lastPhysicalAccess <= 30 * 1000 )
        error = "You need to press and hold the Factory Reset Button for 2 seconds before you can change system settings!";
    if( error.length() > 0) {
        server.sendHeader("Refresh", "5;url=/");
        server.send(400, "text/plain", error);
    } else {
        if( storeConfig ) {
            device.store();
            device.initWiFi();
        }
        EEPROM.commit();
        server.sendHeader("Redirect", "/");
        server.send(200);
    }
}

byte R() {
    return m_R / 4;
}

byte G() {
    return m_G / 4;
}

byte B() {
    return m_B / 4;
}

byte R(byte r) {
    Serial.print("R ");
    Serial.println(r);
    return (m_R = r);
}

byte G(byte g) {
    Serial.print("G ");
    Serial.println(g);
    return (m_G = g);
}

byte B(byte b) {
    Serial.print("B ");
    Serial.println(b);
    return (m_B = b);
}

void setColorHSLPath() {

}

void setColorRGBPath() {

}

void physicalAccess() {
    lastPhysicalAccess = millis();
    Serial.println("Full Access Authorised");
    while( digitalRead(FACTORY_RESET_PIN) ) {
        if( millis() - lastPhysicalAccess >= 9500 ) {
            device.factoryReset();
            factoryEEPROM();
            device.reboot();
        }
    }
}

int hex2bin( const char *s )
{
  int ret = 0;
  int i;
  for( i=0; i<2; i++ )
  {
    char c = *s++;
    int n=0;
    if( '0'<=c && c<='9' )
      n = c-'0';
    else if( 'a'<=c && c<='f' )
      n = 10 + c-'a';
    else if( 'A'<=c && c<='F' )
      n = 10 + c-'A';
    ret = n + ret*16;
  }
  return ret;
}

void urldecode(char *dst, const char *src)
{
    char a, b,c;
    if (dst==NULL) return;
    while (*src) {
        if ((*src == '%') &&
           ((a = src[1]) && (b = src[2])) &&
           (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        }
        else {
            c = *src++;
            if(c=='+')c=' ';
            *dst++ = c;
        }
    }
    *dst++ = '\0';
}
