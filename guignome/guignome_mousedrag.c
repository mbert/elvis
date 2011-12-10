/* guignome/guignome_mousedrag.c */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, and the numbers 42 and 56                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#include "guignome.h"
#include "guignome_mousedrag.h"

/* ================= Mouse click/release/drag handling code ============= */
/* ---------------------------------------------------------------------- */
/* Return the position in character cells given the mouse coordinates     */
void guignome_mouseToChar(ZvtTerm* term, double xin, double yin, 
                                 int* xout, int* yout) {
  *xout=xin/(term->charwidth);
  *yout=yin/(term->charheight);
} /* guignome_mouseToChar */

/* ---------------------------------------------------------------------- */
void guignome_ButtonPress(GtkWidget* w, GdkEventButton* eb, struct guignome_instance* gw) {
  int x,y;
  int dropcount=1;

  /* eb->button gives the mouse button pressed */
  /* eb->state gives the modifier (i.e. ctrl/alt/shift) */
  guignome_mouseToChar(ZVT_TERM(gw->term),eb->x,eb->y,&x,&y);
  g_message("Button Press at %lf,%lf (char: %d/%d) %d/%d\n",
             eb->x,eb->y,x,y,eb->state,eb->button);

  if (GTK_IS_LIST_ITEM(w) &&
    (eb->type==GDK_2BUTTON_PRESS ||
    eb->type==GDK_3BUTTON_PRESS) ) {
    dropcount=(eb->type==GDK_2BUTTON_PRESS ? 2 : 3);
  };
  gw->nClick=dropcount;

  switch (eb->button) {
    case 1: /* Left button - Just move */
      eventclick((GUIWIN*)gw,-1,-1,CLICK_CANCEL);
      eventclick((GUIWIN*)gw,y,x,CLICK_MOVE);
      break;

    case 2: /* Middle - Paste */
      eventclick((GUIWIN*)gw,-1,-1,CLICK_CANCEL);
      eventclick((GUIWIN*)gw,y,x,CLICK_MOVE);
      break;

    case 3: /* Right - extend and yank */
      if (eb->state==0) return; /* This is just bringing the popup menu up */
      eventclick((GUIWIN*)gw,y,x,CLICK_MOVE);
      eventclick((GUIWIN*)gw,-1,-1,CLICK_YANK);
      break;

  };

  eventdraw((GUIWIN*)gw);

  gw->buttonsDown |= (1<<eb->button);
  gw->justClick=True;
  gw->downx=x;
  gw->downy=y;
} /* guignome_ButtonPress */

/* ---------------------------------------------------------------------- */
/* Button release over the terminal                                       */
int guignome_ButtonRelease(GtkWidget* w, GdkEventButton* eb, struct guignome_instance* gw) {
  int x,y;
  /* eb->button gives the mouse button pressed */
  /* eb->state gives the modifier (i.e. ctrl/alt/shift) */
  guignome_mouseToChar(ZVT_TERM(gw->term),eb->x,eb->y,&x,&y);
  g_message("Button Release %d\n",eb->button);

  if (gw->justClick) {
    if (eb->button==2) {
      eventclick((GUIWIN*)gw, -1, -1, CLICK_PASTE);
    };
  } else {
    eventclick((GUIWIN *)gw,y,x, CLICK_MOVE);
    eventclick((GUIWIN *)gw, y, x, CLICK_YANK);
  };
  eventdraw((GUIWIN*)gw);
  gw->buttonsDown &= ~(1<<eb->button);
  return 1;
} /* guignome_ButtonRelease */

/* ---------------------------------------------------------------------- */
int guignome_MotionNotify(GtkWidget* w, GdkEventMotion* em, struct guignome_instance* gw) {
  if (gw->buttonsDown) {
    int x,y;
    g_message("Motion notify %d\n",gw->buttonsDown);
    if (gw->justClick) {
      gw->justClick=False;
      eventclick((GUIWIN*)gw,gw->downy,gw->downx,(gw->nClick==3)?CLICK_SELRECT:CLICK_SELCHAR);
    };
    guignome_mouseToChar(ZVT_TERM(gw->term),em->x,em->y,&x,&y);
    eventclick((GUIWIN*)gw,y,x,CLICK_MOVE);
    eventdraw((GUIWIN*)gw);
  };
  return 1;
} /* guignome_MotionNotify */

