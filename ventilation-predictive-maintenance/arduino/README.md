# Aidia Seco air flow and pollution detection

## Communcation settings

Connection: Serial
Baudrate: 9600
EOL: \r\n

## Commands

| Command | Response               | Notes                       |
| ------- | ---------------------- | --------------------------- |
| hello   | ready                  | Just for testing connection |
| f       | flow: [0.0-1.0]        |                             |
| q       | pollution: [0.0-93.64] | Value in mg/m3              |

; PlatformIO Project Configuration File
;
; Build options: build flags, source filter
; Upload options: custom upload port, speed and extra flags
; Library options: dependencies, extra library storages
; Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[env:nanoatmega328new]
platform = atmelavr
board = nanoatmega328new
framework = arduino
