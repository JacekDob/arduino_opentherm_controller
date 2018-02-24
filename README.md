# Arduino/ESP8266/ESP32 OpenTherm Controller

Based on:
http://ihormelnyk.com/Page/arduino_opentherm_controller

# Usage

It uses serial commands.

## Initialization
```
init
```

## Read

```
OpenthermRead [id]
```
or shortly
```
r [id]
```

There is one exception for id=0 status message, which supports also parameter
```
r 0 3
```

## Write

```
OpenthermWrite [id] [value]
```
or shortly
```
w [id] [value]
```
## What works

- 0-Status - but I am not sure what should master provide: currently enabled mode or generally enabled features?
- 1-Control setpoint - setting works
- 17-Relative Modulation Level - reading works
- 25-Boiler flow water temperature - reading works
- 26-DHW temperature - reading works
- 27-Outside temperature - should work, but I have disconnected outside sensor
- 56-DHW setpoint - works strangly: untill sending 0-Status message it shows currently set DHW setpoint and updates when changing hardware potentiometer position, but: 1. after writing different value it does not show it when reading. 2. after sending 0-Status with either 0 or 3 it shows 5.0
- 57-Max CH water setpoint - works - shows 80

Some other also works.

## Remarks

### MCU

I am using ESP32 but it also should work on Arduino or ESP8266.

### Boiler

I have Vitodens 100-W.
So far I couldn't emulate thermostat to control CH (Central Heating) and DHW (Domestic How Water).

### Discussion

Some discussion on: 
http://ihormelnyk.com/page/arduino_opentherm_controller

# Contributions

Contributions are welcome. Please open issues and/or file Pull Requests.