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

#ifndef _EAS_GET_ITEM_ESTIMATE_REQ_H_
#define _EAS_GET_ITEM_ESTIMATE_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_ITEM_ESTIMATE_REQ             (eas_get_item_estimate_req_get_type ())
#define EAS_GET_ITEM_ESTIMATE_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_REQ, EasGetItemEstimateReq))
#define EAS_GET_ITEM_ESTIMATE_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_ITEM_ESTIMATE_REQ, EasGetItemEstimateReqClass))
#define EAS_IS_GET_ITEM_ESTIMATE_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_REQ))
#define EAS_IS_GET_ITEM_ESTIMATE_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_ITEM_ESTIMATE_REQ))
#define EAS_GET_ITEM_ESTIMATE_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_REQ, EasGetItemEstimateReqClass))

typedef struct _EasGetItemEstimateReqClass EasGetItemEstimateReqClass;
typedef struct _EasGetItemEstimateReq EasGetItemEstimateReq;
typedef struct _EasGetItemEstimateReqPrivate EasGetItemEstimateReqPrivate;

struct _EasGetItemEstimateReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasGetItemEstimateReq
{
	EasRequestBase parent_instance;

	EasGetItemEstimateReqPrivate * priv;
};

GType eas_get_item_estimate_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new email get item estimate request GObject
 *
 * @param[in] sync_key
 *	  The current synchronisation key.
 * @param[in] folder_id
 *	  The identifer for the target server folder.
 * @param[in] context
 *	  A dbus method invocation used to send the completed operation's results
 *	  to the server. Used in MessageComplete
 *
 * @return An allocated EasGetItemEstimateReq GObject or NULL
 */
EasGetItemEstimateReq *eas_get_item_estimate_req_new(const gchar *sync_key, 
                                            const gchar *folder_id,
                                            DBusGMethodInvocation *context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasGetItemEstimateReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_get_item_estimate_req_Activate(EasGetItemEstimateReq *self, 
                                       GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then sending the response back across the dbus
 *
 * @param[in] self
 *	  The EasGetItemEstimateReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if request is finished and needs cleaning up by connection 
 *    object, otherwise FALSE. 
 */
gboolean eas_get_item_estimate_req_MessageComplete(EasGetItemEstimateReq *self, 
                                              xmlDoc* doc, 
                                              GError* error);


G_END_DECLS

#endif /* _EAS_GET_ITEM_ESTIMATE_REQ_H_ */
