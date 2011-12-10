#ifndef ELVIS_GUIGNOME_MOUSEDRAG_HEADER
#define ELVIS_GUIGNOME_MOUSEDRAG_HEADER

/* guignome/guignome_mousedrag.h */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, and the numbers 42 and 56                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#include "guignome.h"

/* ================= Mouse click/release/drag handling code ============= */
/* ---------------------------------------------------------------------- */
/* Return the position in character cells given the mouse coordinates     */
void guignome_mouseToChar(ZvtTerm* term, double xin, double yin, 
                                 int* xout, int* yout);

/* ---------------------------------------------------------------------- */
void guignome_ButtonPress(GtkWidget* w, GdkEventButton* eb, struct guignome_instance* gw);

/* ---------------------------------------------------------------------- */
/* Button release over the terminal                                       */
int guignome_ButtonRelease(GtkWidget* w, GdkEventButton* eb, struct guignome_instance* gw);

int guignome_MotionNotify(GtkWidget* w, GdkEventMotion* em, struct guignome_instance* gw);

#endif
