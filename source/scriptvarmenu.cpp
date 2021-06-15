#include "mainmenu.h"
#include "scriptvarmenu.h"
#include "util.h"

#include <types.h>
#include <spm/swdrv.h>
#include <spm/system.h>
#include <wii/stdio.h>

namespace mod {

enum
{
    TYPE_BYTE,
    TYPE_WORD,
    TYPE_FLAG
};

struct VarGroup
{
    char name[5];
    u8 type;
    u16 count;
};

enum
{
    GW = 0,  GF = 1,
    GSW = 2, GSWF = 3,
    LSW = 4, LSWF = 5
};

static VarGroup groups[] {
    {"GW",  TYPE_WORD, 32},   {"GF",   TYPE_FLAG, 96},
    {"GSW", TYPE_BYTE, 2048}, {"GSWF", TYPE_FLAG, 8192},
    {"LSW", TYPE_BYTE, 1024}, {"LSWF", TYPE_FLAG, 512}
};

void ScriptVarMenu::updateGroupDisp()
{
    // Change message to current group name
    mGroupDisp->mMsg = groups[mGroup].name;
}

void ScriptVarMenu::updateVarDisp()
{
    // Set select display to each individual decimal digit of id
    wii::stdio::sprintf(mSelectStrs[0], "%d", DIGIT_1000(mId));
    wii::stdio::sprintf(mSelectStrs[1], "%d", DIGIT_100(mId));
    wii::stdio::sprintf(mSelectStrs[2], "%d", DIGIT_10(mId));
    wii::stdio::sprintf(mSelectStrs[3], "%d", DIGIT_1(mId));

    // Set value display to each individual hex digit of the value
    u32 val = getVal();
    wii::stdio::sprintf(mEditStrs[0], "%x", NYBBLE_0(val));
    wii::stdio::sprintf(mEditStrs[1], "%x", NYBBLE_1(val));
    wii::stdio::sprintf(mEditStrs[2], "%x", NYBBLE_2(val));
    wii::stdio::sprintf(mEditStrs[3], "%x", NYBBLE_3(val));
    wii::stdio::sprintf(mEditStrs[4], "%x", NYBBLE_4(val));
    wii::stdio::sprintf(mEditStrs[5], "%x", NYBBLE_5(val));
    wii::stdio::sprintf(mEditStrs[6], "%x", NYBBLE_6(val));
    wii::stdio::sprintf(mEditStrs[7], "%x", NYBBLE_7(val));
}

u32 ScriptVarMenu::getVal()
{
    switch (mGroup)
    {
        case GW:
            return EVT_GET_GW(mId);
        case GF:
            return EVT_GET_GF(mId);
        case GSW:
            return spm::swdrv::swByteGet(mId);
        case GSWF:
            return spm::swdrv::swGet(mId);
        case LSW:
            return spm::swdrv::_swByteGet(mId);
        case LSWF:
            return spm::swdrv::_swGet(mId);
        default:
            return 0; // never reached but stops compiler complaining
    }
}

void ScriptVarMenu::setVal(u32 val)
{
    // No invalid values should make it here but they're asserted
    // just in case to prevent save corruption risks
    switch (mGroup)
    {
        case GW:
            EVT_SET_GW(mId, val);
            break;
        case GF:
            assertf(IS_BOOL(val), "ScriptVarMenu::setVal invalid GF 0x%x", val);
            if (val)
                EVT_SET_GF(mId);
            else
                EVT_CLEAR_GF(mId);
            break;
        case GSW:
            assertf(IS_BYTE(val) || (mId == 0), "ScriptVarMenu::setVal invalid GSW 0x%x", val);
            spm::swdrv::swByteSet(mId, val);
            break;
        case GSWF:
            assertf(IS_BOOL(val), "ScriptVarMenu::setVal invalid GSWF 0x%x", val);
            if (val)
                spm::swdrv::swSet(mId);
            else
                spm::swdrv::swClear(mId);
            break;
        case LSW:
            assertf(IS_BYTE(val), "ScriptVarMenu::setVal invalid LSW 0x%x", val);
            spm::swdrv::_swByteSet(mId, val);
            break;
        case LSWF:
            assertf(IS_BOOL(val), "ScriptVarMenu::setVal invalid LSWF 0x%x", val);
            if (val)
                spm::swdrv::_swSet(mId);
            else
                spm::swdrv::_swClear(mId);
            break;
    }
}

void ScriptVarMenu::groupUp(MenuScroller * scroller, void * param)
{
    (void) scroller;

    // Decrement group and reset selected
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);
    instance->mGroup -= 1;
    instance->mId = 0;

    // Loop around if the end of the group list is reached
    if (instance->mGroup < 0)
        instance->mGroup = ARRAY_SIZEOF(groups) - 1;

    // Process new value
    instance->updateGroupDisp();
    instance->updateVarDisp();
}

void ScriptVarMenu::groupDown(MenuScroller * scroller, void * param)
{
    (void) scroller;

    // Increment group and reset selected
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);
    instance->mGroup += 1;
    instance->mId = 0;

    // Loop around if the end of the group list is reached
    if (instance->mGroup >= (int) ARRAY_SIZEOF(groups))
        instance->mGroup = 0;
    
    // Process new value
    instance->updateGroupDisp();
    instance->updateVarDisp();
}

void ScriptVarMenu::idUp(MenuScroller * scroller, void * param)
{
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);

    // Figure out which digit is being changed and its place value
    s32 increment = 1000;
    for (int i = 0; i < 4; i++)
    {
        if (scroller == instance->mSelectScrollers[i])
            // Use current increment and exit loop
            break;
        else
            // Each digit has a value 10 times lower than the previous
            increment /= 10;
    }

    // Update value
    instance->mId += increment;

    // Set value with prevention of overflow
    if (instance->mId >= groups[instance->mGroup].count)
        instance->mId = groups[instance->mGroup].count - 1;

    // Update display
    instance->updateVarDisp();
}

void ScriptVarMenu::idDown(MenuScroller * scroller, void * param)
{
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);

    // Figure out which digit is being changed and its place value
    s32 increment = 1000;
    for (int i = 0; i < 4; i++)
    {
        if (scroller == instance->mSelectScrollers[i])
            // Use current increment and exit loop
            break;
        else
            // Each digit has a value 10 times lower than the previous
            increment /= 10;
    }

    // Update value
    instance->mId -= increment;

    // Change id with prevention of underflow
    if (instance->mId < 0)
        instance->mId = 0;

    // Update display
    instance->updateVarDisp();
}

void ScriptVarMenu::valUp(MenuScroller * scroller, void * param)
{
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);

    // Figure out which digit is being changed and its place value
    u32 increment = 0x10000000;
    for (int i = 0; i < 8; i++)
    {
        if (scroller == instance->mEditScrollers[i])
            // Exit loop with current increment
            break;
        else
            // Each digit has a value 0x10 times lower than the previous
            increment /= 0x10;
    }

    // Find maximum value
    s64 max;
    u8 type = groups[instance->mGroup].type;
    if ((type == TYPE_WORD) || ((instance->mId == 0) && (instance->mGroup == GSW)))
        max = 0xffffffff;
    else if (type == TYPE_BYTE)
        max = 0xff;
    else
        max = 1;
        
    // Set value with prevention of overflow
    s64 val = instance->getVal();
    if ((val + increment) > max)
        val = max;
    else
        val = val + increment;
    instance->setVal(val);

    // Update display
    instance->updateVarDisp();
}

void ScriptVarMenu::valDown(MenuScroller * scroller, void * param)
{
    ScriptVarMenu * instance = reinterpret_cast<ScriptVarMenu *>(param);

    // Figure out which digit is being changed and its place value
    u32 increment = 0x10000000;
    for (int i = 0; i < 8; i++)
    {
        if (scroller == instance->mEditScrollers[i])
            // Exit loop with current increment
            break;
        else
            // Each digit has a value 0x10 times lower than the previous
            increment /= 0x10;
    }

    // Set value with prevention of underflow
    s64 val = instance->getVal();
    if ((val - increment) < 0)
        val = 0;
    else
        val = val - increment;
    instance->setVal(val);

    // Update display
    instance->updateVarDisp();
}

ScriptVarMenu::ScriptVarMenu()
{
    // Init selection
    mGroup = GW;
    mId = 0;

    // Position constants
    const f32 groupDispX = -200.0f;
    const f32 selectDispX = groupDispX + 85.0f;
    const f32 selectXDiff = 20.0f;
    const f32 middleTextX = selectDispX + (selectXDiff * 4.5f);
    const f32 editDispX = middleTextX + (selectXDiff * 3);
    const f32 dispsY = 20.0f;

    // Init group select buttons
    mGroupDisp = new MenuScroller(this, groups[mGroup].name, groupDispX, dispsY, 20.0f, groupUp, groupDown, this);

    // Init id select buttons
    for (int i = 0; i < 4; i++)
    {
        wii::stdio::sprintf(mSelectStrs[i], "%d", 0);
        mSelectScrollers[i] = new MenuScroller(this, mSelectStrs[i], selectDispX + (selectXDiff * i), dispsY, 0.0f, idUp, idDown, this);
    }
    for (int i = 0; i < 3; i++)
        buttonLinkHorizontal(mSelectScrollers[i], mSelectScrollers[i + 1]);
    buttonLinkHorizontal(mGroupDisp, mSelectScrollers[0]);

    // Init value select buttons
    for (int i = 0; i < 8; i++)
    {
        wii::stdio::sprintf(mEditStrs[i], "%x", 0);
        mEditScrollers[i] = new MenuScroller(this, mEditStrs[i], editDispX + (selectXDiff * i), dispsY, 0.0f, valUp, valDown, this);
    }
    for (int i = 0; i < 7; i++)
        buttonLinkHorizontal(mEditScrollers[i], mEditScrollers[i + 1]);
    buttonLinkHorizontal(mSelectScrollers[3], mEditScrollers[0]);

    // Create cosmetic '= 0x' display
    MenuButton * middleText = new MenuButton(this, "= 0x", middleTextX, dispsY);
    (void) middleText;

    // Set starting button and title
    mCurButton = mGroupDisp;
    mTitle = "Script Variable Editor";
}

void ScriptVarMenu::close()
{
    // Change back to parent menu
    delete MenuWindow::sCurMenu;
    MenuWindow::sCurMenu = new MainMenu();
}

}
