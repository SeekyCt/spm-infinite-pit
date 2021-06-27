#pragma once

#include <types.h>
#include <wii/types.h>

namespace spm::hitdrv {

#define HITOBJ_FLAG_DISABLE 1

extern "C" {

void hitGrpFlagOn(bool allowSubname, const char * name, u32 mask);
void hitGrpFlagOff(bool allowSubname, const char * name, u32 mask);
bool hitCheckFilter(float, float, float, float, float, float, void *, float *, float *,
                    float *, float *, float *, float *, float *);
void hitObjGetPos(const char * name, wii::Vec3 * ret);
void hitGetMapEntryBbox(int mapEntryIdx, wii::Vec3 * min, wii::Vec3 * max);

}

}
