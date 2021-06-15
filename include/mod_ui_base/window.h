/*
    Window
    Abstract base window class
*/

#pragma once

#include <types.h>
#include <wii/types.h>

namespace mod {

class Window
{
private:
    Window * mNext;

    static Window * sWindowList;
    static void windowDisp(s8 camId, void * param);

protected:
    virtual void disp();

public:
    /*
        Abstract, don't do anything but are used by most classes inheriting
        Windows are drawn on the debug3d layer so generally:
            x: -375.0 to 375.0 left to right
            y: -225.0 to 225.0 bottom to top
    */
    f32 mPosX, mPosY;

    Window();
    virtual ~Window();
    void drawString(const char * str, f32 x, f32 y, const wii::RGBA * colour = nullptr, f32 scale = 1.0f,
                    bool edge = false, bool noise = false, bool rainbow = false);
    void drawMessage(const char * str, s32 x, s32 y, const wii::RGBA * colour = nullptr, f32 scale = 1.0f,
                     bool edge = false, bool noise = false, bool rainbow = false);
    void drawMessageSearch(const char * name, s32 x, s32 y, const wii::RGBA * colour = nullptr, f32 scale = 1.0f,
                     bool edge = false, bool noise = false, bool rainbow = false);

    static void windowMain();
};

}
