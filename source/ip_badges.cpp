#include "mod_ui_base/window.h"
#include "patch.h"
#include "util.h"

#include "ip_badges.h"

#include <types.h>
#include <spm/icondrv.h>
#include <spm/pausewin.h>
#include <spm/wpadmgr.h>
#include <wii/stdio.h>
#include <wii/wpad.h>

namespace mod {

using spm::icondrv::iconDispGx;
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
using wii::Vec2;
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
#define CUR_BTN (pluswinWp->submenuSelectedButton)

#define BADGE_LINE_HEIGHT 33.0f
#define BADGE_PAGE_SIZE 8

static struct
{
    s8 option; // index into badges on page (0 to BADGE_PAGE_SIZE - 1)
    s8 page; // currently selected page
    s8 pageCount; // number of pages
    s8 pageBadgeCount; // number of badges on page
    s16 badgeCount; // number of badges in all pages
    s16 pageStart; // index of first badge on page
    bool equippedOnly; // whether unequipped badges are displayed
} work;

/*
    Temporary stubs
*/
s32 countBadges()
{
    return 14;
}
s32 countEquippedBadges()
{
    return 4;
}

enum BadgeSubmenuIdx
{
    BADGE_BTN_ALL = 0,
    BADGE_BTN_EQUIPPED = 1,
    // This position is requied, if deleted then this menu is considered closed by the game
    BADGE_BTN_MAIN = 9,
    // No PausewinEntry
    BADGE_BTN_BADGES = 10
};

const wii::RGBA BLACK {0x00, 0x00, 0x00, 0xff};
const wii::RGBA RED {0xff, 0x00, 0x00, 0xff};

static void initPage(s32 page, bool equippedOnly, bool up);
static void initElement(s32 id, s32 idx);
static void moveToBadge();
static void selectButton(s32 idx);
static void menuMain(PausewinEntry * entry);
static void menuDisp(PausewinEntry * entry);
static void menuClose();
static void menuOpen();

/*
    Calculate required values for this page of badges
*/
static void initPage(s32 page, bool equippedOnly, bool up)
{
    // Back up parameters
    work.page = page;
    work.equippedOnly = equippedOnly;

    // Select initial value (bottom or top)
    if (up)
        work.option = BADGE_PAGE_SIZE - 1;
    else
        work.option = 0;

    // Calculate the badge number at the start of this page
    work.pageStart = page * BADGE_PAGE_SIZE;

    // Get count
    if (equippedOnly)
        work.badgeCount = countEquippedBadges();
    else
        work.badgeCount = countBadges();

    // Calculate number of pages (badge count divided by page size, rounded up)
    work.pageCount = (work.badgeCount + BADGE_PAGE_SIZE - 1) / BADGE_PAGE_SIZE;

    // If on the final page and the total count isn't a multiple of 8, the page
    // badge count is the badge count mod 8, otherwise it's just 8
    if (work.page + 1 == work.pageCount && work.badgeCount % BADGE_PAGE_SIZE != 0)
        work.pageBadgeCount = work.badgeCount % BADGE_PAGE_SIZE;
    else
        work.pageBadgeCount = BADGE_PAGE_SIZE;
}

/*
    Stores a new element's id and makes it appear
*/
static void initElement(s32 id, s32 idx)
{
    BADGE_ENTRY_ID(idx) = id;
    pausewinAppear(id);
}

/*
    Moves cursor to the selected badge in the list
*/
static void moveToBadge()
{
    CUR_BTN = BADGE_BTN_BADGES;
    f32 lineY = 155.0f - (33.0f * work.option);
    pluswinWp->cursorMoveDest.x = -25.0f;
    pluswinWp->cursorMoveDest.y = lineY - 20.0f;
}

/*
    Moves cursor to either the equipped or all badges button
    (must not be called for moving to a badge)
*/
static void selectButton(s32 idx)
{
    // Disable highlighting of current button if needed
    if (CUR_BTN == BADGE_BTN_ALL || CUR_BTN == BADGE_BTN_EQUIPPED)
        BADGE_ENTRY(CUR_BTN)->flags &= ~PAUSE_FLAG_HIGHLIGHT;

    // Update current button
    CUR_BTN = idx;

    // Move cursor and enable highlighting
    BADGE_ENTRY(CUR_BTN)->flags |= PAUSE_FLAG_HIGHLIGHT;
    pluswinWp->cursorMoveDest.x = BADGE_ENTRY(CUR_BTN)->pos.x - 25.0f;
    pluswinWp->cursorMoveDest.y = BADGE_ENTRY(CUR_BTN)->pos.y - 20.0f;
}

/*
    Updates the menu
*/
static void menuMain(PausewinEntry * entry)
{
    // Go back if pressing 1
    if (wpadGetButtonsPressed(0) & WPAD_BTN_1)
    {
        if (CUR_BTN == BADGE_BTN_BADGES)
        {
            if (work.equippedOnly)
                selectButton(BADGE_BTN_EQUIPPED);
            else
                selectButton(BADGE_BTN_ALL);
        }
        else
        {
            menuClose();
        }
        return;
    }
    
    // Handle other inputs
    u32 btn = wpadGetButtonsPressed(0);
    switch (CUR_BTN)
    {
        case BADGE_BTN_ALL:
            if (btn & WPAD_BTN_LEFT)
            {
                // Move down to Equipped button
                initPage(0, true, false);
                selectButton(BADGE_BTN_EQUIPPED);
            }
            else if ((btn & WPAD_BTN_DOWN) && countBadges() > 0)
            {
                // Move right to badge list
                moveToBadge();
            }
            break;

        case BADGE_BTN_EQUIPPED:
            if (btn & WPAD_BTN_RIGHT)
            {
                // Move up to All Badges button
                initPage(0, false, false);
                selectButton(BADGE_BTN_ALL);
            }
            else if ((btn & WPAD_BTN_DOWN) && countEquippedBadges() > 0)
            {
                // Move right to badge list
                moveToBadge();
            }
            break;

        case BADGE_BTN_BADGES:
            if (btn & WPAD_BTN_LEFT)
            {
                // Move down one badge
                if (++work.option == work.pageBadgeCount)
                {
                    // Handle being at the bottom of the page

                    if (work.page + 1 == work.pageCount)
                        // Move back if it's the bottom badge of the last page
                        --work.option;
                    else
                        // Move to top of the next page
                        initPage(work.page + 1, work.equippedOnly, false);
                }

                // Move to new badge
                moveToBadge();
            }
            else if (btn & WPAD_BTN_RIGHT)
            {
                // Move up one badge
                if (--work.option == -1)
                {
                    // Handle being at the top of the page

                    if (work.page == 0)
                        // Move back if it's the top of the first page
                        ++work.option;
                    else
                        // Move to the bottom of the previous page
                        initPage(work.page - 1, work.equippedOnly, true);
                }

                // Move to new badge 
                moveToBadge();
            }
            else if (btn & WPAD_BTN_UP)
            {
                // Move to original button
                if (work.equippedOnly)
                    selectButton(BADGE_BTN_EQUIPPED);
                else
                    selectButton(BADGE_BTN_ALL);
            }
            break;

        default:
            break;
    }
}

/*
    Renders the badge list
*/
static void menuDisp(PausewinEntry * entry)
{
    f32 y = 155.0f;
    for (s32 i = work.pageStart; i < work.pageStart + work.pageBadgeCount; i++)
    {
        if (work.option == i - work.pageStart)
            Window::drawString("test string", 0.0f, y, &RED, 0.7f);
        else
            Window::drawString("test string", 0.0f, y, &BLACK, 0.7f);
        y -= BADGE_LINE_HEIGHT;
    }
}

/*
    Closes the badge menu
*/
static void menuClose()
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

    // Restore badge button and cursor
    PausewinEntry * badgeBtn = MAIN_ENTRY(PLUSWIN_BTN_BADGES);
    badgeBtn->flags |= PAUSE_FLAG_OSCILLATE;
    f32 x = badgeBtn->originalPos.x;
    f32 y = badgeBtn->originalPos.y;
    pluswinWp->cursorMoveDest.x = x - 25.0f;
    pluswinWp->cursorMoveDest.y = y - 30.0f;
    pausewinMoveTo(MAIN_ENTRY_ID(PLUSWIN_BTN_BADGES), x, y);

    // Restore help message
    char str[64];
    sprintf(str, "menu_help_%03d", pluswinWp->selectedButton);
    pausewinSetMessage(MAIN_ENTRY(PLUSWIN_BTN_HELP), 0, str);

    // TODO: SFX
}

/*
    Opens the badge menu
*/
static void menuOpen()
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
        pausewinEntry(-120.0f, 170.0f, 375.0f, 280.0f, 1, PAUSETEX_NONE, 0, nullptr, menuMain,
                      menuDisp, nullptr, nullptr),
        BADGE_BTN_MAIN
    );

    // Select default button
    pluswinWp->submenuSelectedButton = -1;
    selectButton(BADGE_BTN_ALL);

    // Move badge button to corner and stop oscillating
    PausewinEntry * bg = MAIN_ENTRY(PLUSWIN_BTN_BG);
    f32 x = bg->pos.x + 10.0f;
    f32 y = bg->pos.y - 10.0f;
    pausewinMoveTo(MAIN_ENTRY_ID(PLUSWIN_BTN_BADGES), x, y);
    MAIN_ENTRY(PLUSWIN_BTN_BADGES)->flags &= ~PAUSE_FLAG_OSCILLATE;

    // Init misc values
    pluswinWp->submenuIsFullClose = false;
    initPage(0, false, false);

    // From key item opening, some of these may be wanted:

    /*
    plusWinWp->field_0x16c = 0;
    plusWinWp->field_0x170 = 0;
    bVar1 = plusWinWp->submenuPausewinIds[0];
    plusWinWp->cursorMoveDest = pausewinWp->entries[(char)bVar1].pos;
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
    writeBranch(spm::pausewin::pluswinChapterWinOpen, 0, menuOpen);
}

/*
    Public function to apply all patches
*/
void ipBadgePatch()
{
    menuPatch();
}

}
