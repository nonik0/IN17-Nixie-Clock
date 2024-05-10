# IN-17 Nixie Clock

## Hardware Used:
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
h | Set 12 hour mode
H | Set 24 hour mode
C | Run a longer cathode cycle routine
THHMMSS | Set ATMega internal RTC time to HH:MM:SS
MXXXXXX | Display value XXXXXX for a few seconds, use '_' to blank digit

## Images

<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/8ab2aef9-2258-4203-8cac-730cf0cc542c"/>
<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/5d3902fc-4cd1-4daf-bb4e-e1d2a2e445a9"/>
<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/4c3f7974-5dd1-4624-a640-3a930930a903"/>
<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/90a4e563-919f-4cfd-9d99-a90d332aa6af"/>
<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/2934a52e-2af5-44b2-97c2-b1b981ed6258"/>

## Animations

<img width="400" src="https://github.com/nonik0/IN17-Nixie-Clock/assets/17152317/bf8bf09e-7777-48de-a668-0451fec2d1b1"/>
