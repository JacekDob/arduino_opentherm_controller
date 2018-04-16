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

- 0-Status - CH - 256, DHW - 512, CH&DHW - 768
- 1-Control setpoint - setting works
- 17-Relative Modulation Level - reading works
- 25-Boiler flow water temperature - reading works
- 26-DHW temperature - reading works
- 27-Outside temperature - should work, but I have disconnected outside sensor
- 56-DHW setpoint - works
- 57-Max CH water setpoint - works - shows 80

Some other also works.

## Remarks

### MCU

I am using Wemos D1 mini with pins for IN:D8; OUT:D7 but it also should work on Arduino or ESP32.

### Boiler

I have Hajdu HGk-28.

### Discussion

Some discussion on: 
http://ihormelnyk.com/page/arduino_opentherm_controller

# Contributions

Contributions are welcome. Please open issues and/or file Pull Requests.
