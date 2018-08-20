#ifndef HT16K33_7SEG_H
#define HT16K33_7SEG_H

void ht16k33_set_blink_rate(uint8_t b);
void ht16k33_set_brightness(uint8_t b);
void ht16k33_write_digit_raw(uint8_t d, uint8_t bitmask);
void ht16k33_write_digit_num(uint8_t d, uint8_t num, uint8_t dot);
void ht16k33_write_display(void);
void ht16k33_clear(void);
void ht16k33_draw_colon(uint8_t state);
void ht16k33_init(uint8_t sda_pin, uint8_t scl_pin);

enum ht16k33_blink_level
{
    HT16K33_BLINK_OFF = 0,
    HT16K33_BLINK_2HZ,
    HT16K33_BLINK_1HZ,
    HT16K33_BLINK_HALFHZ
};

static char _tag[] = "HT16K33 library";

#define ht16k33_print(x, base)                                                    \
    __builtin_choose_expr (                                                       \
            __builtin_types_compatible_p (typeof (x), double),                    \
            _print_double (x, base),                                              \
            __builtin_choose_expr (                                               \
                __builtin_types_compatible_p (typeof (x), int),                   \
                _print_int (x, base),                                             \
                __builtin_choose_expr (                                           \
                    __builtin_types_compatible_p (typeof (x), unsigned int),      \
                    _print_uint ((unsigned int)x, base),                          \
                __builtin_choose_expr (                                           \
                    __builtin_types_compatible_p (typeof (x), char),              \
                    _print_char ((char)x, base),                                  \
                    __builtin_choose_expr (                                       \
                        __builtin_types_compatible_p (typeof (x), unsigned char), \
                        _print_uchar ((unsigned char)x, base),                    \
                        __builtin_choose_expr (                                   \
                            __builtin_types_compatible_p (typeof(x), uint32_t),   \
                            _print_uint (x, base),                                \
                            (void)0))))))


#define ht16k33_println(x, base) \
{ \
    ESP_LOGD(_tag, "println"); \
    ht16k33_print(x, base); \
    _println(); \
}


/*
#define ht16k33_println(x, base)                                                         \
    __builtin_choose_expr (                                                              \
        __builtin_types_compatible_p (typeof (x), double),                               \
        _println_double (x, base),                                                       \
        __builtin_choose_expr (                                                          \
            __builtin_types_compatible_p (typeof (x), int),                              \
            _println_int (x, base),                                                      \
            __builtin_choose_expr (                                                      \
                __builtin_types_compatible_p (typeof (x), char),                         \
                _println_char ((char)x, base),                                           \
                __builtin_choose_expr (                                                  \
                    __builtin_types_compatible_p (typeof (x), unsigned char),            \
                    _println_uchar ((unsigned char)x, base),                             \
                    __builtin_choose_expr (                                              \
                        __builtin_types_compatible_p (typeof(x), uint32_t),              \
                        _println_uint (x, base),                                         \
                        __builtin_choose_expr (                                          \
                            __builtin_types_compatible_p (typeof(x), long),              \
                            _println_long(x, base),                                      \
                            __builtin_choose_expr (                                      \
                                __builtin_types_compatible_p (typeof(x), unsigned long), \
                                _println_ulong(x, base),                                 \
                                    (void)0)))))))
*/
//void print(unsigned long n, int base);

void _print_char(char c, uint8_t base);
void _print_uchar(unsigned char b, uint8_t base);
void _print_int(int n, uint8_t base);
void _print_uint(unsigned int n, uint8_t base);
void _print_double(double n, uint8_t digits);
void _println(void);
/*
void _println_char(char c, uint8_t base);
void _println_uchar(unsigned char b, uint8_t base);
void _println_int(int n, uint8_t base);
void _println_uint(unsigned int n, uint8_t base);
void _println_long(long n, uint8_t base);
void _println_ulong(unsigned long n, uint8_t base);
void _println_double(double n, uint8_t digits);
*/

#endif /* ifndef HT16K33_7SEG_H */

