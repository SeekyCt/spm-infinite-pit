#pragma once

#include "evt_cmd.h"

#include <types.h>

namespace spm::evt_door {

struct DoorDesc
{
    u8 unknown_0x0[0xc - 0x0];
    const char * name;
    u8 unknown_0x10[0x58 - 0x10];
};
static_assert(sizeof(DoorDesc) == 0x58);

struct DokanDesc
{
    u8 unknown_0x0[0x8 - 0x0];
    const char * name;
    u8 unknown_0xc[0x20 - 0xc];
};
static_assert(sizeof(DokanDesc) == 0x20);

struct MapDoorDesc
{
    u8 unknown_0x0[0x4 - 0x0];
    const char * name;
    u8 unknown_0x8[0x20 - 0x8];
};
static_assert(sizeof(MapDoorDesc) == 0x20);

extern "C" {

EVT_DECLARE(evt_8041a2b0)

EVT_DECLARE_USER_FUNC(func_800e11e0, 1)

// evt_door_set_door_descs(DoorDesc * descs, int count)
EVT_DECLARE_USER_FUNC(evt_door_set_door_descs, 2)

// evt_door_set_dokan_descs(DokanDesc * descs, int count)
EVT_DECLARE_USER_FUNC(evt_door_set_dokan_descs, 2)

// evt_door_set_map_door_descs(MapDoorDesc * descs, int count)
EVT_DECLARE_USER_FUNC(evt_door_set_map_door_descs, 2)

// evt_door_set_map_door_descs(bool enable, char * name)
EVT_DECLARE_USER_FUNC(evt_door_enable_disable_map_door_desc, 2)

}

}
