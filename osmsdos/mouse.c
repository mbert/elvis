/* MOUSE - Module of mouse functions. To use it, include the MOUSE.H file
 * in your program. The following functions are public:
 *
 *   MouseInit      - Initialize mouse
 *   GetMouseEvent  - Get information about most recent mouse event
 *   SetPtrVis      - Set visibility of pointer to HIDE or SHOW
 *   SetPtrPos      - Set position of pointer
 *   SetPtrShape    - Set shape of pointer in graphics modes, or
 *                    character and color in text modes
 *   GetPtrPos      - Get pointer position and button status
 *
 * The following structure is defined:
 *
 *   EVENT      -   Defines x, y, and mouse status of a mouse event
 */

#include <graph.h>
#include "mouse.h"

/* Internal information used by various mouse functions. */
struct MOUINFO
{
    int      fExist, fInit, fGraph;
    short    xVirtual,  yVirtual;
    short    xActual,   yActual;
    short    xLast,     yLast;
    unsigned fsBtnLast, cBtn;
} static mi =
{
    1, 0, 0,
    0, 0,
    0, 0,
    0, 0,
    0, 0
};

#pragma optimize( "lge", off )     /* /Ol, /Og, and /Oe cannot be used */
                                   /* with inline assembler            */

/* MouseInit - Initialize mouse and turns on mouse pointer. Initializes
 * all internal variables used by other mouse functions. This function
 * should be called whenever a new video mode is set, since internal
 * variables are mode-dependent.
 *
 * Params: none
 *
 * Return: 0 if no mouse available, otherwise number of buttons available
 */
int MouseInit()
{
    struct videoconfig vc;
    char __far *pMode = (char __far *)0x00000449L; /* Address for mode */

    /* Get video configuration. */
    _getvideoconfig( &vc );

    /* Handle special case of Hercules graphics. To use mouse with video
     * page 0. assume mode 6. To use mouse with page 1, assume mode 5.
     * Since the mouse functions couldn't easily detect and adjust for
     * page changes anyway, this code assumes page 0. Note also that the
     * mouse for Hercules graphics must be set in text mono mode.
     */
    if( vc.mode == _HERCMONO )
    {
        _setvideomode( _TEXTMONO );
        *pMode = 6;
    }

    mi.fInit = 1;
    __asm
    {
        sub     ax, ax              ; Mouse function 0, reset mouse
        mov     mi.cBtn, ax         ; Assume no mouse buttons
        int     33h
        mov     mi.fExist, ax       ; Set existence flag for future calls
        or      ax, ax              ; If AX = 0, there is no mouse
        jz      nomouse
        mov     mi.cBtn, bx         ; Save number of mouse buttons for return
    nomouse:
    }
    if( !mi.fExist )
        return 0;

    /* Set graphics flag. */
    if( vc.numxpixels )
    {
        mi.fGraph = 1;
        mi.yActual = vc.numypixels - 1;
        mi.xActual = vc.numxpixels - 1;
    }
    else
        mi.fGraph = 0;

    /* The mouse works on a virtual screen of 640 x pixels by (8 * textrows)
     * vertical pixels. By default, it assumes 640 x 200 for 25-line mode.
     * You must call function 8 to adjust for other screen sizes.
     */
    mi.xVirtual = 639;
    if( mi.fGraph )
        mi.yVirtual = vc.numypixels - 1;
    else
        mi.yVirtual = (vc.numtextrows << 3) - 1;

    /* Reset Hercules graphics mode and reset the height. */
    if( vc.mode == _HERCMONO )
    {
        _setvideomode( _HERCMONO );
        mi.xVirtual = 719;
    }

    __asm
    {
        mov     ax, 8               ; Set minimum and maximum vertical
        sub     cx, cx              ; Minimum is 0
        mov     dx, mi.yVirtual     ; Maximum is 8 * rows (or rows SHL 3)
        int     33h                 ; Adjust for 25, 30, 43, 50, or 60 lines

        mov     ax, 1               ; Turn on mouse pointer
        int     33h

        mov     ax, 3               ; Get initial position and button status
        int     33h
        mov     mi.xLast, cx        ; Save internally
        mov     mi.yLast, dx
        mov     mi.fsBtnLast, bx
    }
    return mi.cBtn;                 /* Return the number of mouse buttons */
}

/* GetMouseEvent - Check to see if there has been a mouse event. If event
 * occurred, update event structure.
 *
 * Params: pEvent - Pointer to event structure
 *
 * Return: 1 if event, 0 if no event
 */
int GetMouseEvent( EVENT __far *pEvent )
{
    int rtn;

    /* Make sure that mouse is initialized and exists. */
    if( !mi.fInit )
        MouseInit();
    if( !mi.fExist )
        return 0;

    __asm
    {
        mov     ax, 3               ; Get Mouse position and button status
        int     33h
        sub     ax, ax              ; Assume no event

        cmp     cx, mi.xLast        ; Has column changed?
        jne     event
        cmp     dx, mi.yLast        ; Has row changed?
        jne     event
        cmp     bx, mi.fsBtnLast    ; Has button changed?
        je      noevent
event:
        mov     ax, 1               ; If something changed, event occurred
        mov     mi.xLast, cx        ; Update internal variables
        mov     mi.yLast, dx
        mov     mi.fsBtnLast, bx
noevent:
        mov     rtn, ax             ; Set return value
    }

    /* If event, put adjust values in structure. */
    if( rtn )
    {
        /* If graphics mode, adjust virtual mouse position to actual
         * screen coordinates.
         */
        if( mi.fGraph )
        {
            pEvent->x = (short)((long)mi.xLast * mi.xActual) / mi.xVirtual;
            pEvent->y = (short)((long)mi.yLast * mi.yActual) / mi.yVirtual;
        }
        /* If text mode, adjust virtual mouse position to 1-based
         * row/column.
         */
        else
        {
            pEvent->x = (mi.xLast >> 3) + 1;
            pEvent->y = (mi.yLast >> 3) + 1;
        }
        pEvent->fsBtn = mi.fsBtnLast;
    }
    return rtn;
}

/* GetPtrPos - Get mouse pointer position and button status regardless of
 * whether there was an event.
 *
 * Params: pEvent - Pointer to event structure
 *
 * Return: 0 if no mouse, otherwise 1
 */
int GetPtrPos( EVENT __far *pEvent )
{
    /* Make sure that mouse is initialized and exists. */
    if( !mi.fInit )
        MouseInit();
    if( !mi.fExist )
        return 0;

    __asm
    {
        mov     ax, 3               ; Get Mouse position and button status
        int     33h
        les     di, pEvent
        mov     es:pEvent[di].x, cx
        mov     es:pEvent[di].y, dx
        mov     es:pEvent[di].fsBtn, bx
    }

    /* If graphics mode, adjust virtual mouse position to actual
     * screen coordinates.
     */
    if( mi.fGraph )
    {
        pEvent->x = (short)((long)pEvent->x * mi.xActual) / mi.xVirtual;
        pEvent->y = (short)((long)pEvent->y * mi.yActual) / mi.yVirtual;
    }
    /* If text mode, adjust virtual mouse position to 1-based
     * row/column.
     */
    else
    {
        pEvent->x >>= 3;
        pEvent->y >>= 3;
        pEvent->x++;
        pEvent->y++;
    }
    return 1;
}

/* SetPtrVis - Set pointer visibility.
 *
 * Params: state - SHOW or HIDE
 *
 * Return: 0 if no mouse, otherwise 1
 */
int SetPtrVis( PTRVIS pv )
{
    /* Make sure that mouse is initialized and exists. */
    if( !mi.fInit )
        MouseInit();
    if( !mi.fExist )
        return 0;

    __asm
    {
        mov ax, pv                  ; Show or hide mouse pointer
        int 33h
    }
}

/* SetPtrPos - Set mouse pointer position.
 *
 * Params: x - column position in text modes, actual x coordinate in graphics
 *         y - row position in text modes, actual y coordinate in graphics
 *
 * Return: 0 if no mouse, otherwise 1
 */
int SetPtrPos( short x, short y )
{
    /* Make sure that mouse is initialized and exists. */
    if( !mi.fInit )
        MouseInit();
    if( !mi.fExist )
        return 0;

    /* If graphics, adjust actual coordinates to virtual coordinates. */
    if( mi.fGraph )
    {
        x = (short)((long)x * mi.xActual) / mi.xVirtual;
        y = (short)((long)y * mi.yActual) / mi.yVirtual;
    }
    /* If text, adjust row/column to 0-based virtual coordinates. */
    else
    {
        x--;
        y--;
        x <<= 3;
        y <<= 3;
    }

    __asm
    {
        mov     ax, 4               ; Set mouse position
        mov     cx, x
        mov     dx, y
        int     33h
    }
    return 1;
}

/* SetPtrShape - Set mouse pointer shape.
 *
 * Params: x - column position in text modes, actual x coordinate in graphics
 *         y - row position in text modes, actual y coordinate in graphics
 *
 * Return: 0 if no mouse, otherwise 1
 */
int SetPtrShape( PTRSHAPE __far *ps )
{
    /* Make sure that mouse is initialized and exists. */
    if( !mi.fInit )
        MouseInit();
    if( !mi.fExist )
        return 0;

    /* If graphics, use pointer shape bitmask array. */
    if( mi.fGraph )
    {
        __asm
        {
            les     di, ps
            mov     bx, es:[di].g.xHot      ; Load hot spot offsets
            mov     cx, es:[di].g.yHot
            mov     dx, di
            add     dx, 4

            mov     ax, 9                   ; Set graphics pointer
            int     33h
        }
    }
    /* If text, use pointer color/character values. */
    else
    {
        __asm
        {
            les     di, ps
            mov     bx, 0                   ; Use software cursor
            mov     cl, es:[di].t.chScreen
            mov     ch, es:[di].t.atScreen
            mov     dl, es:[di].t.chCursor
            mov     dh, es:[di].t.atCursor

            mov     ax, 10                  ; Set text pointer
            int     33h
        }
    }
    return 1;
}

#pragma optimize( "", on )
