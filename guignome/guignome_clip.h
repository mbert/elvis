#ifndef ELVIS_GUIGNOME_CLIP_HEADER
#define ELVIS_GUIGNOME_CLIP_HEADER

/* guignome/guignome_clip.h */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, and the numbers 42 and 56                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

/* Handles clipboard and selection processing */

#include "guignome.h"
#include "guignome_clip.h"

/* ======================== Clipboard handling code ===================== */
/* Elvis calls us to tell us it wants to do a clipboard operation         */
ELVBOOL guignome_clipopen(ELVBOOL forwrite);

/* ---------------------------------------------------------------------- */
/* Elvis should previously have called clipopen forwrite=true - here comes
   the data!                                                              */
int guignome_clipwrite(CHAR* text, int len);

/* ---------------------------------------------------------------------- */
/* Elvis should previously have called clipopen forread=false - now it    */
/* wants the data.                                                        */
int guignome_clipread(CHAR* text, int len);

/* ---------------------------------------------------------------------- */
/* Elvis believes it has finished the clipboard operation - clean up      */
void guignome_clipclose();

/* ---------------------------------------------------------------------- */
/* This is a special fix to allow people to break out of broken 
		clip board reads; if the user hits a key while a clipboard read is in progress
		cancel it */
void guignome_clip_keypress();

/* ---------------------------------------------------------------------- */
/* Called when another window/application  declares it is going to provide*/
/* the selection                                                          */
gint guignome_selection_clear(GtkWidget* widget, GdkEventSelection* event,
                                struct guignome_instance* gw);

/* ---------------------------------------------------------------------- */
/* Another application wants us to send them a copy of the selection data */
void guignome_selection_handler(GtkWidget* widget,
                                  GtkSelectionData* selectionData,
                                  guint info, guint time,
                                  struct guignome_instance* data);
/* ---------------------------------------------------------------------- */
/* Sometime previously we called 'gtk_selection_convert' to get the       */
/* selection data, X has come back and actually given it us now           */
void guignome_selection_received(GtkWidget* widget,
                                   GtkSelectionData* selectionData,
                                   guint time,
                                   struct guignome_instance* data);

#endif
