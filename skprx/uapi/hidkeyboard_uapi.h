#ifndef HIDKEYBOARD_UAPI_H
#define HIDKEYBOARD_UAPI_H

#define HIDKEYBOARD_ERROR_DRIVER_NOT_REGISTERED		0x91338000
#define HIDKEYBOARD_ERROR_DRIVER_NOT_ACTIVATED		0x91338001
#define HIDKEYBOARD_ERROR_DRIVER_ALREADY_ACTIVATED	0x91338002

extern int HidKeyboardSendChar(unsigned short int c);
extern int HidKeyBoardSendModifierAndKey(char mod, char key);
extern int hidkeyboard_user_start(void);
extern int hidkeyboard_user_stop(void);

#endif
