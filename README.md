# VitaKeyboard
VitaKeyboard is a plugin and application for PS Vita that lets you use it as a USB Keyboard. The host will think your vita is an actual keyboard.

## Installation and Setup
1. Add hidkeyboard.skprx to taiHEN's config (ur0:/tai/config.txt):
```
*KERNEL
ur0:tai/hidkeyboard.skprx
```
2. Install vitakeyboard.vpk
3. Reboot the vita to load the kernel plugin
4. Before typing, make sure the host computer is using the same keyboard layout as the one selected in VitaKeyboard.

## Other considerations
- if there is a keyboard layout that you'd like to be added, submit an issue or follow the instructions at `layout_generation/README.md` and build the project yourself (pull requests are welcome).
- layout savedata is stored in `ux0:data/vitakeyboard_savefile.bin`

## Building
from the root folder:

```shell
cd skprx
mkdir build
cd build
cmake ..
make

cd ../..

cd vpk
mkdir build
cmake ..
make
```

the relevant outputs are `skprx/build/hidkeayboard.skprx` and `vpk/build/vitakeyboard.vpk`

## Credits
- hnaves for making [hidmouse](https://github.com/esxgx/hidmouse) and xerpi for [porting it to vita](https://github.com/xerpi/hidmouse), of which this was initially a fork from
- xerpi for making [vitastick](https://github.com/xerpi/vitastick), from which a lot of this code is based on (looking back, it would probably be easier to fork from it lol)
- SonicMastr for help with libime and loading modules
- LiveArea design is based on the one made for vitastick by [@nkrapivin](https://github.com/nkrapivin)

## Changelog

### v1.1

- added support for other keyboard layouts
  - added pt (BR) layout
  - added es (ES) layout
  - added de (DE) layout
- added LiveArea design
- added better way to close the application (START while IME is closed)

### v0.1-alpha

- First Release.