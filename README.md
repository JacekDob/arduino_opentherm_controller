# arduino_opentherm_controller
Arduino OpenTherm Controller

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

## Remarks

### MCU

I am using ESP32 but it also should work on Arduino or ESP8266.

### Boiler

I have Vitodens 100-W.
So far I couldn't emulate thermostat to control CH (Central Heating) and DHW (Domestic How Water).

# Contributions

Contributions are welcome. Please open issues and/or file Pull Requests.