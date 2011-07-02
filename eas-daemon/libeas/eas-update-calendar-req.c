/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "eas-sync-msg.h"
#include "eas-update-calendar-req.h"


G_DEFINE_TYPE (EasUpdateCalendarReq, eas_update_calendar_req, EAS_TYPE_REQUEST_BASE);

#define EAS_UPDATE_CALENDAR_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_UPDATE_CALENDAR_REQ, EasUpdateCalendarReqPrivate))

struct _EasUpdateCalendarReqPrivate
{
    EasSyncMsg* sync_msg;
    gchar* account_id;
    gchar* sync_key;
    EasItemType item_type;
    gchar* folder_id;
    GSList *serialised_calendar;
};

static void
eas_update_calendar_req_init (EasUpdateCalendarReq *object)
{
    /* initialization code */
    EasUpdateCalendarReqPrivate *priv;
    g_debug ("eas_update_calendar_req_init++");

    object->priv = priv = EAS_UPDATE_CALENDAR_REQ_PRIVATE (object);

    priv->sync_msg = NULL;
    priv->account_id = NULL;
    priv->sync_key = NULL;
    priv->item_type = EAS_ITEM_LAST;
    priv->folder_id = NULL;
    priv->serialised_calendar = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_UPDATE_CALENDAR);

    g_debug ("eas_update_calendar_req_init++");
}

static void
eas_update_calendar_req_dispose (GObject *object)
{
    EasUpdateCalendarReq *req = (EasUpdateCalendarReq *) object;
    EasUpdateCalendarReqPrivate *priv = req->priv;

    g_debug ("eas_update_calendar_req_dispose++");

	if (priv->sync_msg)
	{
		g_object_unref (priv->sync_msg);
		priv->sync_msg = NULL;
	}

    G_OBJECT_CLASS (eas_update_calendar_req_parent_class)->dispose (object);

    g_debug ("eas_update_calendar_req_dispose--");
}

static void
eas_update_calendar_req_finalize (GObject *object)
{
    EasUpdateCalendarReq *req = (EasUpdateCalendarReq *) object;
    EasUpdateCalendarReqPrivate *priv = req->priv;

    g_debug ("eas_update_calendar_req_finalize++");

    g_free (priv->account_id);

    G_OBJECT_CLASS (eas_update_calendar_req_parent_class)->finalize (object);

    g_debug ("eas_update_calendar_req_finalize--");
}

static void
eas_update_calendar_req_class_init (EasUpdateCalendarReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (EasUpdateCalendarReqPrivate));

    object_class->finalize = eas_update_calendar_req_finalize;
    object_class->dispose = eas_update_calendar_req_dispose;

    g_debug ("eas_update_calendar_req_class_init--");
}


// TODO - update this to take a GSList of serialised calendars? rem to copy the list
EasUpdateCalendarReq *eas_update_calendar_req_new (const gchar* account_id, 
                                                   const gchar *sync_key, 
                                                   const EasItemType item_type, 
                                                   const gchar *folder_id, 
                                                   const GSList* serialised_calendar, 
                                                   DBusGMethodInvocation *context)
{
    EasUpdateCalendarReq* self = g_object_new (EAS_TYPE_UPDATE_CALENDAR_REQ, NULL);
    EasUpdateCalendarReqPrivate *priv = self->priv;

    g_debug ("eas_update_calendar_req_new++");

	g_assert (sync_key);
    g_assert (folder_id);
    g_assert (serialised_calendar);
	
    priv->sync_key = g_strdup (sync_key);
    priv->folder_id = g_strdup (folder_id);
    priv->serialised_calendar = (GSList *) serialised_calendar;
    priv->account_id = g_strdup (account_id);
    priv->item_type = item_type;

    eas_request_base_SetContext (&self->parent_instance, context);

    g_debug ("eas_update_calendar_req_new--");
    return self;
}

gboolean 
eas_update_calendar_req_Activate (EasUpdateCalendarReq *self, GError **error)
{
    EasUpdateCalendarReqPrivate *priv = self->priv;
    xmlDoc *doc = NULL;
	gboolean success = FALSE;

    g_debug ("eas_update_calendar_req_Activate++");
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_debug ("eas_update_calendar_req_Activate1");
    //create sync msg object
    priv->sync_msg = eas_sync_msg_new (priv->sync_key, priv->account_id, priv->folder_id, priv->item_type);

	g_debug ("eas_update_calendar_req_Activate2");
    //build request msg
    doc = eas_sync_msg_build_message (priv->sync_msg, FALSE, NULL, priv->serialised_calendar, NULL);

	g_debug ("eas_update_calendar_req_Activate3");
    success = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                           "Sync",
                                           doc,
                                           (struct _EasRequestBase *) self,
                                           error);

	g_debug ("eas_update_calendar_req_Activate4");
	if(!success)
	{
		g_assert(error == NULL || (!success && *error != NULL));
	}

    g_debug ("eas_update_calendar_req_Activate--");
	return success;
}


gboolean eas_update_calendar_req_MessageComplete (EasUpdateCalendarReq *self, 
                                              xmlDoc* doc, 
                                              GError* error)
{
	GError *local_error = NULL;
    EasUpdateCalendarReqPrivate *priv = self->priv;
	gchar *ret_sync_key = NULL;
    g_debug ("eas_update_calendar_req_MessageComplete++");

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

finish:
	if (local_error)
    {
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), local_error);
        g_error_free (local_error);
    }
    else
    {
        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key);
    }
	// We must always free doc and release the semaphore
    xmlFreeDoc (doc);
    g_free(ret_sync_key);

    g_debug ("eas_update_calendar_req_MessageComplete--");
	return TRUE;
}


