#ifndef LAYOUT_GENERAL_H
#define LAYOUT_GENERAL_H

// It has an optional second key for characters like ñ in spanish that require 2 key presses
typedef struct utf16_to_hid_mapping_t {
    unsigned short int utf16_char;
    unsigned char hid_key1;
    unsigned char hid_modifiers1;
    unsigned char hid_key2;
    unsigned char hid_modifiers2;
} utf16_to_hid_mapping;

#endif // LAYOUT_GENERAL_H