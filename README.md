# Prayer Displayer
The Prayer Displayer is a simple program that keeps track of your prayers and displays the next prayer depending on the time of the day.
To get the prayers timings, it uses a simple mobile app (still in development) that sends the timings to the device via BLE. 
The app itself is a simple Flutter app that makes a request to an API to get the timings and then sends them to the device via BLE which then process them to determine the next prayer. 

Note: For now, it is capable of running on its own for the rest of the month after the timings have been sent. After that, the timings will need to be sent again.

## Hardware
- ESP32 device (esp32dev)
- Small OLED display
  - We are using a 128x64 SSD1306 OLED display

