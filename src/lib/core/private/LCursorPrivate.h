#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <LCursor.h>

class Louvre::LCursor::LCursorPrivate
{
public:
    LCursorPrivate()                                    = default;
    ~LCursorPrivate()                                   = default;

    LCursorPrivate(const LCursorPrivate&)               = delete;
    LCursorPrivate &operator=(const LCursorPrivate&)    = delete;

    LCursor *cursor;
    LCompositor *compositor;
    LRect rectC;

    // Called once per main loop iteration
    void update();

    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    LPointF posC;
    LPointF hotspotB;
    LSizeF sizeS;
    LOutput *output                                     = nullptr;
    std::list<LOutput*>intersectedOutputs;
    bool isVisible                                      = true;


    UInt32 lastTextureSerial                            = 0;
    bool textureChanged                                 = false;
    LTexture *texture                                   = nullptr;
    LTexture *defaultTexture                            = nullptr;
    GLuint glFramebuffer, glRenderbuffer;
    UChar8 buffer[64*64*4];

};

#endif // LCURSORPRIVATE_H
