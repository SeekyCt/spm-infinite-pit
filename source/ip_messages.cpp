#include "patch.h"
#include "util.h"

#include <types.h>
#include <spm/msgdrv.h>
#include <wii/string.h>

namespace mod {

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
    {
        "menu_help_004",
        "Equip & dequip your badges and\n"
        "view their effects."
    }
};

/*
    Public function to apply all patches
*/
static const char * (*msgSearchReal)(const char * msgName);
void ipMessagePatch()
{
    msgSearchReal = patch::hookFunction(spm::msgdrv::msgSearch,
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
