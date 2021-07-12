#include "patch.h"
#include "util.h"

#include "ip_badges.h"

#include <types.h>
#include <spm/pausewin.h>
#include <spm/wpadmgr.h>
#include <wii/stdio.h>
#include <wii/wpad.h>

namespace mod {

using spm::pausewin::PausewinEntry;
using spm::pausewin::PAUSETEX_NONE;
using spm::pausewin::PAUSETEX_CHAPTERS_BTN;
using spm::pausewin::PAUSETEX_CHAPTER_1_BTN;
using spm::pausewin::PAUSETEX_CHAPTER_2_BTN;
using spm::pausewin::PLUSWIN_BTN_CHAPTERS;
using spm::pausewin::PLUSWIN_BTN_BG;
using spm::pausewin::PLUSWIN_BTN_HELP;
using spm::pausewin::PLUSWIN_BTN_STATS;
using spm::pausewin::PLUSWIN_STATE_CHAPTERS;
using spm::pausewin::pluswinWp;
using spm::pausewin::pausewinWp;
using spm::pausewin::pausewinEntry;
using spm::pausewin::pausewinMoveTo;
using spm::pausewin::pausewinAppear;
using spm::pausewin::pausewinDisappear;
using spm::pausewin::pausewinDelete;
using spm::pausewin::pausewinSetMessage;
using spm::wpadmgr::wpadGetButtonsPressed;
using wii::stdio::sprintf;

#define PLUSWIN_BTN_BADGES PLUSWIN_BTN_CHAPTERS
#define PAUSETEX_BADGES_BTN PAUSETEX_CHAPTERS_BTN
#define PLUSWIN_STATE_BADGES PLUSWIN_STATE_CHAPTERS
#define PAUSETEX_BADGES_ALL PAUSETEX_CHAPTER_1_BTN
#define PAUSETEX_BADGES_EQUIPPED PAUSETEX_CHAPTER_2_BTN

#define MAIN_ENTRY(idx) (pausewinWp->entries + pluswinWp->entryIds[(idx)])
#define MAIN_ENTRY_ID(idx) (pluswinWp->entryIds[(idx)])
#define BADGE_ENTRY(idx) (pausewinWp->entries + pluswinWp->submenuEntryIds[(idx)])
#define BADGE_ENTRY_ID(idx) (pluswinWp->submenuEntryIds[(idx)])

enum BadgeSubmenuIds
{
    BADGE_BTN_ALL = 0,
    BADGE_BTN_EQUIPPED = 1,
    // This position is requied, if deleted then this menu is considered closed by the game
    BADGE_BTN_MAIN = 9, 
};

static void badgeMenuMain(PausewinEntry * entry);
static void badgeMenuDisp(PausewinEntry * entry);
static void badgeMenuClose();
static void badgeMenuOpen();

/*
    Opens the badge menu
*/
static void badgeMenuMain(PausewinEntry * entry)
{
    if (wpadGetButtonsPressed(0) & WPAD_BTN_1)
        badgeMenuClose();
}

/*
    Render the badge list
*/
static void badgeMenuDisp(PausewinEntry * entry)
{

}

/*
    Close the badge menu
*/
static void badgeMenuClose()
{
    // Delete submenu elements
    for (u32 i = 0; i < ARRAY_SIZEOF(pluswinWp->submenuEntryIds); i++)
    {
        if (BADGE_ENTRY_ID(i) != -1)
            pausewinDelete(BADGE_ENTRY_ID(i));
    }

    // Show main menu elements
    for (s32 i = 0; i < 8; i++)
    {
        if (i != PLUSWIN_BTN_BADGES)
            pausewinAppear(MAIN_ENTRY_ID(i));
    }
    pausewinAppear(MAIN_ENTRY_ID(PLUSWIN_BTN_STATS));

    // Move back badge button
    PausewinEntry * badgeBtn = MAIN_ENTRY(PLUSWIN_BTN_BADGES);
    f32 x = badgeBtn->originalPos.x;
    f32 y = badgeBtn->originalPos.y;
    pausewinMoveTo(MAIN_ENTRY_ID(PLUSWIN_BTN_BADGES), x, y);

    // Restore help message
    char str[64];
    sprintf(str, "menu_help_%03d", pluswinWp->selectedButton);
    pausewinSetMessage(MAIN_ENTRY(PLUSWIN_BTN_HELP), 0, str);
}

/*
    Store a new element's id and make it appear
*/
static void initElement(s32 id, s32 idArrayIdx)
{
    BADGE_ENTRY_ID(idArrayIdx) = id;
    pausewinAppear(id);
}

static void badgeMenuOpen()
{
    // Hide main menu elements
    for (s32 i = 0; i < 8; i++)
    {
        if (i != PLUSWIN_BTN_BADGES)
            pausewinDisappear(MAIN_ENTRY_ID(i));
    }
    pausewinDisappear(MAIN_ENTRY_ID(PLUSWIN_BTN_STATS));

    // Clear element id array (I think this is left un-initialised?)
    for (u32 i = 0; i < ARRAY_SIZEOF(pluswinWp->submenuEntryIds); i++)
        BADGE_ENTRY_ID(i) = -1;

    // Create submenu elements
    initElement(
        pausewinEntry(-250.0f, 130.0f, 0.0f, 0.0f, 0, PAUSETEX_BADGES_ALL, 0, nullptr, nullptr,
                      nullptr, nullptr, nullptr),
        BADGE_BTN_ALL
    );
    initElement(
        pausewinEntry(-250.0f, 80.0f, 0.0f, 0.0f, 0, PAUSETEX_BADGES_EQUIPPED, 0, nullptr, nullptr,
                      nullptr, nullptr, nullptr),
        BADGE_BTN_EQUIPPED
    );
    initElement(
        pausewinEntry(-120.0f, 130.0f, 375.0f, 230.0f, 1, PAUSETEX_NONE, 0, nullptr, badgeMenuMain,
                      badgeMenuDisp, nullptr, nullptr),
        BADGE_BTN_MAIN
    );
    BADGE_ENTRY(BADGE_BTN_EQUIPPED)->flags |= 0x10;

    // Move badge button to corner and stop oscillating
    PausewinEntry * bg = MAIN_ENTRY(PLUSWIN_BTN_BG);
    f32 x = bg->pos.x + 10.0f;
    f32 y = bg->pos.y - 10.0f;
    pausewinMoveTo(MAIN_ENTRY_ID(PLUSWIN_BTN_BADGES), x, y);
    MAIN_ENTRY(PLUSWIN_BTN_BADGES)->flags &= ~0x100;

    // Init misc values
    pluswinWp->submenuIsFullClose = false;

    // From key item opening, some of these may be wanted:

    /*
    plusWinWp->submenuSelectedButton = 0;
    plusWinWp->field_0x16c = 0;
    plusWinWp->field_0x170 = 0;
    bVar1 = plusWinWp->submenuPausewinIds[0];
    plusWinWp->cursorPos2 = pausewinWp->entries[(char)bVar1].pos;
    id = plusWinWp->pauseWinEntryIds[9];
    pMVar5 = mario_pouch::pouchGetPtr();
    pausewin::pausewinSetMessage
        (pausewinWp->entries + id,(uint)pMVar5->keyItem[plusWinWp->submenuSelectedButton],0);
    plusWinWp->keyItemEvt = NULL;
    plusWinWp->selectedItemId = 0;
    plusWinWp->submenuFlags = 0;
    */
}

/*
    Apply the menu related patches
*/
static void menuPatch()
{
    writeBranch(spm::pausewin::pluswinChapterWinOpen, 0, badgeMenuOpen);
}

/*
    Public function to apply all patches
*/
void ipBadgePatch()
{
    menuPatch();
}

}
