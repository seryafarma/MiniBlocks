//---------------------------------------------------------------------------------------------------------------------
// main.cpp
// Dual 8x8 style LED Matrix connected with Firebase and DHT11.
// Style: Procedural
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <DFRobot_DHT11.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>

//---------------------------------------------------------------------------------------------------------------------
// Local Includes
//---------------------------------------------------------------------------------------------------------------------
#include "auth.hpp"
#include "wide_digits.hpp"

//---------------------------------------------------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------------------------------------------------

// Displays.
#define HARDWARE_TYPE  MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES    2
#define CLK_PIN        5
#define DATA_PIN       7
#define CS_PIN         15
#define STATE_DELAY_MS 50
#define DHT11_PIN      D0

//---------------------------------------------------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------------------------------------------------
DFRobot_DHT11 DHT;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

String things_to_show = "";
bool animate = false;

#define PATH_TO_CHECK  "miniblock/display/" // animate and value.
#define PATH_ANIMATION "animate"
#define PATH_NUMBER    "value"

//---------------------------------------------------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
// Setup and Loop
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
void firebase_timeout(bool timeout)
{
    if (timeout)
    {
        Serial.println("stream timeout, resuming...\n");
    }
    else if (!fbdo.httpConnected())
    {
        Serial.printf("Error code: %d, reason: %s\n\n", fbdo.httpCode(), fbdo.errorReason().c_str());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void firebase_stream_cb(FirebaseStream data)
{
    Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n", data.streamPath().c_str(),
                  data.dataPath().c_str(), data.dataType().c_str(), data.eventType().c_str());
    printResult(data);
    Serial.println();

    // Get the path that triggered the function
    String path = String(data.dataPath());

    // // if the data returned is an integer, there was a change on the GPIO state on the following path /{gpio_number}
    // if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer)
    // {
    //     String gpio = path.substring(1);
    //     int state = data.intData();
    //     Serial.print("GPIO: ");
    //     Serial.println(gpio);
    //     Serial.print("STATE: ");
    //     Serial.println(state);
    //     digitalWrite(gpio.toInt(), state);
    // }

    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_string)
    {
        Serial.println("New string data");
        String new_data = data.to<String>();
        Serial.println(new_data);
        Serial.println("path");
        Serial.println(path.c_str());

        if (path.indexOf("value") > 0)
        {
            things_to_show = new_data;
        }
        else if (path.indexOf("animate") > 0)
        {
            animate = (new_data == "true");
        }
    }

    /* When it first runs, it is triggered on the root (/) path and returns a JSON with all keys
    and values of that path. So, we can get all values from the database and updated the GPIO states*/
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json)
    {
        FirebaseJson json = data.to<FirebaseJson>();

        // To iterate all values in Json object
        size_t count = json.iteratorBegin();
        Serial.println("\n---------");
        for (size_t i = 0; i < count; i++)
        {
            FirebaseJson::IteratorValue value = json.valueAt(i);

            String name = value.key.c_str();
            if (name == "value")
            {
                things_to_show = value.value;
            }
            else if (name == "animate")
            {
                animate = (value.value.indexOf("true") > 0);
            }

            Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(),
                          value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
        }
        Serial.println();
        json.iteratorEnd(); // required for free the used memory in iteration (node data collection)
    }

    // This is the size of stream payload received (current and max value)
    // Max payload size is the payload size under the stream path since the stream connected
    // and read once and will not update until stream reconnection takes place.
    // This max value will be zero as no payload received in case of ESP8266 which
    // BearSSL reserved Rx buffer size is less than the actual stream payload.
    Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

//---------------------------------------------------------------------------------------------------------------------
void connect_firebase()
{
    config.api_key = Authentication::FB_WEB_API_KEY;
    config.database_url = Authentication::FB_RTDB_URL;

    auth.user.email = Authentication::FB_EMAIL;
    auth.user.password = Authentication::FB_PASS;

    // Sign in.
    Firebase.begin(&config, &auth);

    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    if (!Firebase.RTDB.beginStream(&fbdo, PATH_TO_CHECK))
    {
        Serial.print("Error: ");
        Serial.println(fbdo.errorReason().c_str());
    }

    // Callback when fbdo changes.
    Firebase.RTDB.setStreamCallback(&fbdo, firebase_stream_cb, firebase_timeout);
}

//---------------------------------------------------------------------------------------------------------------------
void connect_wifi()
{
    WiFi.begin(Authentication::WIFI_SSID, Authentication::WIFI_PASSWORD);
    Serial.print("Connecting to ");
    Serial.println(Authentication::WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.println(WiFi.localIP());
    Serial.println();

    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

//---------------------------------------------------------------------------------------------------------------------
void setup()
{
    Serial.begin(9600);
    P.begin();
    delay(1500);
    connect_wifi();
    connect_firebase();

    P.setFont(WideDigits);
    P.setCharSpacing(2);
}

//---------------------------------------------------------------------------------------------------------------------
void loop()
{
    if (P.displayAnimate())
    {
        // store things_to_show...
        if (Firebase.isTokenExpired())
        {
            Firebase.refreshToken(&config);
            Serial.println("Refresh token");
        }
        textEffect_t text_effect = animate ? PA_SCROLL_LEFT : PA_PRINT;
        // Nothing pending, redraw.
        P.displayText(things_to_show.c_str(), PA_LEFT, 50, 50, text_effect, PA_DISSOLVE);

        // add a small delay if not animated.
        if (!animate)
        {
            delay(800);
        }
    }
    delay(STATE_DELAY_MS * 5);
}
