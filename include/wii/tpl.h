#pragma once

#include <types.h>

namespace wii::tpl {

// http://wiki.tockdom.com/wiki/TPL_(File_Format)

// Unions are offsets in files and pointers after loading

struct ImageHeader
{
    u16 height;
    u16 width;
    u32 format;
    union
    {
        u32 dataOffset;
        void * data;
    };
    u32 wrapS;
    u32 wrapT;
    u32 minFilter;
    u32 magFilter;
    float LODBias;
    bool edgeLODEnable;
    u8 minLOD;
    u8 maxLOD;
    bool unpacked;
};

struct PaletteHeader
{
    u16 entryCount;
    bool unpacked;
    u32 format;
    union
    {
        u32 dataOffset;
        void * data;
    };
};

struct ImageTableEntry
{
    union
    {
        u32 imageOffset;
        ImageHeader * image;
    };
    union
    {
        u32 paletteOffset;
        PaletteHeader * palette;
    };
};

struct TPLHeader
{
    u32 version; // 0x20af30
    u32 imageCount;
    union
    {
        u32 imageTableOffset;
        ImageTableEntry * imageTable;
    };
};

}
