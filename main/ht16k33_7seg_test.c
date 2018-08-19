#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "ht16k33_7seg.h"
#include "esp_log.h"

#define SCL_GPIO 19
#define SDA_GPIO 18

void
app_main()
{
    uint32_t i;
    uint16_t blinkcounter = 0;
    int drawDots = 0;

    ht16k33_init(SDA_GPIO, SCL_GPIO);

    // print too big number
    ht16k33_print(10000, 10);
    ht16k33_write_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // print a hex number
    ht16k33_print(0xBEEF, 16);
    ht16k33_write_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // print a floating point
    ht16k33_print(12.34, 10);
    ht16k33_write_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // brightness test
    for (i = 0; i < 15; ++i)
    {
        printf("Brightness level: %d\n", i);
        ht16k33_set_brightness(i);
        vTaskDelay(100/ portTICK_PERIOD_MS);
    }

    // blink test
    for (i = 0; i < HT16K33_BLINK_HALFHZ; ++i)
    {
        printf("Blink mode: %d\n", i);
        ht16k33_set_blink_rate(i);
        vTaskDelay(4000/ portTICK_PERIOD_MS);
    }
    ht16k33_set_blink_rate(HT16K33_BLINK_OFF);

    // print with print/println
    for (i = 0; i < 1001; i++)
    {
        ht16k33_println(i, 10);
        ht16k33_write_display();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // method #2 - draw each digit
    for (i = 0; i < 1001; i ++)
    {
        ht16k33_write_digit_num(0, (i / 1000), drawDots);
        ht16k33_write_digit_num(1, (i / 100) % 10, drawDots);
        ht16k33_draw_colon(drawDots);
        ht16k33_write_digit_num(3, (i / 10) % 10, drawDots);
        ht16k33_write_digit_num(4, i % 10, drawDots);

        blinkcounter+=50;
        if (blinkcounter < 500)
            drawDots = 0;
        else if (blinkcounter < 1000)
            drawDots = true;
        else
            blinkcounter = 0;

        ht16k33_write_display();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    ht16k33_clear();
    ht16k33_write_display();
    vTaskDelay(500 / portTICK_PERIOD_MS);

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

