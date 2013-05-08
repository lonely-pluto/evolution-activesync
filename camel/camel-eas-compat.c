/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Authors: David Woodhouse <dwmw2@infradead.org>
 *
 * Copyright © 2010-2011 Intel Corporation (www.intel.com)
 * 
 * Based on GroupWise/EWS code which was
 *  Copyright © 1999-2008 Novell, Inc. (www.novell.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "camel-eas-compat.h"

gchar *
camel_session_get_password_compat (CamelSession *session,
				   CamelService *service,
				   const gchar *domain,
				   const gchar *prompt,
				   const gchar *item,
				   guint32 flags,
				   GError **error)
{
	
#if ! EDS_CHECK_VERSION(3,1,0)
	return camel_session_get_password (session, service, domain,
					   prompt, item, flags, error);
#else
	return camel_session_get_password (session, service, prompt,
					   item, flags, error);
#endif	
}

#if 0
CamelService *
camel_session_get_service_compat (CamelSession *session, const gchar *url, CamelProviderType type)
{
#if ! EDS_CHECK_VERSION(3,1,0)
	return camel_session_get_service (session, url, type, NULL);
#else
	CamelURL *curl = camel_url_new (url, NULL);
	CamelService *service;

	service = camel_session_get_service_by_url (session, curl, type);
	camel_url_free (curl);
	return service;
#endif	
}
#endif
#if ! EDS_CHECK_VERSION(3,1,0)

CamelURL *
camel_service_get_camel_url (CamelService *service)
{
	g_return_val_if_fail (CAMEL_IS_SERVICE (service), NULL);

	return service->url;
}

CamelServiceConnectionStatus
camel_service_get_connection_status (CamelService *service)
{
	return service->status;
}

#else

gchar *
camel_session_get_storage_path (CamelSession *session, CamelService *service,
				GError **error)
{
	return g_strdup (camel_service_get_user_data_dir (service));
}

#endif
