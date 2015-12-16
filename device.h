#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

namespace Base {

class Device {
public:
  Device();

  // Base functions
  bool boot();
  void store();
  void load();
  void reboot();
  void initWiFi();

  // Accessors
  IPAddress ip();

  String name();
  void setName(String);

  WiFiMode mode();
  void setMode(WiFiMode);

  String ssid();
  void setSSID(String);

  String password();
  void setPassword(String);

private:
    void printConfig();

    String m_name;
    String m_ssid;
    WiFiMode m_mode;
    String m_version;
    String m_password;
    IPAddress m_ip;
    bool m_nameChanged;
    bool m_ssidChanged;
    bool m_modeChanged;
    bool m_passwordChanged;
    int m_EEPROMSize = 0;
    unsigned long lastPhysicalAccess;
};

} // Base::

#endif // DEVICE_H
