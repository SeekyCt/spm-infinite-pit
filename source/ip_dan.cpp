#include "evt_cmd.h"
#include "util.h"
#include "patch.h"

#include <types.h>
#include <spm/evt_door.h>
#include <spm/evt_map.h>
#include <spm/evt_mobj.h>
#include <spm/evt_hit.h>
#include <spm/evt_npc.h>
#include <spm/evt_snd.h>
#include <spm/evt_sub.h>
#include <spm/itemdrv.h>
#include <spm/lzss10.h>
#include <spm/lz_embedded.h>
#include <spm/mapdata.h>
#include <spm/memory.h>
#include <spm/parse.h>
#include <spm/rel/dan.h>
#include <wii/OSError.h>
#include <wii/string.h>

using spm::evtmgr::EvtEntry;
using spm::dan::DanWork;
using spm::dan::DanDoor;
using spm::dan::DanEnemy;
using spm::dan::DanDungeon;
using spm::dan::danWp;
using spm::lz_embedded::pitText;

namespace mod {

static EVT_BEGIN_EDITABLE(ip_dan_enemy_room_init_evt)
    SET(LW(0), GSW(1))
    USER_FUNC(spm::dan::evt_dan_read_data)
    USER_FUNC(spm::dan::evt_dan_handle_map_parts, LW(0))
    USER_FUNC(spm::dan::evt_dan_handle_dokans, LW(0))
    USER_FUNC(spm::evt_door::evt_door_set_dokan_descs, PTR(&spm::dan::danDokanDescs), 8)
    SET(LW(1), 0)
    // USER_FUNC(evt_dan_handle_doors, LW(0), LW(1), LW(10), LW(11), LW(2), LW(3), LW(4), /* LW(5), LW(6), LW(7) */)
    USER_FUNC(spm::dan::evt_dan_handle_doors, LW(0), LW(1), LW(10), LW(11), LW(2), LW(3), LW(4), LW(5), LW(6), LW(7))
    USER_FUNC(spm::evt_door::evt_door_set_map_door_descs, PTR(&spm::dan::danMapDoorDescs), 2)
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, 0, LW(10))
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, 0, LW(11))
    IF_SMALL(GSW(1), 100)
        USER_FUNC(spm::evt_mobj::evt_mobj_zyo, PTR("lock_00"), 48, LW(2), LW(3), LW(4), 0, PTR(spm::dan::dan_lock_interact_evt),
                  PTR(spm::dan::dan_lock_open_evt), 0)
    ELSE()
        USER_FUNC(spm::evt_mobj::evt_mobj_zyo, PTR("lock_00"), 44, LW(2), LW(3), LW(4), 0, PTR(spm::dan::dan_lock_interact_evt),
                  PTR(spm::dan::dan_lock_open_evt), 0)
    END_IF()
    USER_FUNC(spm::dan::func_80c82d38, LW(0))
    SET(LW(10), 0)
    SET(LW(9), 0)
    DO(16)
        USER_FUNC(spm::dan::evt_dan_get_enemy_info, LW(0), LW(10), LW(11), LW(12))
        IF_LARGE(LW(12), 0)
            DO(LW(12))
                USER_FUNC(spm::dan::evt_dan_get_enemy_spawn_pos, LW(9), LW(0), LW(10), LW(13), LW(14), LW(15))
                ADD(LW(9), 1)
                USER_FUNC(spm::evt_npc::evt_npc_entry_from_template, 0, LW(11), LW(13), LW(14), LW(15), 0, EVT_NULLPTR)
            WHILE()
        END_IF()
        ADD(LW(10), 1)
    WHILE()
    IF_SMALL(GSW(1), 100)
        USER_FUNC(spm::dan::evt_dan_decide_key_enemy, 48)
    ELSE()
        USER_FUNC(spm::dan::evt_dan_decide_key_enemy, 44)
    END_IF()
    USER_FUNC(spm::evt_npc::func_80107cfc)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A2"), 0x40000000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A3"), 0x20000000)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 0, PTR("S"), 0x2)
    USER_FUNC(spm::evt_map::evt_mapobj_flag4_onoff, 1, 1, PTR("S"), 0x10)
    RUN_CHILD_EVT(PTR(spm::evt_door::evt_8041a2b0))
    ADD(GSW(1), 1)
    USER_FUNC(spm::evt_snd::evt_snd_bgmon, 0, PTR("BGM_MAP_100F"))
    USER_FUNC(spm::evt_snd::func_800d36f4, 0)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::func_800e11e0, 0x100)
        USER_FUNC(spm::evt_sub::func_800d74a0, 1, 6)
    END_INLINE()
    USER_FUNC(spm::dan::evt_dan_start_countdown)
    USER_FUNC(spm::evt_map::evt_mapobj_trans, PTR("Tetugoushi"), 0, -1000, 0)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 1, PTR("Tetugoushi"), 1)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::func_800e11e0, 256)
        USER_FUNC(spm::evt_npc::func_80107d20)
        USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_init, 11, 255, 0, 1000)
        DO(0)
            USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_get_value)
            USER_FUNC(spm::dan::evt_dan_get_door_names, LW(2), LW(3))
            USER_FUNC(spm::evt_map::evt_mapobj_color, 1, LW(2), 255, 255, 255, LW(0))
            WAIT_FRM(1)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 1, LW(2), 1)
    END_INLINE()
    INLINE_EVT()
        USER_FUNC(spm::dan::evt_dan_handle_key_failsafe)
    END_INLINE()
    USER_FUNC(spm::evt_sub::func_800d4de4, 1, 0)
    RETURN()
EVT_END()

static EVT_BEGIN_EDITABLE(ip_dan_chest_room_init_evt)
    SET(LW(0), GSW(1))
    USER_FUNC(spm::dan::evt_dan_read_data)
    USER_FUNC(spm::evt_door::evt_door_set_dokan_descs, PTR(&spm::dan::danChestRoomDokanDesc), 1)
    USER_FUNC(spm::dan::evt_dan_handle_chest_room_dokans_and_doors, LW(0))
    USER_FUNC(spm::evt_door::evt_door_set_map_door_descs, PTR(&spm::dan::danChestRoomMapDoorDescs), 2)
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, 0, PTR("doa1_l"))
    SWITCH(GSW(1))
        CASE_EQUAL(9)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(433))
        CASE_EQUAL(19)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(434))
        CASE_EQUAL(29)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(435))
        CASE_EQUAL(39)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(436))
        CASE_EQUAL(49)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(437))
        CASE_EQUAL(59)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(438))
        CASE_EQUAL(69)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(439))
        CASE_EQUAL(79)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(440))
        CASE_EQUAL(89)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(441))
        CASE_EQUAL(109)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(442))
        CASE_EQUAL(119)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(443))
        CASE_EQUAL(129)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(444))
        CASE_EQUAL(139)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(445))
        CASE_EQUAL(149)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(446))
        CASE_EQUAL(159)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(447))
        CASE_EQUAL(169)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(448))
        CASE_EQUAL(179)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(449))
        CASE_EQUAL(189)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), 0, 
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(450))
    END_SWITCH()
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("box"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("box"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_kan, 0, PTR("_kanban"), -280, 0, -158, 0, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_kan, 0, PTR("kanban"), -280, 0, FLOAT(-149.8994140625),
              PTR(&spm::dan::dan_exit_pipe_sign_interact_evt), 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("_kanban"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("kanban"), 0x4040)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("_kanban"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("kanban"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_arrow, PTR("_arrow"), -75, 0, -158, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_arrow, PTR("arrow"), -75, 0, -150, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("_arrow"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 0, PTR("arrow"), 0x4040)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("_arrow"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, 1, 1, PTR("arrow"), 0x10000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, 1, 1, PTR("A2"), 0x40000000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, 1, 1, PTR("A3"), 0x20000000)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 0, PTR("S"), 2)
    USER_FUNC(spm::evt_map::evt_mapobj_flag4_onoff, 1, 1, PTR("S"), 16)
    RUN_CHILD_EVT(spm::evt_door::evt_8041a2b0)
    ADD(GSW(1), 1)
    USER_FUNC(spm::evt_sub::evt_sub_random, 100, LW(0))
    IF_SMALL(LW(0), 30)
        USER_FUNC(spm::evt_npc::evt_npc_entry, PTR("roten"), PTR("n_machi_roten"), 0)
        USER_FUNC(spm::evt_npc::func_801059d0, PTR("roten"), -1)
        USER_FUNC(spm::evt_npc::evt_npc_set_property, PTR("roten"), 14, PTR(spm::dan::rotenTribeAnimDefs))
        USER_FUNC(spm::evt_npc::func_80103108, PTR("roten"), 0, 1)
        USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, PTR("roten"), 1, 0x4400004)
        USER_FUNC(spm::evt_npc::evt_npc_animflag_onoff, PTR("roten"), 1, 0x20)
        USER_FUNC(spm::evt_npc::func_80103054, PTR("roten"))
        USER_FUNC(spm::evt_npc::func_80104694, PTR("roten"), 1)
        USER_FUNC(spm::evt_npc::evt_npc_set_position, PTR("roten"), 398, 0, 0)
        USER_FUNC(spm::evt_npc::func_80108194, PTR("roten"), 0)
        USER_FUNC(spm::evt_npc::evt_npc_set_property, PTR("roten"), 11, 40)
        USER_FUNC(spm::evt_npc::evt_npc_set_property, PTR("roten"), 10, 60)
        USER_FUNC(spm::evt_npc::evt_npc_modify_part, PTR("roten"), -1, 11, 40)
        USER_FUNC(spm::evt_npc::evt_npc_modify_part, PTR("roten"), -1, 10, 60)
        USER_FUNC(spm::evt_shop::evt_shop_set_defs, PTR(&spm::dan::rotenShopDef), 1)
    END_IF()
    USER_FUNC(spm::evt_snd::evt_snd_bgmon, 0, PTR("BGM_MAP_100F"))
    USER_FUNC(spm::evt_snd::func_800d36f4, 0)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::func_800e11e0, 0x100)
        USER_FUNC(spm::evt_sub::func_800d74a0, 1, 6)
    END_INLINE()
    USER_FUNC(spm::evt_map::evt_mapobj_trans, PTR("Tetugoushi"), 0, -1000, 0)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 1, PTR("Tetugoushi"), 1)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::func_800e11e0, 0x100)
        USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_init, 11, 255, 0, 1000)
        DO(0)
            USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_get_value)
            USER_FUNC(spm::evt_map::evt_mapobj_color, 1, PTR("doa_01"), 255, 255, 255, LW(0))
            WAIT_FRM(1)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 1, PTR("doa_01"), 1)
    END_INLINE()
    USER_FUNC(spm::evt_sub::func_800d4de4, 1, 0)
    RETURN()
EVT_END()

//#define ONE_TIME_TXT_LOAD

#ifdef ONE_TIME_TXT_LOAD
static char * sDecompPitText;
static u32 sDecompPitTextSize;
#endif

// #define CONSTANT_MAP_MASK

#ifdef CONSTANT_MAP_MASK
static u32 sMapMask = 0;
#endif

int ip_evt_dan_read_data(EvtEntry * entry, bool isFirstCall)
{
    (void) entry;

    // Allocate work on first run
    // (check is a relD leftover, this only runs once on retail anyway)
    if (isFirstCall)
    {
        danWp = (DanWork *) spm::memory::__memAlloc(1, sizeof(*danWp));
        wii::string::memset(danWp, 0, sizeof(*danWp));
        danWp->dungeons = (DanDungeon *) spm::memory::__memAlloc(1, sizeof(spm::dan::DanDungeon[DUNGEON_MAX]));
        wii::string::memset(danWp->dungeons, 0, sizeof(DanDungeon[DUNGEON_MAX]));
    }

#ifdef ONE_TIME_TXT_LOAD
    spm::parse::parseInit(sDecompPitText, sDecompPitTextSize);
#else    
    // Prepare pit text to be read
    u32 size = spm::lzss10::lzss10GetDecompSize(pitText); // GCC can't handle lzss10ParseHeader
    char * decompPitText = (char *) spm::memory::__memAlloc(0, size);
    spm::lzss10::lzss10Decompress(pitText, decompPitText);
    spm::parse::parseInit(decompPitText, size);
#endif

    // Add all dungeon entries to work
    while (spm::parse::parsePush("<Dungeon>"))
    {
        // Read no (dungeon id)
        int no = 0;
        int i = 0;
        spm::parse::parseTagGet1("<no>", spm::parse::PARSE_VALUE_TYPE_INT, &no);
        // assertf(144, no >= 0 && no < DUNGEON_MAX, "なんか番号がおかしい [%d]", no);

        // Read item id (chest contents in chest rooms, null & unused elsewhere)
        char itemName[64];
        spm::parse::parseTagGet1("<item>", spm::parse::PARSE_VALUE_TYPE_STRING, itemName);
        danWp->dungeons[no].item = spm::itemdrv::itemTypeNameToId(itemName);

#ifdef CONSTANT_MAP_MASK
        danWp->dungeons[no].map = sMapMask;
#else
        // Read map (bitflags for parts of the map to enable and disable in enemy rooms, 0 & unused elsewhere)
        spm::parse::parseTagGet1("<map>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].map);
#endif

        // Read doors
        while (spm::parse::parsePush("<door>"))
        {
            spm::parse::parseTagGet1("<enter>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].doors[i].enter);
            spm::parse::parseTagGet1("<exit>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].doors[i].exit);
            spm::parse::parsePopNext();
            i++;
        }
        danWp->dungeons[no].doorCount = i;

        // Read enemies
        i = 0;
        while (spm::parse::parsePush("<enemy>"))
        {
            spm::parse::parseTagGet1("<name>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].enemies[i].name);
            spm::parse::parseTagGet1("<num>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].enemies[i].num);
            spm::parse::parseTagGet1("<pos>", spm::parse::PARSE_VALUE_TYPE_INT, &danWp->dungeons[no].enemies[i].pos);
            spm::parse::parsePopNext();
            i++;
        }
        danWp->dungeons[no].enemyCount = i;

        // Move to next dungeon
        spm::parse::parsePopNext();
    }

    // Free pit text
    spm::parse::parsePop();
#ifndef ONE_TIME_TXT_LOAD
    spm::memory::__memFree(0, decompPitText);
#endif

    return 2;
}

static const char * danEnemyRoomMaps[] = {
    // Flipside
    "dan_01",
    "dan_02",
    "dan_03",
    "dan_04",

    // Flopside
    "dan_41",
    "dan_42",
    "dan_43",
    "dan_44"
};

static const char * danChestRoomMaps[] = {
    // Flipside
    "dan_21",
    "dan_22",
    "dan_23",
    "dan_24",

    // Flopside
    "dan_61",
    "dan_62",
    "dan_63",
    "dan_64"
};

void ipDanPatch()
{
    for (u32 i = 0; i < ARRAY_SIZEOF(danEnemyRoomMaps); i++)
        spm::mapdata::mapDataPtr(danEnemyRoomMaps[i])->script = ip_dan_enemy_room_init_evt;
    
    for (u32 i = 0; i < ARRAY_SIZEOF(danChestRoomMaps); i++)
        spm::mapdata::mapDataPtr(danChestRoomMaps[i])->script = ip_dan_chest_room_init_evt;

    writeBranch(spm::dan::evt_dan_read_data, 0, ip_evt_dan_read_data);

#ifdef ONE_TIME_TXT_LOAD
    sDecompPitTextSize = spm::lzss10::lzss10GetDecompSize(pitText);
    sDecompPitText = new char[sDecompPitTextSize];
    spm::lzss10::lzss10Decompress(pitText, sDecompPitText);
    wii::OSError::OSReport("sDecompPitText: %x\n", sDecompPitText);
#endif

#ifdef CONSTANT_MAP_MASK
    wii::OSError::OSReport("sMapMask: %x\n",&sMapMask);
    writeWord(0x800576f8, 0, NOP);
    writeWord(0x8079c238, 0, 0);
#endif

}

}
