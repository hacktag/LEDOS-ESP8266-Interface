#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Factory settings
void factoryEEPROM();
const char factoryUUID[6] = "Wd0pw"; // UUID | Used as a Version identificator
const char factoryName[] = "Veronica's Lamp"; // Inital Human Readable Name
const char factorySSID[] = "UTPL"; // Factory SSID for AP mode
const char factoryPass[] = "GreenMoonBrick-1"; // Factory Pass for AP mode
const char factoryDomain[] = "veronicalamp"; // Factory Local Domain Name

// Front End
const char html[] = "<!DOCTYPE html><html><head><meta charset=utf-8><meta content=\"width=device-width,initial-scale=1\" name=viewport><title></title><style>div,nav *{position:relative}i,nav .a:after,p{position:absolute}input,nav *{font-size:15px;background:#fff}nav *,p{text-align:center}body{background:-webkit-linear-gradient(90deg,#485563 10%,#29323c 90%);background:-moz-linear-gradient(90deg,#485563 10%,#29323c 90%);background:-ms-linear-gradient(90deg,#485563 10%,#29323c 90%);background:-o-linear-gradient(90deg,#485563 10%,#29323c 90%);background:linear-gradient(90deg,#485563 10%,#29323c 90%);font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;font-size:20px;color:#fff}div{width:100%;margin:0 auto}h1{font-size:32px;margin:10px 0}h2{font-size:24px;margin:5px 0}nav{overflow:hidden;padding:3px;margin:0}nav *{float:left;width:23.5%;height:15px;padding:10px 0;margin:0 0 10px 2%;border-radius:3px;cursor:pointer;color:#000}#cp,input,p{width:100%}nav :nth-child(4n+1){margin-left:0}nav .a:after{content:'';display:block;top:-2px;bottom:-2px;left:-2px;right:-2px;border-radius:7px;border:2px solid #7BC3FF}i,p{left:0}#cp{clear:both;display:block;height:200px;cursor:crosshair}.hdn,i{display:none}input{display:block;position:relative;margin:0 0 10px;padding:10px;border:none;border-radius:3px;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;-o-box-sizing:border-box;-ms-box-sizing:border-box;box-sizing:border-box;outline:0}input:focus{padding:8px;border-radius:3px;border:2px solid #7BC3FF}#set{cursor:pointer}i{top:0;right:0;bottom:0;background:rgba(0,0,0,.7)}p{top:50%;margin-top:-12px;color:#fff}@media only screen and (min-width:641px){div{width:640px}h1{font-size:48px;margin:30px 0}h2{font-size:36px;margin:10px 0}nav *{height:15px;padding:20px 0}nav .a:after{top:-3px;right:-3px;bottom:-3px;left:-3px;border-width:3px}input{padding:20px 10px;margin:0 0 20px}input:focus{padding:17px 7px;border-width:3px}}</style><body><div><h1></h1>"
"<h2>Colors</h2><nav id=c><a id=c1 class=a></a> <a id=c2></a> <a id=c3></a> <a id=c4></a> <a id=c5></a> <a id=c6></a> <a id=c7></a> <a id=c8></a></nav><canvas id=cp></canvas><h2>Mode</h2><nav><a id=m1>Static Color</a> <a id=m2>3 Color Fade</a> <a id=m3>Data Stream</a> <a id=m4>Full Fade</a></nav>"
"<h2 id=set>Settings</h2><form class=hdn action=/set method=post><b>Device name</b> <input name=name value=\""
"\"> <b>Device Domain</b> <input name=domain value=\""
"\"> <b>Network SSID</b> <input name=ssid value=\""
"\"> <b>Network password</b> <input type=password name=password> <b>Confirm password</b> <input type=password name=confirm> <input type=submit value=Submit></form>"
"</div><i><p></p></i><script>function n(){var e=new XMLHttpRequest;e.onreadystatechange=function(){if(4==e.readyState)if(200===e.status){document.querySelector(\"i\").style.display=\"none\";var t=JSON.parse(e.responseText);for(document.querySelector(\"title\").innerHTML=t.n,document.querySelector(\"h1\").innerHTML=t.n,b=0;b<t.c.length;++b)document.querySelector(\"#c\"+(b+1)).style.background=\"#\"+t.c[b];document.getElementById(\"m\"+t.m).className=\"a\"}else document.querySelector(\"i p\").innerHTML=\"Unable to process data from module. Check logs.\",document.querySelector(\"i\").style.display=\"inline-block\",console.log(e.status,e.responseText)},e.open(\"GET\",\"/status\",!0),e.send(null)}function p(e,t){if(g){var n,l;l=a.getBoundingClientRect(),t?(n=e.changedTouches[0].clientX-l.left,l=e.changedTouches[0].clientY-l.top):(n=e.clientX-l.left,l=e.clientY-l.top),h=n/a.clientWidth*360,k=l/a.clientHeight*100,document.querySelector(\"#c .a\").style.background=\"hsl(\"+h+\", 100%, \"+k+\"%)\",e.preventDefault()}}function r(e){return g=!0,t=document.querySelector(\"#c .a\").style.background,p(e),e.preventDefault(),!1}function u(e){if(g){g=null,clearInterval(q);var t=new XMLHttpRequest,l=new FormData;l.append(\"h\",h/360),l.append(\"s\",1),l.append(\"l\",k/100),t.open(\"POST\",\"/color/hsl\",!0),t.send(l),q=setInterval(n,5e3),e.preventDefault()}}var f,b,q,a,h,k,l,m;for(f=document.querySelectorAll(\"nav a\"),b=0;b<f.length;++b)f[b].onclick=function(){this.parentNode.querySelector(\".a\").className=\"\",this.className=\"a\"};for(b=1;5>b;++b)document.getElementById(\"m\"+b).onclick=function(){var e=b;return function(){var t=new XMLHttpRequest;t.open(\"POST\",\"/mode\",!0),t.send(\"m=\"+e),this.parentNode.querySelector(\".a\").className=\"\",this.className=\"a\"}}();for(document.querySelector(\"#set\").onclick=function(){document.querySelector(\"form\").classList.toggle(\"hdn\"),window.location=\"#set\"},n(),q=setInterval(n,5e3),a=document.getElementById(\"cp\"),f=a.getContext(\"2d\"),a.width=a.clientWidth,a.height=a.clientHeight,m=0;m<=a.clientHeight;++m)for(l=0;l<a.clientWidth;++l)h=l/a.clientWidth*360,k=m/a.clientHeight*100,f.fillStyle=\"hsl(\"+h+\", 100%, \"+k+\"%)\",f.fillRect(l,m,1,1);var g=null,t;a.onmousedown=r,a.onmousemove=p,document.onmouseup=u,a.addEventListener(\"touchstart\",r,!1),a.addEventListener(\"touchmove\",function(e){p(e,!0)},!1),a.addEventListener(\"touchend\",u,!1),a.addEventListener(\"touchcancel\",function(){g=null,document.querySelector(\"#c .a\").style.background=t},!1);</script>";
const char json[] = "{\"c\": [\"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\", \"%02x%02x%02x\"], \"m\": \"%d\", \"n\": \"%s\", \"s\": \"%s\"}";
const chat notFoundHtml[] = "<html><head><meta charset=utf-8><title>Error 404 Not Found</title></head><style>body {font-family: arial,sans-serif}</style><body><blockquote><h2>Error 404 Not Found</h2><p>The page you are looking for does not exist";

// Front End Initial colors
const char white[3]  = {255, 255, 255};
const char yellow[3] = {255, 255, 0};
const char green[3] = {0, 232, 66};
const char blue[3] = {0, 174, 255};
const char red[3] = {255, 0, 66};
const char teal[3] = {0, 128, 128};
const char lilac[3] = {128, 0, 128};
const char orange[3] = {255, 127, 0};

byte WiFiMode, OpMode;
char ssid[64], deviceName[64];

MDNSResponder mdns;
ESP8266WebServer server (80);

void rootPath();
void statusPath();
void setPath();
void setColorHSLPath();
void setColorRGBPath();
void notFoundPath();

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // EEPROM Space Usage
  // EEPROM UUID Mask - 5 bytes + 1 byte \0 | Used to verify & initialise EEPROM
  // Current operating mode - 1 byte | [ Off, Static Color, 3 Color Fade, Data Stream, Full Fade ]
  // Current WiFi Mode - 1 byte      | [ AP, Client ]
  // Device Name - 64 bytes          | Human readable name of the device
  // SSID - 64 bytes                 | SSID of the network to connect to when in client mode
  // Password - 64 bytes             | WiFi Password of the network to connect to when in client mode
  // Domain Name - 64 bytes          | Domain name for local resolution 
  // ------------------------------- | Total memory used for base purposes - 264 bytes
  // 8 colors - 24 bytes             | * colors with 3 bytes each
  // ------------------------------- | TOTAL: 288 bytes
  // Additional EEPROM data should be appended to the end and described
  EEPROM.begin(288);
  
  // EEPROM Intialisation
  for(int i = 0; i < 5; ++i) {
    // Verify EEPROM UUID
    if ( EEPROM.read( i ) != factoryUUID[i] ) {
      Serial.println("Initialising EEPROM");
      factoryEEPROM();
      break; 
    }
  }
  
  // Initialise WiFi
  OpMode = EEPROM.read(6);
  WiFiMode = EEPROM.read(7);
  
  char password[64], domain[64];
  EEPROM.get( 8, deviceName );
  EEPROM.get( 72, ssid );
  EEPROM.get( 136, password );
  EEPROM.get( 200, domain );

wiFiStartUp:  
  if ( WiFiMode == 0 ) { // AP
    Serial.println("Configuring Access Point");
    if( password[0] == '\0' ) {
      WiFi.softAP(ssid);
    } else {
      WiFi.softAP( ssid, password );
    }
  } else if( WiFiMode == 1 ) { // Client
    Serial.println("Connecting to WiFi Network");
    if( password[0] == '\0' ) {
      WiFi.begin(ssid);
    } else {
      WiFi.begin( ssid, password );
    }
  }
  
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
     // If after 30 seconds the module has not been able to connect to a WiFi
    if( WiFiMode == 1 && millis() >= 10000 ) {
      // Fallback to AP mode with the current settings
      Serial.println("Unable to connect to network.");
      WiFi.disconnect();
      WiFiMode = 0;
      goto wiFiStartUp;
    } else if( millis() >= 20000 ) {
      Serial.println("Emergency escape. Attempting to continue!");
      break; 
    }
    
    delay ( 1000 );
    Serial.print (".");
  }
  
  // Report IP Address
  IPAddress IP;
  if ( WiFiMode == 0 ) {
    IP = WiFi.softAPIP();
  } else if( WiFiMode == 1 ) {
    Serial.print("Connected to: ");
    Serial.println(ssid);
    IP = WiFi.localIP();
  }
  Serial.print("IP address: ");
  Serial.println(IP);
  
  // Initialise MDNS Responder
  if ( mdns.begin ( domain, IP ) ) {
    Serial.println ("MDNS responder started");
  }

  // Initialise the Web Server
  server.on ( "/", rootPath );
  server.on ( "/set", setPath );
  server.on ( "/status", statusPath );
  server.on ( "/color/hls", setColorHSLPath );
  server.on ( "/color/rgb", setColorRGBPath );
  server.onNotFound ( notFoundPath );
  server.begin();
  Serial.println("Initialised HTTP server");
  
  Serial.println("done");
}

void loop() {
  mdns.update();
  server.handleClient();
}

void factoryEEPROM() {
  EEPROM.put( 0, factoryUUID ); // UUID
  EEPROM.write( 6, 0 ); // Operating mode / Off
  EEPROM.write( 7, 1 ); // WiFi mode / Client
  EEPROM.put( 8, factoryName ); // Device human readable name
  EEPROM.put( 72, factorySSID ); // SSID for Factory AP mode
  EEPROM.put( 136, factoryPass ); // Password for Factory AP mode
  EEPROM.put( 200, factoryDomain ); // Factory Domain Name
  EEPROM.put( 264 + 0 * 3, white ); // White
  EEPROM.put( 264 + 1 * 3, yellow ); // yellow
  EEPROM.put( 264 + 2 * 3, green ); // green
  EEPROM.put( 264 + 3 * 3, blue ); // blue
  EEPROM.put( 264 + 4 * 3, red ); // red
  EEPROM.put( 264 + 5 * 3, teal ); // teal
  EEPROM.put( 264 + 6 * 3, lilac ); // lilac
  EEPROM.put( 264 + 7 * 3, orange ); // orange
  EEPROM.commit();
}

void rootPath() {
  server.send( 200, "text/html", html );
}

void statusPath() {
  char temp[sizeof(json) + 8 * 6 + 129];
  
  // Stores the colors loaded from EEPROM
  byte c[8][3];
  
  // Get colors from EEPROM
  for(int i = 0; i < 8; ++i) {
    EEPROM.get( 264 + i * 3, c[i] );
  }
  
  snprintf(temp, sizeof(json) + 8 * 6 + 129, json, c[0][0], c[0][1], c[0][2], c[1][0], c[1][1], c[1][2], c[2][0], c[2][1], c[2][2], c[3][0], c[3][1], c[3][2], c[4][0], c[4][1], c[4][2], c[5][0], c[5][1], c[5][2], c[6][0], c[6][1], c[6][2], c[7][0], c[7][1], c[7][2], (int)OpMode, deviceName, ssid );
  server.send( 200, "text/html", temp );
}

void setPath() {
  
}

void setColorHSLPath() {
  
}

void setColorRGBPath() {
  
}

void notFoundPath() {
  server.send( 404, "text/html", html );
}
