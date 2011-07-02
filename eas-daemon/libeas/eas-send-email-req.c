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

#include <stdio.h>

#include "eas-send-email-req.h"
#include "eas-send-email-msg.h"



G_DEFINE_TYPE (EasSendEmailReq, eas_send_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SEND_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqPrivate))

struct _EasSendEmailReqPrivate
{
    EasSendEmailMsg *send_email_msg;
    gchar* account_id;
    gchar* client_id;
    gchar* mime_file;
};

static void
eas_send_email_req_init (EasSendEmailReq *object)
{
    EasSendEmailReqPrivate *priv;
    g_debug ("eas_send_email_req_init++");

    object->priv = priv = EAS_SEND_EMAIL_REQ_PRIVATE (object);

    priv->account_id = NULL;
    priv->send_email_msg = NULL;
    priv->mime_file = NULL;
    priv->client_id = NULL;

    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_SEND_EMAIL);

    g_debug ("eas_send_email_req_init--");
}

static void 
eas_send_email_req_dispose (GObject *object)
{
    EasSendEmailReq *req = (EasSendEmailReq *) object;
    EasSendEmailReqPrivate *priv = req->priv;

    g_debug ("eas_send_email_req_dispose++");
	if (priv->send_email_msg)
	{
		g_object_unref (priv->send_email_msg);
		priv->send_email_msg = NULL;
	}

    G_OBJECT_CLASS (eas_send_email_req_parent_class)->dispose (object);
    g_debug ("eas_send_email_req_dispose--");
}

static void
eas_send_email_req_finalize (GObject *object)
{
    EasSendEmailReq *req = (EasSendEmailReq *) object;
    EasSendEmailReqPrivate *priv = req->priv;

    g_debug ("eas_send_email_req_finalize++");
    g_free (priv->mime_file);
    g_free (priv->client_id);
    g_free (priv->account_id);

    G_OBJECT_CLASS (eas_send_email_req_parent_class)->finalize (object);
    g_debug ("eas_send_email_req_finalize--");
}

static void
eas_send_email_req_class_init (EasSendEmailReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_debug ("eas_send_email_req_class_init++");

    g_type_class_add_private (klass, sizeof (EasSendEmailReqPrivate));

    object_class->finalize = eas_send_email_req_finalize;
    object_class->dispose = eas_send_email_req_dispose;

    g_debug ("eas_send_email_req_class_init--");
}

EasSendEmailReq *
eas_send_email_req_new (const gchar* account_id, 
                        DBusGMethodInvocation *context, 
                        const gchar* client_id, 
                        const gchar* mime_file)
{
    EasSendEmailReq *self = g_object_new (EAS_TYPE_SEND_EMAIL_REQ, NULL);
    EasSendEmailReqPrivate* priv = self->priv;

    g_debug ("eas_send_email_req_new++");

    eas_request_base_SetContext (&self->parent_instance, context);

    priv->mime_file = g_strdup (mime_file);
    priv->account_id = g_strdup (account_id);
    priv->client_id = g_strdup (client_id);

    g_debug ("eas_send_email_req_new--");

    return self;
}

// uses the message object to build xml and sends it to the connection object
gboolean
eas_send_email_req_Activate (EasSendEmailReq *self, GError** error)
{
    gboolean ret = TRUE;
    EasSendEmailReqPrivate* priv = self->priv;
    xmlDoc *doc;
    FILE *file = NULL;
    guint64 size = 0;
    size_t result = 0;
	gchar* mime_string = NULL;

    // store flag in base (doesn't set the flag as in signal the waiters)
    g_debug ("eas_send_email_req_Activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    // open file containing mime data to be sent:
    file = fopen (priv->mime_file, "r"); // mime file can be read as text (attachments will be base 64 encoded)
    if (file == NULL)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FILEERROR,
                     ("failed to open file %s"), priv->mime_file);
        goto finish;
    }

    // read data from the file to mime_string
    // obtain file size:
    fseek (file , 0 , SEEK_END);
    size = ftell (file);
    g_debug ("file size = %llu", size);
    rewind (file);

    // allocate memory to contain the whole file:
    mime_string = (gchar*) g_malloc0 (sizeof (gchar) * size);
    if (mime_string == NULL)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        goto finish;
    }

    // copy the file into the buffer:
    result = fread (mime_string, 1, size, file);
    if (result != size)
    {
        ret = FALSE;
        // set the error
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_FILEERROR,
                     ("failed to open file %s"), priv->mime_file);
        goto finish;
    }

    g_debug ("create msg object");
    //create msg object
    priv->send_email_msg = eas_send_email_msg_new (priv->account_id, priv->client_id, mime_string);

    g_debug ("build messsage");
    //build request msg
#ifdef ACTIVESYNC_14
    //Activesync 14 base64 encodes the Mime and builds an xml message
    doc = eas_send_email_msg_build_message (priv->send_email_msg);
#else
	//Activesync 12.1 just uses the mime string in the body of the message
	doc = (xmlDoc*)mime_string;
#endif
    if (!doc)
    {
        g_set_error (error, EAS_CONNECTION_ERROR,
                     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
                     ("out of memory"));
        ret = FALSE;
        goto finish;
    }
    g_debug ("send message");
    ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                       "SendMail",
                                       doc, // full transfer
                                       (struct _EasRequestBase *) self,
                                       error);
finish:
    if (file == NULL)
    {
        fclose (file);
    }
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);
    }
    g_debug ("eas_send_email_req_Activate--");
    return ret;
}


gboolean
eas_send_email_req_MessageComplete (EasSendEmailReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean ret = TRUE;
    GError *error = NULL;
    EasSendEmailReqPrivate *priv = self->priv;

    g_debug ("eas_send_email_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
		ret = FALSE;
        error = error_in;
        goto finish;
    }

    ret = eas_send_email_msg_parse_response (priv->send_email_msg, doc, &error);
    if (!ret)
    {
        g_assert (error != NULL);
    }

finish:
    xmlFreeDoc (doc);
    if (!ret)
	{
        g_assert (error != NULL);
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
    }
    else
    {
        dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance));
    }

    g_debug ("eas_send_email_req_MessageComplete--");

    return TRUE;
}
