#include "assets/badgetex.h"
#include "patch.h"

#include <types.h>
#include <spm/filemgr.h>
#include <spm/memory.h>
#include <spm/pausewin.h>
#include <spm/system.h>
#include <spm/wpadmgr.h>
#include <wii/stdio.h>
#include <wii/string.h>
#include <wii/tpl.h>
#include <wii/wpad.h>

namespace mod {

using spm::filemgr::FileEntry;
using spm::filemgr::fileAllocf;
using spm::pausewin::PausewinEntry;
using spm::pausewin::PAUSETEX_CHAPTERS;
using spm::pausewin::PLUSWIN_BTN_CHAPTERS;
using spm::pausewin::PLUSWIN_BTN_BG;
using spm::pausewin::PLUSWIN_BTN_HELP;
using spm::pausewin::PLUSWIN_BTN_STATS;
using spm::pausewin::PLUSWIN_STATE_MAIN_MENU;
using spm::pausewin::PLUSWIN_STATE_CHAPTERS;
using spm::pausewin::pluswinWp;
using spm::pausewin::pausewinWp;
using spm::pausewin::pausewinEntry;
using spm::pausewin::pausewinMoveTo;
using spm::pausewin::pausewinAppear;
using spm::pausewin::pausewinDisappear;
using spm::pausewin::pausewinDelete;
using spm::pausewin::pausewinSetMessage;
using wii::tpl::TPLHeader;
using wii::tpl::ImageHeader;
using wii::stdio::sprintf;

#define PLUSWIN_BTN_BADGES PLUSWIN_BTN_CHAPTERS
#define PAUSETEX_BADGES PAUSETEX_CHAPTERS
#define PLUSWIN_STATE_BADGES PLUSWIN_STATE_CHAPTERS

enum BadgeSubmenuIds
{
    // This position is requied, if deleted then this menu is considered closed by the game
    BADGE_BTN_MAIN = 9, 
};

/*
    Override the Chapters button texture
*/
static FileEntry * pauseTplOverride(s32 filetype, const char * format, const char * dvdRoot,
                                    const char * lang)
{
    // Default behaviour at hook
    FileEntry * file = fileAllocf(filetype, format, dvdRoot, lang);   

    // Get chapter button image
    TPLHeader * tpl = (TPLHeader *) file->sp->data;
    ImageHeader * img = tpl->imageTable[PAUSETEX_BADGES].image;

    // Copy custom texture in
    assert(img->width == 120 && img->height == 40 && img->format == 5,
           "Unable to insert badge button texture");
    wii::string::memcpy(img->data, badgeTex, badgeTex_size);

    // Default behaviour at hook
    return file;
}

/*
    Opens the badge menu
*/
static void badgeMenuMain(PausewinEntry * entry);
static void badgeMenuDisp(PausewinEntry * entry);
static void badgeMenuClose();
static void badgeMenuOpen();

static void badgeMenuMain(PausewinEntry * entry)
{
    s8 x  = *(u8 *) 0x80000198;
    s8 y  = *(u8 *) 0x80000199;
    *(u16 *) 0x80000198 = 0xffff;
    if (x != -1)
        pausewinDisappear(pluswinWp->entryIds[x]);
    if (y != -1)
        pausewinAppear(pluswinWp->entryIds[y]);
    
    u32 btn = spm::wpadmgr::wpadGetButtonsPressed(0);
    if (btn & WPAD_BTN_1)
        badgeMenuClose();
}

static void badgeMenuDisp(PausewinEntry * entry)
{

}

static void badgeMenuClose()
{
    // Delete submenu elements
    pausewinDelete(pluswinWp->submenuEntryIds[BADGE_BTN_MAIN]);

    // Show main menu elements
    for (s32 i = 0; i < 8; i++)
    {
        if (i != PLUSWIN_BTN_BADGES)
            pausewinAppear(pluswinWp->entryIds[i]);
    }
    pausewinAppear(pluswinWp->entryIds[PLUSWIN_BTN_STATS]);

    // Move back badge button
    PausewinEntry * badgeBtn = pausewinWp->entries + pluswinWp->entryIds[PLUSWIN_BTN_BADGES];
    f32 x = badgeBtn->originalPos.x;
    f32 y = badgeBtn->originalPos.y;
    pausewinMoveTo(pluswinWp->entryIds[PLUSWIN_BTN_BADGES], x, y);

    // Restore help message
    char str[64];
    sprintf(str, "menu_help_%03d", pluswinWp->selectedButton);
    pausewinSetMessage(pausewinWp->entries + pluswinWp->entryIds[PLUSWIN_BTN_HELP], 0, str);
}

static void badgeMenuOpen()
{
    // Hide main menu elements
    for (s32 i = 0; i < 8; i++)
    {
        if (i != PLUSWIN_BTN_BADGES)
            pausewinDisappear(pluswinWp->entryIds[i]);
    }
    pausewinDisappear(pluswinWp->entryIds[PLUSWIN_BTN_STATS]);

    // Create submenu elements
    s32 id = pausewinEntry(0.0f, 0.0f, 0, 0, 0, 0, 0, nullptr, badgeMenuMain,
                           badgeMenuDisp, nullptr, nullptr);
    pluswinWp->submenuEntryIds[BADGE_BTN_MAIN] = id;
    pausewinAppear(id);

    // Move badge button to corner
    PausewinEntry * bg = pausewinWp->entries + pluswinWp->entryIds[PLUSWIN_BTN_BG];
    f32 x = bg->pos.x + 10.0f;
    f32 y = bg->pos.y - 10.0f;
    pausewinMoveTo(pluswinWp->entryIds[PLUSWIN_BTN_BADGES], x, y);

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
    writeBranchLink(0x80184d6c, 0, pauseTplOverride);
    writeBranch(spm::pausewin::pluswinChapterWinOpen, 0, badgeMenuOpen);
}

/*
    Public function to apply all patches
*/
void ipBadgePatch()
{
    *(u16 *) 0x80000198 = 0xffff;
    menuPatch();
}

}
