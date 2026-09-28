# ESP-01 MQTT LED Control

Turn an LED on and off over Wi-Fi by sending MQTT messages to an ESP-01 (ESP8266) module.

This is the first version (v1) of the device code I wrote for a PNRR-funded IoT prototype in high school. It is a function test: it checks that a device can receive commands from an MQTT broker and report its status back. The rest of that project belongs to the school and is not included here.

## How it works

```
publisher  ── led/control ──►  Mosquitto broker  ── led/control ──►  ESP-01
subscriber ◄── led/status ───  Mosquitto broker  ◄── led/status ───  ESP-01
```

1. The ESP-01 connects to Wi-Fi. If the primary network fails, it tries a backup network.
2. It connects to the MQTT broker and subscribes to `led/control`.
3. When it receives `{"message": "1"}` or `{"message": "0"}`, it turns the LED on or off and publishes the new state on `led/status`.

The built-in LED blinks while the module is connecting to Wi-Fi and to the broker. If the broker connection drops, the module tries to reconnect every 5 seconds.

## Hardware

- ESP-01 or ESP-01S module (ESP8266); the code uses the built-in LED on GPIO2
- USB-to-serial adapter for flashing, **3.3 V only** (5 V damages the module)
- A machine running a Mosquitto broker on the same network

## Software

- Arduino IDE with the ESP8266 board package (board: *Generic ESP8266 Module*)
- Libraries:
  - [PubSubClient](https://github.com/knolleary/pubsubclient)
  - [ArduinoJson](https://arduinojson.org/) (the code uses the version 6 API)

## Repository structure

```
esp01-mqtt-led-control/
├── README.md
├── .gitignore                  # keeps secrets.h out of the repository
├── docs/
│   ├── mosquitto-debian-setup.md
│   └── images/
└── v1_esp01_led_control/
    ├── v1_esp01_led_control.ino
    └── secrets.example.h
```

## Setup

### 1. Set up the broker

Follow the [Mosquitto installation guide](docs/mosquitto-debian-setup.md) to install and test the broker on Debian.

Since Mosquitto 2.0, the broker only accepts connections from the machine it runs on. The ESP-01 connects over the network without a username or password, so the broker needs a configuration file such as `/etc/mosquitto/conf.d/local.conf`:

```
listener 1883
allow_anonymous true
```

Then restart the broker:

```bash
sudo systemctl restart mosquitto
```

Only use this configuration on a trusted local network.

### 2. Add your credentials

Copy the example file and fill in your Wi-Fi networks and the broker's IP address:

```bash
cp v1_esp01_led_control/secrets.example.h v1_esp01_led_control/secrets.h
```

`secrets.h` is listed in `.gitignore`, so your Wi-Fi passwords never end up on GitHub.

### 3. Flash the ESP-01

1. Open `v1_esp01_led_control.ino` in the Arduino IDE and select *Generic ESP8266 Module*.
2. Put the module in flashing mode: connect GPIO0 to GND while powering it on.
3. Upload the sketch, then disconnect GPIO0 from GND and restart the module.
4. Open the Serial Monitor at 115200 baud to follow the connection steps.

## Usage

Watch the status messages in one terminal:

```bash
mosquitto_sub -h <broker-ip> -t led/status
```

Send commands from another terminal:

```bash
mosquitto_pub -h <broker-ip> -t led/control -m '{"message": "1"}'   # LED on
mosquitto_pub -h <broker-ip> -t led/control -m '{"message": "0"}'   # LED off
```

The subscriber shows `Hello from ESP-01` when the module connects, then `LED is ON` or `LED is OFF` after each command.

## Known limitations (v1)

- The value must be a string: `{"message": "1"}` works, while `{"message": 1}` is reported as a missing key.
- If both Wi-Fi networks fail at startup, the code does not check Wi-Fi again and keeps retrying the broker.
- The MQTT connection has no username, password or TLS, so it is only suitable for a trusted local network.
# esp01-mqtt-led-control
