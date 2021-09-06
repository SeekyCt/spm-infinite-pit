#include "patch.h"
#include "util.h"

#include <types.h>
#include <spm/msgdrv.h>
#include <wii/string.h>

namespace ip {

/*
    Custom Message Data
*/
struct CustomMessage
{
    const char * name;
    const char * msg;
};

static CustomMessage msgs[]
{
    // UI
    {
        "menu_help_004",
        "Equip or dequip your badges and\n"
        "view their effects."
    },
    {
        "menu_help_badges_equipped",
        "View all of the badges you have\n"
        "in your inventory."
    },
    {
        "menu_help_badges_all",
        "View the badges that you have\n"
        "currently equipped."
    },

    // Badge Names
    {
        "bn_test_1",
        "Test Badge #1"
    },
    {
        "bn_test_2",
        "Test Badge #2"
    },
    {
        "bn_test_3",
        "Test Badge #3"
    },

    // Badge Descriptions
    {
        "bd_test_1",
        "Testing badge 1"
    },
    {
        "bd_test_2",
        "Testing badge 2"
    },
    {
        "bd_test_3",
        "Testing badge 3"
    },
};

/*
    Public function to apply all patches
*/
static const char * (*msgSearchReal)(const char * msgName);
void messagePatch()
{
    msgSearchReal = mod::patch::hookFunction(spm::msgdrv::msgSearch,
        [](const char * msgName)
        {
            for (u32 i = 0; i < ARRAY_SIZEOF(msgs); i++)
            {
                if (wii::string::strcmp(msgs[i].name, msgName) == 0)
                    return msgs[i].msg;
            }
            return msgSearchReal(msgName);
        }
    );
}

}
