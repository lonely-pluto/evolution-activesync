/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 *
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-send-email-msg.h"
#include <wbxml/wbxml.h>
#include <glib.h>

G_DEFINE_TYPE (EasSendEmailMsg, eas_send_email_msg, EAS_TYPE_MSG_BASE);

#define EAS_SEND_EMAIL_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_MSG, EasSendEmailMsgPrivate))

struct _EasSendEmailMsgPrivate
{
    gchar* account_id;
    gchar* client_id;
    gchar* mime_string;
};

static void
eas_send_email_msg_init (EasSendEmailMsg *object)
{
    EasSendEmailMsgPrivate *priv;

    /* initialization code: */
    g_debug ("eas_send_email_msg_init++");

    object->priv = priv = EAS_SEND_EMAIL_MSG_PRIVATE (object);

    priv->account_id = NULL;
    priv->client_id = NULL;
    priv->mime_string = NULL;

    g_debug ("eas_send_email_msg_init--");
}

static void
eas_send_email_msg_finalize (GObject *object)
{
    /* deinitalization code: */
    EasSendEmailMsg *msg = (EasSendEmailMsg *) object;

    EasSendEmailMsgPrivate *priv = msg->priv;

    g_free (priv->mime_string);
    g_free (priv->client_id);
    g_free (priv->account_id);
    g_free (priv);
    msg->priv = NULL;

    G_OBJECT_CLASS (eas_send_email_msg_parent_class)->finalize (object);
}

static void
eas_send_email_msg_class_init (EasSendEmailMsgClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    // get rid of warnings about above 2 lines
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

    g_type_class_add_private (klass, sizeof (EasSendEmailMsgPrivate));

    object_class->finalize = eas_send_email_msg_finalize;
}

EasSendEmailMsg*
eas_send_email_msg_new (const char* account_id, const gchar* client_id, const gchar* mime_string)
{
    EasSendEmailMsg* msg = NULL;
    EasSendEmailMsgPrivate *priv = NULL;

    g_debug ("eas_send_email_msg_new++");

    msg = g_object_new (EAS_TYPE_SEND_EMAIL_MSG, NULL);
    priv = msg->priv;

    priv->client_id = g_strdup (client_id);
    priv->mime_string = g_strdup (mime_string);
    priv->account_id = g_strdup (account_id);

    g_debug ("eas_send_email_msg_new--");
    return msg;
}

xmlDoc*
eas_send_email_msg_build_message (EasSendEmailMsg* self)
{
    EasSendEmailMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *root  = NULL,
                     *leaf = NULL;
    gchar* base64data = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    root = xmlNewDocNode (doc, NULL, (xmlChar*) "SendMail", NULL);
    xmlDocSetRootElement (doc, root);

    xmlCreateIntSubset (doc,
                        (xmlChar*) "ActiveSync",
                        (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
                        (xmlChar*) "http://www.microsoft.com/");

    // no namespaces required?
    xmlNewNs (root, (xmlChar *) "ComposeMail:", NULL);

    // TEMP LINE DO NOT CHECK IN:
    // leaf = xmlNewChild(root, NULL, (xmlChar *)"AccountId", (xmlChar*)"lorna.mcneil@mobica.com");

    leaf = xmlNewChild (root, NULL, (xmlChar *) "ClientId", (xmlChar*) (priv->client_id));
    leaf = xmlNewChild (root, NULL, (xmlChar *) "SaveInSentItems", NULL); // presence indicates true
    base64data = g_base64_encode ( (const guchar *) priv->mime_string, strlen (priv->mime_string));
    leaf = xmlNewChild (root, NULL, (xmlChar *) "MIME", (xmlChar *) base64data);

    return doc;
}

void
eas_send_email_msg_parse_response (EasSendEmailMsg* self, xmlDoc *doc, GError** error)
{
    xmlNode *root, *node = NULL;
    g_debug ("eas_send_email_msg_parse_response++\n");

    if (!doc)
    {
        g_debug ("Failed: no doc supplied");
        return;
    }
    root = xmlDocGetRootElement (doc);
    if (g_strcmp0 ( (char *) root->name, "SendMail"))
    {
        g_debug ("Failed: not a SendMail response!");
        return;
    }
    for (node = root->children; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0 ( (char *) node->name, "Status"))
        {
            gchar *sendmail_status = (gchar *) xmlNodeGetContent (node);
            g_debug ("SendMail Status:[%s]", sendmail_status);  //TODO - how are errors being propagated to client??
            continue;
        }
    }

    g_debug ("eas_send_email_msg_parse_response++\n");

}

