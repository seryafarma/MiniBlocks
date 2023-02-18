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

#define DHT11_PIN 16

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
bool anon_sign_up = false;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

//---------------------------------------------------------------------------------------------------------------------
// Functions Declaration
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
// Setup and Loop
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
void connect_firebase()
{
    config.api_key = Authentication::FB_WEB_API_KEY;
    config.database_url = Authentication::FB_RTDB_URL;
    // Anonymous access, sign up everytime.
    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        anon_sign_up = true;
    }
    else
    {
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
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

String things_to_show = "";

//---------------------------------------------------------------------------------------------------------------------
void loop()
{
    static textEffect_t text_effect = PA_PRINT;
    if (!anon_sign_up)
    {
        return;
    }
    if (P.displayAnimate())
    {
        // Done displaying, let's check firebase.
        if (Firebase.ready())
        {
            if (Firebase.RTDB.getString(&fbdo, "/test/display/one"))
            {
                things_to_show = fbdo.stringData();
                Serial.println(things_to_show);
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }

            if (Firebase.RTDB.getString(&fbdo, "/test/display/animate"))
            {
                String on = fbdo.stringData();
                if (on == "true")
                {
                    text_effect = PA_SCROLL_LEFT;
                }
                else
                {
                    text_effect = PA_PRINT;
                }
                Serial.println(things_to_show);
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
        }

        // Nothing pending, redraw.
        P.displayText(things_to_show.c_str(), PA_LEFT, 50, 50, text_effect);
    }
    delay(STATE_DELAY_MS * 10);
}