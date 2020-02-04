/* SPIFFS Image Generation on Build Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "mbedtls/md5.h"

const char *TAG = "example";

static void read_hello_txt(void)
{
	ESP_LOGI(TAG, "Reading test.bmp");

	// Open for reading hello.txt
	FILE* f = fopen("/spiffs/test.bmp", "r");

	
printf("Free HEAP: %d\n", esp_get_free_heap_size());
	
	fseek(f, 0, SEEK_END); 
	long lSize = ftell(f);
	printf("%ld\n", lSize);
	
	rewind(f); 
	char * buffer = (char*) malloc(sizeof(char) * 153618);

	printf("Free HEAP: %d\n", esp_get_free_heap_size());
	ESP_LOGE(TAG, "malloc!");
	fread(buffer, 1, 153618, f);
	for (uint32_t i = 0; i < 100; i++)
		printf("%02X", buffer[i]);
	//содержимое файла теперь находится в буфере
	//puts(buffer);
 
	// завершение работы
	fclose(f);
	free(buffer);
}

void compute_alice_txt_md5(void)
{
    ESP_LOGI(TAG, "Computing alice.txt MD5 hash");

    // The file alice.txt lives under a subdirectory, though SPIFFS itself is flat
    FILE* f = fopen("/spiffs/sub/alice.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open alice.txt");
        return;
    }

    // Read file and compute the digest chunk by chunk
    #define MD5_MAX_LEN 16

    char buf[64];
    mbedtls_md5_context ctx;
    unsigned char digest[MD5_MAX_LEN];

    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts_ret(&ctx);

    size_t read;

    do {
        read = fread((void*) buf, 1, sizeof(buf), f);
        mbedtls_md5_update_ret(&ctx, (unsigned const char*) buf, read);
    } while(read == sizeof(buf));

    mbedtls_md5_finish_ret(&ctx, digest);

    // Create a string of the digest
    char digest_str[MD5_MAX_LEN * 2];

    for (int i = 0; i < MD5_MAX_LEN; i++) {
        sprintf(&digest_str[i * 2], "%02x", (unsigned int)digest[i]);
    }

    // For reference, MD5 should be deeb71f585cbb3ae5f7976d5127faf2a
    ESP_LOGI(TAG, "Computed MD5 hash of alice.txt: %s", digest_str);

    fclose(f);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    /* The following calls demonstrate reading files from the generated SPIFFS
     * image. The images should contain the same files and contents as the spiffs_image directory.
     */

    // Read and display the contents of a small text file (hello.txt)
    read_hello_txt();

    // Compute and display the MD5 hash of a large text file (alice.txt) 
    //compute_alice_txt_md5();

    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS unmounted");
}
