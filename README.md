# penguin ESP32

## Dependencies

- ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
- AsyncTCP: https://github.com/me-no-dev/AsyncTCP


You need to switch to `idf-update` branch in AsyncTCP, or you may see the following errors when compiling:

```
Compiling library "AsyncTCP"
"/home/cellie/Arduino/hardware/espressif/esp32/tools/xtensa-esp32-elf/bin/xtensa-esp32-elf-g++" -DESP_PLATFORM -DMBEDTLS_CONFIG_FILE="mbedtls/esp_config.h" ...
/home/cellie/Arduino/libraries/AsyncTCP/src/AsyncTCP.cpp:195:27: error: field 'call' has incomplete type 'tcpip_api_call'
     struct tcpip_api_call call;
                           ^
...
```

( see https://github.com/me-no-dev/AsyncTCP/issues/19 )


## Functionalities

- Connect to WIFI "penguinX" with static IP address 172.24.1.2
- Run async web server on port 80
- Answers to URLs: FD, ST, BK, ang
- Include ACAO header to allow controlling from penguinBlockly

