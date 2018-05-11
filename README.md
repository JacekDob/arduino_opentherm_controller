# Arduino/ESP8266/ESP32 OpenTherm Controller

<p align="center">
  <img src="/Wemos D1 shield/OpenTherm_D1_shield.png" width=80%/>
</p>

# Usage

It uses serial commands.
First you have to initialize. After initialization, the code will perform predefined readings to keep the opentherm connection active, meanwhile watching and executing commands from the serial.
OpenTherm protocoll is defined to activate CH when no valid message arrives within 4 seconds to the boiler. My boiler does this after one second.

## Initialization
```
init
```

## Read
```
r [id]
```

There is one exception for id=0 status message, which supports also parameter
```
r 0 3
```

## Write
```
w [id] [value]
```
## What works

- 0-Status - CH - 256, DHW - 512, CH&DHW - 768
- w 1 55 -Control CH setpoint (to 55 °C)
- r 18 -CH water pressure
- r 25 -Boiler flow water temperature
- r 26 -DHW temperature
- r 27 -Outside temperature
- w 56 40 -DHW setpoint (to 40 °C)

see my video:
https://www.youtube.com/watch?v=Xk3SmCZ2tsc

## Remarks

- CH water pressure sometimes unstable reading (may be caused by CH pump start?)

### MCU

I am using Wemos D1 mini with pins for IN:D8; OUT:D7 but it also should work on Arduino or ESP32.

### Boiler

I have Hajdu HGK-28.

### Discussion

Some discussion on: 
http://ihormelnyk.com/page/arduino_opentherm_controller

# Contributions

Contributions are welcome. Please open issues and/or file Pull Requests.
