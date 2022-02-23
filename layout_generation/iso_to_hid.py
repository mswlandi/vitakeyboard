# https://en.wikipedia.org/wiki/ISO/IEC_9995

# For our purposes I'm not doing keys with numbering above 14.
# modifiers (shift, ctrl, etc) were also removed.

iso_to_hid = {
    "E00": 0x35, # ` and ~
    "E01": 0x1e, # Keyboard 1 and !
    "E02": 0x1f, # Keyboard 2 and @
    "E03": 0x20, # Keyboard 3 and #
    "E04": 0x21, # Keyboard 4 and $
    "E05": 0x22, # Keyboard 5 and %
    "E06": 0x23, # Keyboard 6 and ^
    "E07": 0x24, # Keyboard 7 and &
    "E08": 0x25, # Keyboard 8 and *
    "E09": 0x26, # Keyboard 9 and (
    "E10": 0x27, # Keyboard 0 and )
    "E11": 0x2d, # Keyboard - and _
    "E12": 0x2e, # Keyboard = and +
    "E14": 0x2a, # Keyboard DELETE (Backspace)

    "D00": 0x2b, # Keyboard Tab
    "D01": 0x14, # Keyboard q and Q
    "D02": 0x1a, # Keyboard w and W
    "D03": 0x08, # Keyboard e and E
    "D04": 0x15, # Keyboard r and R
    "D05": 0x17, # Keyboard t and T
    "D06": 0x1c, # Keyboard y and Y
    "D07": 0x18, # Keyboard u and U
    "D08": 0x0c, # Keyboard i and I
    "D09": 0x12, # Keyboard o and O
    "D10": 0x13, # Keyboard p and P
    "D11": 0x2f, # Keyboard [ and {
    "D12": 0x30, # Keyboard ] and }
    "D13": 0x31, # Keyboard \ and |

    "C00": 0x39, # Keyboard Caps Lock
    "C01": 0x04, # Keyboard a and A
    "C02": 0x16, # Keyboard s and S
    "C03": 0x07, # Keyboard d and D
    "C04": 0x09, # Keyboard f and F
    "C05": 0x0a, # Keyboard g and G
    "C06": 0x0b, # Keyboard h and H
    "C07": 0x0d, # Keyboard j and J
    "C08": 0x0e, # Keyboard k and K
    "C09": 0x0f, # Keyboard l and L
    "C10": 0x33, # Keyboard ; and :
    "C11": 0x34, # Keyboard ' and "
    "C12": 0x32, # Keyboard Non-US # and ~
    "C13": 0x28, # Keyboard Return (ENTER)

    "B00": 0x31, # Keyboard \ and |
    "B01": 0x1d, # Keyboard z and Z
    "B02": 0x1b, # Keyboard x and X
    "B03": 0x06, # Keyboard c and C
    "B04": 0x19, # Keyboard v and V
    "B05": 0x05, # Keyboard b and B
    "B06": 0x11, # Keyboard n and N
    "B07": 0x10, # Keyboard m and M
    "B08": 0x36, # Keyboard , and <
    "B09": 0x37, # Keyboard . and >
    "B10": 0x38, # Keyboard / and ?
    "B11": 0x64, # Keyboard Non-US \ and |
    
    "A03": 0x2c, # Keyboard Spacebar
}