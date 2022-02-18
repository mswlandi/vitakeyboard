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
4. On the host computer, make sure you're using the English (US) QWERTY layout

## About Special Characters
vitakeyboard emulates an actual physical keyboard that would use the English (US) layout, so only characters that are in a normal keyboard with that layout are sent.
If other layouts were to be supported, some characters still wouldn't easily be sent. For example, when using the Portuguese (Brazil) layout, to send the special character `á` (that you can find on the vita's virtual keyboard), you'd have to first hit the `´` key and then the `a` key.

## Credits
- hnaves for making [hidmouse](https://github.com/esxgx/hidmouse) and xerpi for [porting it to vita](https://github.com/xerpi/hidmouse), of which this is a fork from
- xerpi for making [vitastick](https://github.com/xerpi/vitastick), from which a lot of this code is based on (looking back, it would probably be easier to fork from it lol)
- SonicMastr for help with libime and loading modules
