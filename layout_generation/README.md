# Keyboard layout generation
To generate a keyboard layout:

1. put the desired layout from [the CLDR repository](https://github.com/unicode-org/cldr/tree/main/keyboards/windows) in cldr_layouts/. (you should rename the file)
2. run layout.py
3. include the corresponding generated header file in skprx/layouts/layouts.h