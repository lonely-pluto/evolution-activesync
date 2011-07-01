/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
#include "eas-sync-msg.h"
#include "eas-add-calendar-req.h"
#include "serialise_utils.h"

G_DEFINE_TYPE (EasAddCalendarReq, eas_add_calendar_req, EAS_TYPE_REQUEST_BASE);

#define EAS_ADD_CALENDAR_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ADD_CALENDAR_REQ, EasAddCalendarReqPrivate))

struct _EasAddCalendarReqPrivate
{
    EasSyncMsg* sync_msg;
    gchar* account_id;
    gchar* sync_key;
    gchar* folder_id;
    GSList *serialised_calendar;
};

static void
eas_add_calendar_req_init (EasAddCalendarReq *object)
{
    /* initialization code */
    EasAddCalendarReqPrivate *priv;
    g_debug ("eas_add_calendar_req_init++");

    object->priv = priv = EAS_ADD_CALENDAR_REQ_PRIVATE (object);

    priv->sync_msg = NULL;
    priv->account_id = NULL;
    priv->sync_key = NULL;
    priv->folder_id = NULL;
    priv->serialised_calendar = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_ADD_CALENDAR);

    g_debug ("eas_add_calendar_req_init++");
}

static void
eas_add_calendar_req_dispose (GObject *object)
{
    EasAddCalendarReq *req = (EasAddCalendarReq *) object;
    EasAddCalendarReqPrivate *priv = req->priv;

    g_debug ("eas_add_calendar_req_dispose++");

	if (priv->sync_msg) {
		g_object_unref (priv->sync_msg);
		priv->sync_msg = NULL;
	}

    G_OBJECT_CLASS (eas_add_calendar_req_parent_class)->dispose (object);

    g_debug ("eas_add_calendar_req_dispose--");
}

static void
eas_add_calendar_req_finalize (GObject *object)
{
    /* deinitalization code */
    EasAddCalendarReq *req = (EasAddCalendarReq *) object;
    EasAddCalendarReqPrivate *priv = req->priv;

    g_debug ("eas_add_calendar_req_finalize++");

    g_free (priv->account_id);

    G_OBJECT_CLASS (eas_add_calendar_req_parent_class)->finalize (object);

    g_debug ("eas_add_calendar_req_finalize--");
}

static void
eas_add_calendar_req_class_init (EasAddCalendarReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (EasAddCalendarReqPrivate));

    object_class->finalize = eas_add_calendar_req_finalize;
    object_class->dispose = eas_add_calendar_req_dispose;

    g_debug ("eas_add_calendar_req_class_init--");
}


// TODO - update this to take a GSList of serialised calendars? rem to copy the list
EasAddCalendarReq *
eas_add_calendar_req_new (const gchar* account_id, 
                          const gchar *sync_key, 
                          const gchar *folder_id, 
                          const GSList* serialised_calendar, 
                          DBusGMethodInvocation *context)
{
    EasAddCalendarReq* self = g_object_new (EAS_TYPE_ADD_CALENDAR_REQ, NULL);
    EasAddCalendarReqPrivate *priv = self->priv;

    g_debug ("eas_add_calendar_req_new++");

    priv->sync_key = g_strdup (sync_key);
    priv->folder_id = g_strdup (folder_id);
    priv->serialised_calendar = (GSList *) serialised_calendar;
    priv->account_id = g_strdup (account_id);

    eas_request_base_SetContext (&self->parent_instance, context);

    g_debug ("eas_add_calendar_req_new--");
    return self;
}

gboolean 
eas_add_calendar_req_Activate (EasAddCalendarReq *self, GError **error)
{
    EasAddCalendarReqPrivate *priv = self->priv;
    xmlDoc *doc = NULL;
    gboolean success = FALSE;

    g_debug ("eas_add_calendar_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    //create sync msg object
    priv->sync_msg = eas_sync_msg_new (priv->sync_key, priv->account_id, priv->folder_id, EAS_ITEM_CALENDAR);

    //build request msg
    doc = eas_sync_msg_build_message (priv->sync_msg, FALSE, priv->serialised_calendar, NULL, NULL);

    success = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                           "Sync",
                                           doc, // full transfer
                                           (struct _EasRequestBase *) self,
                                           error);
	if (!success)
	{
		g_assert(error == NULL || (*error != NULL));
	}

    g_debug ("eas_add_calendar_req_Activate--");
	return success;
}


gboolean
eas_add_calendar_req_MessageComplete (EasAddCalendarReq *self, 
                                      xmlDoc* doc, 
                                      GError* error)
{
    GError *local_error = NULL;
    EasAddCalendarReqPrivate *priv = self->priv;
	GSList* added_items = NULL;
	gchar* ret_sync_key = NULL;
	gchar** ret_added_items_array = NULL;

    g_debug ("eas_add_calendar_req_MessageComplete++");
	
	// If we have an error send it back to client
	if (error)
	{
		local_error = error;
		goto finish;
	}

	if (FALSE == eas_sync_msg_parse_response (priv->sync_msg, doc, &local_error))
	{
		goto finish;
	}

	ret_sync_key = g_strdup (eas_sync_msg_get_syncKey (priv->sync_msg));
    added_items = eas_sync_msg_get_added_items (priv->sync_msg);


	build_serialised_calendar_info_array (&ret_added_items_array, added_items, &error);



finish:

	if(local_error)
	{
		dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), local_error);
        g_error_free (local_error);
	}
	else
	{
		dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key,
                              ret_added_items_array);
	}
	// We always need to free 'doc' and release the semaphore.
    xmlFreeDoc (doc);

    g_debug ("eas_add_calendar_req_MessageComplete--");
	return TRUE;
}

