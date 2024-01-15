/**
 * 
 * AstroPixelsPlus sketch operates as a I2C master that can optionally be connected to one or more
 * Adafruit PCA9685 servo controllers to control dome panels. The sketch also provides serial commands
 * on Serial2 * 
 */

// Define USE_I2C_ADDRESS to enable slave mode. This will disable servo support
//#define USE_I2C_ADDRESS 0x0a
#define USE_DEBUG                     // Define to enable debug diagnostic
#define USE_WIFI                      // Define to enable Wifi support
#ifdef USE_WIFI
#define USE_MDNS
#define USE_OTA
#define USE_WIFI_WEB
#define USE_WIFI_MARCDUINO
//#define LIVE_STREAM
#endif

////////////////////////////////

// Replace with your network credentials
#ifdef USE_WIFI
#define WIFI_ENABLED         true     // default enabled
// Set these to your desired WiFi credentials.
#define WIFI_AP_NAME         "APPDome"
#define WIFI_AP_PASSPHRASE   "Astromech"
#define WIFI_ACCESS_POINT    true  /* true if access point: false if joining existing wifi */
#endif

///////////////////////////////////

#if __has_include("build_version.h")
#include "build_version.h"
#endif

#if __has_include("reeltwo_build_version.h")
#include "reeltwo_build_version.h"
#endif

////////////////////////////////

#define PREFERENCE_WIFI_ENABLED          "wifi"
#define PREFERENCE_WIFI_SSID             "ssid"
#define PREFERENCE_WIFI_PASS             "pass"
#define PREFERENCE_WIFI_AP               "ap"

#define PREFERENCE_MARCSERIAL1_BAUD      "mser1baud"
#define PREFERENCE_MARCSERIAL1_PASS      "mserlpass"
#define PREFERENCE_MARCSERIAL1_ENABLED   "mser1ena"

#define PREFERENCE_MARCSERIAL2_BAUD      "mser2baud"
#define PREFERENCE_MARCSERIAL2_PASS      "mser2pass"
#define PREFERENCE_MARCSERIAL2_ENABLED   "mser2ena"

#define PREFERENCE_MARCWIFI_ENABLED      "mwifi"
#define PREFERENCE_MARCWIFI_SERIAL1_PASS "mwifipass1"
#define PREFERENCE_MARCWIFI_SERIAL2_PASS "mwifipass2"

////////////////////////////////

#define CONSOLE_BUFFER_SIZE     300

////////////////////////////////

#include "ReelTwo.h"
#include "dome/Logics.h"
#include "dome/LogicEngineController.h"
#include "dome/NeoPSI.h"

#ifdef USE_I2C_ADDRESS
#include "i2c/I2CReceiver.h"
#include "ServoDispatchDirect.h"
#else
#include "ServoDispatchPCA9685.h"
#endif
#include "ServoSequencer.h"
#include "core/Marcduino.h"

#include <Preferences.h>

////////////////////////////////

#include "pin-map.h"

////////////////////////////////

#define MARC_SERIAL1_BAUD_RATE          9600
#define MARC_SERIAL2_BAUD_RATE          9600
#define MARC_SERIAL1_PASS               false
#define MARC_SERIAL1_ENABLED            true
#define MARC_SERIAL2_PASS               true
#define MARC_SERIAL2_ENABLED            true
#define MARC_WIFI_ENABLED               true
#define MARC_WIFI_SERIAL1_PASS          false
#define MARC_WIFI_SERIAL2_PASS          true

#include "wifi/WifiAccess.h"

////////////////////////////////

#ifdef USE_MDNS
#include <ESPmDNS.h>
#endif
#ifdef USE_WIFI_WEB
#include "wifi/WifiWebServer.h"
#endif
#ifdef USE_WIFI_MARCDUINO
#include "wifi/WifiMarcduinoReceiver.h"
#endif
#ifdef USE_OTA
#include <ArduinoOTA.h>
#endif
#include "ServoEasing.h"

////////////////////////////////
/// \private
template <uint8_t DATA_PIN = FRONT_PSI_PIN>
class EightLEDPSIPCB_FrontPSI : public FastLEDPCB<WS2812B, DATA_PIN, 8, 0, 8, 5, 5>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
            31, 31,  0, 31, 31,
            31,  7, 31,  1, 31,
            6,  31, 31, 31,  2,
            31,  5, 31,  3, 31,
            31, 31,  4, 31, 31
        };
        return sLEDmap;
    }
};

template <uint8_t DATA_PIN = PIN_FRONT_PSI2>
class EightLEDPSIPCB_FrontPSI2 : public FastLEDPCB<WS2812B, DATA_PIN, 8, 0, 8, 5, 5>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
            31, 31,  0, 31, 31,
            31,  7, 31,  1, 31,
            6,  31, 31, 31,  2,
            31,  5, 31,  3, 31,
            31, 31,  4, 31, 31
        };
        return sLEDmap;
    }
};

template <uint8_t DATA_PIN = PIN_FRONT_PSI3>
class EightLEDPSIPCB_FrontPSI3 : public FastLEDPCB<WS2812B, DATA_PIN, 8, 0, 8, 5, 5>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
            31, 31,  0, 31, 31,
            31,  7, 31,  1, 31,
            6,  31, 31, 31,  2,
            31,  5, 31,  3, 31,
            31, 31,  4, 31, 31
        };
        return sLEDmap;
    }
};


template <uint8_t DATA_PIN = FRONT_PSI_PIN>
using EightLEDFrontPSI = LogicEngineDisplay<EightLEDPSIPCB_FrontPSI<DATA_PIN>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

template <uint8_t DATA_PIN2 = PIN_FRONT_PSI2>
using EightLEDFrontPSI2 = LogicEngineDisplay<EightLEDPSIPCB_FrontPSI2<DATA_PIN2>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

template <uint8_t DATA_PIN3 = PIN_FRONT_PSI3>
using EightLEDFrontPSI3 = LogicEngineDisplay<EightLEDPSIPCB_FrontPSI3<DATA_PIN3>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

/////////////////////////////////

// AstroPixelFrontPSI<PIN_FRONT_PSI> frontPSI(LogicEngineFrontPSIDefault, 4);
EightLEDFrontPSI<PIN_FRONT_PSI> frontPSI(LogicEngineFrontPSIDefault, 4);
EightLEDFrontPSI2<PIN_FRONT_PSI2> frontPSI2(LogicEngineFrontPSIDefault, 4);
EightLEDFrontPSI3<PIN_FRONT_PSI3> frontPSI3(LogicEngineFrontPSIDefault, 4);
AstroPixelRearPSI<PIN_REAR_PSI> rearPSI(LogicEngineRearPSIDefault, 5);

////////////////////////////////

#define SMALL_PANEL         0x0001
#define MEDIUM_PANEL        0x0002
#define BIG_PANEL           0x0004
#define PIE_PANEL           0x0008
#define TOP_PIE_PANEL       0x0010
#define MINI_PANEL          0x0020
#define CONTINUOUS          0x0040
#define HOLO_HSERVO         0x1000
#define HOLO_VSERVO         0x2000

#define DOME_PANELS_MASK        (SMALL_PANEL|MEDIUM_PANEL|BIG_PANEL)
#define PIE_PANELS_MASK         (PIE_PANEL)
#define ALL_DOME_PANELS_MASK    (MINI_PANEL|DOME_PANELS_MASK|PIE_PANELS_MASK|TOP_PIE_PANEL)
#define DOME_DANCE_PANELS_MASK  (DOME_PANELS_MASK|PIE_PANELS_MASK)
#define HOLO_SERVOS_MASK        (HOLO_HSERVO|HOLO_VSERVO)
#define CONTINUOUS_MASK         (CONTINUOUS)
#define PANEL_GROUP_1      (1L<<14)
#define PANEL_GROUP_2      (1L<<15)
#define PANEL_GROUP_3      (1L<<16)
#define PANEL_GROUP_4      (1L<<17)
#define PANEL_GROUP_5      (1L<<18)
#define PANEL_GROUP_6      (1L<<19)
#define PANEL_GROUP_7      (1L<<20)
#define PANEL_GROUP_8      (1L<<21)
#define PANEL_GROUP_9      (1L<<22)
#define PANEL_GROUP_10     (1L<<23)
#define PANEL_GROUP_11     (1L<<24)

////////////////////////////////
// These values will be configurable through the WiFi interface and stored in the preferences.
const ServoSettings servoSettings[] PROGMEM = {
#ifndef USE_I2C_ADDRESS
     // First PCA9685 controller
    { 1,  1150, 2550, PANEL_GROUP_1|PIE_PANEL},    /* 0: pie panel 1 */
    { 2,  1150, 2550, PANEL_GROUP_2|PIE_PANEL},    /* 1: pie panel 2 */
    { 3,  2250, 1150, PANEL_GROUP_3|PIE_PANEL},    /* 2: pie panel 3 */
    { 4,  1150, 2550, PANEL_GROUP_4|PIE_PANEL},    /* 3: pie panel 4 */
    { 5,  2250, 1150, PANEL_GROUP_5|PIE_PANEL},    /* 4: pie panel 5 */
    { 6,  1150, 2515, PANEL_GROUP_6|PIE_PANEL},    /* 5: pie panel 6 */
    { 7,  2250, 1150, PANEL_GROUP_7|PIE_PANEL},    /* 6: pie panel 7 */
    { 8,  1150, 2550, PANEL_GROUP_8|PIE_PANEL},    /* 7: pie panel 8 */
    { 9,  0, 4096, PANEL_GROUP_9|CONTINUOUS},   /* 8: CONTINUOUS 1 Bad Motivator Lift*/
    {10,  0, 4096, PANEL_GROUP_10|CONTINUOUS},  /* 9: CONTINUOUS 2 */
    {11,  0, 4096, PANEL_GROUP_11|CONTINUOUS},  /* 10: CONTINUOUS 3 */
    {12,  800, 2200, MINI_PANEL},  /* 11: mini panel 4 */
    {13,  800, 2200, MINI_PANEL},  /* 12: mini panel 5 */

    // Second PCA9685 controller
    // { 16, 800, 2200, HOLO_HSERVO },                /* 13: horizontal front holo */
    // { 17, 800, 2200, HOLO_VSERVO },                /* 14: vertical front holo */
    // { 18, 800, 2200, HOLO_HSERVO },                /* 15: horizontal top holo */
    // { 19, 800, 2200, HOLO_VSERVO },                /* 16: vertical top holo */
    // { 20, 800, 2200, HOLO_VSERVO },                /* 17: vertical rear holo */
    // { 21, 800, 2200, HOLO_HSERVO },                /* 18: horizontal rear holo */
#endif
};

#ifdef USE_I2C_ADDRESS
ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);
#else
ServoDispatchPCA9685<SizeOfArray(servoSettings)> servoDispatch(servoSettings);
#endif
ServoSequencer servoSequencer(servoDispatch);
AnimationPlayer player(servoSequencer);
MarcduinoSerial<> marcduinoSerial1(player);
MarcduinoSerial<> marcduinoSerial2(player);

/////////////////////////////////////////////////////////////////////////

Preferences preferences;

////////////////////////////////
// This function is called when settings have been changed and needs a reboot
void reboot()
{
    DEBUG_PRINTLN("Restarting...");
    preferences.end();
    delay(1000);
    ESP.restart();
}

////////////////////////////////
// This function is called when aborting or ending Marcduino sequences. It should reset all droid devices to Normal
void resetSequence()
{
    Marcduino::send(F("$s"));
    CommandEvent::process(F( 
        "LE000000|0\n"   // LogicEngine devices to normal
        "FSOFF\n"        // Fire Stripe Off
        "BMOFF\n"        // Bad Motiviator Off
        "HPA000|0\n"     // Holo Projectors to Normal
        "CB00000\n"      // Charge Bay to Normal
        "DP00000\n"));   // Data Panel to Normal
}

////////////////////////////////

int32_t strtol(const char* cmd, const char** endptr)
{
    bool sign = false;
    int32_t result = 0;
    if (*cmd == '-')
    {
        cmd++;
        sign = true;
    }
    while (isdigit(*cmd))
    {
        result = result*10L + (*cmd-'0');
        cmd++;
    }
    *endptr = cmd;
    return (sign) ? -result : result;
}

////////////////////////////////

bool numberparams(const char* cmd, uint8_t &argcount, int32_t* args, uint8_t maxcount)
{
    for (argcount = 0; argcount < maxcount; argcount++)
    {
        args[argcount] = strtol(cmd, &cmd);
        if (*cmd == '\0')
        {
            argcount++;
            return true;
        }
        else if (*cmd != ',')
        {
            return false;
        }
        cmd++;
    }
    return true;
}

////////////////////////////////

#include "MarcduinoSequence.h"
#include "MarcduinoPanel.h"
#include "MarcduinoPSI.h"

////////////////////////////////

#ifdef USE_WIFI
WifiAccess wifiAccess;
bool wifiEnabled;
bool wifiActive;
TaskHandle_t eventTask;
bool otaInProgress;
#endif

#ifdef USE_WIFI_MARCDUINO
WifiMarcduinoReceiver wifiMarcduinoReceiver(wifiAccess);
#endif

////////////////////////////////

#ifdef USE_WIFI_WEB
#include "WebPages.h"
#endif

////////////////////////////////

void setup()
{
    REELTWO_READY();

    if (!preferences.begin("AppDome", false))
    {
        DEBUG_PRINTLN("Failed to init prefs");
    }
#ifdef USE_WIFI
    wifiEnabled = wifiActive = preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED);
#endif
    PrintReelTwoInfo(Serial, "AppDome");

    if (preferences.getBool(PREFERENCE_MARCSERIAL1_ENABLED, MARC_SERIAL1_ENABLED))
    {
        int baudRate = preferences.getInt(PREFERENCE_MARCSERIAL1_BAUD, MARC_SERIAL1_BAUD_RATE);
        COMMAND_SERIAL1.begin(baudRate, SERIAL_8N1, SERIAL1_RX_PIN, SERIAL1_TX_PIN);
        // Check if "Serial pass-through to Serial2" is enabled
        if (preferences.getBool(PREFERENCE_MARCSERIAL1_PASS, MARC_SERIAL1_PASS))
        {
            // Read from COMMAND_SERIAL1 and write to COMMAND_SERIAL1 (pass-through)
            marcduinoSerial1.setStream(&COMMAND_SERIAL1, &COMMAND_SERIAL1);
        }
        else
        {
        #ifdef USE_DEBUG
            // Read from COMMAND_SERIAL1 and write to console (debug)
            marcduinoSerial1.setStream(&COMMAND_SERIAL1, &Serial);
        #else
            // Read from COMMAND_SERIAL1 and dont write
            marcduinoSerial1.setStream(&COMMAND_SERIAL1, nullptr);
        #endif
        }
    }
    if (preferences.getBool(PREFERENCE_MARCSERIAL2_ENABLED, MARC_SERIAL2_ENABLED))
    {
        int baudRate = preferences.getInt(PREFERENCE_MARCSERIAL2_BAUD, MARC_SERIAL2_BAUD_RATE);
        COMMAND_SERIAL2.begin(baudRate, SERIAL_8N1, SERIAL2_RX_PIN, SERIAL2_TX_PIN);
        // Check if "Serial pass-through to Serial2" is enabled
        if (preferences.getBool(PREFERENCE_MARCSERIAL2_PASS, MARC_SERIAL2_PASS))
        {
            // Read from COMMAND_SERIAL2 and write to COMMAND_SERIAL2 (pass-through)
            marcduinoSerial2.setStream(&COMMAND_SERIAL2, &COMMAND_SERIAL2);
        }
        else
        {
        #ifdef USE_DEBUG
            // Read from COMMAND_SERIAL2 and write to console (debug)
            marcduinoSerial2.setStream(&COMMAND_SERIAL2, &Serial);
        #else
            // Read from COMMAND_SERIAL2 and dont write
            marcduinoSerial2.setStream(&COMMAND_SERIAL2, nullptr);
        #endif
        }
    }

#ifndef USE_I2C_ADDRESS
    Wire.begin();
#endif
    SetupEvent::ready();

#ifdef USE_WIFI
    if (wifiEnabled)
    {
    #ifdef USE_WIFI_WEB
        // In preparation for adding WiFi settings web page
        wifiAccess.setNetworkCredentials(
            preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME),
            preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE),
            preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT),
            preferences.getBool(PREFERENCE_WIFI_ENABLED, WIFI_ENABLED));
    #ifdef USE_WIFI_MARCDUINO
        wifiMarcduinoReceiver.setEnabled(preferences.getBool(PREFERENCE_MARCWIFI_ENABLED, MARC_WIFI_ENABLED));
        if (wifiMarcduinoReceiver.enabled())
        {
            wifiMarcduinoReceiver.setCommandHandler([](const char* cmd) {
                printf("cmd: %s\n", cmd);
                Marcduino::processCommand(player, cmd);
                if (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL1_PASS, MARC_WIFI_SERIAL1_PASS))
                {
                    COMMAND_SERIAL1.print(cmd); COMMAND_SERIAL1.print('\r');
                }
                if (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL2_PASS, MARC_WIFI_SERIAL2_PASS))
                {
                    COMMAND_SERIAL2.print(cmd); COMMAND_SERIAL2.print('\r');
                }
            });
        }
    #endif
        wifiAccess.notifyWifiConnected([](WifiAccess &wifi) {
            Serial.print("Connect to http://"); Serial.println(wifi.getIPAddress());
        #ifdef USE_MDNS
            // No point in setting up mDNS if R2 is the access point
            if (!wifi.isSoftAP())
            {
                String mac = wifi.getMacAddress();
                String hostName = mac.substring(mac.length()-5, mac.length());
                hostName.remove(2, 1);
                hostName = String(WIFI_AP_NAME)+String("-")+hostName;
                if (webServer.enabled())
                {
                    Serial.print("Host name: "); Serial.println(hostName);
                    if (!MDNS.begin(hostName.c_str()))
                    {
                        DEBUG_PRINTLN("Error setting up MDNS responder!");
                    }
                }
            }
        #endif
        });
    #endif
    #ifdef USE_OTA
        ArduinoOTA.onStart([]()
        {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
            {
                type = "sketch";
            }
            else // U_SPIFFS
            {
                type = "filesystem";
            }
            DEBUG_PRINTLN("OTA START");
        })
        .onEnd([]()
        {
            DEBUG_PRINTLN("OTA END");
        })
        .onProgress([](unsigned int progress, unsigned int total)
        {
            // float range = (float)progress / (float)total;
        })
        .onError([](ota_error_t error)
        {
            String desc;
            if (error == OTA_AUTH_ERROR) desc = "Auth Failed";
            else if (error == OTA_BEGIN_ERROR) desc = "Begin Failed";
            else if (error == OTA_CONNECT_ERROR) desc = "Connect Failed";
            else if (error == OTA_RECEIVE_ERROR) desc = "Receive Failed";
            else if (error == OTA_END_ERROR) desc = "End Failed";
            else desc = "Error: "+String(error);
            DEBUG_PRINTLN(desc);
        });
    #endif
    }
#endif
#ifdef USE_WIFI_WEB
    // For safety we will stop the motors if the web client is connected
    webServer.setConnect([]() {
        // Callback for each connected web client
        // DEBUG_PRINTLN("Hello");
    });
#endif

#ifdef USE_WIFI
    xTaskCreatePinnedToCore(
          eventLoopTask,
          "Events",
          10000,    // shrink stack size?
          NULL,
          1,
          &eventTask,
          0);
#endif
    DEBUG_PRINTLN("Ready");
}

////////////////

MARCDUINO_ACTION(DirectCommand, ~RT, ({
    // Direct ReelTwo command
    CommandEvent::process(Marcduino::getCommand());
}))

////////////////

MARCDUINO_ACTION(MDDirectCommand, @AP, ({
    // Direct ReelTwo command
    CommandEvent::process(Marcduino::getCommand());
}))

////////////////

MARCDUINO_ACTION(WifiToggle, #APWIFI, ({
#ifdef USE_WIFI
    bool wifiSetting = wifiEnabled;
    switch (*Marcduino::getCommand())
    {
        case '0':
            wifiSetting = false;
            break;
        case '1':
            wifiSetting = true;
            break;
        case '\0':
            // Toggle WiFi
            wifiSetting = !wifiSetting;
            break;
    }
    if (wifiEnabled != wifiSetting)
    {
        if (wifiSetting)
        {
            preferences.putBool(PREFERENCE_WIFI_ENABLED, true);
            DEBUG_PRINTLN("WiFi Enabled");
        }
        else
        {
            preferences.putBool(PREFERENCE_WIFI_ENABLED, false);
            DEBUG_PRINTLN("WiFi Disabled");
        }
        reboot();
    }
#endif
}))

////////////////

MARCDUINO_ACTION(ClearPrefs, #APZERO, ({
    preferences.clear();
    DEBUG_PRINT("Clearing preferences. ");
    reboot();
}))

////////////////

MARCDUINO_ACTION(Restart, #APRESTART, ({
    reboot();
}))

////////////////

static unsigned sPos;
static char sBuffer[CONSOLE_BUFFER_SIZE];

////////////////

#ifdef USE_I2C_ADDRESS
I2CReceiverBase<CONSOLE_BUFFER_SIZE> i2cReceiver(USE_I2C_ADDRESS, [](char* cmd) {
    DEBUG_PRINT("[I2C] RECEIVED=\"");
    DEBUG_PRINT(cmd);
    DEBUG_PRINTLN("\"");
    Marcduino::processCommand(player, cmd);
});
#endif

////////////////

void mainLoop()
{
    AnimatedEvent::process();

    if (Serial.available())
    {
        int ch = Serial.read();
        if (ch == 0x0A || ch == 0x0D)
        {
            Marcduino::processCommand(player, sBuffer);
            sPos = 0;
        }
        else if (sPos < SizeOfArray(sBuffer)-1)
        {
            sBuffer[sPos++] = ch;
            sBuffer[sPos] = '\0';
        }
    }
}

////////////////

#ifdef USE_WIFI
void eventLoopTask(void* )
{
    for (;;)
    {
        if (wifiActive)
        {
        #ifdef USE_OTA
            ArduinoOTA.handle();
        #endif
        #ifdef USE_WIFI_WEB
            webServer.handle();
        #endif
        }
        vTaskDelay(1);
    }
}
#endif

////////////////

void loop()
{
    mainLoop();
}
