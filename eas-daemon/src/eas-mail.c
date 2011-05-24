/*
 * Filename: eas-mail.c
 */
 
#include <libedataserver/e-flag.h>
#include "eas-mail.h"
#include "eas-mail-stub.h"
#include "eas-folder.h"
#include "eas-sync-folder-hierarchy.h"


G_DEFINE_TYPE (EasMail, eas_mail, G_TYPE_OBJECT);

struct _EasMailPrivate
{
    EasConnection* connection;
};

#define EAS_MAIL_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MAIL, EasMailPrivate))


static void
eas_mail_init (EasMail *object)
{
 	g_debug("++ eas_mail_init()");
    EasMailPrivate *priv =NULL;
	object->_priv = priv = EAS_MAIL_PRIVATE(object);                    
	priv->connection = NULL;
	 	g_debug("-- eas_mail_init()");
}

static void
eas_mail_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_mail_parent_class)->finalize (object);
}

static void
eas_mail_class_init (EasMailClass *klass)
{
 	g_debug("++ eas_mail_class_init ()");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;
	
	object_class->finalize = eas_mail_finalize;
		g_debug(">>eas_mail_class_init 01");	
	g_type_class_add_private (klass, sizeof (EasMailPrivate));
			g_debug(">>eas_mail_class_init 02");	
	 /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_MAIL,
                                            &dbus_glib_eas_mail_object_info);
    g_debug("-- eas_mail_class_init ()");
}

EasMail* eas_mail_new(void)
{
	EasMail* easMail = NULL;
	easMail = g_object_new(EAS_TYPE_MAIL, NULL);
	return easMail;
}

void eas_mail_set_eas_connection(EasMail* self, EasConnection* easConnObj)
{
   EasMailPrivate* priv = self->_priv;
   priv->connection = easConnObj;
}

EasConnection*  eas_mail_get_eas_connection(EasMail* self)
{
    EasMailPrivate* priv = self->_priv;
    return priv->connection;
}

gboolean eas_mail_start_sync(EasMail* easMailObj, gint valueIn, GError** error)
{
/*
guint64 account_uid = 12345;
const gchar* sync_key =NULL; 
gchar **ret_sync_key =NULL;
gchar **ret_created_folders_array =NULL;
gchar **ret_updated_folders_array =NULL;
gchar **ret_deleted_folders_array =NULL;


eas_connection_folder_sync(easMailObj->easConnection, 
                                            account_uid,
						sync_key, 
						ret_sync_key,  
						ret_created_folders_array,
						ret_updated_folders_array,
						ret_deleted_folders_array,
						error);
*/
  return TRUE;
}

void eas_mail_test_001(EasMail* obj, DBusGMethodInvocation* context)
{

 	g_debug(">> eas_mail_test_001()");
        GError *error = NULL;
        gchar *ok_str = g_strdup ("OK");
         // ...

        if (error) {
		g_debug(">> eas_mail_test_001 -error-");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		g_debug(">> eas_mail_test_001 -No error-");
                dbus_g_method_return (context, ok_str);
	}

        g_free (ok_str);
}


// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean 
build_serialised_folder_array(gchar ***serialised_folder_array, const GSList *folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

    g_assert(serialised_folder_array);
    g_assert(*serialised_folder_array == NULL);

	guint array_len = g_slist_length((GSList*)folder_list) + 1;	//cast away const to avoid warning. +1 to allow terminating null 
    
	*serialised_folder_array = g_malloc0(array_len * sizeof(gchar*));

	GSList *l = (GSList*)folder_list;
	for(i = 0; i < array_len - 1; i++)
	{
		g_assert(l != NULL);
		EasFolder *folder;
		folder = l->data;

		if(!eas_folder_serialise(folder, &(*serialised_folder_array)[i]))
		{
			g_debug("failed!");
			ret = FALSE;
			goto cleanup;
		}

		l = g_slist_next (l);
	}
	
cleanup:
	if(!ret)
	{
		for(i = 0; i < array_len - 1; i++)
		{
			g_free((*serialised_folder_array)[i]);
		}
		g_free(*serialised_folder_array);
		// TODO set error
	}

	return ret;
}

void eas_mail_sync_email_folder_hierarchy(EasMail* easMailObj,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context)
{
        GError *error = NULL;
        GSList* added_folders = NULL;
        GSList* updated_folders  = NULL;
        GSList* deleted_folders  = NULL;

        gchar* ret_sync_key = NULL;
        gchar** ret_created_folders_array = NULL;
        gchar** ret_updated_folders_array = NULL;
        gchar** ret_deleted_folders_array = NULL;

        EFlag * eflag =NULL;
        
        eflag = e_flag_new ();
	  
        // Create the request
        EasSyncFolderHierarchy *folderHierarchyObj =NULL;

        g_debug("eas_mail_sync_email_folder_hierarchy++");
        folderHierarchyObj = g_object_new(EAS_TYPE_SYNC_FOLDER_HIERARCHY , NULL);

        eas_request_base_SetConnection (&folderHierarchyObj->parent_instance, 
                                        eas_mail_get_eas_connection(easMailObj));
                                        

        g_debug("eas_mail_sync_email_folder_hierarchy - new req");
	    // Start the request
        eas_sync_folder_hierarchy_Activate (folderHierarchyObj, 
                                            sync_key,
                                            account_uid,
                                            eflag);
        g_debug("eas_mail_sync_email_folder_hierarchy - activate req");
	    // Set flag to wait for response
	    e_flag_wait(eflag);

        g_debug("eas_mail_sync_email_folder_hierarchy - get results");
         eas_sync_folder_hierarchy_Activate_Finish (folderHierarchyObj,
                                                    &ret_sync_key,
                                                    &added_folders,
                                                    &updated_folders,
                                                    &deleted_folders);
         e_flag_free (eflag);
         g_debug("eas_mail_sync_email_folder_hierarchy - serialise objects");
         //serialise the folder objects from GSList* to char** and populate  :

        g_debug("  Data: %s, %x, %x, %x", 
                ret_sync_key, (guint)added_folders, (guint)updated_folders, (guint)deleted_folders);

		if(build_serialised_folder_array(&ret_created_folders_array, added_folders, &error))
		{
			if(build_serialised_folder_array(&ret_updated_folders_array, updated_folders, &error))
			{
				build_serialised_folder_array(&ret_deleted_folders_array, deleted_folders, &error);
			}
		}
         
         // Return the error or the requested data to the mail client
        if (error) {
		        g_debug(">> Daemon : Error ");
                dbus_g_method_return_error (context, error);
                g_error_free (error);
        } else{
		        g_debug(">> Daemon : Success-");
                dbus_g_method_return (context,
                                 	ret_sync_key,
                                  	ret_created_folders_array,
						            ret_updated_folders_array,
						            ret_deleted_folders_array);
        }

    g_debug("eas_mail_sync_email_folder_hierarchy--");

}

/**
 * @param[in,out] easMailObj                the instance of the GObject
 * @param[in]     account_uid               the exchange server account UID
 * @param[in]     sync_key                  the current sync_key
 * @param[in]     get_server_changes        TRUE to request server changes in the response, otherwise FALSE
 * @param[in]     collection_id             identifer for the folder to be sync'ed
 * @param[in]     deleted_email_array       list of email ids to be deleted
 * @param[in]     changed_email_array       list of changes to existing emails
 * @param[out]    ret_sync_key              updated sync_key from the server response
 * @param[out]    ret_more_available        TRUE if further sync data is available on the server
 * @param[out]    ret_added_email_array     serialised list of added emails
 * @param[out]    ret_deleted_email_array   serialised list of deleted email ids
 * @param[out]    ret_changed_email_array   serialsed list of changes to existing emails
 * @param[out]    error                     NULL if no error, otherwise a GLib GError
 */
gboolean eas_mail_sync_folder_email(EasMail* easMailObj, 
                                    guint64 account_uid,
									const gchar* sync_key,   
                                    gboolean get_server_changes,
									const gchar *collection_id,	//folder to sync
									const gchar* deleted_email_array,
									const gchar* changed_email_array,                                    
									gchar *ret_sync_key,                                    
									gboolean *ret_more_available,
									gchar **ret_added_email_array,
									gchar **ret_deleted_email_array,
									gchar **ret_changed_email_array,	
									GError** error)
{
	// TODO
  return TRUE;											
}

gboolean eas_mail_delete_email(EasMail* easMailObj,
                                    const gchar* sync_key, 
                                    const gchar *server_id,
                                    GError **error)
{
    g_debug("eas_mail_delete_email++");
    g_debug("eas_mail_delete_email--");
	return TRUE;
}

gboolean
eas_mail_fetch (EasMail* easMailObj, 
                guint64 account_uid, 
                const gchar *server_id, 
                const gchar *collection_id, 
                const gchar *file_reference, 
                const gchar *mime_directory, 
                GError **error)
{
	// TODO
	return TRUE;
}

// 
gboolean eas_mail_send_email(EasMail* easMailObj, 
								guint64 account_uid,                             
								const gchar* clientid,
								const gchar *mime_file,
								GError** error)
{
	g_debug("eas_mail_send_email++");
	
	// TODO

	g_debug("eas_mail_send_email--");
	return TRUE;								
}


