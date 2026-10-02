#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"

#define EXAMPLE_MAX_CHAR_SIZE    64
#define CONFIG_EXAMPLE_PIN_CLK 39
#define CONFIG_EXAMPLE_PIN_CMD 38
#define CONFIG_EXAMPLE_PIN_D0 40

#define MOUNT_POINT "/sdcard"
sdmmc_card_t *card;

// mount function
esp_err_t mount_handler();

// un mount function
esp_err_t unmount_handler();