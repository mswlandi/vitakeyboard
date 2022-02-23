#ifndef LAYOUTS_H
#define LAYOUTS_H

#include "layout_general.h"
#include "en_US.h"
#include "pt_BR.h"

#define VITAKEYBOARD_ERR_MAPPING_NOT_FOUND 0xFFFF

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
