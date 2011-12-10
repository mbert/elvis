/* guignome/guignome_clip.c */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, and the numbers 42 and 56                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

/* Handles clipboard and selection processing */

#include "guignome.h"
#include "guignome_clip.h"

/* ======================== Clipboard handling code ===================== */

/* Pointer to the selection text if we own it, else NULL */
/* We always associate the selection with guignome_app */
static gchar* ownSelection=NULL;

/* A temporary buffer which Elvis pushes text into/pulls it out */
static gchar* clip_buffer=NULL;
static enum {
      clip_closed,
      clip_open_writing, /* After clipopen(1) */
      clip_opening_read, /* During clipopen(0) before we have the text from X */
      clip_open_reading /* After clipopen(0) */
} clip_state=clip_closed;

/* Size of clip_buffer */
static guint clip_buffer_allocated=0;

/* Pointer for reading in clip_buffer */
/* (Writing always happens at the end) */
static guint clip_buffer_ptr=0;

/* Elvis calls us to tell us it wants to do a clipboard operation         */
ELVBOOL guignome_clipopen(ELVBOOL forwrite) {
  if (clip_state!=clip_closed) {
    msg(MSG_ERROR,"guignome: clipopen called when clip still open\n");
    return ElvFalse;
  };

  if (forwrite) {
    /* Simply let Elvis give us text */
    clip_state=clip_open_writing;
    clip_buffer_allocated=clip_buffer_ptr=0;
    if (clip_buffer!=NULL) {
      msg(MSG_WARNING,"guignome: clipopen called when clip_buffer is non-empty\n");
      clip_buffer=NULL;
    };
  } else {
    /* See if we have the selection */
    if (ownSelection!=NULL) {
      /* We have it - easy */
      clip_buffer=g_strdup(ownSelection);
      if (clip_buffer==NULL) {
        msg(MSG_ERROR,"Could not allocate memory for clip buffer\n");
        return ElvFalse;
      };
      clip_state=clip_open_reading;
      clip_buffer_allocated=strlen(clip_buffer)+1;
      clip_buffer_ptr=0;
      return ElvTrue;
    } else {
      /* Someone else has it - get it from X */

      /* We will get an event back saying that the data has arrived */
      gtk_selection_convert(guignome_app,GDK_SELECTION_PRIMARY, GDK_TARGET_STRING,GDK_CURRENT_TIME);
      clip_state=clip_opening_read;

      /* Loop until we etiher get the data (clip_open_reading) or we hit a key to cancel
         (hack in keyhit routine to give clip_closed) */
      /* **** DANGER What about other events which could happen here **** */
      g_message("clip_open:read prior to  gtk_main_iteration_loop\n");
      while (clip_state==clip_opening_read) {
	gtk_main_iteration();
      };
      g_message("clip_open:read post gtk_main_iteration_loop\n");

      return clip_state==clip_open_reading;
    }; /* Do we have it ?  */
  }; /* Read */
  return ElvTrue;
} /* clipopen */

/* ---------------------------------------------------------------------- */
/* Elvis should previously have called clipopen forwrite=true - here comes
   the data!                                                              */
int guignome_clipwrite(CHAR* text, int len) {
  if (clip_state!=clip_open_writing) {
    msg(MSG_ERROR,"clipwrite called when clip buffer not opened for writing!\n");
    return 0;
  };

  if (!len) return 0;

  if (clip_buffer==NULL) {
    clip_buffer=g_malloc(len+1);
    clip_buffer_allocated=len+1; /* +1 for term */
  } else {
    clip_buffer=g_realloc(clip_buffer,clip_buffer_allocated+len);
    clip_buffer_allocated+=len; /* already has terminator from last time */
  };

  if (clip_buffer==NULL) {
    /* oops - couldn't get the memory */
    msg(MSG_ERROR,"clipwrite failed to allocate memory\n");
    clip_buffer=NULL;
    clip_buffer_allocated=0;
    clip_state=clip_closed;
    return 0;
  };

  memcpy(clip_buffer+clip_buffer_ptr,text,len);
  clip_buffer_ptr+=len;
  clip_buffer[clip_buffer_ptr]='\0';

  return len;
} /* clipwrite */

/* ---------------------------------------------------------------------- */
/* Elvis should previously have called clipopen forread=false - now it    */
/* wants the data.                                                        */
int guignome_clipread(CHAR* text, int len) {
  g_message("clipread called len=%d clip_buffer_allocated=%d clip_buffer_ptr=%d\n",len,clip_buffer_allocated,clip_buffer_ptr);
  if (clip_state!=clip_open_reading) {
    msg(MSG_ERROR,"clipread called when clip buffer not opened for reading!\n");
    return 0;
  };
  /* 0=? 1=null string */
  if (clip_buffer_allocated<2) return 0;

  if ((clip_buffer_allocated-(clip_buffer_ptr+1))<len) {
    g_message("clipread small left: diff=%d\n",(clip_buffer_allocated-(clip_buffer_ptr+1)));

    len=(clip_buffer_allocated-(clip_buffer_ptr+1));
  };

  memcpy(text,clip_buffer+clip_buffer_ptr,len);
  clip_buffer_ptr+=len;
  text[len]='\0';
  
  return len;
} /* clipread */

/* ---------------------------------------------------------------------- */
/* Elvis believes it has finished the clipboard operation - clean up      */
void guignome_clipclose() {
  switch (clip_state) {
    case clip_open_reading:
      /* Hohum - just free stuff up */
      if (clip_buffer!=NULL) g_free(clip_buffer);
      clip_buffer_ptr=clip_buffer_allocated=0;
      clip_buffer=NULL;
      break;

    case clip_open_writing:
      /* We must tell X that we have the selection */
      /* First copy it to our own buffer unrelated to Elvis's idea of clipboards  (?not necessary?) */
      if (ownSelection!=NULL) g_free(ownSelection);
      ownSelection=strdup(clip_buffer);
      g_free(clip_buffer);
      clip_buffer=NULL;
      g_message("clipclose:write sel=%s\n",ownSelection);
      if (!gtk_selection_owner_set(guignome_app,GDK_SELECTION_PRIMARY,GDK_CURRENT_TIME)) {
        /* X didn't want it for some reason - hmm */
        g_free(ownSelection);
        ownSelection=NULL;
      };
      break;

    default:
      msg(MSG_ERROR,"clipclose called in invalid clip state\n");
  };
  clip_state=clip_closed;
} /* clipclose */


/* ---------------------------------------------------------------------- */
/* This is a special fix to allow people to break out of broken 
		clip board reads; if the user hits a key while a clipboard read is in progress
		cancel it */
void guignome_clip_keypress() {
  if (clip_state==clip_opening_read) clip_state=clip_closed;
} /* guignome_clip_keypress */

/* ---------------------------------------------------------------------- */
/* Called when another window/application  declares it is going to provide*/
/* the selection                                                          */
gint guignome_selection_clear(GtkWidget* widget, GdkEventSelection* event,
                                struct guignome_instance* gw) {
  g_message("gui_selection_clear\n");
  /* Someone else is taking over the selection - free our selection store */
  if (ownSelection!=NULL) {
    g_free(ownSelection);
    ownSelection=NULL;
  };
  return ElvTrue;
} /* gui_selection_clear */

/* ---------------------------------------------------------------------- */
/* Another application wants us to send them a copy of the selection data */
void guignome_selection_handler(GtkWidget* widget,
                                  GtkSelectionData* selectionData,
                                  guint info, guint time,
                                  struct guignome_instance* data) {
  g_message("gui_selection_handler\n");
  if (ownSelection==NULL) return;

  gtk_selection_data_set(selectionData,GDK_SELECTION_TYPE_STRING,
                         8, /* What the hell is this 8??? I got this from gnumeric... */
                         ownSelection,strlen(ownSelection));
} /* gui_selection_handler */
/* ---------------------------------------------------------------------- */
/* Sometime previously we called 'gtk_selection_convert' to get the       */
/* selection data, X has come back and actually given it us now           */
void guignome_selection_received(GtkWidget* widget,
                                   GtkSelectionData* selectionData,
                                   guint time,
                                   struct guignome_instance* data) {
  g_message("gui_selection_received\n");
  if (clip_state!=clip_opening_read) return;
  /* We will leave here with either the state as clip_closed (on failure)
     or clip_open_reading  - lets presume failure unless we find the light */
  clip_state=clip_closed;

  if (selectionData<0) {
    return;
  };

  if (clip_buffer!=NULL) {
    g_free(clip_buffer);
    clip_buffer=NULL;
  };

  clip_buffer=g_strndup(selectionData->data,selectionData->length);
  clip_buffer_allocated=strlen(clip_buffer)+1;
  clip_buffer_ptr=0;
  clip_state=clip_open_reading;
} /* gui_selection_received */

