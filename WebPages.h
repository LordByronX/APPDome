#ifdef USE_WIFI_WEB

#include "web-images.h"

////////////////////////////////
// List of available sequences by name and matching id
enum
{
    kMAX_FADE = 15,
    kMAX_DELAY = 500,
    kMIN_DELAY = 10,
    kMIN_BRI = 10,

    kMAX_ADJLOOP = 90000,
    kMIN_ADJLOOP = 500,
};

WMenuData mainMenu[] = {
    { "Dome", "/dome" },
    { "Setup", "/setup" }
};

WMenuData setupMenu[] = {
    { "Home", "/" },
    { "Serial", "/serial" },
    { "WiFi", "/wifi" },
    { "Firmware", "/firmware" },
    { "Back", "/" }
};

WElement mainContents[] = {
    WVerticalMenu("menu", mainMenu, SizeOfArray(mainMenu)),
    rseriesSVG
};

WElement setupContents[] = {
    WVerticalMenu("setup", setupMenu, SizeOfArray(setupMenu)),
    rseriesSVG
};

/////////////////////////////////////////////////////////////////////////

String psiSeq[] = {
#define PSI_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    nam,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef PSI_SEQ
};

unsigned psiSeqNumber[] = {
#define PSI_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    val,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef PSI_SEQ
};

String psiColors[] = {
    "Default",
    "Red",
    "Orange",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Purple",
    "Magenta",
    "Pink"
};

// Dont have to set things to zero but for clarity we do
int sPSISequence = 0;
int sPSIColor = LogicEngineRenderer::kDefault;
int sPSISpeedScale = 0;
int sPSINumSeconds = 0;
bool sPSIFront1 = false;
bool sPSIFront2 = false;
bool sPSIFront3 = false;
bool sPSIRear = false;

static void updatePSIState()
{
    static bool psiFront1WasEnabled = false;
    static bool psiFront2WasEnabled = false;
    static bool psiFront3WasEnabled = false;
    static bool psiRearWasEnabled = false;
    if (sPSIFront1) {
        frontPSI.selectSequence(psiSeqNumber[sPSISequence], (LogicEngineRenderer::ColorVal)sPSIColor, sPSISpeedScale, sPSINumSeconds);
        psiFront1WasEnabled = true;
    } else if (psiFront1WasEnabled) {
        // Reset front psi 1 to default
        frontPSI.selectSequence(LogicEngineRenderer::NORMAL);
        psiFront1WasEnabled = false;
    }
    if (sPSIFront2) {
        frontPSI2.selectSequence(psiSeqNumber[sPSISequence], (LogicEngineRenderer::ColorVal)sPSIColor, sPSISpeedScale, sPSINumSeconds);
        psiFront2WasEnabled = true;
    } else if (psiFront2WasEnabled) {
        // Reset front psi 2 to default
        frontPSI2.selectSequence(LogicEngineRenderer::NORMAL);
        psiFront2WasEnabled = false;
    }
    if (sPSIFront3) {
        frontPSI3.selectSequence(psiSeqNumber[sPSISequence], (LogicEngineRenderer::ColorVal)sPSIColor, sPSISpeedScale, sPSINumSeconds);
        psiFront3WasEnabled = true;
    } else if (psiFront3WasEnabled) {
        // Reset front psi 3 to default
        frontPSI3.selectSequence(LogicEngineRenderer::NORMAL);
        psiFront3WasEnabled = false;
    }
    if (sPSIRear) {
        rearPSI.selectSequence(psiSeqNumber[sPSISequence], (LogicEngineRenderer::ColorVal)sPSIColor, sPSISpeedScale, sPSINumSeconds);
        psiRearWasEnabled = true;
    } else if (psiRearWasEnabled) {
        // Reset rear psi to default
        rearPSI.selectSequence(LogicEngineRenderer::NORMAL);
        psiRearWasEnabled = false;
    }
}

/////////////////////////////////////////////////////////////////////////
// Web Interface for logic engine animation sequences
WElement domeContents[] = {
    WSelect("PSI Sequence", "psiseq",
        psiSeq, SizeOfArray(psiSeq),
        []() { return sPSISequence; },
        [](int val) {
            sPSISequence = val;
            updatePSIState();
        } ),
    WSelect("PSI Color", "psicolor",
        psiColors, SizeOfArray(psiColors),
        []() { return sPSIColor; },
        [](int val) {
            sPSIColor = val;
            updatePSIState();
        } ),
    WSlider("Animation Speed", "psispeed", 0, 9,
        []()->int { return sPSISpeedScale; },
        [](int val) {
            sPSISpeedScale = val;
            updatePSIState();
        } ),
    WSlider("Number of seconds", "fldseconds", 0, 99,
        []()->int { return sPSINumSeconds; },
        [](int val) {
            sPSINumSeconds = val;
            updatePSIState();
        } ),
    WCheckbox("Front PSI#1", "frontPSI1",
        []() { return sPSIFront1; },
        [](bool val) {
            sPSIFront1 = val;
            updatePSIState();
        } ),
    WCheckbox("Front PSI#2", "frontPSI2",
        []() { return sPSIFront2; },
        [](bool val) {
            sPSIFront2 = val;
            updatePSIState();
        } ),
    WCheckbox("Front PSI#3", "frontPSI3",
        []() { return sPSIFront3; },
        [](bool val) {
            sPSIFront3 = val;
            updatePSIState();
        } ),
    WCheckbox("Rear PSI", "rearPSI",
        []() { return sPSIRear; },
        [](bool val) {
            sPSIRear = val;
            updatePSIState();
        } ),
    WButtonReload("Reset", "reset",  []() {
        sPSISequence = 0;
        sPSIColor = LogicEngineRenderer::kDefault;
        sPSISpeedScale = 0;
        sPSINumSeconds = 0;
        sPSIFront1 = false;
        sPSIFront2 = false;
        sPSIFront3 = false;
        sPSIRear = false;
        updatePSIState();
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    rseriesSVG
};

////////////////////////////////

String swBaudRates[] = {
    "2400",
    "9600",
    "57600",
    "115200",
};

int swBaudRatesInt[] = {
    2400,
    9600,
    57600,
    115200,
};

int swBaudRateToIndex(int baudRate)
{
    for (int i = 0; i < SizeOfArray(swBaudRatesInt); i++)
    {
        if (baudRate == swBaudRatesInt[i]) {
            return i;
        }
    }
    return 0;
}

int marcSerial1Baud;
int marcSerial2Baud;
bool marcSerial1Pass;
bool marcSerial2Pass;
bool marcSerial1Enabled;
bool marcSerial2Enabled;
bool marcWifiEnabled;
bool marcWifiSerial1Pass;
bool marcWifiSerial2Pass;

WElement serialContents[] = {
    WSelect("Serial1 Baud Rate", "serial1baud",
        swBaudRates, SizeOfArray(swBaudRates),
        []() { return (marcSerial1Baud = swBaudRateToIndex(preferences.getInt(PREFERENCE_MARCSERIAL1_BAUD, MARC_SERIAL1_BAUD_RATE))); },
        [](int val) { marcSerial1Baud = swBaudRatesInt[val]; }),
    WVerticalAlign(),
    WSelect("Serial2 Baud Rate", "serial2baud",
        swBaudRates, SizeOfArray(swBaudRates),
        []() { return (marcSerial2Baud = swBaudRateToIndex(preferences.getInt(PREFERENCE_MARCSERIAL2_BAUD, MARC_SERIAL2_BAUD_RATE))); },
        [](int val) { marcSerial2Baud = swBaudRatesInt[val]; }),
    WVerticalAlign(),
    WCheckbox("Serial1 RX pass-through to Serial1 TX", "serial1pass",
        []() { return (marcSerial1Pass = (preferences.getBool(PREFERENCE_MARCSERIAL1_PASS, MARC_SERIAL1_PASS))); },
        [](bool val) { marcSerial1Pass = val; } ),
    WVerticalAlign(),
    WCheckbox("Serial2 RX pass-through to Serial2 TX", "serial2pass",
        []() { return (marcSerial2Pass = (preferences.getBool(PREFERENCE_MARCSERIAL2_PASS, MARC_SERIAL2_PASS))); },
        [](bool val) { marcSerial2Pass = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite on Serial1", "serial1",
        []() { return (marcSerial1Enabled = (preferences.getBool(PREFERENCE_MARCSERIAL1_ENABLED, MARC_SERIAL1_ENABLED))); },
        [](bool val) { marcSerial1Enabled = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite on Serial2", "serial2",
        []() { return (marcSerial2Enabled = (preferences.getBool(PREFERENCE_MARCSERIAL2_ENABLED, MARC_SERIAL2_ENABLED))); },
        [](bool val) { marcSerial2Enabled = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite on Wifi (port 2000)", "wifienabled",
        []() { return (marcWifiEnabled = (preferences.getBool(PREFERENCE_MARCWIFI_ENABLED, MARC_WIFI_ENABLED))); },
        [](bool val) { marcWifiEnabled = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite Wifi pass-through to Serial1", "wifipass1",
        []() { return (marcWifiSerial1Pass = (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL1_PASS, MARC_WIFI_SERIAL1_PASS))); },
        [](bool val) { marcWifiSerial1Pass = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite Wifi pass-through to Serial2", "wifipass2",
        []() { return (marcWifiSerial2Pass = (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL2_PASS, MARC_WIFI_SERIAL2_PASS))); },
        [](bool val) { marcWifiSerial2Pass = val; } ),
    WVerticalAlign(),
    WButton("Save", "save", []() {
        preferences.putInt(PREFERENCE_MARCSERIAL1_BAUD, marcSerial1Baud);
        preferences.putInt(PREFERENCE_MARCSERIAL2_BAUD, marcSerial2Baud);
        preferences.putBool(PREFERENCE_MARCSERIAL1_PASS, marcSerial1Pass);
        preferences.putBool(PREFERENCE_MARCSERIAL1_ENABLED, marcSerial1Enabled);
        preferences.putBool(PREFERENCE_MARCSERIAL2_PASS, marcSerial2Pass);
        preferences.putBool(PREFERENCE_MARCSERIAL2_ENABLED, marcSerial2Enabled);
        preferences.putBool(PREFERENCE_MARCWIFI_ENABLED, marcWifiEnabled);
        preferences.putBool(PREFERENCE_MARCWIFI_SERIAL1_PASS, marcWifiSerial1Pass);
        preferences.putBool(PREFERENCE_MARCWIFI_SERIAL2_PASS, marcWifiSerial2Pass);
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

String wifiSSID;
String wifiPass;
bool wifiAP;

WElement wifiContents[] = {
    W1("WiFi Setup"),
    WCheckbox("WiFi Enabled", "enabled",
        []() { return wifiEnabled; },
        [](bool val) { wifiEnabled = val; } ),
    WHR(),
    WCheckbox("Access Point", "apmode",
        []() { return (wifiAP = preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT)); },
        [](bool val) { wifiAP = val; } ),
    WTextField("WiFi:", "wifi",
        []()->String { return (wifiSSID = preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME)); },
        [](String val) { wifiSSID = val; } ),
    WPassword("Password:", "password",
        []()->String { return (wifiPass = preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE)); },
        [](String val) { wifiPass = val; } ),
    WHR(),
    WButton("Save", "save", []() {
        DEBUG_PRINTLN("WiFi Changed");
        preferences.putBool(PREFERENCE_WIFI_ENABLED, wifiEnabled);
        preferences.putBool(PREFERENCE_WIFI_AP, wifiAP);
        preferences.putString(PREFERENCE_WIFI_SSID, wifiSSID);
        preferences.putString(PREFERENCE_WIFI_PASS, wifiPass);
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

WElement firmwareContents[] = {
    W1("Firmware Setup"),
    WFirmwareFile("Firmware:", "firmware"),
    WFirmwareUpload("Reflash", "firmware"),
    WLabel("Current Firmware Build Date:", "label"),
    WLabel(__DATE__, "date"),
#ifdef BUILD_VERSION
    WHRef(BUILD_VERSION, "Sources"),
#endif
    WButton("Clear Prefs", "clear", []() {
        DEBUG_PRINTLN("Clear all preference settings");
        preferences.clear();
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Reboot", "reboot", []() {
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

//////////////////////////////////////////////////////////////////

WPage pages[] = {
    WPage("/", mainContents, SizeOfArray(mainContents)),
      WPage("/dome", domeContents, SizeOfArray(domeContents)),
    WPage("/setup", setupContents, SizeOfArray(setupContents)),
      WPage("/serial", serialContents, SizeOfArray(serialContents)),
      WPage("/wifi", wifiContents, SizeOfArray(wifiContents)),
      WPage("/firmware", firmwareContents, SizeOfArray(firmwareContents)),
        WUpload("/upload/firmware",
            [](Client& client)
            {
                if (Update.hasError())
                    client.println("HTTP/1.0 200 FAIL");
                else
                    client.println("HTTP/1.0 200 OK");
                client.println("Content-type:text/html");
                client.println("Vary: Accept-Encoding");
                client.println();
                client.println();
                client.stop();
                if (!Update.hasError())
                {
                    delay(1000);
                    preferences.end();
                    ESP.restart();
                }
                otaInProgress = false;
            },
            [](WUploader& upload)
            {
                if (upload.status == UPLOAD_FILE_START)
                {
                    otaInProgress = true;
                    Serial.printf("Update: %s\n", upload.filename.c_str());
                    if (!Update.begin(upload.fileSize))
                    {
                        //start with max available size
                        Update.printError(Serial);
                    }
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    float range = (float)upload.receivedSize / (float)upload.fileSize;
                    DEBUG_PRINTLN("Received: "+String(range*100)+"%");
                   /* flashing firmware to ESP*/
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    {
                        Update.printError(Serial);
                    }
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    DEBUG_PRINTLN("GAME OVER");
                    if (Update.end(true))
                    {
                        //true to set the size to the current progress
                        Serial.printf("Update Success: %u\nRebooting...\n", upload.receivedSize);
                    }
                    else
                    {
                        Update.printError(Serial);
                    }
                }
            })
};

WifiWebServer<10,SizeOfArray(pages)> webServer(pages, wifiAccess);
#endif
