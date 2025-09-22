# Panasonic AC IR Remote Controller for ESPHome

This is a custom ESPHome external component for controlling Panasonic air conditioners via infrared.

It **inherits from ESPHome's `ClimateIR`** and adds support for:
- **IR Receiver** to detect and decode Panasonic AC remote commands (216 bit frame)
- **Temperature step** (0.5 or 1.0 degree)
- **Fan Level Control** (1–5, plus quiet)
    - Default 3 levels without quiet, can configure 5 levels and quiet separately
- **Swing Control**:
  - **Vertical Swing** (Highest, High, Middle, Low, Lowest, Auto)
  - **Horizontal Swing** (Left Max, Left, Middle, Right, Right Max, Auto)
    - Default off, can configure on.

---

## 🛠 Features

- ✅ Compatible with Panasonic IR protocol (216 bit - 27 bytes frame)
- ✅ Based on ESPHome `ClimateIR` class for climate control
- ✅ IR receiver support to sync state from physical remote
- ✅ `select` components for:
  - Fan level 
  - Swing vertical
  - Swing horizontal
- ✅ Auto state updates when IR signal is received

---

## 📦 Requirements

- ESP32 or ESP8266 board
You have 2 options to install ESPHome module to you AC.

- 1. Invasive way: You must have physical access to Panasonic AC IR board
    - Wiring Panasonic AC IR board with ESP board
        - ESP GND <-> Pana AC IR board's GND
        - ESP 3V3 <-> Pana AC IR board's VCC
        - ESP GPIO <--> Pana AC IR board's IR Led output
- 2. Non-invasive way: If you don't want to mod your AC IR board (invasive way), you have choice to do as below
    - Make your own IR led receiver and IR led transmitter circuit (via transistor). Schematic is everywhere on Internet.
    - Connect IR led receiver circuit and IR led transmitter circuit to ESP GPIOs.
    - Configure ESPHome yaml `remote_receiver` and `remote_transmitter` with respective GPIOs.
    - One important thing is, you have to install your ESP8266 module near the the AC indoor unit:
        - It has to be able to receive signal from physical remote same as AC unit.
        - Its IR transmitter led has to point to the AC IR module, so the command sends from it can come to the AC.
    - This way, you have to additionally configure the `panaac` climate with `ir_control: True`
- NOTE
    - Method 1 brings much more stability and performance, as it captures the IR signal and feeds the control command directly to AC IR board.
    - Recommendation: during testing phase, you can use method 2, but for long-term usage it's highly recommend to install it as method 1.


---

## 📂 Installation

1. Copy `esphome/components/panaac` folder to your HASS ESPHome `components` folder.
2. Add climate component to your ESPHome YAML configuration (refer example `ac-test.yaml`)

    ```yaml
    climate:
    - platform: panaac
        name: "Remote Controller"
        receiver_id: ir_receiver
        supports_heat: True
        supports_quiet: True
        fan_5level: True
        swing_horizontal: True
        temp_step: 0.5
        ir_control: False
    ```

---

## 🖼️ Screenshots

Here is how the component appears in Home Assistant:

![Panasonic AC in Home Assistant](assets/screenshot_panaac.png)
![Panasonic AC Climate control in Home Assistant](assets/screenshot_panaac_climate.png)

