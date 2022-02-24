#ifndef LAYOUTS_H
#define LAYOUTS_H

#define VITAKEYBOARD_ERR_MAPPING_NOT_FOUND 0xFFFF

#include "layout_general.h"

typedef struct layout_list_entry_t {
    char* layout_name;
    const utf16_to_hid_mapping* layout;
    int num_mappings_of_layout;
} layout_list_entry;

#include "en_US.h"
#include "pt_BR.h"

layout_list_entry layout_list[] = {
    {"english (US)", en_US_layout, sizeof(en_US_layout) / sizeof(utf16_to_hid_mapping)},
    {"portuguese (BR)", pt_BR_layout, sizeof(pt_BR_layout) / sizeof(utf16_to_hid_mapping)}
};

utf16_to_hid_mapping getLayoutMappingFromUtf16(unsigned short int utf16char, const utf16_to_hid_mapping layout[], int num_mappings)
{
    int first, last, middle;

    first = 0;
    last = num_mappings - 1;
    middle = (first + last) / 2;

    // binary search
    while (first <= last) {
        if (layout[middle].utf16_char < utf16char)
            first = middle + 1;
        else if (layout[middle].utf16_char == utf16char) {
            return layout[middle];
        }
        else
            last = middle - 1;

        middle = (first + last) / 2;
    }
    
    // not found
    utf16_to_hid_mapping error_char = { VITAKEYBOARD_ERR_MAPPING_NOT_FOUND, 0, 0, 0 };
    return error_char;
}

#endif
