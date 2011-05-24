/**
 *  
 *  Filename: activesyncd-server.c
 *  Project: activesyncd
 *  Description: Daemon setup, DBus setup, GObjects construction and registration.
 *
 */

//system include
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <libsoup/soup.h>


//user include
#include "activesyncd-common-defs.h"
#include "../libeas/eas-connection.h"
#include "../libeas/eas-accounts.h"
#include "eas-calendar.h" 
#include "eas-common.h"
#include "eas-contact.h" 
#include "eas-mail.h"

#ifdef DISABLE_EAS_DAEMON
#define dbg(fmtstr, args...) (g_print(":%s: " fmtstr "\n", __func__, ##args))
#else
#define dbg(dummy...)
#endif


/*
  activesyncd entry point
*/
int main(int argc, char** argv) {
    DBusGConnection* bus = NULL;
    DBusGProxy* busProxy = NULL;
    EasConnection* EasConnObj = NULL;
    EasCalendar* EasCalendarObj = NULL;
    EasCommon* EasCommonObj = NULL;
    EasContact* EasContactObj = NULL;
    EasMail*EasMailObj = NULL;
    
    GMainLoop* mainloop = NULL;
    guint result;
    GError* error = NULL;


    g_type_init();

    mainloop = g_main_loop_new(NULL, FALSE);
    if (mainloop == NULL) {
        g_print("Error: Couldn't create GMainLoop\n");
        exit(EXIT_FAILURE);
    }

    //Creating all the GObjects
    g_print("Creating EEasConnection GObject.\n");

   GError* cnc_error = NULL;
   EasAccounts* account = eas_accounts_new ();
   g_print("eas_accounts_read_accounts_info\n");   
    int err = eas_accounts_read_accounts_info(account);
    if (err !=0)
    {
        g_print("Error reading data from file accounts.cfg\n");
        g_main_loop_quit (mainloop);
        exit(err);    
    }

   g_print("getting data from EasAccounts object\n");       
    guint64 accountId;
   accountId =1234567890;
   gchar* username = eas_accounts_get_user_id (account, accountId);
   gchar* password = eas_accounts_get_password (account, accountId);
   gchar* serverUri = eas_accounts_get_server_uri (account, accountId);

    g_print("Creating EEasConnection GObject.\n");
    EasConnObj = eas_connection_new(serverUri, username, password, &cnc_error);
    if (EasConnObj == NULL) {
        g_print("Error: Failed to create EEasConnection instance\n");
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Creating calendar  gobject.\n");
    EasCalendarObj = g_object_new(EAS_TYPE_CALENDAR , NULL);
    if (EasCalendarObj == NULL) {
        g_print("Error: Failed to create calendar  instance\n");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Creating common  gobject.\n");
    EasCommonObj = g_object_new(EAS_TYPE_COMMON , NULL);
    if (EasCommonObj == NULL) {
        g_print("Error: Failed to create common  instance\n");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Creating contact  gobject.\n");
    EasContactObj = g_object_new(EAS_TYPE_CONTACT , NULL);
    if (EasContactObj == NULL) {
        g_print("Error: Failed to create common  instance\n");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Creating mail  gobject.\n");
    EasMailObj = eas_mail_new ();
    if (EasMailObj == NULL) {
        g_print("Error: Failed to create common  instance\n");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Pass a EasConnection handle to the exposed GObjects\n");
    //ret = eas_calendar_set_eas_connection(EasCalendarObj, EasConnObj);
    //ret = eas_common_set_eas_connection(EasCommonObj, EasConnObj);
    //ret = eas_contact_set_eas_connection(EasContactObj, EasConnObj);
   eas_mail_set_eas_connection(EasMailObj, EasConnObj);
   
    g_print("Connecting to the session DBus\n");
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (error != NULL) {
        g_print("Error: Connecting to the session DBus (%s)\n", error->message);
        g_clear_error (&error);
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
    }

    g_print("Registering the well-known name (%s)\n", EAS_SERVICE_NAME);
    busProxy = dbus_g_proxy_new_for_name(bus,
                                       DBUS_SERVICE_DBUS,
                                       DBUS_PATH_DBUS,
                                       DBUS_INTERFACE_DBUS);
    if (busProxy == NULL) {
        g_print("Error: Failed to get a proxy for D-Bus\n");
        g_main_loop_quit (mainloop);
        exit(EXIT_FAILURE);
  }

  /* register the well-known name.*/
    g_print("D-Bus RequestName RPC \n");
    if (!dbus_g_proxy_call(busProxy,
                         "RequestName",
                         &error,
                         G_TYPE_STRING,
                         EAS_SERVICE_NAME,
                         G_TYPE_UINT,
                         0,
                         G_TYPE_INVALID,
                         G_TYPE_UINT,
                         &result,
                         G_TYPE_INVALID)) {
   g_print("Error: D-Bus RequestName RPC failed (%s)\n", error->message);
   g_main_loop_quit (mainloop);
   exit(EXIT_FAILURE);
  }

    g_print("RequestName returned %d\n", result);
    if (result != 1) {
        g_print("Error: Failed to get the primary well-known name\n");
       exit(EXIT_FAILURE);
  }


/*
 //we don't want EasConnObj to be exposed on DBus Interface anymore
    g_print("Registering Gobjects on the D-Bus.\n");
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_OBJECT_PATH,
                                      G_OBJECT(EasConnObj));

*/

    //	Registering  calendar Gobject 
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_CALENDAR_OBJECT_PATH,
                                      G_OBJECT(EasCalendarObj));

    //	Registering  common Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_COMMON_OBJECT_PATH,
                                      G_OBJECT(EasCommonObj));

    //	Registering  contact Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_CONTACT_OBJECT_PATH,
                                      G_OBJECT(EasContactObj));

    //	Registering  mail Gobject
    dbus_g_connection_register_g_object(bus,
                                      EAS_SERVICE_MAIL_OBJECT_PATH,
                                      G_OBJECT(EasMailObj));


    g_print("Ready to serve requests\n");

#ifndef DISABLE_EAS_DAEMON
    if (daemon(0, 0) != 0) {
        g_print("Failed to daemonize\n");
  }
#else
    g_print("Not daemonizing (built with DISABLE_EAS_DAEMON)\n");
#endif

    g_main_loop_run(mainloop);

   // Clean up
   g_main_loop_unref (mainloop);

    return 0;
}

