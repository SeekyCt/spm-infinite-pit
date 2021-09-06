#include "mod.h"
#include "ip_badgemenu.h"
#include "ip_dan.h"
#include "ip_messages.h"
#include "ip_pausetex.h"
#include "ip_save.h"

#include <patch.h>

#include <mod_ui_base/window.h>
#include <mod_ui_base/colours.h>

#include <spm/fontmgr.h>
#include <spm/seqdrv.h>
#include <spm/seqdef.h>
#include <wii/OSError.h>

namespace mod {

/*
    General mod functions
*/

void ipmain()
{
    wii::OSError::OSReport("SPM Infinite Pit v0 DEV: the mod has ran!\n");

    ipBadgeMenuPatch();
    ipDanPatch();
    ipMessagePatch();
    ipPauseTexPatch();
    ipSavePatch();
}

}
