#pragma once

#include "evt_cmd.h"

namespace spm::evt_mobj {

extern "C" {

// Script to save the game
EVT_DECLARE(mobj_save_blk_sysevt);

// evt_mobj_zyo(char * name, int lockItemId, float x, float y, float z, float yRotation, EvtScriptCode * interactScript
//              EvtScriptCode * openScript, EvtVar collectedVar)
EVT_DECLARE_USER_FUNC(evt_mobj_zyo, 9)

}

}
