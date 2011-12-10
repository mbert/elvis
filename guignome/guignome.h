#ifndef ELVIS_GUIGNOME_HEADER
#define ELVIS_GUIGNOME_HEADER

/* guignome/guignome.h */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, and the numbers 42 and 56                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#include "../elvis.h"
/* DAG: Undo the current hack in my regexp.h - allows me not to have to change
   all the regexp usage in elvis even though it conflicts with POSIX */
#undef regexp
#undef regbuild
#undef regcomp
#undef regdup
#undef regexec
#undef regtilde
#undef regsub
#undef regerror 

#include <gnome.h>
#include <glade/glade.h>
#include <zvt/zvtterm.h>

/* We have one of these for each top level window - we hold a linked list of them
   purely so that we can know if we've just deleted the one we are working with
   */
#define TEMPBUFFERSIZE 1024
struct guignome_instance {
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *scrollbar;
  GtkWidget *menubar;
  GtkWidget *toolbar;
  GtkWidget *statusbar;
  GtkWidget *statusbar_cmdpre;
  GtkWidget *statusbar_pos;
  GtkWidget *statusbar_mode;
  GtkWidget *term;
  GtkWidget *popupmenu;

  /* The amount by which we are scrolled */
  GtkAdjustment *vadj;

  /* The Glade file reference */
  GladeXML* glade_xml;

  /* List of all the winmenubarentries for our window */
  GSList* wmbes;

  /* List of all the winmenuentries for our window - so we can close dialogues
     when we are closed
   */
  GSList* wmes; /* NOTE! Now only holds the ones for popups - to be removed */

  /* List of all the toolbar entries for our window */
  GSList* wtes;

  /* List of all the dactions for our window */
  GSList* wdacs;

  /* List for the currently active special menu */
  GSList* specialdata;

  /* The size of the scroll bar according to elvis */
  long sb_top, sb_bottom, sb_total;

  /* Scheduled callback to resize the elvis area on a window configure */
  guint configureCallBack;

  /* Present known size of window - to stop redraws on window move */
  int width,height;

  /* Buffer of stuff to be written  - try and join repeated write requests
     to reduce rendering time */
  char tempbuffer[TEMPBUFFERSIZE];
  /* Point into tempbuffer where next character will be written */
  unsigned int writeptr;

  /* Last draw bit mask used, used to test if we need to enable/disable
     any terminal features (bold etc.) */
  unsigned int lastDrawBit;
  /* Last foreground/background colours, and bits affecting colour interpretation */
  long lastFore,lastBack,lastDrawColBits;

  /* Mask of mouse buttons that are down */
  unsigned int buttonsDown;
  /* Position of button down x/y */
  unsigned int downx, downy;
  /* ElvTrue after a click, false after first motion */
  ELVBOOL justClick;
  /* Single, double, triple click */
  unsigned int nClick;

};

struct guignome_daction {
  char *name;

  char *cmd; /* On OK/apply */
  char *dialoguename;
  char *applybutton;
  char *cancelbutton;
  char *okbutton;
};

/* One for each of the actions opened, per window */
struct guignome_windaction {
  struct guignome_daction* daction;
  struct guignome_instance* instance;

  /* The dialogue (if any) that we have created */
  GtkWidget* dialogue;
};

/* One of these for each menubar entry (shared between all windows) */
struct guignome_menubarentry {
  char *text;

  /* Entries in the menu hung off this menubar entry */
  GSList* entries; /* Of guignome_menuentry's */
};

/* One of these for each toolbar entry (shared between all windows */
struct guignome_toolbarentry {
  char *text;
  char *icon;
  char *tttext; /* Tooltip text */

  char* action;
};

/* One of these for each menu entry (shared between all windows) */
struct guignome_menuentry {
  char *text;
  char *command;
  char *optname;
  char *optvalue;
};

/* One for each menu bar entry in each window */
struct guignome_winmenubarentry {
  struct guignome_instance* gnomewin;
  struct guignome_menubarentry* menubarentry;
  GtkWidget* menu;
  GSList* wmes;
};

/* One for each menu entry in each window */
struct guignome_winmenuentry {
  struct guignome_instance* gnomewin;
  struct guignome_menuentry* menuentry;
  GtkWidget* menuentrywidget;
};

/* One for each toolbar entry in each window */
struct guignome_wintoolbarentry {
  struct guignome_instance* gnomewin;
  struct guignome_toolbarentry* toolbarentry;
};

extern GSList* guignome_instancelist;
extern GSList* guignome_dactionlist;
extern GSList* guignome_menubarentrylist;
extern GSList* guignome_toolbarentrylist;
extern GSList* guignome_popupmenuentrylist;
extern GtkWidget* guignome_app;
extern char* guignome_escapeString(char* in);

void guignome_doopenfile(char *filename, char *dir);
#endif
