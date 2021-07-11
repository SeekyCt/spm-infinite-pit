#include "assets/badgetex.h"
#include "patch.h"

#include <types.h>
#include <spm/filemgr.h>
#include <spm/memory.h>
#include <spm/system.h>
#include <wii/string.h>
#include <wii/tpl.h>

namespace mod {

using spm::filemgr::FileEntry;
using spm::filemgr::fileAllocf;
using wii::tpl::TPLHeader;
using wii::tpl::ImageHeader;

enum PauseTextureId
{
    PAUSETEX_CHARACTERS,
    PAUSETEX_PIXLS,
    PAUSETEX_ITEMS,
    PAUSETEX_KEY_ITEMS,
    PAUSETEX_CHAPTERS,
    // ...
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
    ImageHeader * img = tpl->imageTable[PAUSETEX_CHAPTERS].image;

    // Copy custom texture in
    assert(img->width == 120 && img->height == 40 && img->format == 5,
           "Unable to insert badge button texture");
    wii::string::memcpy(img->data, badgeTex, badgeTex_size);

    // Default behaviour at hook
    return file;
}

/*
    Apply the menu related patches
*/
static void menuPatch()
{
    writeBranchLink(0x80184d6c, 0, pauseTplOverride);
}

/*
    Public function to apply all patches
*/
void ipBadgePatch()
{
    menuPatch();
}

}
