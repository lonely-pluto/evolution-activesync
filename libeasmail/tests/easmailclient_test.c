/**
 *  
 *  Filename: easmailclient_test.c
 *  Project: 
 *  Description: 
 *
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "../src/libeasmail.h"
#include "../src/eas-folder.h"
#include "../src/eas-attachment.h"

// display an EasFolderFree 
static void 
EasFolderDisplay(EasFolder *folder)
{
    g_print("EasFolderDisplay++");
    g_print("%s", folder->parent_id);
    g_print("%s", folder->folder_id);
    g_print("%s", folder->display_name);
    g_print("EasFolderDisplay--");
}


/**
 *
 */
int 
main(int argc, char** argv) {	
	
    guint64 accountuid = 123456789;
    EasEmailHandler *email_handler = NULL;
	
    email_handler = eas_mail_handler_new(accountuid);

    g_assert(email_handler != NULL);

	g_print("dbus initialised\n");

	/*
	// temp - test folder 'serialisation'/'deserialisation':
 	gchar *serialised = NULL;
	EasFolder *folder = eas_folder_new();
	
	folder->parent_id = g_strdup("5");	// TODO - test where this is an empty string or NULL!
	//folder->parent_id = NULL;
	//folder->parent_id = g_strdup("");
	folder->folder_id = g_strdup("1");
	folder->display_name = g_strdup("Inbox");
	folder->type = 2;	//EAS_FOLDER_TYPE_DEFAULT_INBOX

	if(!eas_folder_serialise(folder, &serialised))
	{
		g_print("call to eas_folder_serialise failed!");
	}
	else
	{
		g_print("result from serialise: %s\n", serialised);
	}

	g_object_unref(folder); // these don't have to be freed!

	
	EasFolder *folder2 = eas_folder_new();

	eas_folder_deserialise(folder2, serialised);

	g_print("result from serialise:\n");
	g_print("parent_id = %s\n", folder2->parent_id);
	g_print("folder_id = %s\n", folder2->folder_id);
	g_print("display name = %s\n", folder2->display_name);
	g_print("type = %d\n", folder2->type);

	g_object_unref(folder2); // these don't have to be freed!

	g_free(serialised);
	*/

	/*
// temp - test attachment 'serialisation'/'deserialisation':
	EasAttachment *attachment = eas_attachment_new();
	gchar *serialised = NULL;

	attachment->file_reference = g_strdup("\\my\\mime\\path");	
	attachment->display_name = g_strdup("myattachment.txt");// TODO - test where this is an empty string or NULL!
	//attachment->display_name = NULL;
	attachment->estimated_size = 23456789;	

	if(!eas_attachment_serialise(attachment, &serialised))
	{
		g_print("call to eas_attachment_serialise failed!");
	}
	else
	{
		g_print("result from serialise: %s\n", serialised);
	}

	g_object_unref(attachment); 

	EasAttachment *attachment2 = eas_attachment_new();

	eas_attachment_deserialise(attachment2, serialised);

	g_print("result from deserialise:\n");
	g_print("file_reference = %s\n", attachment2->file_reference);
	g_print("display name = %s\n", attachment2->display_name);
	g_print("estimated size = %d\n", attachment2->estimated_size);

	g_object_unref(attachment2); // these don't have to be freed!
	
	g_free(serialised);	
	*/

	// temp - test attachment 'serialisation'/'deserialisation':
	// build the email info object:
	//headers
	EasEmailHeader header1;
	EasEmailHeader header2;

	header1.name = g_strdup("To");
	header1.value = g_strdup("Lorna");
	header2.name = g_strdup("From");
	header2.value = g_strdup("Tori");	

	GSList *headers = NULL;

	headers = g_slist_append(headers, &header1);
	headers = g_slist_append(headers, &header2);

	//attachments
	GSList *attachments = NULL;
	EasAttachment *attachment = eas_attachment_new();

	attachment->file_reference = g_strdup("fileref");	
	attachment->display_name = g_strdup("myattachment.txt");// TODO - test where this is an empty string or NULL!
	attachment->estimated_size = 12345;	

	attachments = g_slist_append(attachments, attachment);

	//categories
	GSList *categories = NULL;

	gchar *category = g_strdup("personal");
	categories = g_slist_append(categories, category);
	category = g_strdup("work");
	categories = g_slist_append(categories, category);
	
	EasEmailInfo *email_info = eas_email_info_new ();
	email_info->headers = headers;
	email_info->attachments = attachments;
	email_info->categories = categories;
	email_info->flags = EAS_EMAIL_READ | EAS_EMAIL_ANSWERED;
	email_info->server_id = g_strdup("5:3");
	
	gchar *serialised = NULL;
	if(!eas_email_info_serialise(email_info, &serialised))
	{
		g_print("call to eas_email_info_serialise failed!");
	}
	else
	{
		g_print("result from serialise: %s\n", serialised);
	}

	EasEmailInfo *email_info_2 = eas_email_info_new ();
	eas_email_info_deserialise(email_info_2, serialised);

	g_print("result from deserialise:\n");
	GSList *l = email_info_2->attachments;
	EasAttachment *attachment_out = l->data;
	g_print("file reference = %s\n", attachment_out->file_reference);
	
	l = email_info_2->headers;
	l = g_slist_next(l);
	EasEmailHeader *header_out;
	header_out = l->data;
	g_print("header 2 value = %s\n", header_out->value);

	l = email_info_2->categories;
	gchar *cat_out = l->data;
	g_print("first category = %s\n", cat_out);
	
	// free everything: TODO
	g_print("test: free everything");
	//g_slist_foreach(categories, (GFunc)g_free, NULL);
	//g_slist_free(categories);
	//g_object_unref(attachment);
	//g_slist_free(attachments);
	
	//g_object_unref(email_info);
	//g_object_unref(email_info_2);
	//g_free(serialised);
	
	
	/*
    GSList *created = NULL; //receives a list of EasFolders
    GSList *updated = NULL;
    GSList *deleted = NULL;    
    gchar *sync_key = "0";
    GError *error = NULL;
    gboolean ret;

    ret  = eas_mail_handler_sync_folder_hierarchy(email_handler, sync_key, 	
	        &created,	
	        &updated,
	        &deleted,
	        &error);
	          
    g_print("new sync key = %s\n", sync_key);
    
    g_slist_foreach(created, (GFunc)EasFolderDisplay, NULL);

	//  free everything!
    g_slist_foreach(created, (GFunc)g_object_unref, NULL);
    g_slist_foreach(deleted, (GFunc)g_object_unref, NULL);
    g_slist_foreach(updated, (GFunc)g_object_unref, NULL);
    
    g_slist_free(created);
    g_slist_free(deleted);
    g_slist_free(updated);
	*/
	
    return 0;
}

