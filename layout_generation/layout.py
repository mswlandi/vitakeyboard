import re
from bs4 import BeautifulSoup
from codecs import decode
from iso_to_hid import iso_to_hid
from os import walk

layouts_dir = 'cldr_layouts/'

layout_file_list = []
for (dirpath, dirnames, filenames) in walk(layouts_dir):
    layout_file_list.extend(filenames)
    break

def getModifiersList(soup):
    modifiers = []

    if soup.parent.has_attr('modifiers'):
        for modifier in soup.parent['modifiers'].split(' ')[0].split('+'):
            # ignore optional modifiers
            if modifier[-1] != '?':
                modifiers.append(modifier)
    
    return modifiers

def modifiersFlagFromStrList(modifiers_list):
    flag = 0

    for modifier in modifiers_list:
        if modifier == "ctrl" or modifier == "ctrlL":
            flag |= 0x01

        # supposing there are no keys that need caps and not shift
        elif modifier == "shift" or modifier == "shiftL" or modifier == "caps":
            flag |= 0x02
        elif modifier == "alt" or modifier == "altL" or modifier == "opt" or modifier == "optL":
            flag |= 0x04

        # windows key, left meta
        elif modifier == "cmd":
            flag |= 0x08
        elif modifier == "ctrlR":
            flag |= 0x10
        elif modifier == "shiftR":
            flag |= 0x20
        elif modifier == "altR" or modifier == "optR":
            flag |= 0x40
    
    return flag


for xml_file_name in layout_file_list:
    layout_name = '.'.join(xml_file_name.split('.')[:-1])

    with open(layouts_dir + xml_file_name, 'r') as f:
        xml_doc = f.read()

    soup = BeautifulSoup(xml_doc, 'xml')

    charToIso = {}

    for map in soup.find_all('map'):
        if not map['to'] in charToIso:
            charToIso[map['to']] = (map['iso'], getModifiersList(map), 'empty', [])

    # processes transforms (characters that require more than one key press)
    for transform in soup.find_all('transform'):
        if not transform['to'] in charToIso:
            charToIso[transform['to']] = (
                charToIso[transform['from'][0]][0], # the iso key for the first char
                charToIso[transform['from'][0]][1], # the modifiers for the first char
                charToIso[transform['from'][1]][0], # the iso key for the second char
                charToIso[transform['from'][1]][1]) # the modifiers for the second char

    # get utf8 characters from the escape codes (the ones with \u{XX})
    charToIso_noUnicodeEscape = {}
    for key in charToIso:
        if key.startswith('\\u'):
            utf8_code = key.replace('\\u{', '').replace('}', '').zfill(4)
            # ignore control characters
            if int(utf8_code,16) >= 32:
                new_char = decode(utf8_code, 'hex').decode('utf-8')[-1]
                charToIso_noUnicodeEscape[new_char] = charToIso[key]
        else:
            charToIso_noUnicodeEscape[key[-1]] = charToIso[key]

    # header of the header file
    header_file_lines = [
        f'#ifndef {layout_name.upper()}_H\n',
        f'#define {layout_name.upper()}_H\n\n',
        '#include "layout_general.h"\n\n',
        f'const utf16_to_hid_mapping {layout_name}_layout[] = {{\n'
    ]

    # each line corresponds to a mapping from a character to 1 or 2 keys
    for key, value in sorted(charToIso_noUnicodeEscape.items()):
        utf16char = f'{ord(key):#06x}'

        hidkey1 = f'{iso_to_hid[value[0]]:#04x}'
        hidmodifiers1 = f'{modifiersFlagFromStrList(value[1]):#04x}'
        hidkey2 = '0x00' if value[2] == 'empty' else f'{iso_to_hid[value[2]]:#04x}'
        hidmodifiers2 = f'{modifiersFlagFromStrList(value[3]):#04x}'
        header_file_lines.append(f'    {{ {utf16char}, {hidkey1}, {hidmodifiers1}, {hidkey2}, {hidmodifiers2} }}, /* {key} */ \n')

    header_file_lines.append('};\n')
    header_file_lines.append('\n#endif\n')

    with open(f'../skprx/layouts/{layout_name}.h', 'w') as header_file:
        header_file.writelines(header_file_lines)