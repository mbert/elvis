/* guignome/guignome.c */

/* Copyright 1999-2000 by Dr. David Alan Gilbert (elvis@treblig.org) */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, guys on the gnome-devel list                    */
/*   and the numbers 42 and 56                                       */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#include "../version.h"

#include <time.h>

#include "guignome.h"
#include "guignome_clip.h"
#include "guignome_corba.h"
#include "guignome_mousedrag.h"

#include <libgnorba/gnorba.h>

static ELVBOOL traceBufferedWrites=ElvFalse;

/* Our main application top level window (which we aren't going to use) */
/* Actually....this is where we attach our selection to */
GtkWidget *guignome_app;

/* The glade file name */
static char* gladename;

static int quit=ElvFalse;

/* The palette is global (I'd actually prefer it on a perwindow basis) */
struct gui_palette_s {
  struct palette_entry {
    ELVBOOL free;
    unsigned int red,green,blue;
  } palette[18]; /* 16 possibles, plus 2 defaults */
};

static struct gui_palette_s gui_palette;

/* Options that are global for the GNOME gui */
static OPTDESC guiOptDesc[] = {
  {"defaultrows", "dr",  optnstring, optisnumber, "4:400"},
  {"defaultcols", "dc",  optnstring, optisnumber, "4:400"}
};

/* This is where the option values actually get stored */
static struct gui_optvals_s {
  OPTVAL defaultrows,defaultcols;
} gui_optvals;

/* Macros to get to the values of each option */
#define o_defaultrows gui_optvals.defaultrows.value.number
#define o_defaultcols gui_optvals.defaultcols.value.number

GSList* guignome_instancelist=NULL;

GSList* guignome_dactionlist=NULL;

GSList* guignome_menubarentrylist=NULL;

GSList* guignome_toolbarentrylist=NULL;
GSList* guignome_popupmenuentrylist=NULL;

struct key_s {
  guint keyval;
  int   ctrlstate; /* -1=ignore, 0=must be 0, 1=must be 1 */
  int   altstate; /* -1=ignore, 0=must be 0, 1=must be 1 */
  int   nchars;   /* Number of valid chars in following string  - 0 for last entry*/
  char  *towrite; /* Keys to write */
} keys []={
  { GDK_Return         , -1, -1, 1, "\015" },
  { GDK_KP_Enter       , -1, -1, 1, "\015" },
  { GDK_Escape         , -1, -1, 1, "\033" },
  { GDK_Tab            , -1, -1, 1, "\011" },
  { GDK_BackSpace      , -1, -1, 1, "\010" },
  { GDK_Delete         , -1, -1, 1, "\3ff" },
  { 'a'                ,  1, -1, 1, "\001" },
  { 'b'                ,  1, -1, 1, "\002" },
  { 'c'                ,  1, -1, 1, "\003" },
  { 'd'                ,  1, -1, 1, "\004" },
  { 'e'                ,  1, -1, 1, "\005" },
  { 'f'                ,  1, -1, 1, "\006" },
  { 'g'                ,  1, -1, 1, "\007" },
  { 'h'                ,  1, -1, 1, "\010" },
  { 'i'                ,  1, -1, 1, "\011" },
  { 'j'                ,  1, -1, 1, "\012" },
  { 'k'                ,  1, -1, 1, "\013" },
  { 'l'                ,  1, -1, 1, "\014" },
  { 'm'                ,  1, -1, 1, "\015" },
  { 'n'                ,  1, -1, 1, "\016" },
  { 'o'                ,  1, -1, 1, "\017" },
  { 'p'                ,  1, -1, 1, "\020" },
  { 'q'                ,  1, -1, 1, "\021" },
  { 'r'                ,  1, -1, 1, "\022" },
  { 's'                ,  1, -1, 1, "\023" },
  { 't'                ,  1, -1, 1, "\024" },
  { 'u'                ,  1, -1, 1, "\025" },
  { 'v'                ,  1, -1, 1, "\026" },
  { 'w'                ,  1, -1, 1, "\027" },
  { 'x'                ,  1, -1, 1, "\030" },
  { 'y'                ,  1, -1, 1, "\031" },
  { 'z'                ,  1, -1, 1, "\032" },
  { 'A'                ,  1, -1, 1, "\001" },
  { 'B'                ,  1, -1, 1, "\002" },
  { 'C'                ,  1, -1, 1, "\003" },
  { 'D'                ,  1, -1, 1, "\004" },
  { 'E'                ,  1, -1, 1, "\005" },
  { 'F'                ,  1, -1, 1, "\006" },
  { 'G'                ,  1, -1, 1, "\007" },
  { 'H'                ,  1, -1, 1, "\010" },
  { 'I'                ,  1, -1, 1, "\011" },
  { 'J'                ,  1, -1, 1, "\012" },
  { 'K'                ,  1, -1, 1, "\013" },
  { 'L'                ,  1, -1, 1, "\014" },
  { 'M'                ,  1, -1, 1, "\015" },
  { 'N'                ,  1, -1, 1, "\016" },
  { 'O'                ,  1, -1, 1, "\017" },
  { 'P'                ,  1, -1, 1, "\020" },
  { 'Q'                ,  1, -1, 1, "\021" },
  { 'R'                ,  1, -1, 1, "\022" },
  { 'S'                ,  1, -1, 1, "\023" },
  { 'T'                ,  1, -1, 1, "\024" },
  { 'U'                ,  1, -1, 1, "\025" },
  { 'V'                ,  1, -1, 1, "\026" },
  { 'W'                ,  1, -1, 1, "\027" },
  { 'X'                ,  1, -1, 1, "\030" },
  { 'Y'                ,  1, -1, 1, "\031" },
  { 'Z'                ,  1, -1, 1, "\032" },
  { ']'                ,  1, -1, 1, "\035" },
  { '6'                ,  1, -1, 1, "\036" },
  { GDK_Left           , -1, -1, 2, "\017h" },
  { GDK_Right          , -1, -1, 2, "\017l" },
  { GDK_Up             , -1, -1, 2, "\017k" },
  { GDK_Down           , -1, -1, 2, "\017j" },
  { GDK_Page_Down      , -1, -1, 2, "\017\006" }, /* ctrl-f i.e. down a page */
  { GDK_Page_Up        , -1, -1, 2, "\017\002" },/* ctrl-b i.e. up a page */
  { GDK_Home           , -1, -1, 2, "\0170" },   /* 0 hard start of line (or should this be ^ */
  { GDK_End            , -1, -1, 2, "\017$" },   /* $ end of line */
  { GDK_VoidSymbol     , -1, -1, 0, "That's all folks" }
};

static void destroygw(GUIWIN *gw, ELVBOOL force);
static void flush();
static void redisplayAll();

/* ---------------------------------------------------------------------- */
/* Escape any odd characters in a string - useful for file naames etc     */
/* Returns a newly allocated chunk of memory twice the length of the      */
/* original string                                                        */
char* guignome_escapeString(char* in) {
  char *res=g_malloc(strlen(in)*2+1);
  char *inp, *outp;

  for(inp=in,outp=res;*inp;inp++) {
    if (strchr("\\ '\"",*inp))
      *(outp++)='\\';
    *(outp++)=*inp;
  };

  *outp='\0';

  return res;
} /* guignome_escapestring */

/* ---------------------------------------------------------------------- */
/* Find the daction with the given name in the guignome_dactionlist                */
static struct guignome_daction* findDaction(const char*name) {
  GSList* current=guignome_dactionlist;

  while (current) {
    struct guignome_daction* dact;
    dact=(struct guignome_daction*)(current->data);

    if (strcmp(name,dact->name)==0)
      return dact;

    current=g_slist_next(current);
  };

  return NULL;
} /* findDaction */

/* ---------------------------------------------------------------------- */
static void dndDrop(GtkWidget* w, GdkDragContext* context,
                    gint x, gint y, GtkSelectionData *data, guint info,
                    guint time, gpointer* gw_raw) {
  struct guignome_instance* gw=(struct guignome_instance*)gw_raw;

  /* If we get a message */
  if (data->data) {
    /* We marked our window as a drop site to receive a list of URLs - 
       extract the URLs and open a window for each one */
    GList* urls=gnome_uri_list_extract_uris ((gchar*)data->data);
    GList* tmp=urls;

    /* For each URL we are passed */
    while (tmp) {
      gchar* cmd;
      gchar* escaped=guignome_escapeString((char*)tmp->data);
      /* Build the command (:split opens new window) */
      cmd=g_strdup_printf(":split %s",escaped);
      g_free(escaped);
      if (cmd) {
	eventex((GUIWIN* )gw,cmd,ElvFalse);
	g_free(cmd);
      };
      tmp=g_list_next(tmp);
    };

    gnome_uri_list_free_strings(urls);
  };
} /* dndDrop */

/* ---------------------------------------------------------------------- */
/* Write the text to the terminal in the particular window through a      */
/* buffer for performance.                                                */
static void writeBuffered(struct guignome_instance* gw, char* towrite, int ntowrite) {
  if (traceBufferedWrites) {
    int i;
    fprintf(stderr,"writeBuffered: ");
    for(i=0;i<ntowrite;i++) {
      if (elvprint(towrite[i])) {
        fprintf(stderr,"%c",towrite[i]);
      } else {
        fprintf(stderr,"<%d>",towrite[i]);
      };
    };

    fprintf(stderr,":endwriteBuffered\n");
  }
  /* If there isn't enough space for the text write out the old buffer contents */
  if ((gw->writeptr+ntowrite)>=TEMPBUFFERSIZE) {
    zvt_term_feed(ZVT_TERM(gw->term), gw->tempbuffer, gw->writeptr);
    /* And the buffer is now empty */
    gw->writeptr=0;
  };

  /* If the lump being is written is too large then just write directly
     (the previous buffer full test ensures old stuff is already written) */
  if (ntowrite>TEMPBUFFERSIZE) {
    zvt_term_feed(ZVT_TERM(gw->term), towrite, ntowrite);
  } else {
    /* Normal case - just add to the buffer */
    strncpy(gw->tempbuffer+gw->writeptr,towrite,ntowrite);
    gw->writeptr+=ntowrite;
  };
} /* writeBuffered */

/* ---------------------------------------------------------------------- */
/* Internals of the expansion code - doesn't write if dest ptr is NULL    */
/* returns the number of characters to be written                         */
static int expandCommandString_internal(char* template,char* result,
                                        struct guignome_windaction* wda) {
  int count=0;
  g_message("expandCommandString_internal: template=%s result=%p\n",template,result);
  while (*template) {
    if (*template=='$') {
      char* endwidgetname;
      char *startcommand=template;
      char* widgetname;
      int mustDealloc=0;
      GtkWidget* widget;

      g_message("Hit $ at %s\n",template);
      /* Special command sequence */
      /* $widgetname followed by special character to say what type of command */
      /* find the end of the widget name */
      for(endwidgetname=++template;(*endwidgetname) && (strchr("?$",*endwidgetname)==NULL);endwidgetname++);
      if (endwidgetname==(startcommand+1)) {
        g_message("Hit literal at %s\n",startcommand);
        /* Explicit insert command character */
        count++;
        if (result) {
          *(result++)=*endwidgetname;
        };
        template=endwidgetname+1;
        continue;
      };
      widgetname=g_strndup(startcommand+1,(endwidgetname-1)-startcommand);

      switch (*endwidgetname) {
        default:
        case '\0': /* Eh? Not hit the proper end of string termination */
          msg(MSG_ERROR,"Invalid $ expansion in command string");
          g_free(widgetname);
          return -1;

        case '$': /* Simple string insertion */
          if (widget=glade_xml_get_widget(wda->instance->glade_xml,widgetname),widget==NULL) {
            msg(MSG_ERROR,"[s]Could not find widget for command expansion ($1)",widgetname);
            g_message("Widget name was '%s'\n",widgetname);
            g_free(widgetname);
            return -1;
          };
          {
            char* text;
            if (GTK_IS_ENTRY(widget)) {
              text=gtk_entry_get_text (GTK_ENTRY (widget));
            } else if (GTK_IS_FILE_SELECTION(widget)) {
              mustDealloc=1;
              text=guignome_escapeString(gtk_file_selection_get_filename(GTK_FILE_SELECTION(widget)));
            } else if (GNOME_IS_ENTRY(widget)) {
              /* A gnome entry is like a GTK entry but has a menu box for history */
              text=gtk_entry_get_text (GTK_ENTRY(gnome_entry_gtk_entry(GNOME_ENTRY(widget))));
              /* Update the history */
              gnome_entry_prepend_history(GNOME_ENTRY(widget),ElvTrue,text);
            } else {
              msg(MSG_ERROR,"Unknown widget type for string subsitition");
              g_free(widgetname);
              return -1;
            };

            count+=strlen(text);
            if (result) {
              strcpy(result,text);
              result+=strlen(text);
            };
            if (mustDealloc) g_free(text);
            template=endwidgetname+1;
          };
          g_free(widgetname);
          break;

        case '?': /* ?: substitution */
          g_message("expandCommandString_Internal ?: entry\n");
          if (widget=glade_xml_get_widget(wda->instance->glade_xml,widgetname),widget==NULL) {
            msg(MSG_ERROR,"[s]Could not find widget for command expansion ($1)",widgetname);
            g_message("Widget name was '%s'\n",widgetname);
            g_free(widgetname);
            return -1;
          };
          g_free(widgetname);
          {
            /* Now get the two option strings */
            char *truestring,*falsestring;
            char *start,*end;

            g_message("expandCommandString_Internal ?: pre-true search\n");
            /* NOTE! Fairly noddy - doesn't yet allow )'s in substitution string */
            start=endwidgetname+1;
            if (*start!='(') {
              /* We expect an open bracket */
              msg(MSG_ERROR,"Didn't find expected ( before true string in ?: expansion");
              return -1;
            };
            start++;
            for(end=start;(*end) && (*end!=')');end++);
            if (*end=='\0') {
              msg(MSG_ERROR,"Didn't find expected ) after true string in ?: expansion");
              return -1;
            };
            truestring=g_strndup(start,end-start);
            if (end[1]!=':') {
              msg(MSG_ERROR,"Didn't find expected : after true string in ?: expansion");
              g_free(truestring);
              return -1;
            };
            g_message("expandCommandString_Internal ?: pre-false search (truestring=%s)\n",truestring);
            if (end[2]!='(') {
              msg(MSG_ERROR,"Didn't find expected ( before false string in ?: expansion");
              g_free(truestring);
              return -1;
            };

            start=end+3;
            for(end=start;(*end) && (*end!=')');end++);
            g_message("expandCommandString_Internal ?: post-false search\n");
            if (*end=='\0') {
              msg(MSG_ERROR,"Didn't find expected ) after false string in ?: expansion");
              g_free(truestring);
              return -1;
            };
            g_message("expandCommandString_Internal ?: pre-false copy\n");
            falsestring=g_strndup(start,end-start);
            template=end+1;

            g_message("expandCommandString_Internal ?: pre widget test\n");
            /* Now figure out which way the button is set and then fill in
               the substitution */
            if (GTK_IS_TOGGLE_BUTTON(widget)) {
              unsigned int length;
              char* toexpand;
              toexpand=(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))?truestring:falsestring;
              g_message("expandCommandString_Internal ?: pre recursion\n");
              if (strlen(toexpand)>0) {
                if (result) {
                  length=expandCommandString_internal(truestring,result,wda);
                  result+=length;
                } else {
                  length=expandCommandString_internal(truestring,NULL,wda);
                };
                count+=length;

                /* Check for errors in subcall */
                if (length==-1) {
                  g_free(truestring);
                  g_free(falsestring);
                  return -1;
                };
              };
              g_free(truestring);
              g_free(falsestring);
            } else {
              g_free(truestring);
              g_free(falsestring);
              msg(MSG_ERROR,"Unknown widget type for boolean substitution");
              return -1;
            };
          };
          break;
      };

    } else {
      /* Normal character - just copy through */
      count++;
      if (result!=NULL)
        *(result++)=*(template++);
      else
        template++;
    };
  };

  return count;
} /* expandCommandString_internal */

/* ---------------------------------------------------------------------- */
/* Take a command string with references to fields in a dialog box and    */
/* return a command string with those references replaced by values from  */
/* dialogue box.                                                          */
/* $punct = That exact punctuation mark (including $ to get a real $)     */
/* $name  = some form of substitution from widget value                   */
/* $name$ = Insert the (string) value of the widget as it                 */
/* $name?(text1):(text0) = Insert text1 if the value of the checkbutton   */
/*             widget 'name' is true else insert text0                    */
/* NOTE: Result is a newly allocated string                               */
static char* expandCommandString(char* string,
                                 struct guignome_windaction* wda) {
  char* resptr;

  /* First use the routine to count the number of bytes needed in the result */
  unsigned int length=expandCommandString_internal(string,NULL,wda);
  if (length==-1) return NULL;
  if (resptr=g_malloc(length+1), resptr==NULL) return NULL;
  /* Now really expand into the allocated space */
  expandCommandString_internal(string,resptr,wda);
  resptr[length]='\0';

  g_message("expandCommandString gives '%s'",resptr);
  return resptr;
} /* expandCommandString */

/* ---------------------------------------------------------------------- */
/* Returns true iff this instance is still in our list                    */
static int stillExists(struct guignome_instance* i) {
  return (g_slist_find(guignome_instancelist,i)!=NULL);
} /* stillExists */

#ifndef PACKAGE
#define PACKAGE "Elvis"
#endif
/* ---------------------------------------------------------------------- */
/* Return 0 if GUI unavailable, 1 if available and 2 if we don't know     */
static int test()
{
  int argc=1;
  char *args[2];
  char **argv=args;

  argv[0]="elvis";
  argv[1]=NULL;

  printf("guignome::test entry\n");
  gnomelib_init(PACKAGE,VERSION);

  if (gtk_init_check(&argc,&argv)) {
    printf("guignome:test gnome OK\n");
    return 1;
  };

	printf("guignome:test gnome bad\n");
  return 0;
} /* test */

/* ---------------------------------------------------------------------- */
static int guignome_isClient = FALSE;
static int guignome_isServer = FALSE;

static struct poptOption options[] = {
  {
     "client",
     '\0',
     POPT_ARG_NONE,
     &guignome_isClient,
     0,
     N_("Work in client mode, asking an elvis server to actually do the editing"),
     NULL
  },
  {
     NULL,
     '\0',
     0,
     NULL,
     0,
     NULL,
     NULL
  }
};

/* ---------------------------------------------------------------------- */
void corbaErrCheck(CORBA_Environment* pev,char *msg) {
  if (pev->_major != CORBA_NO_EXCEPTION) {
    g_message("Corba error '%s' at '%s'\n",CORBA_exception_id(pev),msg);
    exit(1);
  };
}

/* ---------------------------------------------------------------------- */
/* Call the server                                                        */
static int guignome_client(int argc, char **argv) {
  char *dir;
  int i;
  int gotflag=0;
  CORBA_Object ElvisPrivate_server = CORBA_OBJECT_NIL;
  CORBA_Environment ev;

  CORBA_exception_init(&ev);

  ElvisPrivate_server = goad_server_activate_with_id(
    NULL,
    "IDL:Elvis:ElvisPrivate:1.0",
    0,
    NULL);
  corbaErrCheck(&ev,"Get Private server");

  if (ElvisPrivate_server==CORBA_OBJECT_NIL) {
    g_message("Couldn't talk to elvis server\n");
    exit(1);
  };

  dir=g_get_current_dir();
  for(i=1;i<argc;i++) {
    if (gotflag) {
      Elvis_ElvisPrivate_openFile(ElvisPrivate_server, dir, argv[i],&ev);
      corbaErrCheck(&ev,"Open file");
    } else {
      if (strcmp(argv[i],"--client")==0) gotflag=1;
    };

  };

  g_free(dir);

  exit(0);

} /* guignome_client */

/* ---------------------------------------------------------------------- */
/* Return (modified) argc, or -1 on failure                               */
static int init(int argc, char **argv)
{
  GladeXML* glade_xml;
  CORBA_Environment ev;
  int i;


  /* Clear the palette (rainbow colour/all free) */
  for(i=0;i<16;i++) {
    unsigned int fullrange=(i & 8)?0x8000:0xffff;

    gui_palette.palette[i].free=ElvTrue;
    gui_palette.palette[i].red=(i & 1)?fullrange:0;
    gui_palette.palette[i].green=(i & 2)?fullrange:0;
    gui_palette.palette[i].blue=(i & 4)?fullrange:0;
  };
  /* 16/17 are terminal default colours */
  gui_palette.palette[16].free=ElvFalse;
  gui_palette.palette[16].red=0;
  gui_palette.palette[16].green=0;
  gui_palette.palette[16].blue=0;
  gui_palette.palette[17].free=ElvFalse;
  gui_palette.palette[17].red=0xffff;
  gui_palette.palette[17].green=0xffff;
  gui_palette.palette[17].blue=0xffff;

  /* Initialise i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  CORBA_exception_init(&ev);
  /* Initialise GNOME */
  orb = gnome_CORBA_init_with_popt_table("elvis", VERSION, &argc, argv,
                                        options, 0, NULL,
                                        GNORBA_INIT_SERVER_FUNC, &ev);

  if (ev._major != CORBA_NO_EXCEPTION) {
    g_message("Corba won't start: %s\n",CORBA_exception_id(&ev));
    exit(1);
  };
  CORBA_exception_free(&ev);

  if (guignome_isClient) {
    return guignome_client(argc,argv);
  };

  if (!guignome_corba_init_server ()) {
    g_warning("Elvis: Could not initialise the CORBA server");
  };

  /* Create a new GNOME application - this gives us a main window (which
  we don't actually want...hmmm) */
  guignome_app = gnome_app_new ("Elvis", _("Elvis for GNOME"));

  /* Note we take a copy of this because we use the name to reread the XML
    later */
  gladename=g_strdup(iopath(tochar8(o_elvispath), "elvis.glade", ElvFalse));
  if (gladename==NULL) {
    msg(MSG_INFO,"Could not find elvis.glade file");

    return ElvFalse;
  };

  glade_gnome_init();

  /* Called here - but we never use it - just to check the file is readable */
  glade_xml=glade_xml_new(gladename, NULL);
  if (glade_xml==NULL) {
    msg(MSG_INFO,"Failed to read elvis.glade file");

    return ElvFalse;
  };

  /* Register our options */
  optinsert("gnome", QTY(guiOptDesc), guiOptDesc, (OPTVAL *)&gui_optvals);

  /* Unless the config files say otherwise these are our default values */
  o_defaultrows=40;
  o_defaultcols=132;

  /* NOTE: Clipboard stuff is bound to guignome_app because elvis holds a global idea
     of clipboards */
  /* Called when something else has claimed the selection */
  gtk_signal_connect(GTK_OBJECT(guignome_app),"selection_clear_event",
                     GTK_SIGNAL_FUNC( guignome_selection_clear), NULL);

  /* Called when we actually get selection data that we requested */
  gtk_signal_connect(GTK_OBJECT(guignome_app),"selection_received",
                     GTK_SIGNAL_FUNC( guignome_selection_received), NULL);

  /* Called when someone else wants us to send the selection data to them */
  gtk_signal_connect(GTK_OBJECT(guignome_app),"selection_get",
                     GTK_SIGNAL_FUNC( guignome_selection_handler), NULL);

  gtk_selection_add_target(guignome_app, GDK_SELECTION_PRIMARY, GDK_SELECTION_TYPE_STRING, 0);

  g_message("Prior to guignome_corba_activate_server in guignome.c\n");
  /* Engage the CORBA stuff */
  guignome_corba_activate_server();

  return argc; /* Should remove gnome options - how ? */
} /* init */

/* ---------------------------------------------------------------------- */
/* GUI dependent options                                                  */
static void usage()
{
  msg(MSG_INFO,"TODO! usage!");
} /* usage */

/* ---------------------------------------------------------------------- */
/* Run round processing events until we have no windows left - for the    */
/* moment I'm just going to run gtk_main                                  */
static void loop()
{
  while (!quit) {
    gtk_main_iteration();
    flush();
  }
} /* loop */

/* ---------------------------------------------------------------------- */
/* Clean up and terminate the GUI                                         */
static void term()
{
  g_message("term\n");
  quit=ElvTrue;
} /* term */

/* ---------------------------------------------------------------------- */
/* Send control code for clr to end of line                               */
static ELVBOOL clrtoeol(GUIWIN* gw) {
  struct guignome_instance *g=(struct guignome_instance *)gw;
#ifdef DEBUG_CALLS
  g_message("clrtoeol\n");
#endif
  writeBuffered(g,"\033[K",3);

  return ElvTrue;
}

/* ---------------------------------------------------------------------- */
/* Set the zvt colour from our palette (on a given window)                */
/* NOTE! Need to call for all!                                            */
static void setTermPalette(struct guignome_instance* gw) {
  gushort r[18],g[18],b[18];
  int i;

  for(i=0;i<18;i++) {
    r[i]=gui_palette.palette[i].red;
    g[i]=gui_palette.palette[i].green;
    b[i]=gui_palette.palette[i].blue;
  };
  zvt_term_set_color_scheme(ZVT_TERM(gw->term),r,g,b);
} /* setTermPalette */

/* ---------------------------------------------------------------------- */
/* Set the zvt colour from our palette on all windows                     */
static void setTermPaletteAll() {
  GSList* currentwin=guignome_instancelist;

  flush();
  while (currentwin!=NULL) {
    setTermPalette((struct guignome_instance*)(currentwin->data));
    currentwin=g_slist_next(currentwin);
  };
  /* zvt requires a forced redraw of all windows after palette change
     (or so it would seem!) */
  /* Note: This is correctly redisplaying ALL text - perhaps its timing with
     when Zvt does its stuff ? */
  redisplayAll();
  flush();
} /* setTermPaletteAll */

/* ---------------------------------------------------------------------- */
/* Elvis wants us to allocate a colour number for the given name          */
static ELVBOOL color(int fontcode, CHAR *colornam, ELVBOOL isfg,
                     long *colorptr, unsigned char rgb[3]) {
  int i;
  g_message("color: fontcode=%d colornam=%s isfg=%d rgb=%d/%d/%d\n",
            fontcode,colornam,isfg,rgb[0],rgb[1],rgb[2]);

  /* Find a free palette entry */
  for(i=0;i<16;i++) {
    if (gui_palette.palette[i].free) {
      GdkColor col;

      if (!gdk_color_parse(colornam,&col)) {
        msg(MSG_ERROR,"Couldn't parse colour name");
        return ElvFalse;
      };
      gui_palette.palette[i].red=col.red;
      gui_palette.palette[i].green=col.green;
      gui_palette.palette[i].blue=col.blue;

      /* Found one */
      gui_palette.palette[i].free=ElvFalse;
      /* Palette index+1 is what we tell Elvis (index 0 = terminal default) */
      *colorptr=(long)(i+1);

      g_message("color: for fontcode=%d allocated=%d %u/%u/%u\n",
                fontcode,i+1,
                (int)(gui_palette.palette[i].red),
                (int)(gui_palette.palette[i].green),
                (int)(gui_palette.palette[i].blue));
      setTermPaletteAll();

      return ElvTrue;
    };
  };


  msg(MSG_ERROR,"Unable to allocate color, out of terminal palette slots\n");
  return ElvFalse;
} /* color */

/* ---------------------------------------------------------------------- */
static void freecolor(long color, ELVBOOL isfg) {
  if ((color>0) && (color<17)) gui_palette.palette[color-1].free=ElvTrue;
  g_message("freecolor: %d\n",color);
} /* freecolor */

/* ---------------------------------------------------------------------- */
/* Called when the user destroys the window                               */
static gint gui_closeWindow(GtkWidget *w, GdkEventAny *e, gpointer data) {
  struct guignome_instance *gw=(struct guignome_instance *)data;

  destroygw(gw, ElvTrue);

  return FALSE;
}

/* ---------------------------------------------------------------------- */
/* Callback (on idle tasks) when the terminal should have resized         */
static gint gui_configureWindowCallBack(gpointer data) {
  struct guignome_instance *gw=(struct guignome_instance *)data;
  ZvtTerm* zvt=ZVT_TERM(gw->term);

  /* Check to see if it is resized or just moved */
  if ((zvt->vx->vt.height!=gw->height) ||
      (zvt->vx->vt.width!=gw->width)) {
    /* Real resize, update the stored size as well */
    gw->height=zvt->vx->vt.height;
    gw->width=zvt->vx->vt.width;
    eventresize((GUIWIN *)gw, zvt->vx->vt.height, zvt->vx->vt.width);
    eventdraw((GUIWIN*)gw);
  };

  gtk_idle_remove(gw->configureCallBack);
  gw->configureCallBack=0;
} /* gui_configureWindowCallBack */

/* ---------------------------------------------------------------------- */
/* Called when the user resizes the window                                */
static gint gui_configureWindow(GtkWidget *w, GdkEventConfigure *e, gpointer data) {
  struct guignome_instance *gw=(struct guignome_instance *)data;
  ZvtTerm* zvt=ZVT_TERM(gw->term);

  /*g_message("gui_configureWindow: vt.height=%d vt.width=%d\n",zvt->vx->vt.height,zvt->vx->vt.width);*/

  /* Problem is that we get called after the window but before the terminal
     is resized */
  /* So schedule a callback later */
  if (!(gw->configureCallBack)) {
    gw->configureCallBack=gtk_idle_add(gui_configureWindowCallBack,gw);
  };
  return ElvTrue;
} /* gui_closeWindow */

/* ---------------------------------------------------------------------- */
/* Called when the user hits a key                                        */
static gint gui_KeyPress(GtkWidget *w, GdkEventKey *event, gpointer data) {
  struct guignome_instance *gw=(struct guignome_instance *)data;
  int ctrl,alt;
  char buf[2];

  guignome_clip_keypress();

  ctrl=(event->state & GDK_CONTROL_MASK)!=0;
  alt=(event->state & GDK_MOD1_MASK)!=0;

  /*g_message("key press %d %s %d\n",event->keyval,gdk_keyval_name(event->keyval),
         ctrl); */

  if ((!ctrl) && ((event->keyval>0) && (event->keyval<0xff) && (elvprint(event->keyval)))) {
    buf[0]=event->keyval;
    buf[1]='\0';
    eventkeys((GUIWIN *)gw, toCHAR(buf), 1);
    if (stillExists(gw)) {
      eventdraw((GUIWIN *)gw);
    };
  } else {
    /* It is not a normal key stroke - so look down our lookup table and figure
       out what we are going to tell Elvis
    */
    struct key_s* kptr=&(keys[0]);

    while (kptr->nchars) {
      if ((event->keyval==kptr->keyval) &&
          ((ctrl==kptr->ctrlstate) || (kptr->ctrlstate==-1)) &&
          ((alt==kptr->altstate) || (kptr->altstate==-1))) {
        eventkeys((GUIWIN *)gw, toCHAR(kptr->towrite), kptr->nchars);
        if (stillExists(gw)) 
          eventdraw((GUIWIN *)gw);
        return ElvTrue;
      };
      kptr++;
    };
    /* We seem to have a key which didn't match */
    /*g_message("Unrecognised key press %d %s %d\n",event->keyval,gdk_keyval_name(event->keyval),
        ctrl); */
  };

  return ElvTrue;
} /* KeyPress */

/* ---------------------------------------------------------------------- */
/* Called by elvis to tell us about the size of the scroll bar            */
static void scrollbar(GUIWIN *gw, long top, long bottom, long total) {
  struct guignome_instance *g=(struct guignome_instance *)gw;
  if (total>0) {
    g->sb_top=top;
    g->sb_bottom=bottom;
    g->sb_total=total;
  } else {
    g->sb_top=0;
    g->sb_bottom=1;
    g->sb_total=1;
  };

  /* Set size of page according to current visible */
  g->vadj->page_size=((double)(g->sb_bottom-g->sb_top))/(double)g->sb_total;
  g->vadj->value=(double)g->sb_top/(double)g->sb_total;
  gtk_adjustment_changed(g->vadj);

} /* scrollbar */

/* ---------------------------------------------------------------------- */
/* Called when the scroll bar moves                                       */
static gint gui_VScrollChanged(GtkAdjustment *w, gpointer data) {
  struct guignome_instance *gw=(struct guignome_instance *)data;

  /*g_message("VScrollChanged: %f\n",gw->vadj->value); */
  /* We use the adjustment to keep a value between 0 and 1 */
  eventscroll((GUIWIN *)gw, SCROLL_PERCENT, (long)(gw->vadj->value*(1<<24)), (1<<24));

  /* And update... */
  eventdraw((GUIWIN *)gw);
} /* gui_VScrollChanged */

/* ---------------------------------------------------------------------- */
/* Execute the commands for the dialog but don't close the dialog         */
static void gui_DialogApply(GtkWidget* w, gpointer data) {
  struct guignome_windaction* wda=(struct guignome_windaction*)data;
  GUIWIN* gw=(GUIWIN*)wda->instance;
  WINDOW win=winofgw(gw);
  char* commandString;

  g_message("gui_DialogApply\n");
  commandString=expandCommandString(wda->daction->cmd,wda);
  if (commandString==NULL) return; /* Something went wrong in the expansion */

  /* Some commands presume that we are in ex : mode and try and pop this off the stack
     - unfortunatly we aren't in it! So put a dummy state in */
  statepush(win,win->state->flags | ELVIS_BOTTOM);
  eventex(gw,commandString,ElvFalse);
  if (stillExists(gw)) {
    statepop(win);
    eventdraw(gw);
  };
  g_free(commandString);
} /* gui_DialogApply */

/* ---------------------------------------------------------------------- */
/* Just close the dialog                                                  */
static void gui_DialogCancel(GtkWidget* w, gpointer data) {
  struct guignome_windaction* wda=(struct guignome_windaction*)data;
  gtk_widget_hide(wda->dialogue);
} /* gui_DialogCancel */

/* ---------------------------------------------------------------------- */
/* Close the dialog and hten execute                                      */
static void gui_DialogOK(GtkWidget* w, gpointer data) {
  struct guignome_windaction* wda=(struct guignome_windaction*)data;
  gtk_widget_hide(wda->dialogue);
  gui_DialogApply(w,data);
} /* gui_DialogOK */

/* ---------------------------------------------------------------------- */
/* Called if the user tries to close a dialogue - just hide it            */
static gint gui_DialogDelete(GtkWidget* w, GdkEventAny *e, gpointer data) {
  gtk_widget_hide(w);

  return ElvTrue;
} /* gui_DialogDelete */
/* ---------------------------------------------------------------------- */
/* Called when the user selects a menuentry                               */
static int gui_MenuEntry(GtkWidget* w, gpointer data) {
  struct guignome_winmenuentry* wme=(struct guignome_winmenuentry*)data;
  WINDOW win=winofgw((GUIWIN*)wme->gnomewin);

  /* Just a plain command */
  /*statepush(win,0);*/
  eventex((GUIWIN*)wme->gnomewin,wme->menuentry->command,ElvFalse);
  if (stillExists(wme->gnomewin)) 
	eventdraw((GUIWIN*)wme->gnomewin);
  /*statepop(win);*/
} /* gui_MenuEntry */

/* ---------------------------------------------------------------------- */
/* Callback when the user clicks on a toolbar entry                       */
static void toolbarcallback(GtkWidget* w, gpointer data) {
  struct guignome_wintoolbarentry* wtbe=(struct guignome_wintoolbarentry*)data;
  struct guignome_instance* gw=wtbe->gnomewin;

  eventex((GUIWIN*)gw,wtbe->toolbarentry->action,ElvFalse);
} /* toolbarcallback */
/* ---------------------------------------------------------------------- */
/* Create an entry in the toolbar                                         */
static void creategw_buildtoolbarentry(gpointer data, gpointer user_data) {
  struct guignome_toolbarentry* tbe=(struct guignome_toolbarentry*)data;
  struct guignome_instance* gw=(struct guignome_instance*)user_data;
  struct guignome_wintoolbarentry* wtbe=g_malloc(sizeof(struct guignome_wintoolbarentry));

  wtbe->gnomewin=gw;
  wtbe->toolbarentry=tbe;
  gw->wtes=g_slist_append(gw->wtes,wtbe);
  
  gtk_toolbar_append_item(GTK_TOOLBAR(gw->toolbar),tbe->text,tbe->tttext,"",
                          gnome_stock_pixmap_widget(gw->window,tbe->icon),
                          toolbarcallback,
                          wtbe);
                          
} /* creategw_buildtoolbarentry */

/* ---------------------------------------------------------------------- */
struct bufferMenuEntry {
  struct guignome_winmenubarentry* wmbe;
  BUFFER buf;
};

/* ---------------------------------------------------------------------- */
/* Called when the user selects an entry on the Buffer menu               */
static int gui_BufferMenuEntrySelect(GtkWidget* w, gpointer data) {
  struct bufferMenuEntry* bme=data;

  char tmpcmd[20];

  sprintf(tmpcmd,":b %d",o_bufid(bme->buf));
  eventex((GUIWIN*)bme->wmbe->gnomewin,tmpcmd,ElvFalse);
  eventdraw((GUIWIN*)bme->wmbe->gnomewin);
}

/* ---------------------------------------------------------------------- */
/* Called when a menu hanging off the menu bar becomes visible - our      */
/* chance to update its ticks/content before it is actualyl displayed     */
static int gui_MenuBarEntryShown(GtkWidget* w, gpointer data) {
  struct guignome_winmenubarentry* wmbe=(struct guignome_winmenubarentry*)data;
  BUFFER current=NULL;

  if (strcmp(wmbe->menubarentry->text,"*Buffers")==0) {
    if (wmbe->gnomewin->specialdata) g_slist_free(wmbe->gnomewin->specialdata);
    wmbe->gnomewin->specialdata=NULL;
    while (GTK_MENU_SHELL(wmbe->menu)->children)
      gtk_widget_destroy(GTK_MENU_SHELL(wmbe->menu)->children->data);
	do {
                        struct bufferMenuEntry* bme=g_malloc(sizeof(struct bufferMenuEntry));

			current=buflist(current);
			if ((current!=NULL) && (!o_internal(current))) {
			  GtkWidget* menuitem;
			  menuitem=gtk_menu_item_new_with_label(o_bufname(current));
			  gtk_widget_show(menuitem);
			  gtk_menu_append(GTK_MENU(wmbe->menu),menuitem);
			/* Call gui_MenuEntry with that structure when clicked */
			gtk_signal_connect(GTK_OBJECT(menuitem),"activate",
					(GtkSignalFunc) gui_BufferMenuEntrySelect, bme);
                        bme->wmbe=wmbe;
                        bme->buf=current;
                        wmbe->gnomewin->specialdata=g_slist_append(wmbe->gnomewin->specialdata,bme);

			};

		} while (current!=NULL);
  } else {
    /* Normal menu - Update ticks on menus */
    struct guignome_winmenuentry* wme;
    GSList* wmes=wmbe->wmes;

    while (wmes!=NULL) {
      wme=(struct guignome_winmenuentry*)(wmes->data);

      if (wme->menuentry->optname!=NULL) {
        /* Menu entry has a tick which should be set if the option name matches the given value */
        char *optvaltext=optgetstr(wme->menuentry->optname,NULL);
        /* What do we do if its null - i.e. invalid option? Perhaps we should
           grey the menu option (not relevant to mode? */
        if (optvaltext!=NULL) {
          if (strcmp(optvaltext,wme->menuentry->optvalue)) {
            gtk_menu_item_configure(GTK_MENU_ITEM(wme->menuentrywidget),ElvFalse,False);
          } else {
            gtk_menu_item_configure(GTK_MENU_ITEM(wme->menuentrywidget),ElvTrue,False);
          };

        }; /* optvaltext!=NULL */
      };
      wmes=g_slist_next(wmes);
    };
  };


	/* NOTE! Return false actually enables the menu to come up */
	return ElvFalse;
}

/* ---------------------------------------------------------------------- */
/* Create an entry in the menubar                                         */
static void creategw_buildmenubarentry(gpointer data, gpointer user_data) {
  struct guignome_menubarentry* mbe=(struct guignome_menubarentry*)data;
  struct guignome_instance* gw=(struct guignome_instance*)user_data;
  GtkWidget* menu;
  GtkWidget* menuitem;
  struct guignome_menuentry* me;
  GSList* current_me;
  struct guignome_winmenuentry* wme;
  struct guignome_winmenubarentry* wmbe;

  wmbe=(struct guignome_winmenubarentry*)g_malloc(sizeof(struct guignome_winmenubarentry));
  wmbe->gnomewin=gw;
  wmbe->menubarentry=mbe;
  gw->wmbes=g_slist_append(gw->wmbes,wmbe);

  menuitem = gtk_menu_item_new_with_label (mbe->text);
  gtk_menu_bar_append(GTK_MENU_BAR(gw->menubar), menuitem);

  /*g_message("creategw_buildmenubarentry\n");*/
  menu=gtk_menu_new();
  gtk_signal_connect(GTK_OBJECT(menu),"show", (GtkSignalFunc) gui_MenuBarEntryShown, wmbe);
  wmbe->menu=menu;
  wmbe->wmes=NULL;
  gtk_widget_show(menuitem);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

  /* Now create the entries on the menu */
  current_me=mbe->entries;
  while (current_me!=NULL) {
    me=(struct guignome_menuentry*)(current_me->data);
    if (me->text!=NULL) {
      if (me->optname!=NULL) {
	menuitem = gtk_check_menu_item_new_with_label (me->text);
        gtk_check_menu_item_set_show_toggle(menuitem,ElvFalse);
      } else {
	menuitem = gtk_menu_item_new_with_label(me->text);
      };

	/* Create a structure for this entry on this window */
	wme=(struct guignome_winmenuentry*)g_malloc(sizeof(struct guignome_winmenuentry));
	wme->gnomewin=gw;
	wme->menuentry=me;
        wme->menuentrywidget=menuitem;
        /* Also add this to the list on the winmenu bar entry */
        wmbe->wmes=g_slist_append(wmbe->wmes,wme);

	/* Call gui_MenuEntry with that structure when clicked */
	gtk_signal_connect(GTK_OBJECT(menuitem),"activate",
			(GtkSignalFunc) gui_MenuEntry, wme);
    } else {
    	menuitem=gtk_hseparator_new();
    }
    gtk_menu_append(GTK_MENU(menu),menuitem);
    gtk_widget_show(menuitem);

    current_me=g_slist_next(current_me);
  };

} /* creategw_buildmenubarentry */

/* ---------------------------------------------------------------------- */
static void creategw_popupmenu(struct guignome_instance* gw) {
  GtkWidget* menu=gw->popupmenu;
  GtkWidget* menuitem;
  struct guignome_menuentry* me;
  GSList* current_me;
  struct guignome_winmenuentry* wme;
  struct guignome_winmenubarentry* wmbe;

  /* Now create the entries on the menu */
  current_me=guignome_popupmenuentrylist;
  while (current_me!=NULL) {
    me=(struct guignome_menuentry*)(current_me->data);
    if (me->text!=NULL) {
      if (me->optname!=NULL) {
	menuitem = gtk_check_menu_item_new_with_label (me->text);
      } else {
	menuitem = gtk_menu_item_new_with_label(me->text);
      };

	/* Create a structure for this entry on this window */
	wme=(struct guignome_winmenuentry*)g_malloc(sizeof(struct guignome_winmenuentry));
	wme->gnomewin=gw;
	wme->menuentry=me;
	/* Add this to the windows list of our entries - so it can tidy up later */
	gw->wmes=g_slist_append(gw->wmes,wme);

	/* Call gui_MenuEntry with that structure when clicked */
	gtk_signal_connect(GTK_OBJECT(menuitem),"activate",
			(GtkSignalFunc) gui_MenuEntry, wme);
    } else {
    	menuitem=gtk_hseparator_new();
    }
    gtk_menu_append(GTK_MENU(menu),menuitem);
    gtk_widget_show(menuitem);

    current_me=g_slist_next(current_me);
  };
}

/* ---------------------------------------------------------------------- */
/* set_hints is borrowed and munged from gnome_terminal. It sets up the   */
/* window manager hints                                                   */

static void set_hints (struct guignome_instance* gw) {
  ZvtTerm *term;
  GdkGeometry hints;
  GtkWidget *app;
  
  term = ZVT_TERM (gw->term);
    
#define PADDING 2
  hints.base_width = (GTK_WIDGET (term)->style->klass->xthickness * 2) + PADDING;
  hints.base_height =  (GTK_WIDGET (term)->style->klass->ythickness * 2);
    
  hints.width_inc = term->charwidth;
  hints.height_inc = term->charheight;
  hints.min_width = hints.base_width + hints.width_inc;
  hints.min_height = hints.base_height + hints.height_inc;   
           
  gtk_window_set_geometry_hints(GTK_WINDOW(gw->window),
              GTK_WIDGET(term),
              &hints,
              GDK_HINT_RESIZE_INC|GDK_HINT_MIN_SIZE|GDK_HINT_BASE_SIZE);
}   

/* ---------------------------------------------------------------------- */
/* Create a new window - return true and simulate a create event if OK    */
static ELVBOOL creategw(char *name, char* firstcmd)
{
  struct guignome_instance *gw=g_malloc(sizeof(struct guignome_instance));
  static GtkTargetEntry target_table[] = {
    { "text/uri-list", 0, 0 },
  };
  static GnomeUIInfo blankpopupguiinfo[] = {
    GNOMEUIINFO_END
  };

  if (gw==NULL) return ElvFalse;

  g_message("creategw: name=%s firstcmd=%s\n",name,firstcmd);
  gw->writeptr=0;
  gw->configureCallBack=0;
  gw->lastDrawBit=-1;
  gw->lastFore=gw->lastBack=gw->lastDrawColBits=-1;
  gw->buttonsDown=0;
  gw->downx=gw->downy=0;
  gw->justClick=ElvFalse;
  gw->specialdata=NULL;

  guignome_instancelist=g_slist_prepend(guignome_instancelist,gw);

  g_message("creategw: Pre-gnome_app_new");
  /* A window containing a vbox, the vbox has a menubar, the main area, and a status
     bar.  The main area is an hbox with the scrollbar to one side */
  gw->window=gnome_app_new("Elvis", name);
  if (gw->window==NULL) g_message("creategw: Didn't get gnome_app_new\n");

  gw->vbox=gtk_vbox_new(ElvFalse,10);
  if (gw->vbox==NULL) g_message("creategw: Din't get gtk_vbox_new\n");
  gnome_app_set_contents(GNOME_APP(gw->window),gw->vbox);

  /* Reread XML for this window - I don't like the way libglade organises this */
  gw->glade_xml=glade_xml_new(gladename, NULL);

  if (gw->glade_xml==NULL) {
    g_message("Unable to reread glade file (%s)",gladename);
    msg(MSG_ERROR,"Unable to reread glade file");
    return ElvFalse;
  };

  gw->wmbes=NULL;
  gw->wmes=NULL;
  gw->wtes=NULL;
  gw->wdacs=NULL;
  gw->menubar=gtk_menu_bar_new();
  g_slist_foreach(guignome_menubarentrylist,creategw_buildmenubarentry,gw);
  gnome_app_set_menus(GNOME_APP(gw->window),GTK_MENU_BAR(gw->menubar));
  if (guignome_toolbarentrylist) {
    gw->toolbar=gtk_toolbar_new(GTK_ORIENTATION_HORIZONTAL,GTK_TOOLBAR_BOTH);
    gnome_app_set_toolbar(GNOME_APP(gw->window),GTK_TOOLBAR(gw->toolbar));
    g_slist_foreach(guignome_toolbarentrylist,creategw_buildtoolbarentry,gw);
  } else {
    gw->toolbar=NULL;
  };

  gw->statusbar=gtk_hbox_new(ElvFalse,0);
  gw->statusbar_cmdpre=gnome_appbar_new(ElvFalse, ElvTrue, GNOME_PREFERENCES_NEVER);
  gw->statusbar_pos=gnome_appbar_new(ElvFalse, ElvTrue, GNOME_PREFERENCES_NEVER);
  gw->statusbar_mode=gnome_appbar_new(ElvFalse, ElvTrue, GNOME_PREFERENCES_NEVER);
  gtk_widget_show(gw->statusbar_cmdpre);
  gtk_widget_show(gw->statusbar_pos);
  gtk_widget_show(gw->statusbar_mode);
  gtk_container_add(GTK_CONTAINER(gw->statusbar),gw->statusbar_cmdpre);
  gtk_container_add(GTK_CONTAINER(gw->statusbar),gw->statusbar_pos);
  gtk_container_add(GTK_CONTAINER(gw->statusbar),gw->statusbar_mode);
  gnome_app_set_statusbar(GNOME_APP(gw->window),gw->statusbar);
  gw->hbox=gtk_hbox_new(ElvFalse,0);
  gtk_container_add(GTK_CONTAINER(gw->vbox),gw->hbox);

  /* To force an initial redraw */
  gw->width=-1;
  gw->height=-1;
  gw->term = zvt_term_new_with_size(o_defaultcols,o_defaultrows);
  zvt_term_set_scrollback(ZVT_TERM(gw->term),0);
  zvt_term_set_scroll_on_keystroke(ZVT_TERM(gw->term),0);
  zvt_term_set_scroll_on_output(ZVT_TERM(gw->term),0);
  gtk_signal_connect(GTK_OBJECT(gw->term),"key_press_event",
                     (GtkSignalFunc) gui_KeyPress, gw);
  gtk_signal_connect(GTK_OBJECT(gw->window),"delete_event",
                     GTK_SIGNAL_FUNC( gui_closeWindow ), gw);
  gtk_signal_connect(GTK_OBJECT(gw->window),"configure_event",
                     GTK_SIGNAL_FUNC( gui_configureWindow ), gw);

  /* Allow drag from file manager */
  gtk_signal_connect(GTK_OBJECT(gw->window),"drag_data_received",
                     GTK_SIGNAL_FUNC( dndDrop ), gw);

  gtk_drag_dest_set (GTK_WIDGET(gw->window),
                     GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_DROP, target_table, 1,
                     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  /* Click to move, drag etc (perhaps some pop up menus) */
  gtk_signal_connect(GTK_OBJECT(gw->term),"button_press_event",
                     GTK_SIGNAL_FUNC(guignome_ButtonPress), gw);
  gtk_signal_connect(GTK_OBJECT(gw->term),"button_release_event",
                     GTK_SIGNAL_FUNC(guignome_ButtonRelease), gw);
  gtk_signal_connect(GTK_OBJECT(gw->term),"motion_notify_event",
                     GTK_SIGNAL_FUNC(guignome_MotionNotify), gw);

  gtk_box_pack_start(GTK_BOX(gw->hbox),gw->term,ElvTrue,True,0);
  gtk_widget_show(gw->term);

  /* Initial dummy scroll bar values */
  gw->sb_top=0;
  gw->sb_bottom=1;
  gw->sb_total=1;

  gw->vadj=GTK_ADJUSTMENT(gtk_adjustment_new(0,0,1,0.1,0.2,1)); /* Fairly arbitrary values ! */
  gw->scrollbar=gtk_vscrollbar_new(gw->vadj);
  gtk_box_pack_start(GTK_BOX(gw->hbox),gw->scrollbar,ElvFalse,False,0);
  gtk_widget_show(gw->scrollbar);

  gtk_widget_show(gw->hbox);
  gtk_widget_show(gw->vbox);

  gtk_window_set_title (GTK_WINDOW (gw->window), name);
  gtk_window_set_wmclass (GTK_WINDOW (gw->window), "elvis", "elvis");
  gtk_widget_show(gw->window);

  /* Turn on some event masks */
  gdk_window_set_events((GdkWindow *)(gw->window->window),
                       gdk_window_get_events((GdkWindow *)(gw->window->window))|
     GDK_KEY_PRESS_MASK);

  gw->popupmenu=gnome_popup_menu_new(blankpopupguiinfo);
  gnome_popup_menu_attach(gw->popupmenu,gw->term,gw);
  creategw_popupmenu(gw);

  g_message("creategw: Pre-eventcreate");
  eventcreate((GUIWIN *)gw, NULL, name, 24, 80);
  g_message("creategw: Post-eventcreate");

  /* if there is a firstcmd, then execute it */
  if (firstcmd)
  {
    winoptions(winofgw((GUIWIN *)gw));
    exstring(windefault, toCHAR(firstcmd), "+cmd");
  }
  g_message("creategw: Post-exstring");

  gtk_signal_connect(GTK_OBJECT(gw->vadj),"value_changed",
                     (GtkSignalFunc) gui_VScrollChanged, gw);
  set_hints(gw);

  setTermPalette(gw);
  /* eventdraw((GUIWIN *)gw); */
  g_message("creategw: creategw exit");
  return ElvTrue;
} /* creategw */

/* ---------------------------------------------------------------------- */
void guignome_doopenfile(char *filename, char *dir) {
  char *tmp;
  BUFFER buffer;

  gchar* escape_dir=guignome_escapeString(dir);
  gchar* escape_file=guignome_escapeString(filename);

  buffer=bufalloc(NULL, 0, ElvFalse);
  tmp=g_strdup_printf(":cd %s\n:e %s\n",escape_dir,escape_file);
  g_free(escape_dir);
  g_free(escape_file);
  g_message("guignome_doopenfile: %s\n",tmp);
  creategw(tochar8(o_bufname(buffer)),tmp);

  g_free(tmp);
} /* guignome_doopenfile */

/* ---------------------------------------------------------------------- */
/* Delete a window                                                        */
static void destroygw(GUIWIN *gwin, ELVBOOL force)
{
  struct guignome_instance* gw=(struct guignome_instance*)gwin;
  /* Run through the list and remove this element */
  struct guignome_instance **prev,*now;
  GSList* wedel;
  GSList* wmbes;

  g_message("destroygw\n");

  if (gw->configureCallBack) gtk_idle_remove(gw->configureCallBack);

  guignome_instancelist=g_slist_remove(guignome_instancelist,gw);

  /* Destroy the appropriate editor window (and automatically via reference
     counting any widgets it contains */
  gtk_widget_destroy(GTK_WIDGET(gw->window));

  /* Destroy any dialogues hanging off menus, and clean up the menu data
     structures */
  wedel=gw->wdacs;
  while (wedel!=NULL) {
    struct guignome_windaction* wde=(struct guignome_windaction*)(wedel->data);
    g_message("destroygw: wde destroy code\n");
    g_message("destroygw: Got dialogue to destroy\n");
    /*gnome_dialog_close(GNOME_DIALOG(wme->dialogue)); */
    gtk_widget_destroy(GTK_WIDGET(wde->dialogue));
    g_free(wde);
    wedel=g_slist_next(wedel);
  };

  /* Destroy the list of them */
  wmbes=gw->wmbes;
  while (wmbes!=NULL) {
    struct guignome_winmenubarentry* wmbe=(struct guignome_winmenubarentry*)(wmbes->data);

    g_slist_free(wmbe->wmes);
    wmbes=g_slist_next(wmbes);
  };

  g_slist_free(gw->wmbes);
  g_slist_free(gw->wmes);
  g_slist_free(gw->wtes);
  g_slist_free(gw->wdacs);
  g_slist_free(gw->specialdata);
  gtk_object_destroy(GTK_OBJECT(gw->glade_xml));
  g_free(gw);

  /* If their is nothing left then quit */
  if (!g_slist_length(guignome_instancelist)) term();
} /* destroygw */

/* ---------------------------------------------------------------------- */
static ELVBOOL focusgw(GUIWIN *gw)
{
  gtk_widget_grab_focus(GTK_WIDGET(((struct guignome_instance *)gw)->window));

  return ElvTrue;
} /* focusgw */

/* ---------------------------------------------------------------------- */
static void retitle(GUIWIN *gw, char* name)
{
  gtk_window_set_title (GTK_WINDOW(((struct guignome_instance *)gw)->window), name);
}

/* ---------------------------------------------------------------------- */
/* Flush a particular window                                              */
static void flushWindow(struct guignome_instance* gw) {
#ifdef DEBUG_CALLS
  g_message("Flushwindow\n");
#endif
  /* Write */
  zvt_term_feed(ZVT_TERM(gw->term), gw->tempbuffer, gw->writeptr);
  /* and empty */
  gw->writeptr=0;
} /* flushWindow */

/* ---------------------------------------------------------------------- */
/* Flush buffers in all windows                                           */
static void flush() {
  GSList* currentwin=guignome_instancelist;

#ifdef DEBUG_CALLS
  g_message("flush\n");
#endif
  while (currentwin!=NULL) {
    flushWindow((struct guignome_instance*)(currentwin->data));
    currentwin=g_slist_next(currentwin);
  };
} /* flush */

/* ---------------------------------------------------------------------- */
/* Force redisplay of all windows - NOTE! This may be expensive           */
static void redisplayAll() {
  GSList* currentwin=guignome_instancelist;

  while (currentwin!=NULL) {
    struct guignome_instance* gw=(struct guignome_instance*)(currentwin->data);
    /* Just clear all the terminal state */
    writeBuffered(gw,"\033[0m\033[39m\033[49m",14);
    eventexpose((GUIWIN*)gw,0,0,gw->height,gw->width);
    currentwin=g_slist_next(currentwin);
  };
} /* redisplayAll */

/* ---------------------------------------------------------------------- */
static void moveto(GUIWIN *gw, int column, int row)
{
  /* Build an escape code to send to the terminal to get it to move -
     Escape[yvalue;xvalueH */
  char tmp[50];

#ifdef DEBUG_CALLS
  g_message("moveto %d,%d\n",column,row);
#endif
  sprintf(tmp,"\033[%d;%dH",row+1,column+1);

  writeBuffered((struct guignome_instance *)gw,tmp,strlen(tmp));

} /* moveto */

/* ---------------------------------------------------------------------- */
/* Put some text on the terminal, 'font' needs to be interpreted for bold
   etc - but we don't do that yet.  */
static void draw(GUIWIN* w, long fg, long bg, int bits,CHAR *text,int len) {
  struct guignome_instance* gw=(struct guignome_instance*)w;

  /*g_message("draw of %d in %ld/%ld\n",len,fg,bg);*/

  /* Test for change in flags */
  if (gw->lastDrawBit!=-1) {
    /* We send an ESC[valm - where val is a code to select features
         1=bold
         4=underline
         7=Reverse

       (val+=20 to turn off)
       (Italic would be nice - that comes down to when zvt supports it)
       We map Elvis's COLOR_BOLD and COLOR_ITALIC flags to terminal Bold
     */
     int changedMask;
     /* -1 = initial state - presume everything has changed */
     if (gw->lastDrawBit!=-1)
       changedMask=bits^gw->lastDrawBit;
     else
       changedMask=COLOR_BOLD|COLOR_ITALIC|COLOR_UNDERLINED|COLOR_BOXED;

     /* Double check for safety */
     if (changedMask) {
       if (changedMask & (COLOR_BOLD | COLOR_ITALIC)) {
 	 writeBuffered(gw,
	              (bits & (COLOR_BOLD|COLOR_ITALIC))?"\033[1m":"\033[21m",
	 (bits & (COLOR_BOLD|COLOR_ITALIC))?4:5);
       }; /* Bold/italic */

       if (changedMask & COLOR_UNDERLINED) {
         writeBuffered(gw,(bits & COLOR_UNDERLINED)?"\033[4m":"\033[24m",
         (bits & COLOR_UNDERLINED)?4:5);
       }; /* underlined */

       if (changedMask & COLOR_BOXED) {
         printf("Boxed\n");
         writeBuffered(gw,(bits & COLOR_BOXED)?"\033[7m":"\033[27m",
         (bits & COLOR_BOXED)?4:5);
       }; /* BOXED (used on selections should be a square box)  */
     }; /* ChangedMask */
    gw->lastDrawBit=bits;
  }; /* Last draw bits */

  /* We use colour code 0 to set to the terminals default scheme, and then
     1..16 as the user defined ones */
  /* Now check for changes in foreground color */
  if ((gw->lastFore!=fg) || (gw->lastDrawColBits != (bits & (COLOR_FG | COLOR_BG)))) {
    long locfg=fg;

    /* If foreground not enabled then presume default foreground */
    if ((bits & COLOR_FG)==0) locfg=0;

    if ((locfg<0) || (locfg>16)) {
      /* Invalid fg number - complain */
      g_message("draw was passed a bad fg colour code %ld\n",fg);
    } else {
      char tmp[50];

      if (locfg==0) {
        /* Terminal code 39=clear foreground colour */
        writeBuffered(gw,"\033[39m",5);
      } else {
	/* Codes 30..37=colours 0..7, codes 90..97=colours 8..15 */
	sprintf(tmp,"\033[%dm",(locfg<9)?(locfg+29):(locfg+(90-9)));
	writeBuffered(gw,tmp,strlen(tmp));
      };
    };

    gw->lastFore=fg;
  };

  /* Now check for changes in background color */
  if ((gw->lastBack!=bg) || (gw->lastDrawColBits != (bits & (COLOR_FG | COLOR_BG)))) {
    long locbg=bg;

    /* If foreground not enabled then presume default foreground */
    if ((bits & COLOR_BG)==0) locbg=0;

    if ((locbg<0) || (locbg>15)) {
      /* Invalid bg number - complain */
      g_message("draw was passed a bad locbg colour code %ld\n",bg);
    } else {
      char tmp[50];

      if (locbg==0) {
        /* Terminal code 49=clear background colour */
        writeBuffered(gw,"\033[49m",5);
      } else {
	/* Codes 40..47=colours 0..7, codes 100..107=colours 8..15 */
	sprintf(tmp,"\033[%dm",(locbg<9)?(locbg+39):(locbg+(100-9)));
	writeBuffered(gw,tmp,strlen(tmp));
      };
    };

    gw->lastBack=bg;
  };

  gw->lastDrawColBits=(bits & (COLOR_FG | COLOR_BG));

  writeBuffered(gw,text,len);
} /* draw */

/* ---------------------------------------------------------------------- */
/* Search for a ?string? (i.e. matching puncts) in line, return a copy    */
/* and update the line pointer                                            */
/* Returns NULL if something is wrong with input string                   */
static char* extractString(char** linep, char* errstring) {
  char* line=*linep;
  char* endofquote;

  char* res;
  /* Skip initial spaces */

  while ((elvspace(*line)) && (*line)) line++;
  /* We use a punctuation character to quote the text - any punctuation */
  if (!elvpunct(*line)) {
    msg(MSG_ERROR,"[s]$1",errstring);
    return NULL;
  };

  if (endofquote=strchr(line+1,*line),endofquote==NULL) {
    msg(MSG_ERROR,"[s]$1",errstring);
    return NULL;
  };
  /* Tack a carriage return on to get the command executed */
  res=g_strdup_printf("%.*s",(endofquote-line)-1,line+1);

  *linep=endofquote+1;
  return res;
} /* extractString */

/* ---------------------------------------------------------------------- */
/* gui menu addentry ?barentry? ?menuitemname? ?command?    '?' can be any character same as end   */
static ELVBOOL guicmd_menu_addentry(char* line) {
  gchar* barentry;
  gchar* menuitemname;
  gchar* command;
  gchar* optname, *optvalue;

  GSList* menubarentry=guignome_menubarentrylist;
  struct guignome_menubarentry* foundentry;
  struct guignome_menuentry* newentry;

  /*g_message("guicmd_menu_addentry\n");*/

  /* Extract menu bar entry name */
  barentry=extractString(&line,"Usage: gui menu addentry ?barentry? ?menuitemname? ?commandname?");
  if (barentry==NULL) return ElvFalse;
  
  /* Search along the bar to find the correct entry */
  while ((menubarentry!=NULL) && 
        (strcmp(((struct guignome_menubarentry*)(menubarentry->data))->text,barentry)))
    menubarentry=g_slist_next(menubarentry);
    
  if (menubarentry==NULL) {
    g_free(menubarentry);
    msg(MSG_ERROR,"Menu bar entry not found");
    return ElvFalse;
  };

  foundentry=(struct guignome_menubarentry*)(menubarentry->data);

  /* Extract menu item name */
  menuitemname=extractString(&line,"Usage: gui menu addentry ?barentry? ?menuitemname? ?commandname?");
  if (menuitemname==NULL) {
    g_free(barentry);
    return ElvFalse;
  };
  

  /* Extract command text */
  command=extractString(&line,"Usage: gui menu addentry ?barentry? ?menuitemname? ?commandname?");
  if (command==NULL) {
    g_free(barentry);
    g_free(menuitemname);
    return ElvFalse;
  };

  /* Create the new entry and add it to the list */
  newentry=g_malloc(sizeof(struct guignome_menuentry));
  newentry->text=menuitemname;
  newentry->command=command;
  newentry->optname=NULL;
  newentry->optvalue=NULL;

  /* Potentially we may have an option to watch to tick */
  optname=extractString(&line,"Usage: gui menu addentry ?barentry? ?menuitemname? ?commandname? (?optname? ?trueoptvalue?)");
  if (optname!=NULL) {
    /* OK - we have an option name, we should have a value */
    optvalue=extractString(&line,"Usage: gui menu addentry ?barentry? ?menuitemname? ?commandname? (?optname? ?trueoptvalue?)");
    if (optvalue==NULL) {
      /* Hmm no option value - should moan */
      g_free(optname);
    } else {
      newentry->optname=optname;
      newentry->optvalue=optvalue;
    }
  }

  foundentry->entries=g_slist_append(foundentry->entries,newentry);

  return ElvTrue;
} /* guicmd_menu_addentry */

/* ---------------------------------------------------------------------- */
/* gui menu addsep ?barentry?                                             */
static ELVBOOL guicmd_menu_addsep(char* line) {
  gchar* barentry;

  GSList* menubarentry=guignome_menubarentrylist;
  struct guignome_menubarentry* foundentry;
  struct guignome_menuentry* newentry;

  /* Extract menu bar entry name */
  barentry=extractString(&line,"Usage: gui menu addsep ?barentry?");
  if (barentry==NULL) return ElvFalse;
  
  /* Search along the bar to find the correct entry */
  while ((menubarentry!=NULL) && 
        (strcmp(((struct guignome_menubarentry*)(menubarentry->data))->text,barentry)))
    menubarentry=g_slist_next(menubarentry);
    
  if (menubarentry==NULL) {
    g_free(menubarentry);
    msg(MSG_ERROR,"Menu bar entry not found");
    return ElvFalse;
  };

  foundentry=(struct guignome_menubarentry*)(menubarentry->data);

  /* Create the new entry and add it to the list */
  newentry=g_malloc(sizeof(struct guignome_menuentry));
  newentry->text=NULL;
  newentry->command=NULL;
  newentry->optname=NULL;
  newentry->optvalue=NULL;

  foundentry->entries=g_slist_append(foundentry->entries,newentry);

  return ElvTrue;
} /* guicmd_menu_addentry */

/* ---------------------------------------------------------------------- */
/* gui menubar addentry ?text?     '?' can be any character same as end   */
static ELVBOOL guicmd_menubar_addentry(char* line) {
  struct guignome_menubarentry* newentry;
  char* endofquote;
  
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  /* We use a punctuation character to quote the text - any punctuation */
  if (!elvpunct(*line)) {
    msg(MSG_ERROR,"Usage: gui menubar addentry /text/ (Missing start char)");
    return ElvFalse;
  };

  /* Now find the matching one */
  if (endofquote=strchr(line+1,*line),endofquote==NULL) {
    msg(MSG_ERROR,"Usage: gui menubar addentry /text/ (Missing end char)");
    return ElvFalse;
  };
  
  /* Allocate a new menubar entry and add it to the list */
  newentry=g_malloc(sizeof(struct guignome_menubarentry));
  newentry->text=g_strndup(line+1,(endofquote-line)-1);
  newentry->entries=NULL;

  /* If the text starts with a * it means it is a special menu entry which Elvis will manage itself */
  if (newentry->text[0]=='*') {
    if (strcmp(newentry->text,"*Buffers")==0) {
      /* Nothing special....yet */
    } else {
      msg(MSG_ERROR,"Unknown special menu bar entry");
      g_free(newentry->text);
      g_free(newentry);
    };
  }
  guignome_menubarentrylist=g_slist_append(guignome_menubarentrylist,newentry);



  return ElvTrue;
} /* guicmd_menubar_addentry */

/* ---------------------------------------------------------------------- */
/* Parse a 'menubar' command                                              */
static ELVBOOL guicmd_menubar(char* line) {
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  /* Look for commands we recognise */
  if (!strncmp(line,"addentry",8)) return guicmd_menubar_addentry(line+8);

  msg(MSG_ERROR, "Invalid GUI (gnome) menubar command");

  return ElvFalse;
} /* guicmd_menubar */

/* ---------------------------------------------------------------------- */
/* Parse a 'menu' command                                                 */
static ELVBOOL guicmd_menu(char* line) {
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  /* Look for commands we recognise */
  if (!strncmp(line,"addentry",8)) return guicmd_menu_addentry(line+8);
  if (!strncmp(line,"addsep",6)) return guicmd_menu_addsep(line+6);

  msg(MSG_ERROR, "Invalid GUI (gnome) menu command");

  return ElvFalse;
} /* guicmd_menu */

/* ---------------------------------------------------------------------- */
/* Parse a 'toolbar' 'addentry' command                                   */
/*   Format:  toolbar addentry text iconname toolbartext action           */
static ELVBOOL guicmd_toolbar_addentry(char* line) {
  struct guignome_toolbarentry tmptbe,*restbe;
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;
  
  tmptbe.text=extractString(&line,"Usage: gui toolbar addentry text iconname toolbartext action");
  if (tmptbe.text==NULL) return ElvFalse;

  tmptbe.icon=extractString(&line,"Usage: gui toolbar addentry text iconname toolbartext action");
  if (tmptbe.icon==NULL) {
    g_free(tmptbe.text);
    return ElvFalse;
  };

  tmptbe.tttext=extractString(&line,"Usage: gui toolbar addentry text iconname toolbartext action");
  if (tmptbe.tttext==NULL) {
    g_free(tmptbe.icon);
    g_free(tmptbe.text);
    return ElvFalse;
  };

  tmptbe.action=extractString(&line,"Usage: gui toolbar addentry text iconname toolbartext action");
  if (tmptbe.action==NULL) {
    g_free(tmptbe.tttext);
    g_free(tmptbe.icon);
    g_free(tmptbe.text);
    return ElvFalse;
  };
  if (restbe=g_malloc(sizeof(tmptbe)), restbe==NULL) {
    g_free(tmptbe.action);
    g_free(tmptbe.tttext);
    g_free(tmptbe.icon);
    g_free(tmptbe.text);
    return ElvFalse;
  };
  *restbe=tmptbe;

  guignome_toolbarentrylist=g_slist_append(guignome_toolbarentrylist,restbe);

  return ElvTrue;
} /* guicmd_toolbar_addentry */

/* ---------------------------------------------------------------------- */
/* Open a dialogue (i.e. perform a daction)                               */
static ELVBOOL guicmd_open(struct guignome_instance* gw, char* line) {
  GtkWidget* tmpwidget;
  GSList* current;
  struct guignome_daction* dtoopen;
  struct guignome_windaction* newdact;
  char* tofind;
  char* err="Usage: gui ioen dialoguename";

  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  if (tofind=extractString(&line,err),tofind==NULL) return ElvFalse;
  
  /* Lets see if we have already got it open */
  for(current=gw->wdacs;current!=NULL;current=g_slist_next(current)) {
    struct guignome_windaction* winda=(struct guignome_windaction*)(current->data);
    if (strcmp(tofind,winda->daction->name)==0) {
      /* Already exists - TODO: should bring to the front, deiconify etc */

      gtk_widget_show(GTK_WIDGET(winda->dialogue));
      g_free(tofind);
      return ElvTrue;
    };
  };

  /* Not found - OK we need to open it */
  if (dtoopen=findDaction(tofind),dtoopen==NULL) {
    g_free(tofind);
    msg(MSG_ERROR,"Unknown dialogue name in gui open");
    return ElvFalse;
  };

  /* Create a data area for this instance of the dialogue (used in callbacks) */
  newdact=g_malloc(sizeof(struct guignome_windaction));
  newdact->daction=dtoopen;
  newdact->instance=gw;
  if (newdact->dialogue=glade_xml_get_widget(gw->glade_xml,dtoopen->dialoguename),newdact->dialogue==NULL) {
    g_free(newdact);
    g_free(tofind);
    msg(MSG_ERROR,"Dialogue box for named daction tdoes not exist in glade file");
    return ElvFalse;
  };

  if (dtoopen->applybutton[0]) {
    if (tmpwidget=glade_xml_get_widget(gw->glade_xml, dtoopen->applybutton),tmpwidget==NULL) {
      g_free(newdact);
      g_free(tofind);
      msg(MSG_ERROR,"Unable to open dialogue because apply button did not exist");
      return ElvFalse;
    };
    gtk_signal_connect(GTK_OBJECT(tmpwidget),"clicked", (GtkSignalFunc) gui_DialogApply, newdact);
  };
  if (dtoopen->cancelbutton[0]) {
    if (tmpwidget=glade_xml_get_widget(gw->glade_xml, dtoopen->cancelbutton),tmpwidget==NULL) {
      g_free(newdact);
      g_free(tofind);
      msg(MSG_ERROR,"Unable to open dialogue because cancel button did not exist");
      return ElvFalse;
    };
    gtk_signal_connect(GTK_OBJECT(tmpwidget),"clicked", (GtkSignalFunc) gui_DialogCancel, newdact);
  };
  if (dtoopen->okbutton[0]) {
    if (tmpwidget=glade_xml_get_widget(gw->glade_xml, dtoopen->okbutton),tmpwidget==NULL) {
      g_free(newdact);
      g_free(tofind);
      msg(MSG_ERROR,"Unable to open dialogue because OK button did not exist");
      return ElvFalse;
    };
    gtk_signal_connect(GTK_OBJECT(tmpwidget),"clicked", (GtkSignalFunc) gui_DialogOK, newdact);
  };

  gw->wdacs=g_slist_append(gw->wdacs,newdact);

  gtk_signal_connect(GTK_OBJECT(newdact->dialogue),"delete_event", GTK_SIGNAL_FUNC(gui_DialogDelete), newdact);
  gtk_widget_show(GTK_WIDGET(newdact->dialogue));

  return ElvFalse;
} /* guicmd_open */

/* ---------------------------------------------------------------------- */
/* gui defaction name cmd dialoguename applybutton cancelbutton okbutton  */
static ELVBOOL guicmd_defaction(char* line) {
  struct guignome_daction tmpact,*newact;
  char* err="Usage: gui toolbar addentry text iconname toolbartext action";
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;
  
  if (tmpact.name=extractString(&line,err),tmpact.name==NULL) return ElvFalse;

  if (findDaction(tmpact.name)!=NULL) {
    msg(MSG_ERROR,"Redefinition of existing daction");
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (tmpact.cmd=extractString(&line,err),tmpact.cmd==NULL) {
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (tmpact.dialoguename=extractString(&line,err),tmpact.dialoguename==NULL) {
    g_free(tmpact.cmd);
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (tmpact.applybutton=extractString(&line,err),tmpact.applybutton==NULL) {
    g_free(tmpact.dialoguename);
    g_free(tmpact.cmd);
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (tmpact.cancelbutton=extractString(&line,err),tmpact.cancelbutton==NULL) {
    g_free(tmpact.applybutton);
    g_free(tmpact.dialoguename);
    g_free(tmpact.cmd);
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (tmpact.okbutton=extractString(&line,err),tmpact.okbutton==NULL) {
    g_free(tmpact.cancelbutton);
    g_free(tmpact.applybutton);
    g_free(tmpact.dialoguename);
    g_free(tmpact.cmd);
    g_free(tmpact.name);
    return ElvFalse;
  };

  if (newact=g_malloc(sizeof(tmpact)),newact==NULL) {
    g_free(tmpact.okbutton);
    g_free(tmpact.cancelbutton);
    g_free(tmpact.applybutton);
    g_free(tmpact.dialoguename);
    g_free(tmpact.cmd);
    g_free(tmpact.name);
    return ElvFalse;
  };

  *newact=tmpact;
  guignome_dactionlist=g_slist_append(guignome_dactionlist,newact);
} /* guicmd_defaction */

/* ---------------------------------------------------------------------- */
/* Parse a 'toolbar' command                                              */
static ELVBOOL guicmd_toolbar(char* line) {
  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  if (!strncmp(line,"addentry",8)) return guicmd_toolbar_addentry(line+8);

  msg(MSG_ERROR,"Invalid GUI (gnome) toolbar command");
  return ElvFalse;
} /* guicmd_toolbar */

/* ---------------------------------------------------------------------- */
/* Parse a 'popupentry' command - adds an entry to popup menu             */
static ELVBOOL guicmd_popupentry(char* line) {
  char *err="gui popupentry ?title? ?command?";
  char *title,*cmdstring;
  struct guignome_menuentry* newentry=g_malloc(sizeof(struct guignome_menuentry));

  /* Skip initial spaces */
  while ((elvspace(*line)) && (*line)) line++;

  title=extractString(&line,err);
  if (!title) {
    msg(MSG_ERROR,err);
    return ElvFalse;
  };

  cmdstring=extractString(&line,err);
  if (!cmdstring) {
    msg(MSG_ERROR,err);
    g_free(title);
    return ElvFalse;
  };

  newentry->text=title;
  newentry->command=cmdstring;
  newentry->optname=NULL;
  newentry->optvalue=NULL;

  g_message("Got popupentry command: %s -> %s\n",newentry->text,newentry->command);

  guignome_popupmenuentrylist=g_slist_append(guignome_popupmenuentrylist,newentry);
} /* guicmd_popupentry */
/* ---------------------------------------------------------------------- */
/* Execute 'gui' commands to add things to the toolbar and menubar        */
static ELVBOOL guicmd(GUIWIN* gw, char* extra) {
  if (!extra) return ElvTrue;

  /* Skip initial spaces */
  while ((elvspace(*extra)) && (*extra)) extra++;

  /* Now we are expecting one of a few keywords to tell us what to do */
  if (!strncmp(extra,"menubar",7)) return guicmd_menubar(extra+7);
  if (!strncmp(extra,"toolbar",7)) return guicmd_toolbar(extra+7);
  if (!strncmp(extra,"menu",4)) return guicmd_menu(extra+4);
  if (!strncmp(extra,"defaction",9)) return guicmd_defaction(extra+9);
  if (!strncmp(extra,"open",4)) return guicmd_open((struct guignome_instance*)gw,extra+4);
  if (!strncmp(extra,"popupentry",10)) return guicmd_popupentry(extra+10);

  msg(MSG_ERROR, "Invalid GUI command");

  return ElvFalse;
} /* guicmd */

/* ---------------------------------------------------------------------- */
/* Update the status bar of the window                                    */
static ELVBOOL status(GUIWIN* gw, CHAR* cmd, long line, long column,
                      _CHAR_ learn, char* mode) {
  char tmp[200]; /* Hope its enough! */

  gnome_appbar_set_default(GNOME_APPBAR(((struct guignome_instance *)gw)->statusbar_cmdpre),cmd);
  
  sprintf(tmp,"%ld/%ld\t %c\n",
     line,column,learn);
  gnome_appbar_set_default(GNOME_APPBAR(((struct guignome_instance *)gw)->statusbar_pos),tmp);
  gnome_appbar_set_default(GNOME_APPBAR(((struct guignome_instance *)gw)->statusbar_mode),mode);

  return ElvTrue;
} /* status */

/* ---------------------------------------------------------------------- */
/* Scroll the window up or down                                           */
static ELVBOOL scroll(GUIWIN* w, int qty, ELVBOOL notlast) {
  WINDOW win=winofgw(w);
  struct guignome_instance* gw=(struct guignome_instance*)w;
  int lastscrollline; /* Last line to affect */
  char tmp[50];

#ifdef DEBUG_SCROLL
  g_message("scroll: qty=%d notlast=%d height=%d cursy=%d\n",
            qty,notlast,gw->height,win->cursy);
#endif

  tmp[0]='\0';
  /* We don't do the 'notlast' case yet */
  if (notlast) {
    lastscrollline=gw->height-1;
  } else {
    lastscrollline=gw->height;
  };

  if (qty<0) {
    /* Scroll from cursor to bottom, down by -qty lines, losing lines off the
       top */
    /* Save cursor, setup scroll region (not inc bottom line possibly)
       restore cursor pos, delete line, restore scroll region */
    sprintf(tmp,"\0337\033[1;%dr\0338\033[%dM\033[1;%dr",lastscrollline,
            -qty,gw->height);
  } else {
    /* Scroll lines from cursor to bottom down losing lines off the bottom */
    /* save cursor, setup scroll region, restore cursor pos, insert lines,
       restore scroll region */
    sprintf(tmp,"\0337\033[1;%dr\0338\033[%dL\033[1;%dr",lastscrollline,
             qty,gw->height);
  }

  writeBuffered((struct guignome_instance *)gw,tmp,strlen(tmp));

  /* We shouldn't need this should we? This should be done by the flush routine ? */
  flushWindow((struct guignome_instance *)gw);

  return ElvTrue;
} /* scroll */

/* ---------------------------------------------------------------------- */
/* This is a set of pointers which the main elvis program calls when it wants
the GUI to do something */
GUI guignome =
{
   "gnome",      /* name */
   "Pretty GNOME user interface",
   ElvFalse,        /* exonly */
   ElvFalse,        /* newblank */
   ElvTrue,         /* minimizeclr */
   ElvFalse,        /* scrolllast */
   ElvTrue,         /* shiftrows */
   0,            /* movecost */
   0,            /* noopts */
   NULL,         /* optdescs */
   test,
   init,
   usage,
   loop,
   NULL,         /* wpoll */
   term,
   creategw,
   destroygw,
   focusgw,
   retitle,
   NULL,         /* reset */
   flush,         /* flush */
   moveto,
   draw,
   NULL,         /* shift */
   scroll,
   clrtoeol,     /* clrtoeol */
   NULL, /* newline */
   NULL,         /* beep, */
   NULL, /* msg */
   scrollbar,
   status,         /* status */
   NULL,         /* keylabel */
   guignome_clipopen,
   guignome_clipwrite,
   guignome_clipread,
   guignome_clipclose,
   color,
   freecolor,         /* freecolor */
   NULL,         /* setbg */
   guicmd,
   NULL,         /* tabcmd */
   NULL,         /* save */
   NULL,         /* wildcard */
   NULL,         /* prgopen */
   NULL,         /* prgclose */
   NULL,         /* stop - ctrl-Z */
};

