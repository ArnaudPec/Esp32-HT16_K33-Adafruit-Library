#include <stdio.h>
#include <esp_log.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "ht16k33_7seg.h"

static uint16_t _display_buffer[8];
static uint8_t _position = 0;
static int _sda_gpio = I2C_MASTER_SDA_IO;
static int _scl_gpio = I2C_MASTER_SCL_IO;

static char _tag[] = "HT16K33 library";

static const uint8_t _number_table[] =
{
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F, /* 9 */
    0x77, /* a */
    0x7C, /* b */
    0x39, /* C */
    0x5E, /* d */
    0x79, /* E */
    0x71, /* F */
};

/**
 * @brief i2c master initialization
 */
static void
_i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;

    ESP_LOGD(_tag, "I2C init");

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = _sda_gpio;
    conf.scl_io_num = _scl_gpio;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(i2c_master_port, &conf);

    i2c_driver_install(i2c_master_port,
            conf.mode,
            I2C_MASTER_RX_BUF_ENABLE,
            I2C_MASTER_TX_BUF_ENABLE,
            0);
}

static esp_err_t
_i2c_master_write_cmd(i2c_port_t i2c_num, uint8_t data_cmd)
{
    ESP_LOGD(_tag, "i2c_master_write_cmd %d", data_cmd);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HT16K33_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, data_cmd, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 10 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void
ht16k33_set_blink_rate(uint8_t b)
{
    ESP_LOGD(_tag, "set_blink_rate %d", b);
    if (b > 3) b = 0; // turn off if not sure

    _i2c_master_write_cmd(I2C_MASTER_NUM, HT16K33_BLINK_CMD |
            HT16K33_BLINK_DISPLAYON |
            (b << 1));
}

void
ht16k33_set_brightness(uint8_t b)
{
    ESP_LOGD(_tag, "set brighness %d", b);
    if (b > 15) b = 15;
    _i2c_master_write_cmd(I2C_MASTER_NUM, HT16k33_BRIGHTNESS_CMD | b);
}

void
ht16k33_write_digit_raw(uint8_t d, uint8_t bitmask)
{
    ESP_LOGD(_tag, "write_digit_raw, d %d bitmask %d", d, bitmask);
    if (d > 4) return;
    _display_buffer[d] = bitmask;
}

void
ht16k33_write_digit_num(uint8_t d, uint8_t num, int dot)
{
    ESP_LOGD(_tag, "write_digit_num %d, %d, %d", d, num,
             _number_table[num]| (dot << 7));

    if (d > 4) return;

    ht16k33_write_digit_raw(d, _number_table[num] | (dot << 7));
}

size_t
write(uint8_t c)
{
    ESP_LOGD(_tag, "write %c %d", c, c);

    uint8_t r = 0;

    if (c == '\n') _position = 0;
    if (c == '\r') _position = 0;

    if ((c >= '0') && (c <= '9')) {
        ht16k33_write_digit_num(_position, c-'0', 0);
        r = 1;
    }

    _position++;
    if (_position == 2) _position++;

    return r;
}

void
ht16k33_write_display(void)
{
    uint8_t i;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (HT16K33_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, (uint8_t)0x00, 1);

    for (i = 0; i < 8; i++)
    {
        i2c_master_write_byte(cmd, _display_buffer[i] & 0xFF, 1);
        i2c_master_write_byte(cmd, _display_buffer[i] >> 8, 1);
    }
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 10/ portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
}

void
ht16k33_clear(void)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
        _display_buffer[i] = 0;
}

static void
_print_error(void)
{
    uint8_t i;
    ESP_LOGE(_tag, "Print Error");
    for(i = 0; i < HT16K33_7SEG_DIGITS; ++i)
        ht16k33_write_digit_raw(i, (i == 2 ? 0x00 : 0x40));
}

void
print_float(double n, uint8_t fracDigits, uint8_t base)
{

    uint8_t numericDigits = 4;   // available digits on display
    uint8_t i;
    int isNegative = 0;  // true if the number is negative
    double toIntFactor = 1.0;
    uint32_t displayNumber;
    uint32_t tooBig = 1;

    // is the number negative?
    if(n < 0) {
        isNegative = 1;  // need to draw sign later
        --numericDigits;    // the sign will take up one digit
        n *= -1;            // pretend the number is positive
    }

    // calculate the factor required to shift all fractional digits
    // into the integer part of the number
    for(i = 0; i < fracDigits; ++i) toIntFactor *= base;

    // create integer containing digits to display by applying
    // shifting factor and rounding adjustment
    displayNumber = n * toIntFactor + 0.5;

    // calculate upper bound on displayNumber given
    // available digits on display
    for(i = 0; i < numericDigits; ++i) tooBig *= base;

    // if displayNumber is too large, try fewer fractional digits
    while(displayNumber >= tooBig)
    {
        --fracDigits;
        toIntFactor /= base;
        displayNumber = n * toIntFactor + 0.5;
    }

    // did toIntFactor shift the decimal off the display?
    if (toIntFactor < 1)
        _print_error();
    else
    {
        // otherwise, display the number
        int8_t displayPos = 4;

        if (displayNumber)  //if displayNumber is not 0
        {
            for(i = 0; displayNumber || i <= fracDigits; ++i)
            {
                int displayDecimal = (fracDigits != 0 && i == fracDigits);
                ht16k33_write_digit_num(displayPos--, displayNumber % base, displayDecimal);
                if(displayPos == 2) ht16k33_write_digit_raw(displayPos--, 0x00);
                displayNumber /= base;
            }
        }
        else
            ht16k33_write_digit_num(displayPos--, 0, 0);

        // display negative sign if negative
        if(isNegative) ht16k33_write_digit_raw(displayPos--, 0x40);

        // clear remaining display positions
        while(displayPos >= 0) ht16k33_write_digit_raw(displayPos--, 0x00);
    }
}

void
ht16k33_draw_colon(int state)
{
    if (state)
        _display_buffer[2] = 0x2;
    else
        _display_buffer[2] = 0;
}

/*void*/
/*write_colon(void)*/
/*{*/
/*Wire.beginTransmission(i2c_addr);*/
/*Wire.write((uint8_t)0x04); // start at address $02*/

/*Wire.write(_display_buffer[2] & 0xFF);*/
/*Wire.write(_display_buffer[2] >> 8);*/

/*Wire.endTransmission();*/
/*}*/

void
print_number(long n, uint8_t base)
{
    print_float(n, 0, base);
}

void
_bprint(unsigned long n, int base)
{
    if (base == 0) write(n);
    else print_number(n, base);
}

void
_print_char(char c, int base)
{
    ESP_LOGD(_tag, "print char");
    _bprint((long) c, base);
}

void
_print_uchar(unsigned char b, int base)
{
    ESP_LOGD(_tag, "print uchar");
    _bprint((unsigned long) b, base);
}

void
_print_int(int n, int base)
{
    ESP_LOGD(_tag, "print int");
    _bprint((long) n, base);
}

void
_print_uint(unsigned int n, int base)
{
    ESP_LOGD(_tag, "print uint");
    _bprint((unsigned long) n, base);
}

void
_println(void)
{
    ESP_LOGD(_tag, "println");
    _position = 0;
}

void
_println_char(char c, int base)
{
    ESP_LOGD(_tag, "println char");
    _bprint(c, base);
    _println();
}

void
_println_uchar(unsigned char b, int base)
{
    ESP_LOGD(_tag, "println uchar");
    _bprint(b, base);
    _println();
}

void
_println_int(int n, int base)
{
    ESP_LOGD(_tag, "println int");
    _bprint(n, base);
    _println();
}

void
_println_uint(unsigned int n, int base)
{
    ESP_LOGD(_tag, "println uint");
    _bprint(n, base);
    _println();
}

void
_println_long(long n, int base)
{
    ESP_LOGD(_tag, "println long");
    _bprint(n, base);
    _println();
}

void
_println_ulong(unsigned long n, int base)
{
    ESP_LOGD(_tag, "println ulong");
    _bprint(n, base);
    _println();
}

void
_print_double(double n, int digits)
{
    ESP_LOGD(_tag, "print double");
    print_float(n, digits, 10);
}

void
_println_double(double n, int digits)
{
    ESP_LOGD(_tag, "println double");
    _print_double(n, digits);
    _println();
}

void
ht16k33_init(int sda_gpio, int scl_gpio)
{
    _sda_gpio = sda_gpio;
    _scl_gpio = scl_gpio;

    _i2c_master_init();
    _i2c_master_write_cmd(I2C_MASTER_NUM, 0x21);

    ht16k33_set_blink_rate(0);
    ht16k33_set_brightness(10);
}

