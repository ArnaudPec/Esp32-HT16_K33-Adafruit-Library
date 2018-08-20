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
    ht16k33_print(x, base); \
    _println(); \
}

void _print_char(char c, uint8_t base);
void _print_uchar(unsigned char b, uint8_t base);
void _print_int(int n, uint8_t base);
void _print_uint(unsigned int n, uint8_t base);
void _print_double(double n, uint8_t digits);
void _println(void);

#endif /* ifndef HT16K33_7SEG_H */

