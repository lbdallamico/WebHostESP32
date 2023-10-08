set(FILE_COMUNI
        <src/comuni/wifi/WiFi.cpp>
        <src/comuni/web_server/WebServer.cpp>
)


idf_component_register(
    SRCS 
        ${FILE_COMUNI}
        "src/main.cpp"
    INCLUDE_DIRS
        "src/comuni/wifi/"
        "src/comuni/web_server/"
    REQUIRES
        esp_http_client
        esp_http_server
        esp32
        esp_netif
)