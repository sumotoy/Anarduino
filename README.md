# Anarduino
An addon for Arduino IDE to better program Anarduino boards.
This has mainly designed to simplify coding on Anarduino boards (www.anarduino.com). The <b>Anarduino</b> folder should be placed in Documents->Arduino->Hardware<br>
then restart ide. You should have a new option on arduino ide board menu for Anarduino 16Mh/3.3v.<br>
There's 3 options for 'Processor' menu:<br>
 - ATmega328_Standard:This is the board as it come from anarduino.com
 - ATmega328_Optimod:In case you reflashed bootloader with Optimod, CAUTION! Never tested<br>
 - ATmega328_DualOptiboot: Similar to Moteino (this will be added shortly).
 
Everithing come with NO WARRANTIES, it's just an early experiment, you can contribute but please it's no time for bugs or whatever.<br>

Note about Anarduino board.<br>
This board come with a standard ATmega328p CPU plus a 128M Flash SPI chip, an RTC chip from Microchip MCP7940 and a RFM radio transceiver module plus a little LDO for powering everithing at 3V3.<br>
Other differences are 8 analog inputs and led connected to pin 9 instead 13.<br>
The first potential problem it's working at 3v3, the ATmega 328 has been designed 'officially' to work at 16Mhz only at 5V and 8Mhz it's fine for 3V3, but here it's working at 16Mhz, this mean there's no warranties everithing should work as expected!<br>
Flashing a new bootloader can be tricky, the PIN 10 seems pulldown internally and it's not good when you try to inject a new bootloader, a 47K resistor from +3v3 and pin 10 it's necessary only for burn another bootloader!<br>
I got a couple of modules that react as the bootloader it's not present at all, this was not true at the end, I'm not sure why this happened bu reflashing a new bootloader seems solved he problem.<br>
This module has several devices sharing the same SPI bus, the RFM module and the SPI flash. In this situation it's raccomandable using SPI libraries that are fully SPI transaction compatible!<br>
Actually, all the RFM libraries are officially 'not', just some variants, I will try to work on this in the near future at list with the 2 main libraries needed for SPI Flash and Radio Transceiver.<br>
This is a list of the pins shared internally:<br>

 - Pin 2:  RFM transceiver INT (connected to INT 0) pullup (should always be used as IN)
 - Pin 3:  SQW from RTC chip, pullup (should always be used as IN)
 - Pin 5:  SPI flash 128M SS (should always be used as OUT and cannot be shared)
 - Pin 6:  Connected to RFM DIO2 (this is missed from anarduino schematics but connected!) <-----!!!
 - Pin 9:  LED
 - Pin 10: RFM transceiver SS (should always be used as OUT and cannot be shared if RFM module soldered)
 - Pin 11: MOSI shared between RFM and SPI flash, shareable with other devices
 - Pin 12: MISO shared between RFM and SPI flash, shareable with other devices
 - Pin 13: SCLK shared between RFM and SPI flash, shareable with other devices
 - Pin A4: SDA (pullup) shared with RTC chip, shareable with other devices.
 - Pin A5: SCL shared with RTC chip, shareable with other devices.<br>
 
 I will solder a 22K pullup resistor to Pin 10 and to Pin 5, this prevent devices to interfere each other when SPI is used, the guys at anarduino.com should update their hardware design and include that!<br>
 I don't understand why they pullup only SDA and not SCL!<br>

