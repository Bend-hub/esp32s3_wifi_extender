idf_component_register(SRCS "sd_card.c" "wifi_extender.c" "scan.c"
                    PRIV_REQUIRES esp_wifi nvs_flash
                    INCLUDE_DIRS ".")
