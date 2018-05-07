# Arduino/ESP8266/ESP32 OpenTherm Controller

# Usage

It uses serial commands.

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
- 1-Control CH setpoint
- 18-CH water pressure
- 25-Boiler flow water temperature
- 26-DHW temperature
- 27-Outside temperature
- 56-DHW setpoint

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
