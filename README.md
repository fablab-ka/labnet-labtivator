# LabNet Labtivator

a small library to enable esp8266 wemos D1 with a custom shield to connect to MQTT and trigger actions as well as receive feedback.

## MQTT Protokoll

### >>> Outgoing

```
/FLKA/labtivators/discover
```
* description: is sent on connect, payload contains the mac of the newly connected labtivator
* payload type: string

```
/FLKA/labtivators/cmd/button1
/FLKA/labtivators/cmd/button2
```
* description: is sent on button press
* payload type: empty

```
/FLKA/labtivators/stat/pong
```
* description: is sent on ping message
* payload type: empty

### <<< Incoming

```
/FLKA/labtivators/<mac>/cmd/setbutton1color
/FLKA/labtivators/<mac>/cmd/setbutton2color
```
* description: change the LED color of the button
* values: 'red', 'green', 'black'/any
* ​​​​​​​type: string

```
/FLKA/labtivators/<mac>/cmd/ping
```
* description: produces a pong message. Used for health check
