#pragma once

#include <types.h>
#include <wii/tpl.h>

namespace ip {

extern "C" {

extern const u8 allTex[];
extern const u32 allTex_size;

extern const u8 badgeTex[];
extern const u32 badgeTex_size;

extern const u8 equippedTex[];
extern const u32 equippedTex_size;

extern wii::tpl::TPLHeader badgeIconTpl;
extern u32 badgeIconTpl_size;

}

}
