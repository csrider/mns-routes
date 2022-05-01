/************************************************************************
** 	Module: 	smajax.c 
**
**	Author:		(Redacted), Chris Rider
**			Copyright (c) 1991-2021
***********************************************************************/

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/***** REDACTED *****/

/***********************************************************************
** void main(int argc, char *argv[], char *env[])
**
***********************************************************************/
int main(int argc, char *argv[], char *env[])
{

/***** REDACTED *****/

else if(cgi_parsed_lookup("loadSetYouAreHereBasedOnIP=", buffer, sizeof(buffer)))
	{
	char *user_ip = getenv("REMOTE_ADDR");

	if(db_syspa_init() || db_hard_init())
		{
		DIAGNOSTIC_LOG("Database initialization error");

		printf("-1");
		}
	else
		{
		/* look up the IP address of the mediaport and and return the device id */
		if(user_ip
			&& ((HardwareFindIPandType(user_ip, DEVICE_MEDIAPORT_LCD_GA) > 0 || HardwareFindIPandType(user_ip, DEVICE_MEDIAPORT_LCD) > 0) || (HardwareFindIPandType(user_ip, DEVICE_MEDIAPORT_LCD_TVC) > 0)))
			{
			printf("%s", db_hard->dhc_deviceid);
			DIAGNOSTIC_LOG_2("loadSetYouAreHereBasedOnIP found '%s' '%s'", db_hard->dhc_deviceid, user_ip);
			}
		else
			{
			printf("-1");		/* nothing found */	
			DIAGNOSTIC_LOG_1("ERROR: Server either couldn't determine IP address from REMOTE_ADDR, or didn't return anything from HardwareFindIPandType()... IP: '%s'", user_ip);
			}
		}

	cgi_parsed_lookup("ajaxFloorplanNameForIconset=", ajaxFloorplanNameForIconset, sizeof(ajaxFloorplanNameForIconset));	/* pull floorplan name from what was sent */
	
	db_syspa_close();
	db_hard_close();
	}
else if(cgi_parsed_lookup("loadSetIconsetBasedOnFloorplan=", buffer, sizeof(buffer)))
	{
	int i = 0;

	char posted_floorplan_name[FLOORPLAN_NAME_LENGTH];
	char strResponseStaging[500];
	char strResponse[10000];

	cgi_parsed_lookup("specifiedFloorplan=", posted_floorplan_name, sizeof(posted_floorplan_name));		/* parse out the AJAX-posted floorplan name */
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("START: loadSetIconsetBasedOnFloorplan for '%s'", posted_floorplan_name);

/* get a table of icon images that are unique to the floorplan and return it as an array to be eval'd by the callback function in smcgi_xmlhttp.c */

	/* get icon data for the specified floorplan, from the database */
	if(db_syspa_init() || db_list_init() || db_hard_init())
		{
		printf("-1\n");

		DIAGNOSTIC_LOG("Database initialization error");
		}
	else
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("       Databases opened");

		db_list_select(6);											/* select with type, name, and width, height (dli_decode_primitive) */

		strcpy(db_list->res_id, res_id);
		strcpysl(db_list->dli_name, posted_floorplan_name, sizeof(db_list->dli_name));
		strcpy(db_list->dli_decode_primitive, "");
		db_list->dli_type = LIST_FLOORPLAN_MAP;
		next = db_list_find();

		strcpy(strResponse, "aisItemType=0; aisItemLeft=1; aisItemTop=2; aisDeviceID=3; aisText=4; aisActiveState=5; aisIsLsd=6; aisRecno=7;");

		if(!strcmp(db_list->dli_name, posted_floorplan_name)							/* if database record's floorplan name matches what was posted to this, */
			&& db_list->dli_type == LIST_FLOORPLAN_MAP)							/* AND the record's type is that of a floorplan map, then... */
			{
			strcat(strResponse, "var arrIconset=[];");
			while(next > 0												/* for each device-icon record matching this floorplan... */
				&& db_list->dli_type == LIST_FLOORPLAN_MAP
				&& !strcmp(db_list->res_id, res_id)
				&& !strcmp(db_list->dli_name, posted_floorplan_name))
				{
				ptr = strchr(db_list->dli_decode_primitive, ',');
				if(ptr)
					{
					*ptr = 0;
					ptr++;
					top = atoi(db_list->dli_decode_primitive);
					left = atoi(ptr);
					}
				else
					{
					left = 0;
					top = 0;
					}
				sprintf(strResponseStaging, "arrIconset[%d]=[];", i);								/* initialize a second dimension to the array that will be returned */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				sprintf(strResponseStaging, "arrIconset[%d][aisItemType]=%d;", i, db_list->dli_floor_plan_type);		/* save type of icon */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				sprintf(strResponseStaging, "arrIconset[%d][aisItemLeft]=%d;", i, left);					/* save icon's position from left */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				sprintf(strResponseStaging, "arrIconset[%d][aisItemTop]=%d;", i, top);						/* save icon's position from top */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				sprintf(strResponseStaging, "arrIconset[%d][aisDeviceID]=\"%s\";", i, db_list->dli_associated_sign_group);	/* save device name */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));
				if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("       DeviceID: %s " FORMAT_DBRECORD_STR "", db_list->dli_associated_sign_group, db_list_getcur());

				sprintf(strResponseStaging, "arrIconset[%d][aisText]=\"%s\";", i, db_list->dli_decode_help_launch_string);	/* save text in case this is a text type icon */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE))						/* if we can find the device in the hardware database... */
					{
					sprintf(strResponseStaging, "arrIconset[%d][aisActiveState]=%d;", i, db_hard->dhc_device_in_alarm_trouble_clear);	/* get and save the device's state (parsed by smlocations:getIconFilenameFromType() function) */
					strcatl(strResponse, strResponseStaging, sizeof(strResponse));
					}
				else														/* else... */
					{
					sprintf(strResponseStaging, "arrIconset[%d][aisActiveState]=%d;", i, DEVICE_STATE_NONE);				/* save explicit no alarm state by default */
					strcatl(strResponse, strResponseStaging, sizeof(strResponse));
					}

				sprintf(strResponseStaging, "arrIconset[%d][aisIsLsd]=\"%d\";", i, 0);						/* currently not used */
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				//HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE);
				sprintf(strResponseStaging, "arrIconset[%d][aisRecno]=\"" FORMAT_DBRECORD_STR "\";", i, db_list_getcur());
				strcatl(strResponse, strResponseStaging, sizeof(strResponse));

				i++;														/* increment counter for next device icon */
				printf(strResponse);												/* print to client now, to prevent buffer overflows (dev-note: not sure how well these multiple prints will work with httpRequest object, but seems to work) */
				strcpy(strResponse, "");
				next = db_list_next();												/* move to next device icon */
				}

			if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("       responseText 'arrIconset' generated (%d icon/items found)", i+1);
			}

		db_syspa_close();
		db_hard_close();
		db_list_close();
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("       Databases closed");
		}
	}

/* Get all floorplans, etc. about the posted life safety device, and return as a two-dimensional JavaScript array */
else if(cgi_parsed_lookup("loadGetLifeSafetyDeviceDataBasedOnDeviceID=", buffer, sizeof(buffer)))
	{
	int i = 0;

	char posted_life_safety_device_id[DEVICEID_LENGTH];
	char strResponseStaging[sizeof(buffer)];
	char strResponse[10000];

	/* parse the life safety device name from the posted data (smcgi_xmlhttp.c) */
	cgi_parsed_lookup("lsd=", posted_life_safety_device_id, sizeof(posted_life_safety_device_id));
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("START: loadGetLifeSafetyDeviceDataBasedOnDeviceID for '%s'", posted_life_safety_device_id);

	strcpy(strResponse, "almiDeviceID=0; almiFloorplanName=1; almiActiveFlag=2; almiRecNo=3;");

	sprintf(strResponseStaging, "var thisDeviceID=\"%s\";", posted_life_safety_device_id);
	strcat(strResponse, strResponseStaging);

	strcat(strResponse, "var arrLsdMaps=[];");									/* initialize an array to return to the callback function (it will then determine if it's populated or not) */

	/* gather data about the device from the database, and return as javascript that can then be eval'd by the calling code and then interpreted as needed (similar to above) */
	if(db_syspa_init() || db_list_init())									/* if the list database can not be opened, then... (syspa is needed to populate res_id) */
		{
		printf("-1\n");

		DIAGNOSTIC_LOG("Database initialization error");
		}
	else													/* else the list database successfully opened, so... */
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("       Databases opened");
		db_list_select(8);											/* select with res_id, associated sign group (device name), and name (floorplan name) */

		strcpy(db_list->res_id, res_id);
		strcpysl(db_list->dli_associated_sign_group, posted_life_safety_device_id, sizeof(db_list->dli_associated_sign_group));
		strcpy(db_list->dli_name, "");

		next = db_list_find();

		while(next > 0)												/* as long as there are records... */
			{
			if(!strcmp(db_list->res_id, res_id) 									/* if this one matches what we're looking for, then... */
				&& strcmp(db_list->dli_associated_sign_group, posted_life_safety_device_id) == 0)
				{
				sprintf(strResponseStaging, "arrLsdMaps[%d]=[];", i);								/* initialize a second dimension to the array that will be returned */
				strcat(strResponse, strResponseStaging);

				sprintf(strResponseStaging, "arrLsdMaps[%d][almiDeviceID]=\"%s\";", i, db_list->dli_associated_sign_group);	/* save device name */
				strcat(strResponse, strResponseStaging);

				sprintf(strResponseStaging, "arrLsdMaps[%d][almiFloorplanName]=\"%s\";", i, db_list->dli_name);			/* save floorplan name */
				strcat(strResponse, strResponseStaging);

				sprintf(strResponseStaging, "arrLsdMaps[%d][almiActiveFlag]=%d;", i, db_list->dli_font_size);			/* save active flag */
				strcat(strResponse, strResponseStaging);

				sprintf(strResponseStaging, "arrLsdMaps[%d][almiRecNo]="FORMAT_DBRECORD_STR";", i, db_list_getcur());		/* save this record number */
				strcat(strResponse, strResponseStaging);

				i++;														/* increment counter for next device icon */
				}
				
			printf(strResponse);											/* print to client now, to prevent buffer overflows (dev-note: not sure how well these multiple prints will work with httpRequest object, but seems to work) */
			strcpy(strResponse, "");

			next = db_list_next();											/* move to next device icon */
			}
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("       responseText 'arrLsdMaps' generated (%d items found)", i+1);
		
		db_syspa_close();											/* close the syspa database */
		db_list_close();											/* close the list database */
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("       Databases closed");
		}
	}

/* Get number of floorplans that already exist in the list database, with the given name */
else if(cgi_parsed_lookup("loadNumberOfMapsBasedOnMapName=", buffer, sizeof(buffer)))
	{
	char posted_floorplan_name[FLOORPLAN_NAME_LENGTH];
	int numberOfMaps = 0;

	cgi_parsed_lookup("mapName=", posted_floorplan_name, sizeof(posted_floorplan_name));		/* parse out the AJAX-posted floorplan name */

	DIAGNOSTIC_LOG_1("smajax: loadNumberOfMapsBasedOnMapName - floorplan = '%s'", posted_floorplan_name);
	//printf("1");
	if(db_syspa_init() || db_list_init() || db_hard_init())
		{
		DIAGNOSTIC_LOG("smajax: loadNumberOfMapsBasedOnMapName - Database initialization error");
		printf("-1");
		}
	else
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("smajax: loadNumberOfMapsBasedOnMapName - Databases opened");

		db_list_select(6);

		strcpy(db_list->res_id, res_id);
		strcpysl(db_list->dli_name, posted_floorplan_name, sizeof(db_list->dli_name));
		strcpy(db_list->dli_decode_primitive, "");
		db_list->dli_type = LIST_FLOORPLAN_MAP_HEADER;

		if(db_list_find()
			&& db_list->dli_type == LIST_FLOORPLAN_MAP_HEADER
			&& !strcmp(db_list->dli_name, posted_floorplan_name))
			{
			numberOfMaps++;
			next = db_list_next();
			while(next > 0
				&& db_list->dli_type == LIST_FLOORPLAN_MAP_HEADER
				&& !strcmp(db_list->dli_name, posted_floorplan_name))
				{
				numberOfMaps++;
				next = db_list_next();
				}
			}
		
		printf("%d", numberOfMaps);
		}

	db_syspa_close();
	db_list_close();
	db_hard_close();
	}

/***** REDACTED *****/

return(0);
}
