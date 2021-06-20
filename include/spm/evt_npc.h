#pragma once

#include <spm/evtmgr.h>

namespace spm::evt_npc {

extern "C" {

// evt_npc_teleport_effect(bool appearDisappear, const char * npcName)
// Found by JohnP55
EVT_DECLARE_USER_FUNC(evt_npc_teleport_effect, 2)

// evt_npc_entry_from_template(int instanceIdOverride, int templateId, int x, int y,
//                             int z, &int instanceId, &char * instanceName)
EVT_DECLARE_USER_FUNC(evt_npc_entry_from_template, 7)

EVT_DECLARE_USER_FUNC(func_80107cfc, 0)
EVT_DECLARE_USER_FUNC(func_80107d20, 0)

}

}
