Hardware Used:
- 6x [IN-17 Nixie Tube](https://www.swissnixie.com/tubes/IN17/)
- [UNO R3 + WIFI ESP8266 + CH340G Arduino and WIFI](https://www.instructables.com/UNO-R3-WIFI-ESP8266-CH340G-Arduino-and-WIFI-a-Vers/)
- [Arduino IN-17 Nixie Shield](https://www.tindie.com/products/florinc/arduino-nixie-shield-kit-for-six-in-17-tubes/)
- [Polycase AN-12P Diecast Aluminum NEMA Enclosure](https://www.polycase.com/an-12p#AN-12P-02)
- [Female Barrel Socket Panel Mount](https://www.amazon.com/dp/B07C46XMPT)

The ATMega runs the clock firmware and takes simple commands via serial from the ESP8266, which is managed using ESPHome. Below is a table of the serial commands used by the ESPHome component to control the ATMega component.

 | Serial Command | Description |
--- | ---
0 | Turns off display
1 | Turns on display
C | Run a longer cathode cycle routine
THHMMSS | Set ATMega internal RTC time to HH:MM:SS
MXXXXXX | Display value XXXXXX for a few seconds, use '_' to blank digit

TODO: pictures/videos
