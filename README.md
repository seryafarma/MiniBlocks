# MiniBlocks
Mini display of 8 x 16 leds together with DHT11 as a simple Firebase Client to Get and Set data.

![Firebase Logo](https://www.gstatic.com/devrel-devsite/prod/vd277a93d7226f1fcf53372e6780919bb823bca6ca1c3adbaa8a14ef6554ad67d/firebase/images/lockup.svg)

Another client on an Android phone is then used to add the value to the database, indirectly controlling the display of the Led Matrix.

## Hardware
- The hardware used in this project is the ESP8266 Node MCU v3 from AZ Delivery: https://www.az-delivery.de/en/products/nodemcu-lolin-v3-modul-mit-esp8266
- This is connected to low cost DHT11 sensor.
- This is also connected to two sets of MAX7219 from ICStation: http://www.icstation.com/icstation-max7219-light-lamp-display-matrix-module-panel-kits-p-2609.html this results in the 8 x 16 leds array.
- The hardware is cased with simple lego cases to protect and give style to the ESP boards 😊

## Software
- A simple get and set through firebase real time database project utilizing firebase ESP client https://github.com/mobizt/Firebase-ESP-Client.
- Wide font for MAX7219 from Max: https://github.com/maxbanton/MD_Parola_Fonts/
- DHT11 DF Robot https://github.com/DFRobot/DFRobot_DHT11
- and of course the magical Majic Design MD Parola Library 🎨! https://github.com/MajicDesigns/MD_Parola
