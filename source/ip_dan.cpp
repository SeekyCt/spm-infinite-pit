#include "evt_cmd.h"
#include "util.h"
#include "patch.h"

#include <types.h>
#include <spm/evtmgr.h>
#include <spm/evtmgr_cmd.h>
#include <spm/evt_door.h>
#include <spm/evt_map.h>
#include <spm/evt_mobj.h>
#include <spm/evt_hit.h>
#include <spm/evt_npc.h>
#include <spm/evt_snd.h>
#include <spm/evt_sub.h>
#include <spm/hitdrv.h>
#include <spm/hud.h>
#include <spm/itemdrv.h>
#include <spm/item_data.h>
#include <spm/lzss10.h>
#include <spm/lz_embedded.h>
#include <spm/mapdata.h>
#include <spm/mapdrv.h>
#include <spm/mario_pouch.h>
#include <spm/memory.h>
#include <spm/mobjdrv.h>
#include <spm/parse.h>
#include <spm/seq_title.h>
#include <spm/seqdrv.h>
#include <spm/somewhere.h>
#include <spm/system.h>
#include <spm/rel/dan.h>
#include <wii/OSError.h>
#include <wii/stdio.h>
#include <wii/string.h>

using spm::evtmgr::EvtEntry;
using spm::evtmgr::EvtScriptCode;
using spm::dan::DanWork;
using spm::dan::DanDoor;
using spm::dan::DanEnemy;
using spm::dan::DanDungeon;
using spm::dan::danWp;
using spm::lz_embedded::pitText;
using spm::item_data::DAN_KEY;
using spm::item_data::URA_DAN_KEY;
using spm::npcdrv::NPCEnemyTemplate;
using spm::npcdrv::NPCTribe;
using spm::npcdrv::NPCEntry;
using spm::npcdrv::NPCWork;
using spm::itemdrv::ItemWork;
using spm::itemdrv::ItemEntry;
using spm::mobjdrv::MOBJEntry;
using wii::Vec3;

namespace ip {

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
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, false, LW(10))
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, false, LW(11))
    IF_SMALL(GSW(1), 100)
        USER_FUNC(spm::evt_mobj::evt_mobj_zyo, PTR("lock_00"), DAN_KEY, LW(2), LW(3), LW(4), 0, PTR(spm::dan::dan_lock_interact_evt),
                  PTR(spm::dan::dan_lock_open_evt), 0)
    ELSE()
        USER_FUNC(spm::evt_mobj::evt_mobj_zyo, PTR("lock_00"), URA_DAN_KEY, LW(2), LW(3), LW(4), 0, PTR(spm::dan::dan_lock_interact_evt),
                  PTR(spm::dan::dan_lock_open_evt), 0)
    END_IF()
    USER_FUNC(spm::dan::evt_dan_make_spawn_table, LW(0))
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
        USER_FUNC(spm::dan::evt_dan_decide_key_enemy, DAN_KEY)
    ELSE()
        USER_FUNC(spm::dan::evt_dan_decide_key_enemy, URA_DAN_KEY)
    END_IF()
    USER_FUNC(spm::evt_npc::func_80107cfc)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A2"), 0x40000000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A3"), 0x20000000)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, true, false, PTR("S"), 0x2)
    USER_FUNC(spm::evt_map::evt_mapobj_flag4_onoff, true, true, PTR("S"), 0x10)
    RUN_CHILD_EVT(PTR(spm::evt_door::door_init_evt))
    ADD(GSW(1), 1)
    USER_FUNC(spm::evt_snd::evt_snd_bgmon, 0, PTR("BGM_MAP_100F"))
    USER_FUNC(spm::evt_snd::func_800d36f4, 0)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::evt_door_wait_flag, 0x100)
        USER_FUNC(spm::evt_sub::func_800d74a0, 1, 6)
    END_INLINE()
    USER_FUNC(spm::dan::evt_dan_start_countdown)
    USER_FUNC(spm::evt_map::evt_mapobj_trans, PTR("Tetugoushi"), 0, -1000, 0)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, true, true, PTR("Tetugoushi"), 0x1)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::evt_door_wait_flag, 0x100)
        USER_FUNC(spm::evt_npc::func_80107d20)
        USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_init, 11, 255, 0, 1000)
        DO(0)
            USER_FUNC(spm::evt_sub::evt_sub_intpl_msec_get_value)
            USER_FUNC(spm::dan::evt_dan_get_door_names, LW(2), LW(3))
            USER_FUNC(spm::evt_map::evt_mapobj_color, 1, LW(2), 0xff, 0xff, 0xff, LW(0))
            WAIT_FRM(1)
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
        WHILE()
        USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, true, true, LW(2), 0x1)
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
    USER_FUNC(spm::evt_door::evt_door_enable_disable_map_door_desc, false, PTR("doa1_l"))
    SWITCH(GSW(1))
        CASE_EQUAL(9)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(433))
        CASE_EQUAL(19)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(434))
        CASE_EQUAL(29)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(435))
        CASE_EQUAL(39)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(436))
        CASE_EQUAL(49)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(437))
        CASE_EQUAL(59)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(438))
        CASE_EQUAL(69)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(439))
        CASE_EQUAL(79)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(440))
        CASE_EQUAL(89)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(441))
        CASE_EQUAL(109)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(442))
        CASE_EQUAL(119)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(443))
        CASE_EQUAL(129)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(444))
        CASE_EQUAL(139)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(445))
        CASE_EQUAL(149)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(446))
        CASE_EQUAL(159)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(447))
        CASE_EQUAL(169)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(448))
        CASE_EQUAL(179)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(449))
        CASE_EQUAL(189)
            USER_FUNC(spm::evt_mobj::evt_mobj_thako, 1, PTR("box"), 75, 25, FLOAT(-87.5), PTR(nullptr),
                      PTR(spm::dan::dan_chest_open_evt), 0, GSWF(450))
    END_SWITCH()
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, false, PTR("box"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, true, PTR("box"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_kan, 0, PTR("_kanban"), -280, 0, -158, 0, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_kan, 0, PTR("kanban"), -280, 0, FLOAT(-149.8994140625),
              PTR(&spm::dan::dan_exit_pipe_sign_interact_evt), 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, false, PTR("_kanban"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, false, PTR("kanban"), 0x4040)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, true, PTR("_kanban"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, true, PTR("kanban"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_arrow, PTR("_arrow"), -75, 0, -158, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_arrow, PTR("arrow"), -75, 0, -150, 0)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, false, PTR("_arrow"), 0x40)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, false, PTR("arrow"), 0x4040)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, true, PTR("_arrow"), 0x10000)
    USER_FUNC(spm::evt_mobj::evt_mobj_flag_onoff, true, true, PTR("arrow"), 0x10000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A2"), 0x40000000)
    USER_FUNC(spm::evt_hit::evt_hitobj_attr_onoff, true, true, PTR("A3"), 0x20000000)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, true, false, PTR("S"), 2)
    USER_FUNC(spm::evt_map::evt_mapobj_flag4_onoff, true, true, PTR("S"), 16)
    RUN_CHILD_EVT(spm::evt_door::door_init_evt)
    ADD(GSW(1), 1)
    USER_FUNC(spm::evt_sub::evt_sub_random, 100, LW(0))
    IF_SMALL(LW(0), 30)
        USER_FUNC(spm::evt_npc::evt_npc_entry, PTR("roten"), PTR("n_machi_roten"), 0)
        USER_FUNC(spm::evt_npc::func_801059d0, PTR("roten"), -1)
        USER_FUNC(spm::evt_npc::evt_npc_set_property, PTR("roten"), 14, PTR(spm::dan::rotenTribeAnimDefs))
        USER_FUNC(spm::evt_npc::evt_npc_set_anim, PTR("roten"), 0, 1)
        USER_FUNC(spm::evt_npc::evt_npc_flag8_onoff, PTR("roten"), true, 0x4400004)
        USER_FUNC(spm::evt_npc::evt_npc_animflag_onoff, PTR("roten"), true, 0x20)
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
        USER_FUNC(spm::evt_door::evt_door_wait_flag, 0x100)
        USER_FUNC(spm::evt_sub::func_800d74a0, 1, 6)
    END_INLINE()
    USER_FUNC(spm::evt_map::evt_mapobj_trans, PTR("Tetugoushi"), 0, -1000, 0)
    USER_FUNC(spm::evt_map::evt_mapobj_flag_onoff, 1, 1, PTR("Tetugoushi"), 1)
    INLINE_EVT()
        USER_FUNC(spm::evt_door::evt_door_wait_flag, 0x100)
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
//#define CONSTANT_MAP_MASK
//#define CONSTANT_ENTRY_DOOR

#ifdef ONE_TIME_TXT_LOAD
static char * sDecompPitText;
static u32 sDecompPitTextSize;
#endif

#ifdef CONSTANT_MAP_MASK
static u32 sMapMask = 0;
#endif

#ifdef CONSTANT_ENTRY_DOOR
static u32 sEnterDoor = 0;
#endif

s32 ip_evt_dan_read_data(EvtEntry * entry, bool isFirstCall)
{
    (void) entry;

    // Allocate work on first run
    // (check is a relD leftover, this only runs once on retail anyway)
    if (isFirstCall)
    {
        danWp = new (spm::memory::Heap::MAP) DanWork;
        wii::string::memset(danWp, 0, sizeof(*danWp));
        danWp->dungeons = new (spm::memory::Heap::MAP) DanDungeon[DUNGEON_MAX];
        wii::string::memset(danWp->dungeons, 0, sizeof(DanDungeon[DUNGEON_MAX]));
    }

#ifdef ONE_TIME_TXT_LOAD
    spm::parse::parseInit(sDecompPitText, sDecompPitTextSize);
#else    
    // Prepare pit text to be read
    u32 size = spm::lzss10::lzss10GetDecompSize(pitText); // GCC can't handle lzss10ParseHeader
    char * decompPitText = new char[size];
    spm::lzss10::lzss10Decompress(pitText, decompPitText);
    spm::parse::parseInit(decompPitText, size);
#endif

    // Add all dungeon entries to work
    while (spm::parse::parsePush("<Dungeon>"))
    {
        // Read no (dungeon id)
        s32 no = 0;
        s32 i = 0;
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
    delete decompPitText;
#endif

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_handle_map_parts(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, entry->pCurData[0]);
    DanDungeon * dungeon = danWp->dungeons + no;

    // Turn off all parts by default
    spm::mapdrv::mapGrpFlagOn(false, "parts", MAPOBJ_FLAG_HIDE);
    spm::hitdrv::hitGrpFlagOn(false, "A2_parts", HITOBJ_FLAG_DISABLE);
    spm::mapdrv::mapGrpFlag4On(false, "block", 0x20);

    // Enable parts from map mask
    for (u32 i = 0; i < DAN_PARTS_COUNT; i++)
    {
        if (dungeon->map & (1 << i))
        {
            char a2Part[256];
            spm::mapdrv::mapGrpFlagOff(false, spm::dan::danMapParts[i], MAPOBJ_FLAG_HIDE);
            wii::stdio::sprintf(a2Part, "A2_%s", spm::dan::danMapParts[i]);
            spm::hitdrv::hitGrpFlagOff(false, a2Part, HITOBJ_FLAG_DISABLE);
        }
    }

    // Enable merged parts where possible
    if (CHECK_ALL_MASK(dungeon->map, 0xC))
    {
        spm::mapdrv::mapGrpFlagOff(false, "parts_12_a", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_12_b", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_12_c", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2_parts_12_a", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_12_b", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_12_c", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ALL_MASK(dungeon->map, 0xc0))
    {
        spm::mapdrv::mapGrpFlagOff(false, "parts_09_a", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_09_b", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_09_c", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2_parts_09_a", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_09_b", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_09_c", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ALL_MASK(dungeon->map, 0x300))
    {
        spm::mapdrv::mapGrpFlagOff(false, "parts_11_a", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_11_b", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_11_c", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2_parts_11_a", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_11_b", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_11_c", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ALL_MASK(dungeon->map, 0x3000))
    {
        spm::mapdrv::mapGrpFlagOff(false, "parts_10_a", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_10_b", MAPOBJ_FLAG_HIDE);
        spm::mapdrv::mapGrpFlagOn(false, "parts_10_c", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2_parts_10_a", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_10_b", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOn(false, "A2_parts_10_c", HITOBJ_FLAG_DISABLE);
    }

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_handle_dokans(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, entry->pCurData[0]);
    DanDungeon * dungeon = danWp->dungeons + no;

    // Turn off all pipes by default
    spm::mapdrv::mapGrpFlagOn(false, "dokan", MAPOBJ_FLAG_HIDE);
    spm::hitdrv::hitGrpFlagOn(false, "A2D_dokan", HITOBJ_FLAG_DISABLE);
    spm::hitdrv::hitGrpFlagOn(false, "A3D_dokan", HITOBJ_FLAG_DISABLE);

    // Turn on enabled pipes
    if (CHECK_ANY_MASK(dungeon->map, 0x10000)) {
        spm::mapdrv::mapGrpFlagOff(false, "dokan_01", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_01", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_01", HITOBJ_FLAG_DISABLE);
        spm::mapdrv::mapGrpFlagOff(false, "dokan_02", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_02", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_02", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ANY_MASK(dungeon->map, 0x20000)) {
        spm::mapdrv::mapGrpFlagOff(false, "dokan_03", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_03", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_03", HITOBJ_FLAG_DISABLE);
        spm::mapdrv::mapGrpFlagOff(false, "dokan_04", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_04", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_04", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ANY_MASK(dungeon->map, 0x40000)) {
        spm::mapdrv::mapGrpFlagOff(false, "dokan_05", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_05", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_05", HITOBJ_FLAG_DISABLE);
        spm::mapdrv::mapGrpFlagOff(false, "dokan_06", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_06", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_06", HITOBJ_FLAG_DISABLE);
    }
    if (CHECK_ANY_MASK(dungeon->map, 0x80000)) {
        spm::mapdrv::mapGrpFlagOff(false, "dokan_07", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_07", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_07", HITOBJ_FLAG_DISABLE);
        spm::mapdrv::mapGrpFlagOff(false, "dokan_08", MAPOBJ_FLAG_HIDE);
        spm::hitdrv::hitGrpFlagOff(false, "A2D_dokan_08", HITOBJ_FLAG_DISABLE);
        spm::hitdrv::hitGrpFlagOff(false, "A3D_dokan_08", HITOBJ_FLAG_DISABLE);
    }

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_handle_doors(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon and room
    EvtScriptCode * args = entry->pCurData;
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, args[0]);
    s32 room = spm::evtmgr_cmd::evtGetValue(entry, args[1]);
    DanDungeon * dungeon = danWp->dungeons + no;

    // Hide all doors by default
    spm::mapdrv::mapGrpFlagOn(false, "doa", MAPOBJ_FLAG_HIDE);

    // Determine which door definition to use
    // (room is an internal name from relD debug prints)
    if (room > dungeon->doorCount)
        room = 1;
    if (room == 0)
        room = (spm::system::rand() % dungeon->doorCount) + 1;
    room -= 1;

    // Store door ids
#ifdef CONSTANT_ENTRY_DOOR
    danWp->doorInfo.enter = sEnterDoor;
#else
    danWp->doorInfo.enter = dungeon->doors[room].enter;
#endif
    danWp->doorInfo.exit = dungeon->doors[room].exit;

    char str[64];

    // Show enter door & make tangible
    wii::stdio::sprintf(str, "doa_%02d", danWp->doorInfo.enter);
    spm::mapdrv::mapGrpFlagOff(false, str, MAPOBJ_FLAG_HIDE);
    wii::stdio::sprintf(str, "A2_doa_%02d", danWp->doorInfo.enter);
    spm::hitdrv::hitGrpFlagOff(false, str, HITOBJ_FLAG_DISABLE);
    wii::stdio::sprintf(str, "A3_doa_%02d", danWp->doorInfo.enter);
    spm::hitdrv::hitGrpFlagOff(false, str, HITOBJ_FLAG_DISABLE);

    // Show exit door & make tangible
    wii::stdio::sprintf(str, "doa_%02d", danWp->doorInfo.exit);
    spm::mapdrv::mapGrpFlagOff(0, str, 1);
    wii::stdio::sprintf(str, "A2_doa_%02d", danWp->doorInfo.exit);
    spm::hitdrv::hitGrpFlagOff(0, str, 1);
    wii::stdio::sprintf(str, "A3_doa_%02d", danWp->doorInfo.exit);
    spm::hitdrv::hitGrpFlagOff(0, str, 1);

    // Generate names for enter DoorDesc
    wii::stdio::sprintf(danWp->enterDoorName_l, "doa%d_l", danWp->doorInfo.enter);
    wii::stdio::sprintf(danWp->enterDoorName_r, "doa%d_r", danWp->doorInfo.enter);
    wii::stdio::sprintf(danWp->enterDoorHitName2d, "A2_doa_%02d", danWp->doorInfo.enter);
    wii::stdio::sprintf(danWp->enterDoorHitName3d, "A3_doa_%02d", danWp->doorInfo.enter);
    wii::stdio::sprintf(danWp->prevMapName, "");
    wii::stdio::sprintf(danWp->enterDoor_desc0x18, "");

    // Generate names for exit DoorDesc
    wii::stdio::sprintf(danWp->exitDoorName_l, "doa%d_l", danWp->doorInfo.exit);
    wii::stdio::sprintf(danWp->exitDoorName_r, "doa%d_r", danWp->doorInfo.exit);
    wii::stdio::sprintf(danWp->exitDoorHitName2d, "A2_doa_%02d", danWp->doorInfo.exit);
    wii::stdio::sprintf(danWp->exitDoorHitName3d, "A3_doa_%02d", danWp->doorInfo.exit);
    wii::stdio::sprintf(danWp->nextMapName, spm::seq_title::getNextDanMapname(no + 1));
    wii::stdio::sprintf(danWp->exitDoor_desc0x18, "");

    // Fill in enter DoorDesc
    spm::dan::danMapDoorDescs[0].name = danWp->enterDoorName_l;
    spm::dan::danMapDoorDescs[0].name_r = danWp->enterDoorName_r;
    spm::dan::danMapDoorDescs[0].hitName2d = danWp->enterDoorHitName2d;
    spm::dan::danMapDoorDescs[0].hitName3d = danWp->enterDoorHitName3d;
    spm::dan::danMapDoorDescs[0].destMapName = danWp->prevMapName;
    spm::dan::danMapDoorDescs[0].unknown_0x18 = danWp->enterDoor_desc0x18;

    // Fill in exit DoorDesc
    spm::dan::danMapDoorDescs[1].name = danWp->exitDoorName_l;
    spm::dan::danMapDoorDescs[1].name_r = danWp->exitDoorName_r;
    spm::dan::danMapDoorDescs[1].hitName2d = danWp->exitDoorHitName2d;
    spm::dan::danMapDoorDescs[1].hitName3d = danWp->exitDoorHitName3d;
    spm::dan::danMapDoorDescs[1].destMapName = danWp->nextMapName;
    spm::dan::danMapDoorDescs[1].unknown_0x18 = danWp->exitDoor_desc0x18;

    // Output door name
    wii::stdio::sprintf(danWp->enterDoorName, "doa_%02d", danWp->doorInfo.enter);
    wii::stdio::sprintf(danWp->exitDoorName, "doa_%02d", danWp->doorInfo.exit);
    wii::string::strcpy(spm::spmario::gp->doorName, danWp->enterDoorName_l);
    spm::evtmgr_cmd::evtSetValue(entry, args[2], (s32) danWp->enterDoorName_l);
    spm::evtmgr_cmd::evtSetValue(entry, args[3], (s32) danWp->exitDoorName_l);

    // Unknown
    spm::evtmgr_cmd::evtSetValue(entry, GSWF(23), 0);

    // Output lock position
    Vec3 doorPos;
    spm::hitdrv::hitObjGetPos(danWp->exitDoorHitName2d, &doorPos);
    spm::evtmgr_cmd::evtSetFloat(entry, args[4], doorPos.x);
    spm::evtmgr_cmd::evtSetFloat(entry, args[5], doorPos.y - 40.0f);
    spm::evtmgr_cmd::evtSetFloat(entry, args[6], doorPos.z);

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_get_door_names(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    EvtScriptCode * args = entry->pCurData;
    spm::evtmgr_cmd::evtSetValue(entry, args[0], (s32) danWp->enterDoorName);
    spm::evtmgr_cmd::evtSetValue(entry, args[1], (s32) danWp->exitDoorName);

    return EVT_RET_CONTINUE;
}


s32 ip_evt_dan_get_exit_door_name_l(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    EvtScriptCode * args = entry->pCurData;
    spm::evtmgr_cmd::evtSetValue(entry, args[0], (s32) danWp->exitDoorName_l);

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_get_enemy_info(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon and enemy index
    EvtScriptCode * args = entry->pCurData;    
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, args[0]);
    s32 enemyIdx = spm::evtmgr_cmd::evtGetValue(entry, args[1]);
    DanDungeon * dungeon = danWp->dungeons + no;

    if ((enemyIdx < 0) || (enemyIdx >= 16))
    {
        // Return 0 for invalid enemies
        spm::evtmgr_cmd::evtSetValue(entry, args[2], 0);
        spm::evtmgr_cmd::evtSetValue(entry, args[3], 0);

        return EVT_RET_CONTINUE;
    }
    else
    {
        // Get enemy
        DanEnemy * enemy = dungeon->enemies + enemyIdx;

        if (enemy->num > 0)
        {
            // Find template with correct tribe id
            s32 tribeId = enemy->name - 1;
            s32 i;
            NPCEnemyTemplate * curTemplate = spm::npcdrv::npcEnemyTemplates;
            for (i = 0; i < NPCTEMPLATE_MAX; i++, curTemplate++)
            {
                if (((curTemplate->unknown_0x8 & 1) == 0) && (curTemplate->tribeId == tribeId))
                    break;
            }
            // assertf(628, i < NPCTEMPLATE_MAX, "みつかりませんでした[%d]", tribeId);

            // Return template id and num
            spm::evtmgr_cmd::evtSetValue(entry, args[2], i);
            spm::evtmgr_cmd::evtSetValue(entry, args[3], enemy->num);
        }
        else
        {
            // Return 0 for empty enemy slots
            spm::evtmgr_cmd::evtSetValue(entry, args[2], 0);
            spm::evtmgr_cmd::evtSetValue(entry, args[3], 0);
        }

        return EVT_RET_CONTINUE;
    }
}

inline void ip_danPushSpawnTable(s32 doorId)
{
    danWp->spawnTable[danWp->spawnTableCount++] = doorId;
}

s32 ip_evt_dan_make_spawn_table(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, entry->pCurData[0]);
    DanDungeon * dungeon = danWp->dungeons + no;

    // Build spawn table with all available doors
    danWp->spawnTableCount = 0;
    if (CHECK_ANY_MASK(dungeon->map, 0x400))
        ip_danPushSpawnTable(1);
    ip_danPushSpawnTable(2);
    ip_danPushSpawnTable(3);
    if (CHECK_ANY_MASK(dungeon->map, 0x1000))
    {
        ip_danPushSpawnTable(4);
        ip_danPushSpawnTable(5);
    }
    ip_danPushSpawnTable(6);
    ip_danPushSpawnTable(7);
    if (CHECK_ANY_MASK(dungeon->map, 0x8000))
        ip_danPushSpawnTable(8);
    if (CHECK_ANY_MASK(dungeon->map, 0x40))
    {
        ip_danPushSpawnTable(10);
        ip_danPushSpawnTable(11);
    }
    ip_danPushSpawnTable(12);
    ip_danPushSpawnTable(13);
    if (CHECK_ANY_MASK(dungeon->map, 0x100))
    {
        ip_danPushSpawnTable(14);
        ip_danPushSpawnTable(15);
    }
    if (CHECK_ANY_MASK(dungeon->map, 0x1))
        ip_danPushSpawnTable(17);
    ip_danPushSpawnTable(18);
    ip_danPushSpawnTable(19);
    if (CHECK_ANY_MASK(dungeon->map, 0x4))
    {
        ip_danPushSpawnTable(20);
        ip_danPushSpawnTable(21);
    }
    ip_danPushSpawnTable(22);
    ip_danPushSpawnTable(23);
    if (CHECK_ANY_MASK(dungeon->map, 0x20))
        ip_danPushSpawnTable(24);
    ip_danPushSpawnTable(25);
    ip_danPushSpawnTable(26);
    ip_danPushSpawnTable(27);
    ip_danPushSpawnTable(28);
    ip_danPushSpawnTable(29);
    ip_danPushSpawnTable(30);
    ip_danPushSpawnTable(31);
    ip_danPushSpawnTable(32);

    // Randomise spawn table
    for (s32 i = 0; i < 100; i++)
    {
        s32 idx1 = spm::system::rand() % danWp->spawnTableCount;
        s32 idx2 = spm::system::rand() % danWp->spawnTableCount;
        s32 temp = danWp->spawnTable[idx1];
        danWp->spawnTable[idx1] = danWp->spawnTable[idx2];
        danWp->spawnTable[idx2] = temp;
    }

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_get_enemy_spawn_pos(EvtEntry * entry, bool isInitialCall)
{
    (void) isInitialCall;

    // Get enemy number, dungeon, and enemy
    EvtScriptCode * args = entry->pCurData;
    s32 enemyNum = spm::evtmgr_cmd::evtGetValue(entry, args[0]);
    s32 no = spm::evtmgr_cmd::evtGetValue(entry, args[1]);
    s32 enemyIdx = spm::evtmgr_cmd::evtGetValue(entry, args[2]);
    DanDungeon * dungeon = danWp->dungeons + no;
    DanEnemy * enemy = dungeon->enemies + enemyIdx;

    // Find the (enemyNum % danWp->spawnTableCount)th available door
    s32 targetPos = enemyNum % danWp->spawnTableCount;
    s32 j = 0;
    char doorName[64];
    if (enemy->pos != 0)
    {
        wii::stdio::sprintf(doorName, "A2_doa_%02d", enemy->pos);
    }
    else
    {
        s32 i;
        for (i = 0; i < danWp->spawnTableCount; i++)
        {
            if ((danWp->doorInfo.enter != danWp->spawnTable[i]) && (danWp->doorInfo.exit != danWp->spawnTable[i]))
            {
                if (j == targetPos)
                    break;
                j++;
            }
        }
        wii::stdio::sprintf(doorName, "A2_doa_%02d", danWp->spawnTable[i]);
    }

    // Get the position of the door to spawn the enemy at
    Vec3 doorPos;
    spm::hitdrv::hitObjGetPos(doorName, &doorPos);
    doorPos.z = 0.0f;

    // Unknown, outputs coords with some changes if needed
    f32 f1, f2, f3, f4, f5, f6, f7;
    f4 = 1000.0f;
    if (spm::hitdrv::hitCheckFilter(doorPos.x, doorPos.y, 0.0f, 0.0f, -1.0f, 0.0f, nullptr, &f1, &f2, &f3, &f4, &f5, &f6, &f7))
    {
        spm::evtmgr_cmd::evtSetFloat(entry, args[3], f1);
        spm::evtmgr_cmd::evtSetFloat(entry, args[4], f2);
        spm::evtmgr_cmd::evtSetFloat(entry, args[5], f3);
    }
    else
    {
        spm::evtmgr_cmd::evtSetFloat(entry, args[3], doorPos.x);
        spm::evtmgr_cmd::evtSetFloat(entry, args[4], doorPos.y);
        spm::evtmgr_cmd::evtSetFloat(entry, args[5], doorPos.z);
    }

    return EVT_RET_CONTINUE;
}

s32 ip_evt_dan_decide_key_enemy(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get the item id of the key
    s32 itemId = spm::evtmgr_cmd::evtGetValue(entry, entry->pCurData[0]);

    // Make a list of all available enemies
    NPCWork * npcWp = spm::npcdrv::npcGetWorkPtr();
    NPCEntry * curNpc = npcWp->entries;
    s32 enemyCount = 0;
    NPCEntry * enemies[80];
    for (s32 i = 0; i < npcWp->num; curNpc++, i++)
    {
        if (CHECK_ANY_MASK(curNpc->flags_8, 0x1) && !CHECK_ANY_MASK(curNpc->flags_8, 0x40000))
            enemies[enemyCount++] = curNpc;
    }

    // Allocate key
    enemies[spm::system::rand() % enemyCount]->dropItemId = itemId;

    return EVT_RET_CONTINUE;
}

void ip_danCountdownDone()
{
    // Kill the player
    spm::seqdrv::seqSetSeq(spm::seqdrv::SEQ_GAMEOVER, nullptr, nullptr);
}

s32 ip_evt_dan_start_countdown(EvtEntry * entry, bool isFirstCall)
{
    (void) entry;
    (void) isFirstCall;

    // Start the 5 minute timer
    spm::hud::hudStartCountdown(300, spm::dan::danCountdownDone);

    return EVT_RET_CONTINUE;
}

bool ip_danCheckKeyInMapBbox()
{
    // Get the item list
    ItemWork * itemWp = spm::itemdrv::itemGetWork();
    ItemEntry * item = itemWp->entries;

    // Get the range of valid coordinates
    Vec3 min, max;
    spm::hitdrv::hitGetMapEntryBbox(0, &min, &max);
    
    // Check whether any item is the key within valid coordinates
    int itemCount = itemWp->num;
    int i;
    for (i = 0; i < itemCount; i++, item++)
    {
        if (
            CHECK_ANY_MASK(item->flags, 0x1) &&
            ((item->type == DAN_KEY) || (item->type == URA_DAN_KEY)) &&
            (min.x <= item->position.x) && (max.x >= item->position.x) &&
            (min.y <= item->position.y) && (max.y >= item->position.y) &&
            (min.z <= item->position.z) && (max.z >= item->position.z)
        )
            break;
    }

    if (i < itemCount)
        return true;
    else
        return false;
}

bool ip_danCheckKeyEnemyInMapBbox()
{
    // Get the enemy list
    NPCWork * npcWp = spm::npcdrv::npcGetWorkPtr();
    NPCEntry * npc = npcWp->entries;

    // Get the range of valid coordinates
    Vec3 min, max;
    spm::hitdrv::hitGetMapEntryBbox(0, &min, &max);

    // Maybe a typo?
    min.x -= 5.0f;
    min.y -= 5.0f;
    min.z -= 5.0f;
    min.x += 5.0f;
    min.y += 5.0f;
    min.z += 5.0f;

    // Check whether any NPC is an enemy with the key within valid coordinates
    int npcCount = npcWp->num;
    int i;
    for (i = 0; i < npcCount; i++, npc++)
    {
        if (CHECK_ANY_MASK(npc->flags_8, 0x1))
        {
            Vec3 tempMin = {min.x, min.y, min.z};
            tempMin.y -= npc->unknown_0x3ac;
            if (
                (tempMin.x <= npc->position.x) && (max.x >= npc->position.x) &&
                (tempMin.y <= npc->position.y) && (max.y >= npc->position.y) &&
                (tempMin.z <= npc->position.z) && (max.z >= npc->position.z)
            )
                break;
        }
    }

    if (i < npcCount)
        return true;
    else
        return false;
}

int ip_evt_dan_handle_key_failsafe(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    using spm::dan::danCheckKeyEnemyInMapBbox;
    using spm::dan::danCheckKeyInMapBbox;
    using spm::mario_pouch::pouchCheckHaveItem;
    using spm::itemdrv::itemCheckForId;

    // Check whether the key exists anywhere
    if (
        !danCheckKeyEnemyInMapBbox() && !danCheckKeyInMapBbox() &&
        !pouchCheckHaveItem(DAN_KEY) && !pouchCheckHaveItem(URA_DAN_KEY) &&
        !itemCheckForId(DAN_KEY) && !itemCheckForId(URA_DAN_KEY)
    )
    {
        // Spawn the key at the lock if not
        MOBJEntry * lock = spm::mobjdrv::mobjNameToPtr("lock_00");
        s32 keyId = DAN_KEY;
        if (spm::evtmgr_cmd::evtGetValue(entry, GSW(1)) >= 100)
            keyId = URA_DAN_KEY;
        
        spm::itemdrv::itemEntry(nullptr, lock->pos.x, lock->pos.y, 0.0f, keyId, 1, nullptr, 0);
        spm::somewhere::func_800cd554(lock->pos.x, lock->pos.y, 0.0f, 0.0f, -1.0f, 0.0f, 4, 8);
        spm::somewhere::func_800b426c(lock->pos.x, lock->pos.y, 0.0f, 1, 0);

        return EVT_RET_CONTINUE;
    }
    else
    {
        return EVT_RET_BLOCK_WEAK;
    }
}

int ip_evt_dan_handle_chest_room_dokans_and_doors(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon number
    int no = spm::evtmgr_cmd::evtGetValue(entry, entry->pCurData[0]);
    
    // Update destination of exit door
    spm::dan::danChestRoomMapDoorDescs[1].destMapName = spm::seq_title::getNextDanMapname(no + 1);

    // Set the entering door name
    wii::string::strcpy(spm::spmario::gp->doorName, "doa1_l");
    
    // Set exit pipe destination
    if (no < 100)
    {
        // Flipside pit
        spm::dan::danChestRoomDokanDesc.destMapName = "mac_05";
        spm::dan::danChestRoomDokanDesc.unknown_0x1c = "dokan_1";
    }
    else
    {
        // Flopside pit
        spm::dan::danChestRoomDokanDesc.destMapName = "mac_15";
        spm::dan::danChestRoomDokanDesc.unknown_0x1c = "dokan_1";
    }

    return EVT_RET_CONTINUE;
}

int ip_evt_dan_get_chest_room_item(EvtEntry * entry, bool isFirstCall)
{
    (void) isFirstCall;

    // Get dungeon number
    EvtScriptCode * args = entry->pCurData;
    int no = spm::evtmgr_cmd::evtGetValue(entry, args[0]);
    
    // Return the item in this room's chest
    spm::evtmgr_cmd::evtSetValue(entry, args[1], danWp->dungeons[no - 1].item);

    return EVT_RET_CONTINUE;
}

int ip_evt_dan_boss_room_set_door_name(EvtEntry * entry, bool isFirstCall)
{
    (void) entry;
    (void) isFirstCall;

    // Set the entering door name
    wii::string::strcpy(spm::spmario::gp->doorName, "doa1_l");

    return EVT_RET_CONTINUE;
}

// func_80c839cc, wracktailDispCb, evt_dan_set_wracktail_disp_cb, func_80c83c48,
// screenBlinkDisp, evt_dan_screen_blink, func_80c83f6c not re-implemented

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

void danPatch()
{
    for (u32 i = 0; i < ARRAY_SIZEOF(danEnemyRoomMaps); i++)
        spm::mapdata::mapDataPtr(danEnemyRoomMaps[i])->script = ip_dan_enemy_room_init_evt;
    
    for (u32 i = 0; i < ARRAY_SIZEOF(danChestRoomMaps); i++)
        spm::mapdata::mapDataPtr(danChestRoomMaps[i])->script = ip_dan_chest_room_init_evt;

    writeBranch(spm::dan::evt_dan_read_data, 0, ip_evt_dan_read_data);
    writeBranch(spm::dan::evt_dan_handle_map_parts, 0, ip_evt_dan_handle_map_parts);
    writeBranch(spm::dan::evt_dan_handle_dokans, 0, ip_evt_dan_handle_dokans);
    writeBranch(spm::dan::evt_dan_handle_doors, 0, ip_evt_dan_handle_doors);
    writeBranch(spm::dan::evt_dan_get_door_names, 0, ip_evt_dan_get_door_names);
    writeBranch(spm::dan::evt_dan_get_exit_door_name_l, 0, ip_evt_dan_get_exit_door_name_l);
    writeBranch(spm::dan::evt_dan_get_enemy_info, 0, ip_evt_dan_get_enemy_info);
    writeBranch(spm::dan::evt_dan_make_spawn_table, 0, ip_evt_dan_make_spawn_table);
    writeBranch(spm::dan::evt_dan_get_enemy_spawn_pos, 0, ip_evt_dan_get_enemy_spawn_pos);
    writeBranch(spm::dan::evt_dan_decide_key_enemy, 0, ip_evt_dan_decide_key_enemy);
    writeBranch(spm::dan::danCountdownDone, 0, ip_danCountdownDone);
    writeBranch(spm::dan::evt_dan_start_countdown, 0, ip_evt_dan_start_countdown);
    writeBranch(spm::dan::danCheckKeyInMapBbox, 0, ip_danCheckKeyInMapBbox);
    writeBranch(spm::dan::danCheckKeyEnemyInMapBbox, 0, ip_danCheckKeyEnemyInMapBbox);
    writeBranch(spm::dan::evt_dan_handle_key_failsafe, 0, ip_evt_dan_handle_key_failsafe);
    writeBranch(spm::dan::evt_dan_handle_chest_room_dokans_and_doors, 0, ip_evt_dan_handle_chest_room_dokans_and_doors);
    writeBranch(spm::dan::evt_dan_get_chest_room_item, 0, ip_evt_dan_get_chest_room_item);
    writeBranch(spm::dan::evt_dan_boss_room_set_door_name, 0, ip_evt_dan_boss_room_set_door_name);

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

#ifdef CONSTANT_ENTRY_DOOR
    wii::OSError::OSReport("sEnterDoor: %x\n",&sEnterDoor);
#endif

}

}
