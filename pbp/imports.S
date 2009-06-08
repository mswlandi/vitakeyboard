	.set noreorder

#include "pspimport.s"

	IMPORT_START	"pspDebug",0x00090000
	IMPORT_FUNC	"pspDebug",0x2F6CD8A4,get_flush_function
	IMPORT_FUNC	"pspDebug",0x45A6C2FA,set_flush_function

