/* guignome/guignome_corba.h */
  
/* Copyright 2000 by Dr. David Alan Gilbert (elvis@treblig.org)      */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, guys on the gnome-devel list, gmc,              */
/*   and the numbers 42 and 56. Chunks of this CORBA code are from   */
/*   gmc                                                             */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#ifndef GUIGNOME_CORBA_HEADER
#define GUIGNOME_CORBA_HEADER

#include <libgnorba/gnorba.h>

extern CORBA_ORB orb;

/* Activate the CORBA server */
void guignome_corba_activate_server (void);

/* Initializes the CORBA server */
int guignome_corba_init_server (void);

extern int guignome_corba_have_server;

#endif
