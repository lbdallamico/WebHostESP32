# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

# INICIO
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(ECP $ENV{ESP_COMPONENTS_PATH})
set(EXTRA_COMPONENT_DIRS "${EXTRA_COMPONENT_DIRS} ${ECP}")

#set(COMPONENTS esptool_py esp_wifi esp_netif esp32 esp_http_client arduino)

# FIM
project(main)