# WebHost Esp32

This is a repository for testing the esp_http_server from the Espressif library.

## Description

The idea is to create an access point in AP mode. When someone connects to the ESP's Wi-Fi, the initial access will lead to an HTTP page. On this page, there are two small boxes where you can input an RSSI and Wi-Fi password. After pressing the submit button, the ESP will switch to STA mode and connect to the modem.

I hope that example can be helpful for your own project.