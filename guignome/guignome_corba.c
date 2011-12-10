/* guignome/guignome_corba.c */

/* Copyright 2000 by Dr. David Alan Gilbert (elvis@treblig.org)      */
/*   based on guix11.c, with help from the gnome-hello code          */
/*   the Glade code, guys on the gnome-devel list, gmc,              */
/*   and the numbers 42 and 56. Chunks of this CORBA code are from   */
/*   gmc                                                             */
/* It may be distributed and used under the same license terms       */
/* as the rest of the Elvis package which it is part of.             */

#include <libgnorba/gnorba.h>
#include "elvisCorba.h"

/* The ORB for the whole program */
CORBA_ORB orb = CORBA_OBJECT_NIL;
      
/* The POA */
PortableServer_POA poa = CORBA_OBJECT_NIL;

int guignome_corba_have_server = FALSE;

/* -------------------------------------------------------------------------*/
static void guignome_Elvis_ElvisPrivate_openFile (PortableServer_Servant _servant,
                                         const CORBA_char * dir,
                                         const CORBA_char * toopen,
                                         CORBA_Environment * ev) {
  g_message("Elvis_ElvisPrivate_openFile: %s %s\n",dir,toopen);

  guignome_doopenfile(toopen,dir);
} /* Elvis_ElvisPrivate_openFile */

/* -------------------------------------------------------------------------*/

/* Our servant - this will contain pointers to the routines to be called via
/* CORBA - these are setup by Elvis_ElvisPrivate_init */
typedef struct {
  POA_Elvis_ElvisPrivate servant;
} ElvisPrivateServant;

static PortableServer_ServantBase__epv elvis_elvisprivate_base_epv;
static POA_Elvis_ElvisPrivate__epv elvis_elvisprivate_epv;
static POA_Elvis_ElvisPrivate__vepv elvis_elvisprivate_vepv;

/* -------------------------------------------------------------------------*/
/* This initialises the class 'Elvis_ElvisPrivate' - this needs to have     */
/* pointers to the routines that will get called via CORBA                  */
static void Elvis_ElvisPrivate_init (void) {
  static int inited = FALSE;

  if (inited) return;
  inited = TRUE;

  /* Fill in the routines that can be called */
  elvis_elvisprivate_epv.openFile=guignome_Elvis_ElvisPrivate_openFile;

  /* Some sort of virtual function setup? */
  elvis_elvisprivate_vepv._base_epv = &elvis_elvisprivate_base_epv;
  elvis_elvisprivate_vepv.Elvis_ElvisPrivate_epv = &elvis_elvisprivate_epv;
} /* Elvis_ElvisPrivate_init */

/* -------------------------------------------------------------------------*/
/* Reference to the ElvisPrivate factory */
static CORBA_Object ElvisPrivate_server = CORBA_OBJECT_NIL;

/* Create a reference for our private server object */
static Elvis_ElvisPrivate 
ElvisPrivateServer_create (PortableServer_POA poa, CORBA_Environment *ev)
{
  ElvisPrivateServant *eps;
  PortableServer_ObjectId *objid;

  /* Intialise the function pointers in the class */
  Elvis_ElvisPrivate_init ();

  /* Create a servant instance and set its virtual method table up? */
  eps=g_new0(ElvisPrivateServant, 1);
  eps->servant.vepv = &elvis_elvisprivate_vepv;

  POA_Elvis_ElvisPrivate__init ((PortableServer_Servant) eps, ev);
  objid = PortableServer_POA_activate_object (poa, eps, ev);
  CORBA_free (objid);

  return PortableServer_POA_servant_to_reference (poa, eps, ev);
}

/* Creates and registers the CORBA servers.  Returns TRUE on success, FALSE
 * otherwise.
 */
static int
register_servers (void)
{
  CORBA_Environment ev;
  int retval;
  int v;
  
  g_message("In register_servers (1)\n");

  retval = FALSE;
  CORBA_exception_init (&ev);

  /* Register our private server and see if it was already there */
  ElvisPrivate_server = ElvisPrivateServer_create (poa, &ev);
  if (ev._major != CORBA_NO_EXCEPTION)
    goto out;

  g_message("In register_servers (2)\n");
  v = goad_server_register (CORBA_OBJECT_NIL, ElvisPrivate_server,
          "IDL:Elvis:ElvisPrivate:1.0", "object", &ev);
  g_message("In register_servers (3) v=%d\n",v);
  if (ev._major != CORBA_NO_EXCEPTION)
    goto out;
  g_message("In register_servers (3b) v=%d\n",v);

  switch (v) {
  case 0:
    guignome_corba_have_server = FALSE;
    break;
  
  case -2:
    guignome_corba_have_server = FALSE;
    break;
  }

  retval = TRUE;

  /* Done */
 out:
  { CORBA_char *str=CORBA_exception_id(&ev);
  g_message("In register_servers (4) %s\n", str==NULL? "<NULL>" : str);
  CORBA_exception_free (&ev);
  }
  
  return retval;
}

/**
 * corba_init_server:
 * @void:
 * 
 * Initializes the CORBA server for GMC.  Returns whether initialization was
 * successful or not, and sets the global guignome_corba_have_server variable.
 *
 * Return value: TRUE if successful, FALSE otherwise.
 **/
int
guignome_corba_init_server (void)
{
  int retval;
  CORBA_Environment ev;

  retval = FALSE;
  CORBA_exception_init (&ev);
   
  /* Get the POA and create the server */

  g_message("guignome_corba_init_server (1)\n");

  poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
  if (ev._major != CORBA_NO_EXCEPTION)
    goto out;
    
  CORBA_exception_free (&ev);
  g_message("guignome_corba_init_server (2)\n");

  /* See if the servers are there */
  ElvisPrivate_server = goad_server_activate_with_id (
    NULL,
    "IDL:Elvis:ElvisPrivate:1.0",
    GOAD_ACTIVATE_EXISTING_ONLY,
    NULL); 

  if (ElvisPrivate_server != CORBA_OBJECT_NIL) {
    guignome_corba_have_server = TRUE;
    retval = TRUE;
    g_message("guignome_corba_init_server (2b)\n");
  } else
    retval = register_servers ();

  g_message("guignome_corba_init_server (3)\n");
 out:
  return retval;
}

/**
 * guignome_corba_activate_server:
 * @void:
 *
 * Activates the POA manager and thus makes the services available to the
 * outside world.
 **/
void
guignome_corba_activate_server (void)
{
  CORBA_Environment ev;
  PortableServer_POAManager poa_manager;
  
  g_message("In guignome_corba_activate_server (1)\n");

  /* Do nothing if the server is already running */
  if (guignome_corba_have_server)
    return;
   
  g_message("In guignome_corba_activate_server (2)\n");
  CORBA_exception_init (&ev);
    
  poa_manager = PortableServer_POA__get_the_POAManager (poa, &ev);
  if (ev._major != CORBA_NO_EXCEPTION)
    goto out;

  g_message("In guignome_corba_activate_server (3)\n");
  PortableServer_POAManager_activate (poa_manager, &ev);
  if (ev._major != CORBA_NO_EXCEPTION)
    goto out;

  g_message("In guignome_corba_activate_server (4)\n");
 out:
  { CORBA_char *str=CORBA_exception_id(&ev);;
  g_message("In guignome_corba_activate_server (4a) exception=%d - %s\n",
    ev._major,str==NULL? "<NULL>" : str);
  CORBA_exception_free (&ev);
  g_message("In guignome_corba_activate_server (5)\n");
  }
}
