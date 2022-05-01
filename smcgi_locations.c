/************************************************************************
** 	Module: 	smcgi_locations.c 
**
**	Author:		(Redacted), Chris Rider
**			Copyright (c) 1991-2021
**
** April 2010 - Updated by Chris Rider to support route creation and display
**
************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/***** REDACTED *****/

static char floor_plan_record_numbers[MAX_MULTI_SELECTION_LIST_BUFFER] = "";

#define SELECTION_TYPE_NONE					(0x00000)
#define SELECTION_TYPE_SIGNS				(0x00001)
#define SELECTION_TYPE_PHONES				(0x00002)
#define SELECTION_TYPE_SPEAKERS				(0x00004)
#define SELECTION_TYPE_MESSAGES				(0x00008)
#define SELECTION_TYPE_PERSON				(0x00010)
#define SELECTION_TYPE_BUTTON				(0x00020)
#define SELECTION_TYPE_PC_ALERT				(0x00040)
#define SELECTION_TYPE_TEXT					(0x00080)
#define SELECTION_TYPE_CAMERA				(0x00100)
#define SELECTION_TYPE_MEDIAPORT			(0x00200)
#define SELECTION_TYPE_PULL_STATION			(0x00400)
#define SELECTION_TYPE_SMOKE_DETECTOR		(0x00800)
#define SELECTION_TYPE_DUCT_DETECTOR		(0x01000)
#define SELECTION_TYPE_WATER_FLOW			(0x02000)
#define SELECTION_TYPE_DOOR_OPEN 			(0x04000)
#define SELECTION_TYPE_FIRE_EXTINGUISHER	(0x08000)
#define SELECTION_TYPE_MOTION_DETECTOR 		(0x10000)
#define SELECTION_TYPE_SOUND_METER 			(0x20000)

static int title_show_floorplan_edit_map = FALSE;		/* need to show the edit map button on the cgi_title() */
static int translation_mediaport_index = 0;
static int visible_list_index = 0;

extern int list_key;

/***** REDACTED *****/

/****************************************************************
** void loadNewMap(DBRECORD me_msg_template_recno, int flag_locations_tab_or_msg)
**
** 	Originally, this was the upload a new map tool.
**	This corresponds to (and results from) the function, add_CreateNewMap, above.
**
**	Updated by Chris Rider, 2011 - summer:
**	Adds multiple functionalities, including selection of existing/system maps, cloning, associating with msgs, etc.
**
****************************************************************/
void loadNewMap(DBRECORD me_msg_template_recno, int flag_locations_tab_or_msg)
{
int font_size = 3;
int i;

char record_number_str[FLOORPLAN_MULTI_RECORD_LENGTH];
char record_number_str_temp[FLOORPLAN_MULTI_RECORD_LENGTH];

/***** REDACTED *****/

char present_floorplan_name[FLOORPLAN_NAME_LENGTH];

char dtsec[DTSEC_LENGTH];	//to support unique map naming whenever cloning a map (will append dtsec to existing-selected map name)
get_dtsec(dtsec);

/* Read all of the associated floorplans from the banner message options file, and put it all together into a comma separated string that will need to be posted */
if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadNewMap(): me_msg_template_recno = "FORMAT_DBRECORD_STR, me_msg_template_recno);
BannerOptions(me_msg_template_recno, DB_ISAM_READ);                     //initialize the structure so we can use it
strcpyl(record_number_str, "", FLOORPLAN_MULTI_RECORD_LENGTH);		//initialize the string to empty, or a run-time error will occur
for(i = 0; i < MAX_MULTI_FLOORPLAN_SELECT; i++)				//for each of the potential floorplans from the banner message options file...
	{
	if(CheckValidFloorPlan(db_bann_message_options->mo_floorplan_group[i]))
		{
		//sprintf(record_number_str, " " FORMAT_DBRECORD_STR ",", db_bann_message_options->mo_floorplan_group_recno[i]);
		sprintf(record_number_str_temp, " " FORMAT_DBRECORD_STR ",", db_bann_message_options->mo_floorplan_group_recno[i]);
		strcatl(record_number_str, record_number_str_temp, sizeof(record_number_str));
		}
	}
if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadNewMap(): map records read from banner options file... record_number_str='%s'", record_number_str);

strcpy(present_floorplan_name, fm_bannerdata.bb_msg_destin);

printf("<HTML>");
add_CopyRight(NULL, TRUE);

add_CreateHelp();
add_operation_complete();
add_CreateMapName();
add_CreateListNames();
add_font_style(NULL, DEFAULT_FONT_POINT_SIZE);
add_HelpTextLayerCheck(TRUE, FALSE, FALSE);

if(record_number == 0)
	{
	cgi_html_title("New Locations Plan");
	memset(db_list, 0, sizeof(DB_CLIST));
	}
else
	{
	db_list_setcur(record_number);
	cgi_html_title("Update Locations Plan");
	}

cgi_body_with_background(NULL, "onLoad=\"SetFormIsLoaded(); return true;\"");
printf("<FORM action=%s method=post enctype='multipart/form-data'>", cgi_server);

add_FormLoadedControl();
add_CreateSystemChoiceList();
add_CreateSystemChoiceListExtended();

html_space(1);

if(record_number == 0)
	{
	printf("<h3 style=\"margin-top:-0.5em; margin-bottom:0.7em;\">Add a Map...</h3>");
	}
else
	{
	printf("<h3 style=\"margin-top:-0.5em; margin-bottom:0.7em;\">Change a Map...</h3>");
	}

printf("<INPUT type=\"hidden\" name=\"me_msg_template_recno\" value=\""FORMAT_DBRECORD_STR"\"></INPUT>", me_msg_template_recno);
printf("<INPUT type=\"hidden\" name=\"selected_map_multi\" value=\"%s\"></INPUT>", record_number_str);	/* for storing floorplan record numbers in a comma-separated list, in the case of multiple (empty if single or no maps selected) */
printf("<INPUT type=\"hidden\" name=\"save_new_map\" value=\"\"></INPUT>");	/* this field, by merely being present in the posted form, triggers smcgi to process */
printf("<INPUT type=\"hidden\" name=\"%s\" value=\"%s\"></INPUT>", cgi_identification_field, cgi_EncodeLoginPin(CurrentUserPin));
printf("<INPUT type=\"hidden\" name=\"record_number\" value=\"" FORMAT_DBRECORD_STR "\"></INPUT>", record_number);

printf("<TABLE border=0>");

/* Render the import-map form elements */
printf("<TR><TD nowrap align=\"right\">");
if(record_number == 0)
	{
	printf("<FONT color=\"%s\">Upload a New Map:</FONT>", text_field_color);
	}
else
	{
	printf("<FONT color=\"%s\">Change Map:</FONT>", text_field_color);
	}
printf("</TD><TD>");
printf("<INPUT type=\"file\" name=\"import_filename\" value=\"\" onchange=\"alternateFields()\"></INPUT>");
printf("</TD></TR>");

if(record_number == 0)
	{
	printf("<tr><td style=\"font-size:11px; font-weight:bold; text-align:right;\">- <em>OR</em> -&nbsp;&nbsp;&nbsp;</td><td style=\"font-size:11px;\">&nbsp;</td></tr>");
/* UPDATED: Per Kevin (7/12/11), the selection should be from ALL maps in the global map database (list DB) --and, no more multi-selection! */
	/* Render a single-selection list so they can select a map to create a copy of */
	printf("<tr><td nowrap align=\"right\">");
	printf("<font color=\"%s\">Select Existing Map:</font>", text_field_color);
	printf("</td><td>");
	printf("<INPUT %s type=\"text\" name=\"selected_map\" value=\"%s\" size=\"%d\" maxlength=\"%d\" onFocus=\"this.blur()\"></INPUT>", HTML_INPUT_STYLE, present_floorplan_name, FLOORPLAN_NAME_LENGTH, FLOORPLAN_NAME_LENGTH);	/* stores a single map name or " - MULTIPLE - " */
	/* The extended command in this case will take the selected map name, append dtsec, and populate the new map name field automatically */
	printf("<A HREF=\"javascript: OperationComplete()\" \
			onClick=\"if(HasFormBeenLoaded(1)) {CreateSystemChoiceListExtended(document.forms[0].selected_map.value, 0, 0, 0, '', 'selected_map', %d, 0, '', '', 'opener.document.forms[0].import_filename.value=null;opener.document.forms[0].new_map_name.value=null;'); return true;} else {return false;}\" \
			onMouseOut=\"CancelHelpText(); return true\" \
			onMouseOver=\"SetHelpText(0, 0, 'Select an existing map to create a copy of.'); return true\">", LIST_FLOORPLAN_MAP_HEADER);
	printf("<IMG SRC=\"%s/smchoice.gif\" alt=\"Click for choices.\" align=\"ABSCENTER\" ></A>", cgi_icons);
	printf("</td></tr>");
	}
else
	{
	printf("<INPUT %s type=\"hidden\" name=\"selected_map\" value=\"\"></INPUT>", HTML_INPUT_STYLE);	/* basically must be rendered (even though we're not using it in this case) just so the validation will work properly */
	printf("<input type=\"hidden\" name=\"flagrepop\" value=\"true\"></input>");				/* tell this page that we need to give the above field a value (found below) */
	}

/* Render the rest of the common form elements */
printf("<tr><td colspan=\"2\" style=\"border-bottom:3px double #666; font-size:3px;\">&nbsp;</td></tr>");
printf("<TR><TD nowrap align=\"right\" style=\"padding-top:6px; padding-bottom:6px;\">");
printf("<FONT color=\"%s\">Map Name:</FONT>", text_field_color);
printf("</TD><TD style=\"padding-top:6px; padding-bottom:6px;\">");

if(CheckValidFloorPlan(db_list->dli_name))
	{
	sprintf(dli_name_no_space, "%s", db_list->dli_name);
	}
else
	{
	sprintf(dli_name_no_space, "%s", "");
	}
remove_trailing_space(dli_name_no_space);

if(record_number == 0)
	{
	printf("<INPUT type=\"text\" name=\"new_map_name\" value=\"\" size=\"%d\" maxlength=\"%d\" onfocus=\"this.value=document.forms[0].selected_map.value+%s\"></INPUT>", FLOORPLAN_NAME_LENGTH, FLOORPLAN_NAME_LENGTH, dtsec);
	}
else
	{
	printf("<input type=\"hidden\" name=\"new_map_name_backup\" value=\"%s\"></input>", dli_name_no_space);
	printf("<INPUT type=\"text\" name=\"new_map_name\" value=\"%s\" size=\"%d\" maxlength=\"%d\"></INPUT>", dli_name_no_space, FLOORPLAN_NAME_LENGTH, FLOORPLAN_NAME_LENGTH);
	}
printf("</TD></TR>");

/* if necessary, repopulate the selected_map field with the new_map_name value (to fix 'rename should be cloning, not renaming' bug - Nov 2011) */
printf("<script type=\"text/javascript\">if(document.forms[0].flagrepop && document.forms[0].flagrepop.value==\"true\"){document.forms[0].selected_map.value=document.forms[0].new_map_name.value;}</script>");

printf("<tr><td colspan=\"2\" style=\"border-top:3px double #666; font-size:3px;\">&nbsp;</td></tr>");

printf("<TR>");
cgi_input_table_list_choice_list(OPTION_TABLE | OPTION_MGR_ALLOW_CHANGE, font_size, text_field_color, "Who can use:", "fp_who_can_use", db_list->dli_who_can_use, LIST_NAME_LENGTH/2, LIST_NAME_LENGTH - 1, "Select to specify a the list of users who can use this map.", "", 0, "0", "");
printf("</TR>");

printf("<TR>");
cgi_input_table_list_choice_list(OPTION_TABLE | OPTION_MGR_ALLOW_CHANGE, font_size, text_field_color, "Who can modify:", "fp_who_can_mod", db_list->dli_who_can_modify, LIST_NAME_LENGTH/2, LIST_NAME_LENGTH - 1, "Select to specify a the list of users who can modify this map.", "", 0, "0", "");
printf("</TR>");

printf("<TR><td colspan=\"2\" align=\"center\">");
cgi_button(NULL, 0, button_font_point_size, " ", "OK", "validateNameField();", "", "");
html_space(1);
cgi_button(NULL, 0, button_font_point_size, " ", "Cancel", "self.close();", "", "");
printf("</td></TR>");

printf("</TABLE>");

printf("<script type=\"text/javascript\">");
printf("	var objXML = false;");
printf("	function removeTrailingSpace(str){");	//required by validation checks below
printf("		str = \"\"+str+\"\";");
printf("		return str.replace(/\\s+$/g,'');");
printf("	}\n");
printf("	function alternateFields(){");
			//if the import_filename field has changed to some actual value
printf("		if(removeTrailingSpace(document.forms[0].import_filename.value)!=\"\"){");
printf("			document.forms[0].selected_map.value = \"\";");		//clear any possibly-existing map selection
printf("			document.forms[0].new_map_name.value = \"\";");		//clear any possibly-existing new map name
printf("			document.forms[0].new_map_name.focus();");		//bring the new map name field into focus so they can provide a name (it should be the next step anyway)
printf("		}");
printf("	}\n");
printf("	function ajaxCheckIfMapNameAlreadyExists_stateChange(){");
printf("		var intMapsWithSameName;");
printf("		if(objXML.readyState == 4){");
printf("			if(objXML.status == 200){");
printf("				intMapsWithSameName = parseInt(objXML.responseText);");
					/* if the server returns a number of zero, then allow the form submission */
printf("				if(intMapsWithSameName == 0){");
printf("					document.forms[0].submit();");
printf("				}");
					/* if the server returns one or more, then they'll need to change the map name before being allowed to submit the form */
printf("				else if(intMapsWithSameName > 0){");
printf("					window.alert(\"That name already exists. Please try another.\");");
printf("					document.forms[0].new_map_name.focus();");
printf("				}");
					/* for some reason the server returned nothing, or didn't return anything that could be parsed as an integer */
printf("				else{");
printf("					window.alert(\"An error occurred... please try again or re-open your browser.\");");
printf("				}");
printf("			}");
printf("		}");
printf("	}\n");
printf("	function ajaxCheckIfMapNameAlreadyExists_open(mapName){");
switch(cgi_detect_browser())
        {
        case BROWSER_EXPLORER:
                printf("try {");
                printf("        objXML = new ActiveXObject('Msxm12.XMLHTTP');");
                printf("    } catch (e) {");
                printf("try {");
                printf("        objXML = new ActiveXObject('Microsoft.XMLHTTP');");
                printf("    } catch (E) {");
                printf("        objXML = false;");
                printf("    }");
                printf(" }");
                break;

        case BROWSER_GECKO:
	case BROWSER_OPERA:
        case BROWSER_NETSCAPE:
                printf(" objXML = new XMLHttpRequest();");
                break;
        }
printf("		if(objXML){");
printf("			objXML.onreadystatechange = ajaxCheckIfMapNameAlreadyExists_stateChange;");
printf("			objXML.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("			objXML.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("			objXML.send('loadNumberOfMapsBasedOnMapName=1&mapName='+mapName);");
printf("		}");
printf("	}\n");
printf("	function validateNameField(){");	//called when they click the OK button
			//if no fields are populated (they need to be told to do something in order to be able to continue)
printf("		if(removeTrailingSpace(document.forms[0].import_filename.value)==\"\" && removeTrailingSpace(document.forms[0].selected_map.value)==\"\"){");
if(record_number == 0)
	{
	printf("		window.alert(\"To continue, you must either upload a new map or select an existing map.\");");
	}
else
	{
				//(in the case of changing a map instead of adding...) if any other necessary fields have NOT been changed
	printf("		if(removeTrailingSpace(document.forms[0].new_map_name.value)==removeTrailingSpace(document.forms[0].new_map_name_backup.value) && removeTrailingSpace(document.forms[0].fp_who_can_use.value)==removeTrailingSpace(\"%s\") && removeTrailingSpace(document.forms[0].fp_who_can_mod.value)==removeTrailingSpace(\"%s\")){",db_list->dli_who_can_use ,db_list->dli_who_can_modify);
	printf("			window.alert(\"To continue, you must either upload a new map file or change this map in some way.\");");
	printf("		}");
				//(in the case of changing a map instead of adding...) else there are detected changes, so post the form
	printf("		else{");
	printf("			ajaxCheckIfMapNameAlreadyExists_open(document.forms[0].new_map_name.value);");
	printf("		}");
	}
printf("		}");
			//else if uploading a new map (detected by "import_filename" field being populated)
printf("		else if(removeTrailingSpace(document.forms[0].import_filename.value)!=\"\" && removeTrailingSpace(document.forms[0].selected_map.value)==\"\"){");
				//if a new map name has not been provided (detected by "new_map_name" being empty)
printf("			if(removeTrailingSpace(document.forms[0].new_map_name.value)==\"\"){");
printf("				window.alert(\"You must provide a name for this new map.\");");
printf("			}");
				//else if the new map name is too large
printf("			else if(document.forms[0].new_map_name.value.length > %d){", FLOORPLAN_NAME_LENGTH);
printf("				window.alert(\"The new map name must not exceed %d characters (including spaces).\");", FLOORPLAN_NAME_LENGTH);
printf("			}");
				//else everything looks ok, so continue with submission to upload the new map and associate it with the message/tab
printf("			else{");
printf("				ajaxCheckIfMapNameAlreadyExists_open(document.forms[0].new_map_name.value);");
printf("			}");
printf("		}");
			//else if selecting an existing map from the list DB (detected by "selected_map" field being populated)
printf("		else if(removeTrailingSpace(document.forms[0].selected_map.value)!=\"\" && removeTrailingSpace(document.forms[0].import_filename.value)==\"\"){");
				//if a new map name has been provided (they wish to clone the map, in addition to adding it to the message/tab)...
printf("			if(removeTrailingSpace(document.forms[0].new_map_name.value)!=\"\"){");
					//if the new map (clone's) name is the same as the selected (original) map, this is an error (it must be different)
printf("				if(removeTrailingSpace(document.forms[0].new_map_name)!=\"\" && removeTrailingSpace(document.forms[0].new_map_name.value)==removeTrailingSpace(document.forms[0].selected_map.value)){");
//printf("					window.alert(\"If you wish to clone the selected map, then you must provide a different name. Otherwise, you should not provide a new map name.\");");
printf("					window.alert(\"Map name already in use, try another name\");");	/* changed at Kevin's request 12/2/2011 */
printf("				}");
					//else if the new map name is too large
printf("				else if(document.forms[0].new_map_name.value.length > %d){", FLOORPLAN_NAME_LENGTH);
printf("					window.alert(\"The new map name must not exceed %d characters (including spaces).\");", FLOORPLAN_NAME_LENGTH);
printf("				}");
					//else everything looks ok, so continue with submission to clone an existing map and add it to the message/tab
printf("				else{");
printf("					ajaxCheckIfMapNameAlreadyExists_open(document.forms[0].new_map_name.value);");
printf("				}");
printf("			}");
				//else a new map name has NOT been provided (detected by "new_map_name" being empty) ~ so we should only associate the existing/selected map with the message/tab
printf("			else{");
printf("				ajaxCheckIfMapNameAlreadyExists_open(document.forms[0].new_map_name.value);");
printf("			}");
printf("		}");
			//else something wrong may be happening... (this should never happen)
printf("		else{");
printf("			window.alert(\"Unspecified Error. Please cancel and try again.\");");
printf("		}");
printf("	}\n");
printf("</script>");

add_HelpTextLayer(TRUE, FALSE);

printf("</FORM>");
printf("</BODY>");
printf("</HTML>");
}

/***** REDACTED *****/

/***********************************************************
** static int cgi_show_locations_item(int fp_edit_mode, int unique_name, int width, int height, int div_left, int div_top, FILE *mediaport_translation_fp, FILE *mediaport_visible_list_fp, int locations_alert_status, int smbanner_locations)
**
*********************************************************/
static int cgi_show_locations_item(int fp_edit_mode, int unique_name, int width, int height, int div_left, int div_top, FILE *mediaport_translation_fp, FILE *mediaport_visible_list_fp, int locations_alert_status, int smbanner_locations)
{
int valid_span = FALSE;

char onclick_text[600] = "";
char onclick_sign[600] = "";
char onclick_pc_alert[600] = "";
char onclick_mediaport[600] = "";
char onclick_link[600] = "";
char onclick_phone[600] = "";
char onclick_speaker[600] = "";
char onclick_message[600] = "";
char onclick_person[600] = "";
char onclick_button[600] = "";
char onclick_camera[600] = "";
char onclick_pull_station[600] = "";
char onclick_water_flow[600] = "";
char onclick_duct_detector[600] = "";
char onclick_smoke_detector[600] = "";
char onclick_motion_detector[600] = "";
char onclick_sound_meter[600] = "";
char onclick_fire_extinguisher[600] = "";
char onclick_door_open[600] = "";

char speaker_phone[PHONE_LENGTH] = "";
char debug_coordinates[40] = "";

int list_cur_key;

DBRECORD list_cur;

/* remember current position */
list_cur_key = list_key;
list_cur = db_list_getcur();

remove_trailing_space(db_list->dli_sister_audio_group);

if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_UP
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_DOWN
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_LEFT
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_RIGHT
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP_UP
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL
	|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL_EMERGENCY)
	{
	if(CheckValidAudioGroup(db_list->dli_associated_sign_group))
		{
		strcpyl(speaker_phone, db_list->dli_phone_number, sizeof(speaker_phone));
		remove_trailing_space(speaker_phone);
		}

	db_list_setcur(list_cur);
	}

if(db_list->dli_floor_plan_type == FLOORPLAN_MEDIAPORT
	|| db_list->dli_floor_plan_type == FLOORPLAN_PC_ALERT
	|| db_list->dli_floor_plan_type == FLOORPLAN_BUTTON
	|| db_list->dli_floor_plan_type == FLOORPLAN_PULL_STATION
	|| db_list->dli_floor_plan_type == FLOORPLAN_WATER_FLOW
	|| db_list->dli_floor_plan_type == FLOORPLAN_DUCT_DETECTOR
	|| db_list->dli_floor_plan_type == FLOORPLAN_SMOKE_DETECTOR
	|| db_list->dli_floor_plan_type == FLOORPLAN_MOTION_DETECTOR
	|| db_list->dli_floor_plan_type == FLOORPLAN_SOUND_METER
	|| db_list->dli_floor_plan_type == FLOORPLAN_FIRE_EXTINGUISHER
	|| db_list->dli_floor_plan_type == FLOORPLAN_DOOR_OPEN
	|| db_list->dli_floor_plan_type == FLOORPLAN_CAMERA)
	{
	if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE) == FALSE)
		{
		memset(db_hard, 0, sizeof(DB_FM_HRD));
		}
	}

if(smbanner_locations)
	{
	/* no click on VIEW only */
	}
else
	{
	snprintf(onclick_text, sizeof(onclick_text), " onclick=\"LocationsScreenTimerReset(); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } return false;\"", SELECTION_TYPE_TEXT, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_sign, sizeof(onclick_sign), " onclick=\"LocationsScreenTimerReset(); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR ");} } return false;\"", SELECTION_TYPE_SIGNS, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_pc_alert, sizeof(onclick_pc_alert), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR "); }} return false;\"", db_hard->dhc_device_owner, SELECTION_TYPE_PC_ALERT, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_mediaport, sizeof(onclick_mediaport), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetAddRouteMode()) {clickLCDtoAdd('%s');} else if(GetShowRouteMode()) {clickLCDtoShow('%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR "); }} return false;\"", db_hard->dhc_device_owner, SELECTION_TYPE_MEDIAPORT, db_list_getcur(), db_hard->dhc_deviceid, db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_link, sizeof(onclick_link), " onclick=\"LocationsScreenTimerReset(); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { MapLink(" FORMAT_DBRECORD_STR "); } return false;\"", db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_phone, sizeof(onclick_phone), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR "); }} return false;\"", db_list->dli_pin, SELECTION_TYPE_PHONES, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_speaker, sizeof(onclick_speaker), " onclick=\"LocationsScreenTimerReset(); SetSelectionType(%d); SetSpeakerPhone('%s'); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR ");} } return false;\"", SELECTION_TYPE_SPEAKERS, speaker_phone, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_message, sizeof(onclick_message), " onclick=\"LocationsScreenTimerReset(); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { document.forms[0].set_launch_directory.value='%s'; document.forms[0].set_launch_message.value='%s'; OneClickSendMessage(" FORMAT_DBRECORD_STR "); } return false;\"", SELECTION_TYPE_MESSAGES, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list->dli_decode_help_launch_string, db_list->dli_sister_audio_group, db_list_getcur());
	snprintf(onclick_person, sizeof(onclick_person), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { if(ShowMultiIconSelection(%d, " FORMAT_DBRECORD_STR ")) {SetSendMessage(" FORMAT_DBRECORD_STR ");} } return false;\"", db_list->dli_pin, SELECTION_TYPE_PERSON, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur());
	snprintf(onclick_button, sizeof(onclick_button), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickSendMessage(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_BUTTON, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_camera, sizeof(onclick_camera), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { CreateShowCamera(" FORMAT_DBRECORD_STR ", 0); if(%d) { CreateCameraMovementControl(" FORMAT_DBRECORD_STR "); } } return false;\"", db_list->dli_pin, SELECTION_TYPE_CAMERA, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_hard_getcur(), cgi_camera_has_movement_control(), db_hard_getcur());
	snprintf(onclick_pull_station, sizeof(onclick_pull_station), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_PULL_STATION, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_water_flow, sizeof(onclick_water_flow), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_WATER_FLOW, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_duct_detector, sizeof(onclick_duct_detector), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_DUCT_DETECTOR, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_smoke_detector, sizeof(onclick_smoke_detector), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_SMOKE_DETECTOR, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_motion_detector, sizeof(onclick_motion_detector), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_MOTION_DETECTOR, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_sound_meter, sizeof(onclick_sound_meter), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_SOUND_METER, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_fire_extinguisher, sizeof(onclick_fire_extinguisher), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_FIRE_EXTINGUISHER, db_list_getcur(), db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	snprintf(onclick_door_open, sizeof(onclick_door_open), " onclick=\"LocationsScreenTimerReset(); SetFirstPin('%s'); SetSelectionType(%d); if(GetDeleteMode()) { DeleteImageLocation(" FORMAT_DBRECORD_STR "); } else if(GetLsdMode()){clickLsd(" FORMAT_DBRECORD_STR ",'%s');} else if(GetEditItemMode()) { EditImageLocation(%d, " FORMAT_DBRECORD_STR "); } else if (GetEditMode()) { MoveImageLocation(%d, " FORMAT_DBRECORD_STR ", 'image" FORMAT_DBRECORD_STR "') } else { OneClickIgnore(" FORMAT_DBRECORD_STR "); } return false;\"", db_list->dli_pin, SELECTION_TYPE_DOOR_OPEN, db_list_getcur(), db_list_getcur(), db_hard->dhc_deviceid, db_list->dli_floor_plan_type, db_list_getcur(), unique_name, db_list_getcur(), db_list_getcur(), db_list_getcur());
	}

remove_trailing_space(db_list->dli_associated_sign_group);

if(div_top >= 0 && div_left >= 0)
	{
  	printf("<SPAN ID='image" FORMAT_DBRECORD_STR "' style='position: absolute; top: %d; left: %d; visibility: visible;'>", db_list_getcur(), div_top, div_left);
	}

#if DEBUG_SHOW_COORDINATES
mn_snprintf(debug_coordinates, sizeof(debug_coordinates), " Left:%d Top:%d", div_left, div_top);
#endif

if(db_list->dli_floor_plan_type == FLOORPLAN_SIGN_UP)
	{
	if(fp_edit_mode || (CheckValidDestination(db_list->dli_associated_sign_group)
		&& cgi_check_sign_access(CurrentUserPin) == SIGN_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this sign.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this sign location - %s.'); else SetHelpText(0, 0, 'Send a message to the sign group %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_sign);

		if(cgi_floorplan_sign_status(db_list->dli_associated_sign_group) == DEVICE_CONNECTION_CLOSED)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_off.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_up_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_up.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SIGN_DOWN)
	{
	if(fp_edit_mode || (CheckValidDestination(db_list->dli_associated_sign_group)
		&& cgi_check_sign_access(CurrentUserPin) == SIGN_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this sign.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this sign location - %s.'); else SetHelpText(0, 0, 'Send a message to the sign group %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_sign);

		if(cgi_floorplan_sign_status(db_list->dli_associated_sign_group) == DEVICE_CONNECTION_CLOSED)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_off.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_down_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_down.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SIGN_LEFT)
	{
	if(fp_edit_mode || (CheckValidDestination(db_list->dli_associated_sign_group)
		&& cgi_check_sign_access(CurrentUserPin) == SIGN_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this sign.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this sign location - %s.'); else SetHelpText(0, 0, 'Send a message to the sign group %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_sign);

		if(cgi_floorplan_sign_status(db_list->dli_associated_sign_group) == DEVICE_CONNECTION_CLOSED)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_off.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_left_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_left.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SIGN_RIGHT)
	{
	if(fp_edit_mode || (CheckValidDestination(db_list->dli_associated_sign_group)
		&& cgi_check_sign_access(CurrentUserPin) == SIGN_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this sign.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this sign location - %s.'); else SetHelpText(0, 0, 'Send a message to the sign group %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_sign);

		if(cgi_floorplan_sign_status(db_list->dli_associated_sign_group) == DEVICE_CONNECTION_CLOSED)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_off.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_right_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sign_right.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_UP)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);

		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_up_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_up.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_DOWN)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);

		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_down_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_down.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_LEFT)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);

		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_left_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_left.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_RIGHT)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);

		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_right_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_right.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);
		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_group_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_group.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP_UP)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);
		if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_group_up_blinking.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_group_up.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_all_call.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL_EMERGENCY)
	{
	if(fp_edit_mode || (CheckValidAudioGroup(db_list->dli_associated_sign_group)
		&& cgi_check_speaker_access(CurrentUserPin) == SPEAKER_ACCESS_OK))
		{
		db_list_setcur(list_cur);
		remove_trailing_space(db_list->dli_associated_sign_group);
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this speaker.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this speaker location - %s.'); else SetHelpText(0, 0, 'Send a message to the speaker group %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_speaker);
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_speaker_all_call_emergency.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_PHONE)
	{
	char full_name[100] = "";

	if(fp_edit_mode || cgi_check_person_access(db_list->dli_pin, CurrentUserPin) == PERSON_ACCESS_OK)
		{
		if(db_staff_pin_valid(db_list->dli_pin))
			{
			lm_format_fullname(full_name, sizeof(full_name), db_staf->dss_first_name_nocase, db_staf->dss_last_name_nocase);
			}
	
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this phone.'); else if(GetCallMode()) SetHelpText(0, 0, 'Communicate with %s.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this phone location - %s.'); else SetHelpText(0, 0, 'Send a message to the phone for %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, full_name, full_name, full_name, debug_coordinates, unique_name, unique_name);
		printf("%s", onclick_phone);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_phone.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_LINK_UP)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to goto %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_link_up.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_LINK_DOWN)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to goto %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_link_down.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_LINK_LEFT)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to goto %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_link_left.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_LINK_RIGHT)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to goto %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_link_right.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_ZOOM_IN)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to zoom in to %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_zoom_in_blinking.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_ZOOM_OUT)
	{
	if(fp_edit_mode || cgi_check_person_access_map(db_list->dli_associated_sign_group))
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this link.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this link - %s.'); else SetHelpText(0, 0, 'Click to zoom out to %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_link);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_zoom_out_blinking.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_PULL_STATION)
	{
	int pull_station_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		pull_station_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || pull_station_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this pull station.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this pull station - %s.'); else SetHelpText(0, 0, 'This is a pull station - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a pull station.');\
				SetHelpText(0, 0, 'This is a pull station - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_pull_station);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_pull_station_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_pull_station_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_pull_station_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map, as well as various other features*/
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_WATER_FLOW)
	{
	int water_flow_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		water_flow_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || water_flow_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this water flow valve.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this water flow valve - %s.'); else SetHelpText(0, 0, 'This is a water flow valve - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a water flow valve.');\
				SetHelpText(0, 0, 'This is a water flow valve - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_water_flow);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_water_flow_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_water_flow_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_water_flow_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_DUCT_DETECTOR)
	{
	int duct_detector_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		duct_detector_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || duct_detector_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this duct detector.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this duct detector - %s.'); else SetHelpText(0, 0, 'This is a duct detector - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a duct detector.');\
				SetHelpText(0, 0, 'This is a duct detector - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_duct_detector);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_duct_detector_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_duct_detector_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(fp_edit_mode)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_duct_detector_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_duct_detector_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SMOKE_DETECTOR)
	{
	int smoke_detector_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		smoke_detector_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || smoke_detector_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this smoke detector.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this smoke detector - %s.'); else SetHelpText(0, 0, 'This is a smoke detector - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a smoke detector.');\
				SetHelpText(0, 0, 'This is a smoke detector - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_smoke_detector);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_smoke_detector_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_smoke_detector_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_smoke_detector_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_MOTION_DETECTOR)
	{
	int motion_detector_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		motion_detector_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || motion_detector_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this smoke detector.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this smoke detector - %s.'); else SetHelpText(0, 0, 'This is a smoke detector - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a motion detector.');\
				SetHelpText(0, 0, 'This is a motion detector - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_motion_detector);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_motion_detector_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_motion_detector_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_motion_detector_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_SOUND_METER)
	{
	int sound_detector_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		sound_detector_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || sound_detector_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this smoke detector.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this smoke detector - %s.'); else SetHelpText(0, 0, 'This is a smoke detector - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a sound meter.');\
				SetHelpText(0, 0, 'This is a sound meter - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_sound_meter);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sound_meter_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sound_meter_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_sound_meter_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_FIRE_EXTINGUISHER)
	{
	int fire_extinguisher_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		fire_extinguisher_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || fire_extinguisher_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this fire extinguisher.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this fire extinguisher - %s.'); else SetHelpText(0, 0, 'This is a fire extinguisher - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a fire extinguisher.');\
				SetHelpText(0, 0, 'This is a fire extinguisher - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_fire_extinguisher);

		printf("><IMG id='cell%d' name='cell%d' src='%s/map_fire_extinguisher.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_DOOR_OPEN)
	{
	int door_open_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		door_open_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || door_open_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
//		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this door open.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this door open - %s.'); else SetHelpText(0, 0, 'This is a door open - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR ADDED -modified the above two lines to the following: */
		printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"\
			mouseover_hold.src = document.forms[0].cell%d.src;\
			if(GetEditItemMode()){\
				popRouteHelpMouse(this,'%s<br>Click to edit this item.');\
				SetHelpText(0, 0, 'Click to edit this item - %s.');\
				}\
			else if(GetDeleteMode()){\
				popRouteHelpMouse(this,'%s<br>Click to delete this item.');\
				SetHelpText(0, 0, 'Click to delete this item - %s.');\
				}\
			else if(GetLsdMode() && numberOfMapsThatExistFor('%s')>1){\
				popRouteHelpMouse(this,'%s<br>Click to select the<br>active map for this item.');\
				}\
			else if(GetEditMode()){\
				popRouteHelpMouse(this,'%s<br>Click to move this item.');\
				SetHelpText(0, 0, 'Click to move this item - %s.');\
				}\
			else{\
				popRouteHelpMouse(this,'%s<br>This is a door open.');\
				SetHelpText(0, 0, 'This is a door open - %s.');\
				}\
			document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
			return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
/* CR END */
		printf("%s", onclick_door_open);

		if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_ALARM)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_door_open_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(db_hard->dhc_device_in_alarm_trouble_clear == DEVICE_STATE_TROUBLE)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_door_open_trouble.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(fp_edit_mode)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_door_open_alarming.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_door_open_idle.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		/* CR ADD to attempt support for auto-refresh after placing a new life safety device icon */
		printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by clickLsd() function to determine if there are any unidentified/unsaved life safety devices placed on the map */
		/* CR END */
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_BUTTON)
	{
	int button_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		button_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || button_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this button.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this button - %s.'); else SetHelpText(0, 0, 'Click to simulate a message from this button - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_button);
	
		if(cgi_check_button_history(db_list->dli_associated_sign_group, 1*60))
			{
			/* button has been pressed less than 1 minutes */
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_button_blink_new_msg.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(cgi_check_button_history(db_list->dli_associated_sign_group, 5*60))
			{
			/* button has been pressed less than 5 minutes */
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_button_blink_active.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_button_blink_inactive.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_PERSON)
	{
	char full_name[100] = "";
	char return_msg[MSG_NAME_LENGTH] = "";

	if(fp_edit_mode || cgi_check_person_access(db_list->dli_pin, CurrentUserPin) == PERSON_ACCESS_OK)
		{
		if(db_staff_pin_valid(db_list->dli_pin))
			{
			lm_format_fullname(full_name, sizeof(full_name), db_staf->dss_first_name_nocase, db_staf->dss_last_name_nocase);
			cgi_determine_personal_page_message(return_msg, sizeof(return_msg));
			}
	
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"person_page_message='%s'; mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this person.'); else if(GetCallMode()) SetHelpText(0, 0, 'Communicate with %s'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this person - %s.'); else SetHelpText(0, 0, 'Send a message to %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", return_msg, unique_name, full_name, full_name, full_name, unique_name, unique_name);
		printf("%s", onclick_person);
	
		if(!strcmp(db_staf->dss_secur_class, security_class[SECURITY_LST])
			|| !strcmp(db_staf->dss_secur_class, security_class[SECURITY_LSI]))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_person_list.gif' >", unique_name, unique_name, cgi_icons);
			}
		else if(!strcmp(db_staf->dss_secur_class, security_class[SECURITY_JOB]))
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_person_job.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_person.gif' >", unique_name, unique_name, cgi_icons);
			}

		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_MESSAGE)
	{
	if(fp_edit_mode || (find_record_in_db_bann(db_list->dli_decode_help_launch_string, bb_valid_types[BBT_LF].string, db_list->dli_sister_audio_group)
		&& cgi_check_message_access(CurrentUserPin) == MESSAGE_ACCESS_OK))
		{
		db_list_setcur(list_cur);

		remove_trailing_space(db_list->dli_sister_audio_group);

		/* dont show the icon if the user cannot access it */
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this message.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this message - %s.'); else SetHelpText(0, 0, 'Click to launch the message %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_sister_audio_group, db_list->dli_sister_audio_group, unique_name, unique_name);
		printf("%s", onclick_message);
	
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_msg.gif' >", unique_name, unique_name, cgi_icons);
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_TEXT
	|| db_list->dli_floor_plan_type == FLOORPLAN_TEXT_RED
	|| db_list->dli_floor_plan_type == FLOORPLAN_TEXT_BLUE
	|| db_list->dli_floor_plan_type == FLOORPLAN_TEXT_YELLOW
	|| db_list->dli_floor_plan_type == FLOORPLAN_TEXT_GREEN)
	{
	char *color;

	remove_trailing_space(db_list->dli_decode_help_launch_string);

	switch(db_list->dli_floor_plan_type)
		{
		case FLOORPLAN_TEXT_RED:
			color = "red";
			break;

		case FLOORPLAN_TEXT_BLUE:
			color = "blue";
			break;

		case FLOORPLAN_TEXT_YELLOW:
			color = "yellow";
			break;

		case FLOORPLAN_TEXT_GREEN:
			color = "green";
			break;

		default:
			color = "black";
			break;
		}

	if(fp_edit_mode)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); return false;\" ");
		printf(" onmouseover=\"if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this text.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this text.'); return true;\"");
		printf("%s", onclick_text);
		printf(">");
		}

	printf("<TABLE border=0><TR><TD nowrap>");

	/* this is here so we can move the text */
	printf("<IMG style=\"display: none; visibility: hidden; \" id='cell%d' name='cell%d' src='%s/map_letter_t_blue.gif' >", unique_name, unique_name, cgi_icons);

	printf("<FONT face=\"Arial,Helvetica\" size=\"2\" color=\"%s\">%s</FONT>", color, db_list->dli_decode_help_launch_string);
	printf("</TD></TR></TABLE>");

	if(fp_edit_mode)
		{
		printf("</A>");
		}

	valid_span = TRUE;
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_CAMERA)
	{
	int camera_access = FALSE;

	DBRECORD list_cur = db_list_getcur();

	if(DistributionFindListName(db_hard->dhc_who_can_use)
		&& (DistributionPersonInListName(CurrentUserPin, db_hard->dhc_who_can_use) == DL_EXCLUDE))
		{
		/* no access */
		}
	else
		{
		camera_access = TRUE;
		}

	db_list_setcur(list_cur);

	if(fp_edit_mode || camera_access)
		{
		printf("<A HREF=\"javascript: OperationComplete()\" ");
		printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
		printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this camera.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this camera - %s.'); else SetHelpText(0, 0, 'Click to view this camera - %s.'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, unique_name, unique_name);
		printf("%s", onclick_camera);

		if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE))
			{
			if(db_hard->dhc_device_connected == DEVICE_CONNECTION_ACTIVE)
				{
				if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
					{
					printf("><IMG id='cell%d' name='cell%d' src='%s/map_video_camera.gif' >", unique_name, unique_name, cgi_icons);
					}
				else
					{
					printf("><IMG id='cell%d' name='cell%d' src='%s/map_video_camera.gif' >", unique_name, unique_name, cgi_icons);
					}
				}
			else
				{
				if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
					{
					printf("><IMG id='cell%d' name='cell%d' src='%s/map_video_camera.gif' >", unique_name, unique_name, cgi_icons);
					}
				else
					{
					printf("><IMG id='cell%d' name='cell%d' src='%s/map_video_camera.gif' >", unique_name, unique_name, cgi_icons);
					}
				}
			}
		else if(fp_edit_mode)
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_video_camera.gif' >", unique_name, unique_name, cgi_icons);
			}
		else
			{
			printf("><IMG id='cell%d' name='cell%d' src='%s/map_invisible.gif' >", unique_name, unique_name, cgi_icons);
			}
		printf("</A>");
		valid_span = TRUE;
		}
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_MEDIAPORT)
	{
	printf("<A HREF=\"javascript: OperationComplete()\" ");
/* CR ADDED -modified */
	printf(" onmouseout=\"CancelHelpText(); unpopRouteHelpMouse(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
//	printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this MediaPort.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this Mediaport - %s.'); else SetHelpText(0, 0, 'Click to send a message to this MediaPort - %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
	printf(" onmouseover=\"\
		mouseover_hold.src = document.forms[0].cell%d.src;\
		if(GetAddRouteMode()\
			&& document.forms[0].routeMediaportOrigin.value == '')\
			{\
			popRouteHelpMouse(this.parentNode,'%s<br>Click this LCD to begin drawing routes');\
			}\
		else if(GetShowRouteMode())\
			{\
			popRouteHelpMouse(this,'%s<br>Click this LCD to show its existing route(s)');\
			}\
		else if(GetEditItemMode())\
			{\
			popRouteHelpMouse(this,'Click to edit this item.');\
			SetHelpText(0, 0, 'Click to edit this item.');\
			}\
		else if(GetDeleteMode())\
			{\
			popRouteHelpMouse(this,'Delete this MediaPort.');\
			SetHelpText(0, 0, 'Delete this MediaPort.');\
			}\
		else if(GetEditMode())\
			{\
			popRouteHelpMouse(this,'Click to move this Mediaport - %s.');\
			SetHelpText(0, 0, 'Click to move this Mediaport - %s.');\
			}\
		else\
			{\
			popRouteHelpMouse(this,'Click to send a message to this Mediaport - %s.%s');\
			SetHelpText(0, 0, 'Click to send a message to this MediaPort - %s.%s');\
			}\
		document.forms[0].cell%d.src=document.forms[0].cell%d.src;\
		return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
/* CR END */
	printf("%s", onclick_mediaport);

	if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE))
		{
		if(mediaport_translation_fp
			&& HardwareTypeIsMediaPortGraphicalAnnunciator(db_hard->dhc_device_type))
			{
			/* only translate graphical annunciator devices */
			fprintf(mediaport_translation_fp, " translation_id[%d]=\"cell%d\"; translation_deviceid[%d]=\"%s\";\n", translation_mediaport_index, unique_name, translation_mediaport_index, db_hard->dhc_deviceid);
			translation_mediaport_index++;
			}

		if(db_hard->dhc_device_connected == DEVICE_CONNECTION_ACTIVE)
			{
			if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_connected_blinking.gif' >", unique_name, unique_name, cgi_icons);
				}
			else
				{
				/* printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_connected.gif' >", unique_name, unique_name, cgi_icons); */  /* CR ADDED - modified with below */
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_connected.gif' ><script type=\"text/javascript\">registerLCDiconImg(\"cell%d\",\"%s\");</script>", unique_name, unique_name, cgi_icons, unique_name, db_hard->dhc_deviceid);
				}
			}
		else
			{
			if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_not_connected_blinking.gif' >", unique_name, unique_name, cgi_icons);
				}
			else
				{
				/* printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_not_connected.gif' >", unique_name, unique_name, cgi_icons); */  /* CR ADDED - modified with below */
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_not_connected.gif' ><script type=\"text/javascript\">registerLCDiconImg(\"cell%d\",\"%s\");</script>", unique_name, unique_name, cgi_icons, unique_name, db_hard->dhc_deviceid);
				}
			}
		}
	else if(fp_edit_mode)
		{
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_mediaport_not_connected.gif' >", unique_name, unique_name, cgi_icons);
		}
	else
		{
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_invisible.gif' >", unique_name, unique_name, cgi_icons);
		}

	printf("</A>");
/* CR ADD to attempt support for auto-refresh after placing a new LCD icon */
	printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by SetRouteMode function to determine if there are any unidentified/unsaved LCD devices placed on the map */
/* CR END */
	valid_span = TRUE;
	}
else if(db_list->dli_floor_plan_type == FLOORPLAN_PC_ALERT)
	{
	printf("<A HREF=\"javascript: OperationComplete()\" ");
	printf(" onmouseout=\"CancelHelpText(); document.forms[0].cell%d.src=mouseover_hold.src; return false;\" ", unique_name);
	printf(" onmouseover=\"mouseover_hold.src=document.forms[0].cell%d.src; if(GetEditItemMode()) { SetHelpText(0, 0, 'Click to edit this item.'); } else if(GetDeleteMode()) SetHelpText(0, 0, 'Delete this PC Alert.'); else if(GetEditMode()) SetHelpText(0, 0, 'Click to move this PC Alert - %s.'); else SetHelpText(0, 0, 'Click to send a message to this PC Alert - %s.%s'); document.forms[0].cell%d.src=document.forms[0].cell%d.src; return true;\"", unique_name, db_list->dli_associated_sign_group, db_list->dli_associated_sign_group, debug_coordinates, unique_name, unique_name);
	printf("%s", onclick_pc_alert);

	if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE))
		{
		if(db_hard->dhc_device_connected == DEVICE_CONNECTION_ACTIVE)
			{
			if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_pc_alert_connected_blinking.gif' >", unique_name, unique_name, cgi_icons);
				}
			else
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_pc_alert_connected.gif' >", unique_name, unique_name, cgi_icons);
				}
			}
		else
			{
			if(cgi_check_record_in_multi_select(db_list_getcur(), NULL))
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_pc_alert_not_connected_blinking.gif' >", unique_name, unique_name, cgi_icons);
				}
			else
				{
				printf("><IMG id='cell%d' name='cell%d' src='%s/map_pc_alert_not_connected.gif' >", unique_name, unique_name, cgi_icons);
				}
			}
		}
	else if(fp_edit_mode)
		{
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_pc_alert_not_connected.gif' >", unique_name, unique_name, cgi_icons);
		}
	else
		{
		printf("><IMG id='cell%d' name='cell%d' src='%s/map_invisible.gif' >", unique_name, unique_name, cgi_icons);
		}

	printf("</A>");
	valid_span = TRUE;
	}

if(div_top >= 0 && div_left >= 0)
	{
	printf("</SPAN>");
	}

if(valid_span 
	&& mediaport_visible_list_fp
	&& locations_alert_status >= 0
	&& div_top >= 0 && div_left >= 0)
	{
	/* HIDE all icons for life safety until later */ 
	/* tell the device to NOT show until confirmed the device is a graphical annunciator */
	/* this is done with AJAX later using ShowGraphicalAnnunciatorIcons() */
	printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
	printf("document.getElementById('image" FORMAT_DBRECORD_STR"').style.display='none';", db_list_getcur());		/* values are none and block */
	fprintf(mediaport_visible_list_fp, " visible_list[%d]=\"image" FORMAT_DBRECORD_STR"\";\n", visible_list_index, db_list_getcur());
	visible_list_index++;
	printf("</SCRIPT>");
	}

/* restore last position */
db_list_select(list_cur_key);
db_list_setcur(list_cur);

return(0);
}

/*****************************************************
** void deactivateAllFloorplansInSupportOfLifeSafetyDevice(void)
**
**	This will go through each and every life-safety-device record (for a specific device) 
**	and set its floorplan-active flag to 0. Since the floorplan name is part of that record,
**	doing this will essentially say that the floorplan should not show on a GA 
**	if the device goes into an alarm state.
**
**	NOTE: The active flag is stored in the dli_font_size field.
**
*****************************************************/
void deactivateAllFloorplansInSupportOfLifeSafetyDevice(void)
{
	char lsdnametodeactivate[DEVICEID_LENGTH];
	int next;

	cgi_parsed_lookup("lsdnametodeactivate=", lsdnametodeactivate, sizeof(lsdnametodeactivate));

	db_list_select(8);		/* key with device name, floorplan name */
	strcpy(db_list->res_id, res_id);
	strcpysl(db_list->dli_associated_sign_group, lsdnametodeactivate, sizeof(db_list->dli_associated_sign_group));
	strcpy(db_list->dli_name, "");				/* all floorplans */

	next = db_list_find();

	while(next > 0)
		{

		/* note:
			dli_floor_plan_type = type of device
			dli_associated_sign_group = device id
			dli_font_size = active flag
		*/

		if(strcmp_ignore_trailing_space(db_list->dli_associated_sign_group, lsdnametodeactivate) == 0)
			{
			if(db_list->dli_floor_plan_type == FLOORPLAN_PULL_STATION
				|| db_list->dli_floor_plan_type == FLOORPLAN_WATER_FLOW
				|| db_list->dli_floor_plan_type == FLOORPLAN_DUCT_DETECTOR
				|| db_list->dli_floor_plan_type == FLOORPLAN_SMOKE_DETECTOR
				|| db_list->dli_floor_plan_type == FLOORPLAN_MOTION_DETECTOR
				|| db_list->dli_floor_plan_type == FLOORPLAN_DOOR_OPEN)
				{
				db_list->dli_font_size = 0;		/* set active flag to 0 */
				db_list_write();
				}
			}

		next = db_list_next();
		}

}

/*****************************************************
** void activateItemByRecNo(void)
**
**	Set the active map for a life safety device
**
*****************************************************/
void activateItemByRecNo(void)
{
DBRECORD record_number = cgi_parsed_lookup_record_number("record_number=", 0);		/* parse the record number that was posted */

if(db_list_setcur(record_number) < 0)							/* this is where we actually set the currency, but also continue testing conditions below (as long as the record exists, that is) */
	{
	printf("<HTML>");
	printf("<BODY>");
	printf("<FORM action=%s method=post>", cgi_server);
	printf("Invalid record number " FORMAT_DBRECORD_STR "", record_number);
	printf("</FORM>");
	printf("</BODY>");
	printf("</HTML>");
	}
else											/* else, the record was able to be set to current, so... */
	{
	db_list->dli_font_size = 1;								/* set the current record to be marked as active */

	if(db_list_write() > 0)									/* try to save the change to the record */
		{
		cgi_logging(DB_LOCATIONS_MANAGER, 0, DB_ISAM_WRIT, "updated life-safety item active flag");
		}

	}
}

/***** REDACTED *****/

/* CR ADDED */
#define ROUTE_ACTIVE_FLAG_OFF	(0)
#define ROUTE_ACTIVE_FLAG_ON	(1)	
#define ROUTE_PATHCOLOR_LENGTH	(9)			/* a size is specified that should be large enough to contain a color representative string (e.g. '#003366') */
#define ROUTE_COORDS_LENGTH	(5000)			/* an arbitrary size is specified that should be large enough to contain anticipated points - make larger if needed (a single coordinate pair should average about 6-10 characters with delimiters) e.g. 512 should allow about 52-85 total points
							 * NOTE: factor of character-to-points is 10 (ex. 500 = 50pts, 5000 = 500pts, etc.)... */
char route_msg_name[MSG_NAME_LENGTH] = "";
char route_msg_directory[DIRECTORY_LENGTH] = "";
char route_mediaport_origin[DEVICEID_LENGTH];		/* to be included as part of the route record */
char route_mediaport_origin_old[DEVICEID_LENGTH];	/* to be included as part of the route record */
char route_floorplan_name[FLOORPLAN_NAME_LENGTH];	/* to be included as part of the route record */
char route_floorplan_name_old[FLOORPLAN_NAME_LENGTH];	/* to be included as part of the route record */
int route_active_flag;					/* to be included as part of the route record */
int route_active_flag_old;				/* to be included as part of the route record */
int route_thickness;					/* an integer representing the approximate pixel width of the route */
int route_thickness_old;				/* an integer representing the approximate pixel width of the route */
char route_path_color[ROUTE_PATHCOLOR_LENGTH];
char route_path_color_old[ROUTE_PATHCOLOR_LENGTH];
char route_coordinates_string[ROUTE_COORDS_LENGTH];
char route_coordinates_string_old[ROUTE_COORDS_LENGTH];
char route_user[PIN_LENGTH];				/* to be included as part of the route record so we know who was working with a route */
char route_dtsec[DTSEC_LENGTH];				/* to be included as part of the route record to help ascertain its uniqueness */
char route_last_submit_reason[32];
char state_change_message[32];				/* this temporarily contains a message that will display whenever a submit action takes place (ex. Saved) */
char selected_active_floorplan[FLOORPLAN_NAME_LENGTH];

/***********************************************************
** int change_active_status_for_all_routes_for(device_id, floorplan_name, active_flag)
**
**	Changes the active flag for all routes pertaining to the specified device and floorplan
**	
**	Returns...
**	-1	no route records changed
**	 0	default
**	 1	route records changed
**
***********************************************************/	
int change_active_status_for_all_routes_for(char device_id[DEVICEID_LENGTH], char floorplan_name[FLOORPLAN_NAME_LENGTH], int active_flag)
{

int return_value = 0;
char str_routeStoreFilename[256];
char str_routeStoreFilenameTemp[256];
char line_buffer[sizeof(route_coordinates_string)+BUFSIZ];
char *routeTokenPtr;
char recordLine_original[sizeof(route_coordinates_string)+BUFSIZ];
char recordLine_modified[sizeof(route_coordinates_string)+BUFSIZ];
char iDeviceID[DEVICEID_LENGTH];								/* variable intended to temporarily store a particular iteration's device id */
char iFloorplanName[FLOORPLAN_NAME_LENGTH];							/* variable intended to temporarily store a particular iteration's floorplan name */

FILE *file_msgRoutes_log = NULL;
FILE *file_msgRoutes = NULL;
FILE *file_msgRoutesTemp = NULL;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("change_active_status_for_all_routes_for():  Change active status operation started by '%s' for message '%s'...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));

/* Prepare a log file specific to this function... */
file_msgRoutes_log = fopen("/home/silentm/record/routes/changeActiveStatus.log", "w");
fprintf(file_msgRoutes_log, "\n*** LOG FOR MOST RECENT CHANGE-ACTIVE-STATUS OPERATION ***\n");
fprintf(file_msgRoutes_log, "Argument:  device_id = \"%s\"\n", remove_trailing_space(device_id));
fprintf(file_msgRoutes_log, "Argument:  floorplan_name = \"%s\"\n", remove_trailing_space(floorplan_name));
fprintf(file_msgRoutes_log, "Argument:  active_flag = \"%d\"\n", active_flag);

/* Prepare original route-record file... */
sprintf(str_routeStoreFilename, "/home/silentm/record/routes/%s.%s.routes", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Original route-records filename determined: %s\n", str_routeStoreFilename);

/* Prepare a temporary route-record file... */
sprintf(str_routeStoreFilenameTemp, "/home/silentm/record/routes/%s.%s.routes.temp", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilenameTemp);
fprintf(file_msgRoutes_log, "- Temporary route-records filename determined: %s\n", str_routeStoreFilenameTemp);

/* Work with the route-records... */
file_msgRoutes = fopen(str_routeStoreFilename, "r");
if(file_msgRoutes)										/* if the route-records file opened successfully, then... */
	{
	fprintf(file_msgRoutes_log, "- Opened routes file for reading\n");
	file_msgRoutesTemp = fopen(str_routeStoreFilenameTemp, "w");
	fprintf(file_msgRoutes_log, "- Opened temporary routes file for writing\n");
	fprintf(file_msgRoutes_log, "- Now going to loop through each record line...\n");
	while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutes))					/* looping through each line of the route-records file... */
		{
		strcpy(recordLine_original, line_buffer);							/* get the original version of the line from the buffer, before it's tokenized */
		fprintf(file_msgRoutes_log, "\n   %s", recordLine_original);
		routeTokenPtr = strtok(line_buffer, ",");							/* go to the part of the record-line up to the first comma delimiter (equates to device id) */
		strcpysl(iDeviceID, routeTokenPtr, DEVICEID_LENGTH);						/* store this record's device id into the temporary iteration variable */
		routeTokenPtr = strtok(NULL, ",");								/* go to the part of the record-line up to the second comma delimiter (equates to floorplan name) */
		strcpysl(iFloorplanName, routeTokenPtr, FLOORPLAN_NAME_LENGTH);					/* store this record's floorplan name into the temporary iteration variable */
		routeTokenPtr = strtok(NULL, ",");								/* go to the part of the record-line up to the third comma delimiter (equates to active flag) */
		routeTokenPtr = strtok(NULL, "\n");								/* go to the part of the record-line up to the end of the line (equates to everything afterward) */
		if(strcmp_ignore_trailing_space(iDeviceID, device_id) == 0					/* if this record pertains to the specified device and floorplan, then... */
			&& strcmp_ignore_trailing_space(iFloorplanName, floorplan_name) == 0)
			{
			fprintf(file_msgRoutes_log, "   - This route-record matches the specified device \"%s\" and floorplan \"%s\"", remove_trailing_space(device_id), remove_trailing_space(floorplan_name));
			fprintf(file_msgRoutes_log, "  (will now set its active flag to \"%d\" and save to the temporary file)\n", active_flag);
			sprintf(recordLine_modified, "%s,%s,%d,%s\n", iDeviceID, iFloorplanName, active_flag, routeTokenPtr);	/* routeTokenPtr should now contain the remaining part of the line after the first three fields */
			fprintf(file_msgRoutesTemp, "%s", recordLine_modified);
			fprintf(file_msgRoutes_log, "       Original ->  %s", recordLine_original);
			fprintf(file_msgRoutes_log, "       Modified ->  %s", recordLine_modified);
			return_value = 1;
			}
		else												/* else for all other non-applicable records, we don't need to make any changes to the active flag... */
			{
			fprintf(file_msgRoutes_log, "   - This route-record does not match the specified device \"%s\" and floorplan \"%s\"  (will now save it AS-IS to the temporary file, making no changes)\n", remove_trailing_space(device_id), remove_trailing_space(floorplan_name));
			fprintf(file_msgRoutesTemp, "%s", recordLine_original);					/* save this existing line to the temporary file, in its original state, as-is */
			}
		routeTokenPtr = strtok(NULL, "\n");							/* get the part of the record-line up to but excluding the new-line character */
		}
	fclose(file_msgRoutes);										/* close the route-records file */
	fprintf(file_msgRoutes_log, "\n- Closed routes file\n");
	fclose(file_msgRoutesTemp);									/* close the temporary route-records file */
	fprintf(file_msgRoutes_log, "- Closed temporary routes file\n");
	}
else												/* else, the route-records file did NOT open successfully, so... */
	{
	fprintf(file_msgRoutes_log, "- Failed to open routes file\n");
	return_value = -1;										/* return an appropriate response */
	}

/* Rename the temporary file to that of the original, thereby overwriting the original with the new and updated version */
rename(str_routeStoreFilenameTemp, str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Temporary file renamed/overwritten to original: %s -> %s\n", str_routeStoreFilenameTemp, str_routeStoreFilename);
	
/* Cleanup and completion... */
fprintf(file_msgRoutes_log, "*** CHANGE-ACTIVE-STATUS OPERATION COMPLETE ***\n\n");
fclose(file_msgRoutes_log);									/* close the log file */
printf("autoEnterRouteMode=1;");
strcpy(state_change_message, "Active Floorplan Updated");					/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
return return_value;										/* return the result */
}

/***********************************************************
** void change_active_status_for_all_routes_except_floorplan(device_id, floorplan_name, active_flag)
**
**	Changes the active flag for all routes pertaining to the specified device but not the specified floorplan
**
***********************************************************/	
void change_active_status_for_all_routes_except_floorplan(char device_id[DEVICEID_LENGTH], char floorplan_name[FLOORPLAN_NAME_LENGTH], int active_flag)
{

char str_routeStoreFilename[256];
char str_routeStoreFilenameTemp[256];
char line_buffer[sizeof(route_coordinates_string)+BUFSIZ];
char *routeTokenPtr;
char recordLine_original[sizeof(route_coordinates_string)+BUFSIZ];
char recordLine_modified[sizeof(route_coordinates_string)+BUFSIZ];
char iDeviceID[DEVICEID_LENGTH];								/* variable intended to temporarily store a particular iteration's device id */
char iFloorplanName[FLOORPLAN_NAME_LENGTH];							/* variable intended to temporarily store a particular iteration's floorplan name */
int iActiveFlag;

FILE *file_msgRoutes_log = NULL;
FILE *file_msgRoutes = NULL;
FILE *file_msgRoutesTemp = NULL;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("change_active_status_for_all_routes_except_floorplan():  Change active status operation started by '%s' for message '%s'...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));

/* Prepare a log file specific to this function... */
file_msgRoutes_log = fopen("/home/silentm/record/routes/changeActiveStatusInverse.log", "w");
fprintf(file_msgRoutes_log, "\n*** LOG FOR MOST RECENT CHANGE-ACTIVE-STATUS-FOR-INVERSE OPERATION ***\n");
fprintf(file_msgRoutes_log, "Parameter:  device_id = \"%s\"\n", remove_trailing_space(device_id));
fprintf(file_msgRoutes_log, "Parameter:  floorplan_name = \"%s\"\n", remove_trailing_space(floorplan_name));

/* Prepare original route-record file... */
sprintf(str_routeStoreFilename, "/home/silentm/record/routes/%s.%s.routes", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Original route-records filename determined: %s\n", str_routeStoreFilename);

/* Prepare a temporary route-record file... */
sprintf(str_routeStoreFilenameTemp, "/home/silentm/record/routes/%s.%s.routes.temp", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilenameTemp);
fprintf(file_msgRoutes_log, "- Temporary route-records filename determined: %s\n", str_routeStoreFilenameTemp);

/* Work with the route-records... */
file_msgRoutes = fopen(str_routeStoreFilename, "r");
if(file_msgRoutes)										/* if the route-records file opened successfully, then... */
	{
	fprintf(file_msgRoutes_log, "- Opened routes file for reading\n");
	file_msgRoutesTemp = fopen(str_routeStoreFilenameTemp, "w");
	fprintf(file_msgRoutes_log, "- Opened temporary routes file for writing\n");
	fprintf(file_msgRoutes_log, "- Now going to loop through each record line...\n");
	while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutes))					/* looping through each line of the route-records file... */
		{
		strcpy(recordLine_original, line_buffer);							/* get the original version of the line from the buffer, before it's tokenized */
		routeTokenPtr = strtok(line_buffer, ",");							/* go to the part of the record-line up to the first comma delimiter (equates to device id) */
		strcpysl(iDeviceID, routeTokenPtr, DEVICEID_LENGTH);						/* store this record's device id into the temporary iteration variable */
		routeTokenPtr = strtok(NULL, ",");								/* go to the part of the record-line up to the second comma delimiter (equates to floorplan name) */
		strcpysl(iFloorplanName, routeTokenPtr, FLOORPLAN_NAME_LENGTH);					/* store this record's floorplan name into the temporary iteration variable */
		routeTokenPtr = strtok(NULL, ",");								/* go to the part of the record-line up to the third comma delimiter (equates to active flag) */
		iActiveFlag = atoi(routeTokenPtr);								/* store the record's active flag into the temporary iteration variable */
		routeTokenPtr = strtok(NULL, "\n");								/* go to the part of the record-line up to the end of the line (equates to everything afterward) */
		if(strcmp_ignore_trailing_space(iDeviceID, device_id) == 0					/* if this record pertains to the specified device but for another floorplan, then... */
			&& strcmp_ignore_trailing_space(iFloorplanName, floorplan_name) != 0)
			{
			fprintf(file_msgRoutes_log, "- This route-record matches the specified device \"%s\" but is for a floorplan other than \"%s\"", remove_trailing_space(device_id), remove_trailing_space(floorplan_name));
			fprintf(file_msgRoutes_log, "  (will now invert its active flag from \"%d\" to \"%d\" and save to the temporary file)\n", iActiveFlag, active_flag);
			sprintf(recordLine_modified, "%s,%s,%d,%s\n", iDeviceID,iFloorplanName,active_flag,routeTokenPtr);/* line buffer should now contain the remaining part of the line after the first three fields */
			fprintf(file_msgRoutesTemp, "%s", recordLine_modified);
			fprintf(file_msgRoutes_log, "    Original ->  %s", recordLine_original);
			fprintf(file_msgRoutes_log, "    Modified ->  %s", recordLine_modified);
			}
		else												/* else for all other non-applicable records, we don't need to make any changes to the active flag... */
			{
			fprintf(file_msgRoutes_log, "- This route-record does not match the specified device \"%s\" or it matches floorplan \"%s\"\n  (will now save it AS-IS to the temporary file, making no changes)\n", remove_trailing_space(device_id), remove_trailing_space(floorplan_name));
			fprintf(file_msgRoutesTemp, "%s", recordLine_original);						/* save this existing line to the temporary file, in its original state, as-is */
			}
		routeTokenPtr = strtok(NULL, "\n");								/* get the part of the record-line up to but excluding the new-line character */
		}
	fclose(file_msgRoutes);										/* close the route-records file */
	fprintf(file_msgRoutes_log, "\n- Closed routes file\n");
	fclose(file_msgRoutesTemp);									/* close the temporary route-records file */
	fprintf(file_msgRoutes_log, "- Closed temporary routes file\n");
	}
else												/* else, the route-records file did NOT open successfully, so... */
	{
	fprintf(file_msgRoutes_log, "- Failed to open routes file\n");
	}

/* Rename the temporary file to that of the original, thereby overwriting the original with the new and updated version */
rename(str_routeStoreFilenameTemp, str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Temporary file renamed/overwritten to original: %s -> %s\n", str_routeStoreFilenameTemp, str_routeStoreFilename);
	
/* Cleanup and completion... */
fprintf(file_msgRoutes_log, "*** CHANGE-ACTIVE-STATUS-FOR-INVERSE OPERATION COMPLETE ***\n\n");
fclose(file_msgRoutes_log);									/* close the log file */
printf("autoEnterRouteMode=1;");
strcpy(state_change_message, "Active Floorplan Updated");					/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
}

/***********************************************************
** void do_add_route(void)
**
**	When the form is submitted, take care of adding to the route record file if necessary.
**
***********************************************************/	
void do_add_route(void)
{

char str_routeStoreFilename[256];
char line_buffer[sizeof(route_coordinates_string)+BUFSIZ];	/* instantiate and initialize a buffer for holding a line once it has been read from the file */
char *routeTokenPtr;						/* instantiate a place in memory for a pointer that will be used to refer to each delimited part of a line, excluding meta */
char tmpStrCmp[sizeof(route_coordinates_string)+BUFSIZ];	/* declare a temporary variable to hold some data that is being passed from the form posting, that will be used for comparison of existing records */
int matchFound = 0;

FILE *file_msgRoutes_log = NULL;
FILE *file_msgRoutes = NULL;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("do_add_route():  Add new route operation started by '%s' for message '%s'...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));

/* prepare a log file... */
file_msgRoutes_log = fopen("/home/silentm/record/routes/lastAddNewOperation.log", "w");
fprintf(file_msgRoutes_log, "\n*** LOG FOR MOST RECENT ADD-NEW-ROUTE OPERATION ***\n");

/* prepare file info... */
sprintf(str_routeStoreFilename, "/home/silentm/record/routes/%s.%s.routes", route_msg_directory, route_msg_name);  /* filename will consist of the message's directory and name */
remove_trailing_space(str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Filename determined: %s\n", str_routeStoreFilename);

/* first, open the file for reading (so we can check for matching - already existing - records before adding) */
sprintf(tmpStrCmp, "%s,%s,%d,%d,%s,%s", route_mediaport_origin, route_floorplan_name, route_active_flag, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string));
file_msgRoutes = fopen(str_routeStoreFilename, "r");		/* open the file for read-only */
if(file_msgRoutes)						/* if the file is successfully opened, then... */
	{
	while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutes))	/* looping through each line of the file... */
		{
		routeTokenPtr = strtok(line_buffer, ";");			/* get (refer to) the part of the line up to (but excluding) the semicolon character, so we can ignore user and dtsec data when matching (since not important and could cause bugs) */
		if(strcmp(routeTokenPtr, tmpStrCmp) == 0)			/* if what we're passing matches this line in the file, then... */
			{
			matchFound = 1;							/* flag that a match was found */
			fprintf(file_msgRoutes_log, "- Error: Data being passed matches an already-existing route");
			if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("do_add_route():  ERROR adding new route because the submitted route already exists in the file, \"%s\"", str_routeStoreFilename);
			}
		}
	fclose(file_msgRoutes);						/* close the file so it can be reopened later, possibly for appending */
	}

/* then, add the route (or not), depending on whether a matching route already exists or not */
if(!matchFound)	/* we are only going to append this route if no others like it already exist in the file */
	{
	fprintf(file_msgRoutes_log, "- Opening file for appending.... ");
	file_msgRoutes = fopen(str_routeStoreFilename, "a");
	if(file_msgRoutes)  /* if successfully opened (file exists), then proceed with appending new route data */
		{
		fprintf(file_msgRoutes_log, "SUCCESS\n- Writing the new route data to a new line in the file.... ");
		fprintf(file_msgRoutes, "%s,%s,%d,%d,%s,%s;%s,%s\n", route_mediaport_origin, route_floorplan_name, route_active_flag, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string), route_user, route_dtsec);  /* write the string to a line in the file */
		fprintf(file_msgRoutes_log, "DONE\n  (%s,%s,%d,%d,%s,%s) (USER: %s, DTSEC:%s)\n", route_mediaport_origin, route_floorplan_name, route_active_flag, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string), route_user, route_dtsec);
		fclose(file_msgRoutes);
		fprintf(file_msgRoutes_log, "- File closed, Operation complete\n\n");
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_6("                New route successfully added: %s,%s,%d,%d,%s,%s", route_mediaport_origin, route_floorplan_name, route_active_flag, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string));
		}
	else  /* else, do not proceed */
		{
		fprintf(file_msgRoutes_log,"FAILED\n  (Problem opening file, new route data was not saved)\n- Operation did NOT complete successfully (this should not happen merely because of a bug, but rather something major)\n\n");
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                Failed to open the file, '%s'", str_routeStoreFilename);
		}

	printf("autoEnterRouteMode=1;");
	if(strcmp(route_last_submit_reason, "SaveNewViaAutoSave") == 0)
		{
		strcpy(state_change_message, "New Route Automatically Saved");		/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
		}
	else
		{
		strcpy(state_change_message, "New Route Saved");			/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
		}
	}
else
	{
	printf("autoEnterRouteMode=1;");
	strcpy(state_change_message, "New Route Already Exists");	/* do not change this without also changing its corresponding condition-test toward the end of this file! */
	}

if(file_msgRoutes_log)
	{
	fclose(file_msgRoutes_log);
	}
}

/***********************************************************
** void do_delete_route(void)
**
**	When the form is submitted, take care of deleting from the route record file if necessary.
**
***********************************************************/	
void do_delete_route(void)
{

char routeLineToDelete[ROUTE_COORDS_LENGTH+BUFSIZ];  /* provide a bit of overkill to the length (will trim later) - make larger if needed */
char str_routeStoreFilename[256];
char str_routeStoreFilenameTemp[256];
int i = 0;
char *routeTokenPtr2;		/* instantiate a place in memory for a pointer that will be used to refer to each delimited part of a line */
char *routeTokenPtr2_ex;	/* instantiate a place in memory for a pointer that will be used to refer to each delimited part of a line, excluding meta */
char line_buffer[ROUTE_COORDS_LENGTH+BUFSIZ];	/* instantiate and initialize a buffer for holding a line once it has been read from the file - to be used in rewriting the line to the new file */
char line_buffer_ex[ROUTE_COORDS_LENGTH+BUFSIZ];	/* instantiate and initialize a buffer for holding a line (excluding user and dtsec) once it has been read from the file - to be used in matching */

FILE *file_msgRoutes_log = NULL;
FILE *file_msgRoutes = NULL;
FILE *file_msgRoutesTemp = NULL;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("do_delete_route() Route delete operation started by '%s' for message '%s'...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));

/* prepare a log file... */
file_msgRoutes_log = fopen("/home/silentm/record/routes/lastDeleteOperation.log", "w");
fprintf(file_msgRoutes_log, "\n*** LOG FOR MOST RECENT DELETE-EXISTING-ROUTE OPERATION ***\n");

/* prepare a variable containing the expected route string (must match what would be in the file so can be found in a compare operation)... */
sprintf(routeLineToDelete, "%s,%s,%d,%d,%s,%s", route_mediaport_origin, route_floorplan_name, route_active_flag, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string));  /* construct how the line in the file would look like, irrespective of user and dtsec */
remove_trailing_space(routeLineToDelete);  /* strip off the trailing overkill of spaces */
fprintf(file_msgRoutes_log, "Record to delete: \"%s\"\n", routeLineToDelete);

/* prepare file info... */
sprintf(str_routeStoreFilename, "/home/silentm/record/routes/%s.%s.routes", route_msg_directory, route_msg_name);
sprintf(str_routeStoreFilenameTemp, "/home/silentm/record/routes/%s.%s.routes.temp", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilename);
remove_trailing_space(str_routeStoreFilenameTemp);

/* open the original file for reading... */
file_msgRoutes = fopen(str_routeStoreFilename, "r");
fprintf(file_msgRoutes_log, "- Opened original file for reading: /home/silentm/record/routes/%s\n", str_routeStoreFilename);

/* open the temporary file for writing (it will ultimately replace the original file)... */
file_msgRoutesTemp = fopen(str_routeStoreFilenameTemp, "w");
fprintf(file_msgRoutes_log, "- Opened temporary file for writing: /home/silentm/record/routes/%s\n", str_routeStoreFilenameTemp);

/* iterate through each line of the original file and write its non-matching records to the temporary file... */
fprintf(file_msgRoutes_log, "- Iterating through each line of the file to determine what to save to the temporary file:\n");
while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutes))	/* looping through each complete line in the file... */
	{
	strcpy(line_buffer_ex, line_buffer);					/* copy the line to a temporary buffer that will be used for excluding the non-matching date */
	routeTokenPtr2 = strtok(line_buffer, "\n");				/* get (refer to) the part of the line up to (but excluding) the line-feed character, so we can reliably to a strcmp operation (remember to do this again right after using the data)*/
	routeTokenPtr2_ex = strtok(line_buffer_ex, ";");			/* get (refer to) the part of the line up to (but excluding) the semicolon character, so we can ignore user and dtsec data when matching */
	fprintf(file_msgRoutes_log, "    i=%d: \"%s\"  ", i, routeTokenPtr2);
	if(strcmp(routeTokenPtr2_ex,routeLineToDelete))
		{
		fprintf(file_msgRoutes_log, "   (SAVING)\n");
		fprintf(file_msgRoutesTemp, "%s\n", line_buffer); 
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                Route to keep:   %s", line_buffer_ex);
		i++;
		}
	else
		{
		fprintf(file_msgRoutes_log, "   (DELETING)  (USER: '%s', DTSEC: '%s')\n", route_user, route_dtsec);
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                Route to delete: %s", routeLineToDelete);
		i++;
		}
	routeTokenPtr2 = strtok(NULL, "\n");	/* advance the pointer to the end, so it can reset itself for the next record */
	}
/* close the files... */
fclose(file_msgRoutes);
fclose(file_msgRoutesTemp);
fprintf(file_msgRoutes_log, "- Original and Temporary files closed\n");

/* rename the temp file thereby overwriting the original... */
rename(str_routeStoreFilenameTemp, str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Temporary file renamed/overwritten to original: %s -> %s\n", str_routeStoreFilenameTemp, str_routeStoreFilename);

/* DEV NOTE: upgrade later to delete the file if no routes now exist... use remove(char *path) */
fprintf(file_msgRoutes_log, "- Operation complete\n\n");
fclose(file_msgRoutes_log);
printf("autoEnterRouteMode=1;");
strcpy(state_change_message, "Route Deleted");			/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
}

/***********************************************************
** void do_edit_route(void)
**
**	When the form is submitted, take care of editing the route record file if necessary.
**
***********************************************************/	
void do_edit_route(char route_mediaport_origin_old[DEVICEID_LENGTH], char route_mediaport_origin[DEVICEID_LENGTH], char route_floorplan_name[FLOORPLAN_NAME_LENGTH], int route_active_flag_old, int route_active_flag, int route_thickness_old, int route_thickness, char route_path_color_old[ROUTE_PATHCOLOR_LENGTH], char route_path_color[ROUTE_PATHCOLOR_LENGTH], char route_coordinates_string_old[ROUTE_COORDS_LENGTH], char route_coordinates_string[ROUTE_COORDS_LENGTH])
{

char routeLineToEditFrom[ROUTE_COORDS_LENGTH+BUFSIZ];  /* provide a bit of overkill to the length (will trim later) - make larger if needed */
char routeLineToEditTo[ROUTE_COORDS_LENGTH+BUFSIZ];  /* provide a bit of overkill to the length (will trim later) - make larger if needed */
char str_routeStoreFilename[256];
char str_routeStoreFilenameTemp[256];
int i = 0;
char *routeTokenPtr2;		/* instantiate a place in memory for a pointer that will be used to refer to each delimited part of a line */
char *routeTokenPtr2_ex;	/* instantiate a place in memory for a pointer that will be used to refer to each delimited part of a line, excluding meta */
char line_buffer[ROUTE_COORDS_LENGTH+BUFSIZ];	/* instantiate and initialize a buffer for holding a line once it has been read from the file */
char line_buffer_ex[ROUTE_COORDS_LENGTH+BUFSIZ];	/* instantiate and initialize a buffer for holding a line (excluding user and dtsec) once it has been read from the file - to be used in matching */

FILE *file_msgRoutes_log = NULL;
FILE *file_msgRoutes = NULL;
FILE *file_msgRoutesTemp = NULL;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("do_edit_route() Function called by '%s' for message '%s'...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));

/* prepare a log file... */
file_msgRoutes_log = fopen("/home/silentm/record/routes/lastEditOperation.log", "w");
fprintf(file_msgRoutes_log, "\n*** LOG FOR MOST RECENT EDIT-EXISTING-ROUTE OPERATION ***\n");

/* prepare a variable containing the expected route string (must match what would be in the file so can be found in a compare operation)... */
sprintf(routeLineToEditFrom, "%s,%s,%d,%d,%s,%s", route_mediaport_origin_old, route_floorplan_name, route_active_flag_old, route_thickness_old, remove_trailing_space(route_path_color_old), remove_trailing_space(route_coordinates_string_old));  /* construct how the line in the file would look like */
remove_trailing_space(routeLineToEditFrom);  /* strip off the trailing overkill of spaces */

/* prepare a variable containing the new route string (this is what they are changing to) */
sprintf(routeLineToEditTo, "%s,%s,%d,%d,%s,%s;%s,%s", route_mediaport_origin, route_floorplan_name, route_active_flag_old, route_thickness, remove_trailing_space(route_path_color), remove_trailing_space(route_coordinates_string), route_user, route_dtsec);  /* construct how the new line in the file will look like */
remove_trailing_space(routeLineToEditTo);  /* strip off the trailing overkill of spaces */
fprintf(file_msgRoutes_log, "Record to change:      \"%s\"\n", routeLineToEditFrom);
fprintf(file_msgRoutes_log, "Record will change to: \"%s\"\n", routeLineToEditTo);

/* prepare file info... */
sprintf(str_routeStoreFilename, "/home/silentm/record/routes/%s.%s.routes", route_msg_directory, route_msg_name);
sprintf(str_routeStoreFilenameTemp, "/home/silentm/record/routes/%s.%s.routes.temp", route_msg_directory, route_msg_name);
remove_trailing_space(str_routeStoreFilename);
remove_trailing_space(str_routeStoreFilenameTemp);

/* open the original file for reading... */
file_msgRoutes = fopen(str_routeStoreFilename, "r");
fprintf(file_msgRoutes_log, "- Opened original file for reading: %s\n", str_routeStoreFilename);

/* open the temporary file for writing (it will ultimately replace the original file)... */
file_msgRoutesTemp = fopen(str_routeStoreFilenameTemp, "w");
fprintf(file_msgRoutes_log, "- Opened temporary file for writing: %s\n", str_routeStoreFilenameTemp);

/* iterate through each line of the original file and */
fprintf(file_msgRoutes_log, "- Iterating through each line of the file to determine what to save to the temporary file:\n");
while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutes))
	{
	strcpy(line_buffer_ex, line_buffer);					/* copy the line to a temporary buffer that will be used for excluding the non-matching date */
	routeTokenPtr2 = strtok(line_buffer, "\n");				/* get (refer to) the part of the line up to (but excluding) the line-feed character, so we can reliably to a strcmp operation (remember to do this again right after using the data)*/
	routeTokenPtr2_ex = strtok(line_buffer_ex, ";");			/* get (refer to) the part of the line up to (but excluding) the semicolon character, so we can ignore user and dtsec data when matching */
	fprintf(file_msgRoutes_log, "    i=%d: \"%s\"  ", i, routeTokenPtr2);
	if(strcmp(routeTokenPtr2_ex, routeLineToEditFrom))
		{
		fprintf(file_msgRoutes_log, "   (UNCHANGED)\n");
		fprintf(file_msgRoutesTemp, "%s\n", line_buffer);
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                Route to keep unchanged: %s", line_buffer);
		i++;
		}
	else
		{
		fprintf(file_msgRoutes_log, "   (THIS RECORD WILL BE OVERWRITTEN WITH THE NEW VALUES ABOVE)\n");
		fprintf(file_msgRoutesTemp, "%s\n", routeLineToEditTo);
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                Route to change:         %s", routeTokenPtr2);
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("                    changing to:         %s", routeLineToEditTo);
		i++;
		}
	routeTokenPtr2 = strtok(NULL, "\n");	/* advance the pointer to the end, so it can reset itself for the next record */
	}

/* close the files... */
fclose(file_msgRoutes);
fclose(file_msgRoutesTemp);
fprintf(file_msgRoutes_log, "- Original and Temporary files closed\n");

/* rename the temp file thereby overwriting the original... */
rename(str_routeStoreFilenameTemp, str_routeStoreFilename);
fprintf(file_msgRoutes_log, "- Temporary file renamed/overwritten to original: %s -> %s\n", str_routeStoreFilenameTemp, str_routeStoreFilename);

/* DEV NOTE: upgrade later to delete the file if no routes now exist... use remove(char *path) -- shouldn't be critical*/

fprintf(file_msgRoutes_log, "- Operation complete\n\n");
fclose(file_msgRoutes_log);
printf("autoEnterRouteMode=1;");
if(strcmp(route_last_submit_reason, "SaveExistingViaAutoSave") == 0)
	{
	strcpy(state_change_message, "Route Automatically Saved");		/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
	}
else
	{
	strcpy(state_change_message, "Route Saved");				/* do not change this without first checking for corresponding condition-tests toward the end of this file! */
	}
}

char route_msg_name_fromDB[MSG_NAME_LENGTH];
char route_msg_directory_fromDB[DIRECTORY_LENGTH];
int route_number_of_total_routes = 0;
/***********************************************************
** void addClientsideRoutesData(void)
**
**	Add client-side javascript structure to hold all routes data for this message.
**	
**	Since a single html file is generated for all mediaports to download from the server,
**	 all routes associated with this message will need to be stored in a client-side 
**	 structure that each mediaport can then parse (using data from AJAX) to determine which 
**	 route data to use to display the route on itself.
**
**	Process Overview:
**	 - Get ALL routes data
**	 - Go through each route, and create a client-side structure (JS array) to hold the data, that the mediaport can later use to display its route
**
***********************************************************/
void addClientsideRoutesData(void)
{
int i = 0;
char tmp_msg_name[MSG_NAME_LENGTH];
char tmp_msg_directory[DIRECTORY_LENGTH];
char str_routeStoreFilenameForRead[sizeof(tmp_msg_name) + sizeof(".") + sizeof(tmp_msg_directory) + sizeof(".routes")];
FILE *file_msgRoutesForRead = NULL;					/* instantiate a place in memory for the file object */
char line_buffer[sizeof(route_coordinates_string)+BUFSIZ];		/* instantiate and initialize a buffer for holding a line once it has been read from the file */
int tmpFPwidth;
int tmpFPheight;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("addClientsideRoutesData():  adding javascript-array of all routes for message '%s'...", remove_trailing_space(route_msg_name));

get_dtsec(route_dtsec);

if(*route_msg_name_fromDB 
	&& *route_msg_directory_fromDB)	/* if message info is coming from banner, then assume this is being loaded on a mediaport rather than by the user during route editing */
	{
	strcpy(tmp_msg_name, remove_trailing_space(route_msg_name_fromDB));
	strcpy(tmp_msg_directory, remove_trailing_space(route_msg_directory_fromDB));
	}
else					/* else, the message info should be coming from previous form states - implying from route editing */
	{
	strcpy(tmp_msg_name, route_msg_name);
	strcpy(tmp_msg_directory, route_msg_directory);
	}

/* redundant logic -- if for some reason, we didn't get msg info in the above logic, grab from form as a backup */
if(!strcmp(remove_trailing_space(tmp_msg_name),""))
	{
	strcpy(tmp_msg_name, route_msg_name);
	}
if(!strcmp(remove_trailing_space(tmp_msg_directory),""))
	{
	strcpy(tmp_msg_directory, route_msg_directory);
	}

sprintf(str_routeStoreFilenameForRead, "/home/silentm/record/routes/%s.%s.routes", tmp_msg_directory, tmp_msg_name);

file_msgRoutesForRead = fopen(str_routeStoreFilenameForRead, "r");	/* open the file for reading */
printf("\n<script type=\"text/javascript\">\n");
printf("var rtiDeviceName = 0;");	/* (naming convention, rti, stands for routes table index) - having to use var instead of const to appease the IE gods +rolls eyes+ */
printf("var rtiFloorplanName = 1;");
printf("var rtiActiveFlag = 2;");
printf("var rtiRouteThickness = 3;");
printf("var rtiRouteColor = 4;");
printf("var rtiCoords = 5;");
printf("var rtiUserPin = 6;");
printf("var rtiDtsec = 7;\n");
if(file_msgRoutesForRead)  						/* if file exists (file pointer is not null)... */
	{
	char *routeTokenPtr;							/* instantiate a place in memory for a pointer that will be used to look through a line */

	printf("var routesTable = [];\n");					/* initialize main multidimensional array structure that will hold our routes in client-side scope */
	printf("var arrFloorplanSizes = []; var f=0;");
	while(fgets(line_buffer, sizeof(line_buffer), file_msgRoutesForRead))	/* loop through each line of the file, one-by-one (storing each line temporarily in line_buffer while we're working with it) */
		{
		routeTokenPtr = strtok(line_buffer, ",");				/* delimit the line by comma, storing all characters UP TO the comma in routeTokenPtr, while at the same time the strtok function is keeping track of pointer location until we call it again on next iteration */
		while(routeTokenPtr != NULL)						/* loop through the parts of the line -- again, as determined by the comma delimiter (internal process can't be further looped, due to having different delimiters) */
			{
			printf("routesTable[%d] = [];\n", i);		  			/* create a record (like a row) in the multidimensional array (like a table) that represents a specific route-path */

			/* device id */
			/* (routeTokenPtr is already at the first token because we already did strtok above) */
			printf("routesTable[%d][rtiDeviceName] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for mediaport name, which is currently contained in routeTokenPtr */

			/* floorplan name */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store the substring up before it */
			printf("routesTable[%d][rtiFloorplanName] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for floorplan name, which is currently contained in routeTokenPtr */
			printf("arrFloorplanSizes[f]=[];");
			printf("arrFloorplanSizes[f][0]=\"%s\";", routeTokenPtr);
			sprintf(routeTokenPtr, "%s.gif", remove_trailing_space(routeTokenPtr));
			cgi_read_gif_width_height("floor_plans", routeTokenPtr, &tmpFPwidth, &tmpFPheight);	/* grab the width and height of this floorplan's GIF file (will use to populate the JS array, arrFloorplanSizes) */
			printf("arrFloorplanSizes[f][1]=%d;", tmpFPwidth);
			printf("arrFloorplanSizes[f][2]=%d;", tmpFPheight);
			printf("f++;\n");

			/* active flag */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store the substring up before it */
			printf("routesTable[%d][rtiActiveFlag] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for the active flag, which is currently contained in routeTokenPtr */
			
			/* line thickness */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store it just as we did with the first one */
			printf("routesTable[%d][rtiRouteThickness] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for route thickness, which is currently contained in routeTokenPtr */

			/* line color */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store it just as we did previously */
			printf("routesTable[%d][rtiRouteColor] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for route-path color, which is currently contained in routeTokenPtr */

			/* coordinates string */
			routeTokenPtr = strtok(NULL, ";");					/* move pointer to next delimited section (token/field) and temporarily store it just as we did previously */
			printf("routesTable[%d][rtiCoords] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for coordinates string path, which is currently contained in routeTokenPtr */

			/* user pin */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store it just as we did previously */
			printf("routesTable[%d][rtiUserPin] = \"%s\";\n", i, routeTokenPtr);		/* write the field (like a column) for user pin, which is currently contained in routeTokenPtr */

			/* dtsec */
			routeTokenPtr = strtok(NULL, ",");					/* move pointer to next delimited section (token/field) and temporarily store it just as we did previously */
			printf("routesTable[%d][rtiDtsec] = \"%s\";\n", i, remove_trailing_white_space(routeTokenPtr));/* write the field (like a column) for dtsec, which is currently contained in routeTokenPtr (NOTE: the remove_trailing_white_space is important in order to remove the line-feed character!) */

			routeTokenPtr = strtok(NULL, ",");					/* advance the pointer, even though there are no more delimiters, because this should reset it, helping us prepare for the next line's iterations */

			i++;
			}
		}
	fclose(file_msgRoutesForRead);
	route_number_of_total_routes = i;
	}
else 									/* else file does not exist */
	{
	printf("var routesTable = null;\n");					/* initialize the array we would typically use, but set it to null instead so it would be easy to test for later, if necessary */
	}
printf("</script>\n");
}

/***********************************************************
** static void addClientsideGraphicalAnnunciatorDevicesData_fromDB(present_floorplan_name_no_suffix_with_space)
**
**	Add client-side javascript structure to hold all graphical annunciator (GA) data for this message and currently loaded floorplan.
**	
**	(disregard below comment block... instead this will only be for the user interaction portion -- not for mediaport display??)
**	Since a single html file is generated for all mediaports to download from the server,
**	 all life-safety devices associated with this message will need to be stored in a client-side 
**	 structure that each mediaport-GA can then parse (using deviceid data from AJAX) to determine which 
**	 ga-data to use to display the proper map/floorplan on itself. (basically this is just a clone of the 
**	 routes (above), but supporting devices that would have no routes... and thus wouldn't know what map to show.
**
**	Process Overview:
**	 - Get ALL GA life-safety device (lsd) data (within the message and loaded-floorplan context, of course)
**	 - Go through each device, and create a client-side structure (JS array) to hold the data, that the mediaport can later use to display its proper map
**
***********************************************************/
static void addClientsideGraphicalAnnunciatorDevicesData_fromDB(char floorplan_name_local[FLOORPLAN_NAME_LENGTH])
{

int nextLoc;
int iLoc = 0;
char *ptrLoc;
int topLoc = 0;
int leftLoc = 0;
int active_state_loc;

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("addClientsideGraphicalAnnunciatorDevicesData_fromDB():  adding javascript-array of all placed-devices for floorplan '%s' and message '%s'...", floorplan_name_local, route_msg_name);

/* begin writing client code */
printf("\n<script type=\"text/javascript\">\n");

printf("var dtiType = 0;\n");			/* device type id */
printf("var dtiLeft = 1;\n");			/* device's icon position on this map */
printf("var dtiTop = 2;\n");			/* device's icon position on this map */
printf("var dtiName = 3;\n");			/* device name */
printf("var dtiState = 4;\n");			/* device current alarm state */
printf("var dtiActiveState = 5;\n");		/* device's flag for marking this map as active or not */
printf("var dtiIsLsd = 6;\n");			/* device is life safety or not */
printf("var dtiRecNo = 7;\n");			/* device's icon's record number */

printf("var devicesTable;\n");

if(1)
	{
	printf("devicesTable = [];\n");
	db_list_select(6);								/* floorplan name (should be in global scope by the time this function is called), and primitive (don't care, so nothing) */

	strcpy(db_list->res_id, res_id);
	strcpysl(db_list->dli_name, floorplan_name_local, sizeof(db_list->dli_name));
	strcpy(db_list->dli_decode_primitive, "");
	db_list->dli_type = LIST_FLOORPLAN_MAP;

	nextLoc = db_list_find();								/* advance to the record */

	if(!strcmp(db_list->dli_name, floorplan_name_local) 
		&& db_list->dli_type == LIST_FLOORPLAN_MAP)
		{

		while(nextLoc > 0)									/* while there are records... */
			{
			if(!strcmp(db_list->res_id, res_id)						/* if this record-iteration matches what we're looking for (res_id, map name, type, and is a type of life-safety-device), then... */
				&& !strcmp(db_list->dli_name, floorplan_name_local)
				&& db_list->dli_type == LIST_FLOORPLAN_MAP)
				{
				ptrLoc = strchr(db_list->dli_decode_primitive, ',');
				if(ptrLoc)
					{
					*ptrLoc = 0;
					ptrLoc++;
					topLoc = atoi(db_list->dli_decode_primitive);
					leftLoc = atoi(ptrLoc);
					}
				printf("devicesTable[%d] = [];\n", iLoc);		  							/* create a record (like a row) in the multidimensional array (like a table) that represents a specific route-path */
				printf("devicesTable[%d][dtiType] = %d;\n", iLoc, db_list->dli_floor_plan_type);				/* save the type of device */
				printf("devicesTable[%d][dtiLeft] = %d;\n", iLoc, leftLoc);							/* save the device's icon's position from left for the currently loaded floorplan */
				printf("devicesTable[%d][dtiTop] = %d;\n", iLoc, topLoc);							/* save the device's icon's position from top for the currently loaded floorplan */
				printf("devicesTable[%d][dtiName] = \"%s\";\n", iLoc, db_list->dli_associated_sign_group);			/* save the device's name */
				if(HardwareFindDeviceIDName(db_list->dli_associated_sign_group, FALSE))						/* if we can find the device in the hardware database... */
					{
					printf("devicesTable[%d][dtiState] = %d;\n", iLoc, db_hard->dhc_device_in_alarm_trouble_clear);			/* save the device's state, overwriting the default above */
					}
				else
					{
					printf("devicesTable[%d][dtiState] = %d;\n", iLoc, DEVICE_STATE_NONE);						/* save the device's state, overwriting the default above */
					}
				active_state_loc = db_list->dli_font_size;
				printf("devicesTable[%d][dtiActiveState] = %d;\n", iLoc, active_state_loc);
				printf("devicesTable[%d][dtiIsLsd] = %d;\n", iLoc, HardwareTypeIsLifeSafetyDevices(db_hard->dhc_device_type));
				printf("devicesTable[%d][dtiRecNo] = "FORMAT_DBRECORD_STR";\n", iLoc, db_list_getcur());
				iLoc++;
				}
			/* end if */
			nextLoc = db_list_next();
			}
		/* end while */

		}
	/* end if */

	/* close databases */
//	db_syspa_close();		/* jerry said these need to remain open... so don't close */
//	db_hard_close();
//	db_list_close();

	}/* end else */

printf("</script>\n");
}

/***********************************************************
** void addClientsideFloorplanData_fromBannerMessageOptions(DBRECORD me_msg_template_recno)
**
**	Add client-side javascript structure to hold all floorplan data.
**	(used, primarily, for ascertaining a floorplan's dimensions for use by DHTML; but also for other things)
**
***********************************************************/
void addClientsideFloorplanData_fromBannerMessageOptions(DBRECORD me_msg_template_recno)
{
int i = 0;
int j;
int width;
int height;
char filename[100];

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("addClientsideFloorplanData_fromBannerMessageOptions():  adding javascript-array of all msg-associated floorplans and relevant data");

/* begin writing client code */
printf("\n<script type=\"text/javascript\">\n");
printf("/* This section of JavaScript stores a client-side array of all floorplans that appear to be associated with this message */\n");
printf("var fpmotiName = 0;\n");
printf("var fpmotiWidth = 1;\n");
printf("var fpmotiHeight = 2;\n");

printf("var fpmoTable;\n");

if(BannerOptions(me_msg_template_recno, DB_ISAM_READ))		/* if successfully read from the banner message options file, then... */
	{
	printf("fpmoTable = [];\n");					/* initialize the array */
	for(j = 0; j < MAX_MULTI_FLOORPLAN_SELECT; j++)			/* for each of the possible (up to maximum) floorplans, if it is populated with a floorplan, write it to the javascript array... */
		{
		if(notjustspace(db_bann_message_options->mo_floorplan_group[j], FLOORPLAN_NAME_LENGTH))
			{
			printf("fpmoTable[%d] = [];\n", i);
			printf("fpmoTable[%d][fpmotiName] = \"%s\";\n", i, db_bann_message_options->mo_floorplan_group[j]);
			sprintf(filename, "%s.gif", remove_trailing_space(db_bann_message_options->mo_floorplan_group[j]));
			cgi_read_gif_width_height("floor_plans", filename, &width, &height);
			printf("fpmoTable[%d][fpmotiWidth] = %d;\n", i, width);							/* save the device's icon's position from left for the currently loaded floorplan */
			printf("fpmoTable[%d][fpmotiHeight] = %d;\n", i, height);							/* save the device's icon's position from top for the currently loaded floorplan */
			i++;
			}
		}
	}
else								/* else, set to null */
	{
	printf("fpmoTable = null;\n");					/* initialize the array handle we would typically use, but set it to null instead of an array, so it would be easy to test for later */
	}

printf("</script>\n");
}

/***********************************************************
** static void addClientsideFloorplanData_fromDB(void)
**
**	Add client-side javascript structure to hold all floorplan data.
**	(used, primarily, for ascertaining a floorplan's dimensions for use by DHTML; but also for other things)
**
***********************************************************/
static void addClientsideFloorplanData_fromDB(void)
{

int next;
int i = 0;
int width;
int height;
char filename[100];
char prevFloorplanName[FLOORPLAN_NAME_LENGTH];			/* used to keep track of the previously iterated floorplan (just to help keep the js array down in size... not as good as a full duplicate-avoiding scheme though... good for now) */

if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("addClientsideFloorplanData_fromDB():  adding javascript-array of all floorplans and relevant data");

/* begin writing client code */
printf("\n<script type=\"text/javascript\">\n");
printf("/* This section of JavaScript stores a client-side array of all floorplans currently in the system (global scope) */\n");
printf("var fptiName = 0;\n");
printf("var fptiWidth = 1;\n");
printf("var fptiHeight = 2;\n");

printf("var fpTable;\n");

if(1)
	{
	printf("fpTable = [];\n");
	db_list_select(6);								/* floorplan name (should be in global scope by the time this function is called), and primitive (don't care, so nothing) */

	strcpy(db_list->res_id, res_id);
	strcpysl(db_list->dli_name, "", sizeof(db_list->dli_name));
	strcpy(db_list->dli_decode_primitive, "");
	db_list->dli_type = LIST_FLOORPLAN_MAP_HEADER;

	next = db_list_find();								/* advance to the first record */

	printf("fpTable[%d] = [];\n", i);		  							/* create a record (like a row) in the multidimensional array (like a table) that represents a specific route-path */
	printf("fpTable[%d][fptiName] = \"%s\";\n", i, db_list->dli_name);					/* save the device's name */
	sprintf(prevFloorplanName, "%s", db_list->dli_name);
	sprintf(filename, "%s.gif", remove_trailing_space(db_list->dli_name));
	cgi_read_gif_width_height("floor_plans", filename, &width, &height);
	printf("fpTable[%d][fptiWidth] = %d;\n", i, width);							/* save the device's icon's position from left for the currently loaded floorplan */
	printf("fpTable[%d][fptiHeight] = %d;\n", i, height);							/* save the device's icon's position from top for the currently loaded floorplan */
	i++;
	next = db_list_next();

	while(next > 0									/* while there are records... */
		&& db_list->dli_type == LIST_FLOORPLAN_MAP_HEADER)
		{
		if(strcmp(db_list->dli_name, prevFloorplanName))				/* if this floorplan is different than the previous one, write another record */
			{
			printf("fpTable[%d] = [];\n", i);		  							/* create a record (like a row) in the multidimensional array (like a table) that represents a specific route-path */
			printf("fpTable[%d][fptiName] = \"%s\";\n", i, db_list->dli_name);					/* save the device's name */
			sprintf(prevFloorplanName, "%s", db_list->dli_name);
			sprintf(filename, "%s.gif", remove_trailing_space(db_list->dli_name));
			cgi_read_gif_width_height("floor_plans", filename, &width, &height);
			printf("fpTable[%d][fptiWidth] = %d;\n", i, width);							/* save the device's icon's position from left for the currently loaded floorplan */
			printf("fpTable[%d][fptiHeight] = %d;\n", i, height);							/* save the device's icon's position from top for the currently loaded floorplan */
			i++;
			}
		next = db_list_next();
		}
	}
printf("</script>\n");
}

/****************************************************************
** void loadFloorPlan(void)
**
**
**	Locations storage or encoding is as follows:
**		dli_name			map name
**		dli_decode_primitive		x, y
**		dli_help_string			note 			
**		dli_type			type
**		dli_associated_organization	parent map name
**		dli_sister_audio_group		child map name
**		dli_associated_sign_group	sign group or audio group
**		dli_who_can_use			who_can_use
**		dli_who_can_modify		who_can_modify
**
**	Testing:		(NOTE: sign and vsign are handled the same)
**
**			   Record Phone-Select	Multi-Select Single
**		OK 	-                                      X	sign (pop text entry, then deliver)
**		OK 	-                                      X        speaker (pop text entry, then deliver)
**		OK	-                                      X        phone (pop text entry, then deliver)
**		OK 	-                            X                  speaker1, speaker2 (pop text entry, then deliver)
**		OK 	-                            X                  sign1, sign2, (pop text entry, then deliver)
**		OK 	-                            X                  speaker1, speaker2, sign1, sign2 (pop text entry, then deliver)
**		OK	-                            X                  phone1, phone2 (pop text entry, then deliver)
**		OK 	-                            X                  speaker1, speaker2, phone1, phone2 (pop text entry, then deliver)
**		OK 	-                            X                  speaker1, sign1, phone1, (pop text entry, then deliver)
**		OK	-               X                      X        phone (dial person logged in, then dial the phone and connect)
**		OK 	-               X                      X        speaker (dial person logged in, then dial the intercom pulse zone)
**		OK 	-               X                      X        sign (force text mode, then deliver)
**		OK   	-               X            X                  sign1, sign2 (force text mode, then deliver)
**		OK 	-               X            X                  speaker1, speaker2, phone1 (dial person logged in, record message, the deliver)
**
**	Behavior: from kevin brown 8/26/03
**		Phone:
**
**			Call (default)	Call logged in person then call call Work number because it is a physical phone (most normal thing to use a phone for)
**			Voice Msg	Call logged in person for record then call call Work (physical phone)
**			Text		Enter text then call Work (physical phone)
**
**		Person:
**			Call		Call logged in person then call call at
**			Voice Msg	Call logged in person for record then call call at
**			Text (default)	Enter text goes to current paging device (most normal way to send a message to a person)
**
****************************************************************/
void loadFloorPlan(void)
{
int next;
int width = 0;
int height = 0;
int additional_space = 2;
int backwidth = 300;
int backheight = 300;
int next_map;
int fp_call_mode; 						/* 0 no call mode, 1 call mode, 2 call mode force text */
int fp_call_mode_redo;
int fp_edit_mode;
int fp_selection_type;						/* 1 for signs 2 for speakers 3 for both */
int fp_selection_type_redo;
int fp_set_send_message_count;
int save_image_type;
int live_intercom_access;
int hspace = 2;
int show_ok_button = TRUE;
int smbanner_locations = cgi_parsed_lookup_integer("smbanner_locations=", 0);
int locations_screen_refresh_timer_default = configuration_lookup_integer(DB_PATH_LOCATION, "SMCGI_LOCATIONS_REFRESH_TIMER", 0);

int space_width;
int space_height;

int save_width;
int save_height;

int locations_alert_status;

DBRECORD msg_name_recno;
DBRECORD me_msg_template_recno;					/* message template record number (originally obtained via the message editor screen - could also be obtained as representing the locations-tab dummy message recno) */

char change_map[FLOORPLAN_NAME_LENGTH + 10] = "";
char present_floorplan_name[FLOORPLAN_NAME_LENGTH + 10] = "";				/* floor1.gif */
char present_floorplan_name_no_suffix[FLOORPLAN_NAME_LENGTH] = "";			/* floor1 */
char present_floorplan_name_no_suffix_with_space[FLOORPLAN_NAME_LENGTH] = "";
char compare_width_height[50];
char floor_command[20];
char floor_plan_record_numbers_for_redo[MAX_MULTI_SELECTION_LIST_BUFFER] = "";
char fp_speaker_phone[PHONE_LENGTH];
char *alignment = "ABSMIDDLE";
char other[100] = "";
char locations_text[PAGE_MESSAGE_LENGTH] = "";
char check_exist[300];

/* CR ADDED */
#define ROUTE_COLOR_DEFAULT	("#EE0000")		/* define a default color of red */
#define ROUTE_THICKNESS_THIN	(3)			/* use this as the master definition for route thickness, in pixels */
#define ROUTE_THICKNESS_NORMAL	(6)			/* use this as the master definition for route thickness, in pixels */
#define ROUTE_THICKNESS_THICK	(9)			/* use this as the master definition for route thickness, in pixels */
int assumed_need_route_mode = 0;			/* boolean integer: for flagging whether we should allow option to enter route-mode or not */
int auto_enter_route_mode = cgi_parsed_lookup_integer("autoEnterRouteMode=", 0);
int fp_route_need_auto_submit_to_enter_edit_mode = cgi_parsed_lookup_integer("fp_route_need_auto_submit_to_enter_edit_mode=", 0);  /* boolean integer: needed at the C level to prevent an infinite loop with auto-submitting this page to get into edit mode -- this will be checked by C at the end, which may trigger JS to submit the form or not */
int submission_to_add_route;				/* boolean integer: for flagging whether we should do the save to text file operation or not */
int submission_to_delete_route;				/* boolean integer: for flagging whether we should do the delete from text file operation or not */
int submission_to_edit_route;				/* boolean integer: for flagging whether we should do the edit route in the text file operation or not */
char route_last_user_intention[32];
int route_index_at_submit_time;
//int msg_alert_status;
char msg_alert_status[2];
int load_edit_sign_speaker_after_change_active_map = cgi_parsed_lookup_integer("loadEditSignSpeakerAfterChangeActiveMap=", 0);
int locations_ga_default_fp_need_to_save;
char locations_ga_default_floorplan[FLOORPLAN_NAME_LENGTH];
/* CR END */

DBRECORD map_speakers_message_recno;
DBRECORD floor_plan_header_recno;
DBRECORD delete_recno;
DBRECORD call_recno;
DBRECORD push_list;

title_show_floorplan_edit_map = TRUE;

call_recno = cgi_parsed_lookup_record_number("call_recno=", 0);
delete_recno = cgi_parsed_lookup_record_number("delete_recno=", 0);
floor_plan_header_recno = cgi_parsed_lookup_record_number("floor_plan_header_recno=", 0);

main_volatile_parameters_read();

if(DistributionFindListName(db_sysp->dsy_live_public_address_access)
	&& (DistributionPersonInListName(CurrentUserPin, db_sysp->dsy_live_public_address_access) == DL_EXCLUDE))
	{
	live_intercom_access = FALSE;
	}
else
	{
	live_intercom_access = TRUE;
	}

msg_name_recno = cgi_parsed_lookup_record_number("msg_name_recno=", 0);

printf("<HTML>");
add_CopyRight(NULL, TRUE);
/* CR NOTE: if stuff looks like it's caching when it shouldn't be, try uncommenting the following 5 lines? */
//printf("<HEAD>");
cgi_html_title("Locations Manager");
//printf("<meta http-equiv='cache-control' content='no-cache'>");
//printf("<meta http-equiv='expires' content='0'>");
//printf("<meta http-equiv='pragma' content='no-cache'>");
//printf("</HEAD>");
if(msg_name_recno > 0
	&& db_bann_setcur(msg_name_recno) > 0)
	{
	cgi_body_with_background(ScreenColorToString(db_bann->dbb_screen_color), "onresize=\"SetSpanLocation();\" onLoad=\"SetFormIsLoaded(); PopMessage(); return true;\"");
	}
else
	{
	cgi_body_with_background(NULL, "onresize=\"SetSpanLocation();\" onLoad=\"SetFormIsLoaded(); PopMessage(); return true;\"");
	}

printf("<FORM action=%s method=post>", cgi_server);
printf("<input type=\"hidden\" name=\"needtoresetstatus\" value=\"0\"></input>");				/* for being able to post the flag back to this function when a submission happens */
printf("<input type=\"hidden\" name=\"lsdnametodeactivate\" value=\"\"></input>");				/* for being able to post the flag back to this function when a submission happens */

cgi_set_help("", "locations_manager_help.htm");

/* this is passed in from smbanner to display the message on screen */
cgi_parsed_lookup("locations_text=", locations_text, sizeof(locations_text));
locations_alert_status = cgi_parsed_lookup_integer("locations_alert_status=", BB_ALERT_STATUS_NOT_ACTIVE);		/* indicates db_bann->dbb_alert_status */
/* CR ADDED */
msg_name_recno = cgi_parsed_lookup_record_number("msg_name_recno=", 0);

cgi_parsed_lookup("msg_name_fromdb=", route_msg_name_fromDB, MSG_NAME_LENGTH);
cgi_parsed_lookup("msg_directory_fromdb=", route_msg_directory_fromDB, DIRECTORY_LENGTH);

me_msg_template_recno = cgi_parsed_lookup_record_number("me_msg_template_recno=", 0);		/* will find from either the querystring or submitted form, due to same argument/field name */
flag_locations_tab_or_msg = cgi_parsed_lookup_integer("flag_locations_tab_or_msg=", 0);	/* attempt to get the previous post's flag (in case of repeated locations-tab load) */
/* Now we'll determine whether this is a load from a first click on the "locations" tab or from a message editor session, and set a flag (flag_locations_tab_or_msg) */
/* NOTE: even from a locations-tab-based interaction, we'll populate the form with the recno, so subsequent tab loads may seem like a message-based load so be careful! Beyond this point, you should depend solely on the flag that gets set below! */
if(me_msg_template_recno == 0)
	{
	flag_locations_tab_or_msg = FLAG_LOCATIONS_TAB;

	if(msg_name_recno > 0
		&& db_bann_setcur(msg_name_recno) > 0)
		{
		me_msg_template_recno = db_bann->dbb_parent_record;
		}
	else
		{
		me_msg_template_recno = find_record_in_db_bann(silentm_default_directory, bb_valid_types[BBT_LF].string, df_locations_maps_visible_for_tab);
		}
	}
else
	{
	if(flag_locations_tab_or_msg == FLAG_LOCATIONS_TAB)	/* if this is a repeated loading of the locations tab screen (either via manual user click of the tab, edit-map button, etc. or via automatic refresh) */
		{
		flag_locations_tab_or_msg = FLAG_LOCATIONS_TAB;
		}
	else
		{
		flag_locations_tab_or_msg = FLAG_LOCATIONS_MSG;
		}
	}

/* These lines parse the values out of the URL line, as generated by CreateSessionWindowCloseLoginWindow in smcgi.c  -- do not move! */
cgi_parsed_lookup("msgName=", route_msg_name, MSG_NAME_LENGTH);
remove_trailing_space(route_msg_name);
cgi_parsed_lookup("msgDir=", route_msg_directory, DIRECTORY_LENGTH);
remove_trailing_space(route_msg_directory);
//msg_alert_status = cgi_parsed_lookup_integer("msgAlertStatus=", -1);
cgi_parsed_lookup("msgAlertStatus=", msg_alert_status, sizeof(msg_alert_status));

if(*route_msg_name)
	{
	assumed_need_route_mode=1;
	}

addClientsideRoutesData();
addClientsideFloorplanData_fromBannerMessageOptions(me_msg_template_recno);
addClientsideFloorplanData_fromDB();
/* CR END */

cgi_parsed_lookup("change_map=", change_map, sizeof(change_map));
remove_trailing_space(change_map);

cgi_parsed_lookup("fp_speaker_phone=", fp_speaker_phone, sizeof(fp_speaker_phone));
remove_trailing_space(fp_speaker_phone);

cgi_parsed_lookup("fp_set_send_message=", floor_plan_record_numbers, sizeof(floor_plan_record_numbers));
remove_trailing_space(floor_plan_record_numbers);

cgi_parsed_lookup("fp_redo_send_message=", floor_plan_record_numbers_for_redo, sizeof(floor_plan_record_numbers_for_redo));
remove_trailing_space(floor_plan_record_numbers_for_redo);

cgi_parsed_lookup("floor_command=", floor_command, sizeof(floor_command));
remove_trailing_space(floor_command);

fp_call_mode = cgi_parsed_lookup_integer("fp_call_mode=", CALL_MODE_NONE);
fp_call_mode_redo = cgi_parsed_lookup_integer("fp_call_mode_redo=", CALL_MODE_NONE);
fp_edit_mode = cgi_parsed_lookup_integer("fp_edit_mode=", 0);

fp_selection_type = cgi_parsed_lookup_integer("fp_selection_type=", 0);
fp_selection_type_redo = cgi_parsed_lookup_integer("fp_selection_type_redo=", 0);
fp_set_send_message_count = cgi_parsed_lookup_integer("fp_set_send_message_count=", 0);

save_width = cgi_parsed_lookup_integer("save_width=", 0);
save_height = cgi_parsed_lookup_integer("save_height=", 0);
save_image_type = cgi_parsed_lookup_integer("save_image_type=", FLOORPLAN_NONE);

load_change_map:
if(me_msg_template_recno > 0)	/* if a message template recno is available... */
	{
	BannerOptions(me_msg_template_recno, DB_ISAM_READ);	/* read-in the banner options file, so we can use it later */
	}

if(floor_plan_header_recno == 0 || strlen(change_map))
	{
	int i;

	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("smcgi_locations.c: loadFloorPlan(): No floorplan header record number specified (floor_plan_header_recno == 0)");

	db_list_select(6);		/* key with type, name, decode primitive */

	strcpy(db_list->res_id, res_id);
	db_list->dli_type = LIST_FLOORPLAN_MAP_HEADER;
	strcpy(db_list->dli_decode_primitive, "");
	if(notjustspace(change_map, FLOORPLAN_NAME_LENGTH))
		{
		strcpyl(db_list->dli_name, change_map, sizeof(db_list->dli_name));
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadFloorPlan(): No floorplan header record number specified, so got it from change_map field (%s)", db_list->dli_name);
		}
	else
		{
		strcpyl(db_list->dli_name, cgi_form_states.main_floor_plan_name, FLOORPLAN_NAME_LENGTH);
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadFloorPlan(): No floorplan header record number specified, so got it from main_floor_plan_name field (%s)", db_list->dli_name);
		}

	next = db_list_find();

	/* find a map this user can see or dli_who_can_use has access */
	do
		{
		if(db_list->dli_type == LIST_FLOORPLAN_MAP_HEADER)
			{
			if(DistributionFindListName(db_list->dli_who_can_use)
				&& (DistributionPersonInListName(CurrentUserPin, db_list->dli_who_can_use) == DL_EXCLUDE))
				{
				/* no access */
				}
			else
				{
				/* if this is in the context of a message (or a message template recno is available - should always be, now, by the way) */
				if(me_msg_template_recno > 0)
					{
					/* need to check through this message's banner-options file to see if this iteration's floorplan matches any in that file... if so, then set necessary fields, otherwise don't */
					for(i = 0; i < MAX_MULTI_FLOORPLAN_SELECT; i++)
						{
						if(strcmp(db_bann_message_options->mo_floorplan_group[i], db_list->dli_name) == 0)
							{
							floor_plan_header_recno = db_list_getcur();
							cgi_add_or_change("main_floor_plan_name=", db_list->dli_name);
							strcpyl(cgi_form_states.main_floor_plan_name, db_list->dli_name, FLOORPLAN_NAME_LENGTH);
							goto exit_nested_loop_load_change_map;	/* had to use in order to break out of the nested loop AND the parent loop */
							}
						}
					}
				/* now that the locations tab has a banner message options file, this should never happen, but leave it for good measure - at least for now */
				else
					{
					if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("smcgi_locations.c: loadFloorPlan() Execution should maybe not be stepping into this, now that the locations tab behaves as a message template for maps (unverified for all potential cases, though)");
					floor_plan_header_recno = db_list_getcur();
					cgi_add_or_change("main_floor_plan_name=", db_list->dli_name);
					strcpyl(cgi_form_states.main_floor_plan_name, db_list->dli_name, FLOORPLAN_NAME_LENGTH);
					break;
					}
				}

			next = db_list_next();
			}
		else
			{
			next = 0;
			}
		}
	while(next > 0);
	}

exit_nested_loop_load_change_map:

if(db_list_setcur(floor_plan_header_recno) > 0)
	{
	strcpyl(present_floorplan_name, db_list->dli_name, sizeof(present_floorplan_name));
	remove_trailing_space(present_floorplan_name);
	strcatl(present_floorplan_name, ".gif", sizeof(present_floorplan_name));

	strcpyl(present_floorplan_name_no_suffix, db_list->dli_name, sizeof(present_floorplan_name_no_suffix));
	remove_trailing_space(present_floorplan_name_no_suffix);

	strcpysl(present_floorplan_name_no_suffix_with_space, present_floorplan_name_no_suffix, FLOORPLAN_NAME_LENGTH);
	}

if(!strcmp(floor_command, "LINK"))
	{
	DBRECORD change_map_record = cgi_parsed_lookup_record_number("link_record=", 0);
	db_list_setcur(change_map_record);

	strcpyl(change_map, db_list->dli_associated_sign_group, sizeof(change_map));

	strcpy(floor_command, "");
	goto load_change_map;
	}
else if(!strcmp(floor_command, "REPLAY"))
	{
	int found = FALSE;

	db_bann_select(11);			/* key with launch_pin, type, nines dtsec */

	strcpy(db_bann->res_id, res_id);
	strcpysl(db_bann->dbb_launch_pin, CurrentUserPin, sizeof(db_bann->dbb_launch_pin));
	strcpy(db_bann->dbb_msg_type_cod, bb_valid_types[BBT_ZX].string);
	strcpy(db_bann->dbb_rec_dtsec_nines, "");

	/* message is found and it is NOT the df_locations_map_call message */
	/* then look for the message having gone to speakers */
	if(db_bann_find() > 0
		&& !strcmp(db_bann->res_id, res_id)
		&& !strcmp(db_bann->dbb_launch_pin, CurrentUserPin)
		&& strcmp(db_bann->dbb_msg_name, df_locations_map_call)
		&& !strcmp(db_bann->dbb_msg_type_cod, bb_valid_types[BBT_ZX].string))
		{
		DBRECORD list_record;

		list_record = FindMultiAudioSignStreamNumberData(db_bann->dbb_stream_number);
		if(list_record > 0)
			{
			do
				{
				if(db_list_setcur(list_record) > 0)
					{
					if(db_list->dli_floor_plan_type == FLOORPLAN_MEDIAPORT
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_UP
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_DOWN
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_LEFT
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_RIGHT
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_GROUP_UP
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL
						|| db_list->dli_floor_plan_type == FLOORPLAN_SPEAKER_ALL_CALL_EMERGENCY)
						{
						db_wtc->dwc_record_num = db_bann_getcur();
						found = TRUE;

						if(command_wtc(WTC_WRITE, WTC_INTERCOM_REPLAY, WTC_BROWSER, WTC_BANNER_BOARD, 0, 0))
							{
							wtc_write_error_log(__FILE__, __LINE__);
							}

						break;
						}
					}
	
				list_record = NextMultiAudioSignStreamNumberData(db_bann->dbb_stream_number);
				}
			while(list_record > 0);
			}
		}

	if(found == FALSE)
		{
		sprintf(cgi_popup_message, "No message was found to replay.");
		}
	}
else if(!strcmp(floor_command, "MOVE"))
	{
	DBRECORD fp_move_record = cgi_parsed_lookup_record_number("fp_move_record=", 0);

	if(fp_move_record > 0)
		{
		/* we are moving a gif not adding a new one */
		db_list_setcur(fp_move_record);
		cgi_FloorPlanEncodeWidthHeight(db_list->dli_decode_primitive, sizeof(db_list->dli_decode_primitive), save_width, save_height);
		db_list_write();

		cgi_logging(DB_LOCATIONS_MANAGER, 0, DB_ISAM_WRIT, "moved image");
		}
	}
else if(!strcmp(floor_command, "SAVE"))
	{
	strcpy(db_list->res_id, res_id);
	strcpysl(db_list->dli_name, present_floorplan_name_no_suffix, sizeof(db_list->dli_name));
	db_list->dli_type = LIST_FLOORPLAN_MAP;
	strcpy(db_list->dli_who_can_use, "");
	strcpy(db_list->dli_who_can_modify, "");

	cgi_FloorPlanEncodeWidthHeight(db_list->dli_decode_primitive, sizeof(db_list->dli_decode_primitive), save_width, save_height);

	db_list->dli_floor_plan_type = save_image_type;

	strcpy(db_list->dli_pin, "");				//pin for person and phones ???? 
	strcpy(db_list->dli_help_string, "");			//note 
	strcpy(db_list->dli_decode_help_launch_string, "");	//directory for a message 			
	strcpy(db_list->dli_associated_organization, "");	//parent map name
	strcpy(db_list->dli_sister_audio_group, "");		//message name for a message
	strcpy(db_list->dli_associated_sign_group, "");		//sign group

	db_list_add();	/* add the icon/item record (will assign deviceid/name next ) */

	sprintf(other, "EditImageLocation(%d, " FORMAT_DBRECORD_STR ");", db_list->dli_floor_plan_type, db_list_getcur());
	}
else if(!strcmp(floor_command, "DELETEMAP"))
	{
	if(delete_recno > 0
		&& db_list_setcur(delete_recno) > 0)
		{
		int found;

		char delete_map_location[100];

		snprintf(delete_map_location, sizeof(delete_map_location), "/home/silentm/public_html/floor_plans/%s%s", TimeShareCompanyNameDirectoryGet(), present_floorplan_name);
		remove_trailing_space(delete_map_location);
		if(unlink(delete_map_location) < 0)
			{
			DIAGNOSTIC_LOG_4("User %s deleted map %s error %d %s", CurrentUserPin, delete_map_location, errno, strerror(errno));
			}
		else
			{
			DIAGNOSTIC_LOG_2("User %s deleted map %s", CurrentUserPin, delete_map_location);
			}

		db_list_delete();

		db_list_select(6);			/* select with type, name, and width, height (dli_decode_primitive) */

		/* need to delete all items in the map */
		do
			{
			strcpy(db_list->res_id, res_id);
			strcpysl(db_list->dli_name, present_floorplan_name_no_suffix, sizeof(db_list->dli_name));
			strcpy(db_list->dli_decode_primitive, "");
			db_list->dli_type = LIST_FLOORPLAN_MAP;
			if(db_list_find() > 0
				&& db_list->dli_type == LIST_FLOORPLAN_MAP
				&& !strcmp(db_list->res_id, res_id)
				&& !strcmp(db_list->dli_name, present_floorplan_name_no_suffix_with_space))
				{
				found = 1;
				db_list_delete();
				}
			else
				{
				found = 0;
				}
			}
		while(found);
		}

	/* meed to delete out of message record */
	if(me_msg_template_recno > 0
		&& db_bann_setcur(me_msg_template_recno) > 0)
		{
		int i;

		BannerOptions(me_msg_template_recno, DB_ISAM_READ);			/* initialize the structure so we can use it (grabs the current string of floorplan record numbers: db_bann_message_options->mo_multi_floorplan_records) */

		for(i = 0; i < MAX_MULTI_FLOORPLAN_SELECT; i++)
			{
			if(db_bann_message_options->mo_floorplan_group_recno[i] == delete_recno)
				{
				db_bann_message_options->mo_floorplan_group_recno[i] = 0;
				strcpy(db_bann_message_options->mo_floorplan_group[i], "");
				break;
				}
			}
		
		BannerOptions(me_msg_template_recno, DB_ISAM_WRIT);
		}

	strcpy(floor_command, "");
	floor_plan_header_recno = 0;			/* reload any map */
	goto load_change_map;
	}
else if(!strcmp(floor_command, "DISASSOCIATEMAP"))
	{
	char record_number_str[FLOORPLAN_MULTI_RECORD_LENGTH];
	int i;
	
	BannerOptions(me_msg_template_recno, DB_ISAM_READ);			/* initialize the structure so we can use it (grabs the current string of floorplan record numbers: db_bann_message_options->mo_multi_floorplan_records) */

	/* first, clear out the structure member that holds the string of floorplan record numbers (in preparation for re-populating it below) */
	strcpy(db_bann_message_options->mo_multi_floorplan_records, "");

	/* for each of the potential floorplans from the banner message options file... */
	for(i = 0; i < MAX_MULTI_FLOORPLAN_SELECT; i++)
		{
		/* if this iteration's floorplan is different than the one they intend to disassociate from the message, then concatenate it back into the structure... (this should exclude the one they want to disassociate, so it won't be saved back in) */
		if(db_bann_message_options->mo_floorplan_group_recno[i] != 0 
			&& db_bann_message_options->mo_floorplan_group_recno[i] != delete_recno)
			{
			sprintf(record_number_str, " " FORMAT_DBRECORD_STR ",", db_bann_message_options->mo_floorplan_group_recno[i]);
			strcatl(db_bann_message_options->mo_multi_floorplan_records, record_number_str, FLOORPLAN_MULTI_RECORD_LENGTH);
			}
		}

	/* write banneroptions to save */
	BannerOptions(me_msg_template_recno, DB_ISAM_WRIT);

	/* not sure, but these three lines may not be required -- suspect them if further bugs develop */
	strcpy(floor_command, "");
	floor_plan_header_recno = 0;			/* reload any map */
	goto load_change_map;
	}
/* just need to give this the dummy message's (df_locations_maps_visible_for_tab) record number rather than any other message number */
/* (dev-note 6/14/12 update): this routine should still be needed, in case a map actually has devices placed on it, in which case a delete from the list DB should not happen */
else if(!strcmp(floor_command, "DISASSOCIATEMAP_DELETE"))
	{
	char record_number_str[FLOORPLAN_MULTI_RECORD_LENGTH];
	int i;
	
	BannerOptions(me_msg_template_recno, DB_ISAM_READ);			/* initialize the structure so we can use it (grabs the current string of floorplan record numbers: db_bann_message_options->mo_multi_floorplan_records) */

	/* first, clear out the structure member that holds the string of floorplan record numbers (in preparation for re-populating it below) */
	strcpy(db_bann_message_options->mo_multi_floorplan_records, "");

	/* for each of the potential floorplans from the banner message options file... */
	for(i = 0; i < MAX_MULTI_FLOORPLAN_SELECT; i++)
		{
		/* if this iteration's floorplan is different than the one they intend to disassociate from the message, then concatenate it back into the structure... (this should exclude the one they want to disassociate, so it won't be saved back in) */
		if(db_bann_message_options->mo_floorplan_group_recno[i] != 0 
			&& db_bann_message_options->mo_floorplan_group_recno[i] != delete_recno)
			{
			sprintf(record_number_str, " " FORMAT_DBRECORD_STR ",", db_bann_message_options->mo_floorplan_group_recno[i]);
			strcatl(db_bann_message_options->mo_multi_floorplan_records, record_number_str, FLOORPLAN_MULTI_RECORD_LENGTH);
			}
		}

	/* write banneroptions to save */
	BannerOptions(me_msg_template_recno, DB_ISAM_WRIT);

	/* not sure, but these three lines may not be required -- suspect them if further bugs develop */
	strcpy(floor_command, "");
	floor_plan_header_recno = 0;			/* reload any map */
	goto load_change_map;
	}
else if(!strcmp(floor_command, "DELETE"))
	{
	if(db_list_setcur(delete_recno) > 0)
		{
		db_list_delete();

		cgi_logging(DB_LOCATIONS_MANAGER, 0, DB_ISAM_DELT, "delete image");

		}
	}

if(cgi_read_gif_width_height("floor_plans", present_floorplan_name, &backwidth, &backheight) < 0)
	{
	backwidth = 16*30;
	backheight = 16*30;
	}

if(cgi_read_gif_width_height("gifs", "map_invisible.gif", &space_width, &space_height) < 0)
	{
	space_width = 16;
	space_height = 16;
	}

add_CreateHelp();
add_CommentCall();
add_CreateNewMap(me_msg_template_recno);
add_GetRealPosition();
add_CreateShowCamera();
add_FormLoadedControl();
add_CreatePersonalPage();
add_operation_complete();
add_CreateMapCommentCall();
add_CreateMsgNameChoiceList();
add_font_style(NULL, DEFAULT_FONT_POINT_SIZE);
add_HelpTextLayerCheck(TRUE, FALSE, FALSE);
/* CR ADDED */
//addClientsideRoutesData();
addClientsideGraphicalAnnunciatorDevicesData_fromDB(present_floorplan_name_no_suffix_with_space);
/* CR END */

/* handle checking or unchecking of the "GA Default" checkbox */
locations_ga_default_fp_need_to_save = cgi_parsed_lookup_integer("locations_ga_default_fp_need_to_save=", 0);
cgi_parsed_lookup("locations_ga_default_floorplan=", locations_ga_default_floorplan, sizeof(locations_ga_default_floorplan));
//DIAGNOSTIC_LOG_1("locations_ga_default_floorplan = '%s'", locations_ga_default_floorplan);

BannerOptions(me_msg_template_recno, DB_ISAM_READ);			/* initialize the structure so we can use it (or write to it to save the user's checkbox selection after they submit the form by clicking done) */

//if(me_msg_template_recno > 0 		/* if we are flagged to save and we know the message template record number, continue */
if(flag_locations_tab_or_msg == FLAG_LOCATIONS_MSG 		/* if we are flagged to save and we know the message template record number, continue */
	&& locations_ga_default_fp_need_to_save)
	{
	/* save ga-default floorplan name to banner message options file */
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("Called BannerOptions: READ for msg template recno = "FORMAT_DBRECORD_STR, me_msg_template_recno);
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("    Going to write '%s' to db_bann_message_options->mo_ga_default_locations", locations_ga_default_floorplan);
	strcpyl(db_bann_message_options->mo_ga_default_locations, locations_ga_default_floorplan, FLOORPLAN_NAME_LENGTH);
	BannerOptions(me_msg_template_recno, DB_ISAM_WRIT);
	}

printf("<script type=\"text/javascript\">");
printf("var thisMsgDefaultFloorplan = \"%s\";", db_bann_message_options->mo_ga_default_locations);
printf("var currentlyLoadedFloorplan = \"%s\";", cgi_form_states.main_floor_plan_name);
printf("function gaDefaultMap_saveChange(fpToSave, parseRadioGroup){");
printf("	if(parseRadioGroup){");
printf("		objRadioGroup = document.getElementsByName(\"radiogroup_defaultgamsgfloorplans\");");
printf("		for(var i=0; i<objRadioGroup.length; i++){");
printf("			if(objRadioGroup[i].checked){");
printf("				fpToSave = objRadioGroup[i].value;");
printf("				break;");
printf("			}");
printf("		}");
printf("	}");
printf("	if(fpToSave != thisMsgDefaultFloorplan){");
printf("		document.forms[0].locations_ga_default_floorplan.value = fpToSave;");	/* set the floorplan that we're going to save */
printf("		document.forms[0].locations_ga_default_fp_need_to_save.value = 1;");	/* flag for saving the currently showing map as the default for this message */
//printf("window.alert(\"locations_ga_default_floorplan.value = '\"+document.forms[0].locations_ga_default_floorplan.value+\"'\\n(now going to submit form)\");");
printf("		document.forms[0].submit();");
printf("	}");
printf("	else{");
printf("		return false;");
printf("	}");
printf("}");
printf("function gaDefaultFloorplanCheckbox_onChange(){");
printf("	if(document.getElementById(\"locations_ga_default_floorplan\").checked){");
printf("		var checkboxState = \"checked\";");
printf("	}");
printf("	else{");
printf("		var checkboxState = \"unchecked\";");
printf("	}");
printf("	if(checkboxState == \"checked\"){");					/* the checkbox is checked, indicating that they want the currently loaded floorplan to become the default, so let's do some further checks... */
printf("		if(removeTrailingSpace(thisMsgDefaultFloorplan) == \"\"){");		/* if there is no default floorplan for this message (should not ever happen), then just save it without question */
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>You've chosen to make '\"+removeTrailingSpace(currentlyLoadedFloorplan)+\"' the default G.A. map for this message... Click OK to continue.</p>\";");
printf("			strHTML+=\"<p>Note: It is recommended you continue, since no other maps are currently default for this message. Without a default map, the G.A. message will not display any map on the device(s) it's sent to!</p>\";");
printf("			strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'document.getElementById(\\\"locations_ga_default_floorplan\\\").checked=\\\"\\\";Modalbox.hide();\\'>Cancel</button>&nbsp;&nbsp;\";");
printf("			strHTML+=\"<button class=\\'mb_button\\' onclick=\\'gaDefaultMap_saveChange(\\\"\"+currentlyLoadedFloorplan+\"\\\", false);\\'>OK</button></div>\";");
printf("			Modalbox.show(strHTML,{title:\"Confirm default map selection...\"});");
printf("		}");
printf("		else if(currentlyLoadedFloorplan != thisMsgDefaultFloorplan){");	/* if the currently loaded floorplan is not this message's default, then we will need to save the change (and ultimately de-default the other) */
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>You've chosen to change the default map for this G.A. message to '\"+removeTrailingSpace(currentlyLoadedFloorplan)+\"'... Click OK to continue.\";");
printf("			strHTML+=\"<p>Currently, the default map is '\"+thisMsgDefaultFloorplan+\"'. If you make this change, '\"+removeTrailingSpace(currentlyLoadedFloorplan)+\"' will become the floorplan that shows on the graphical annunciator device(s) for this message, instead of '\"+thisMsgDefaultFloorplan+\"'.</p>\";");
printf("			strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'document.getElementById(\\\"locations_ga_default_floorplan\\\").checked=\\\"\\\";Modalbox.hide();\\'>Cancel</button>&nbsp;&nbsp;\";");
printf("			strHTML+=\"<button class=\\'mb_button\\' onclick=\\'gaDefaultMap_saveChange(\\\"\"+currentlyLoadedFloorplan+\"\\\", false); Modalbox.hide();\\'>OK</button></div>\";");
printf("			Modalbox.show(strHTML,{title:\"Confirm default map change...\"});");
printf("		}");
printf("	}");
printf("	else{");								/* the checkbox is cleared, indicating that they don't want the currently loaded floorplan to become the default, so let's do some further checks... */
printf("		if(removeTrailingSpace(thisMsgDefaultFloorplan) == \"\"){");		/* if there is no default floorplan for this message (should not ever happen), then ??? */
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>There was no default map set for this message, and you've chosen to NOT make '\"+removeTrailingSpace(currentlyLoadedFloorplan)+\"' the default map for this message... Click OK to continue.</p>\";");
printf("			strHTML+=\"<p>Note: If you do not set this map as default, you should at least load another map and make it default. Not having a default map may result in this message not displaying a map on the device(s) it's sent to!</p>\";");
printf("			strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'gaDefaultMap_saveChange(\\\"\"+currentlyLoadedFloorplan+\"\\\", false);\\'>Set this map as default</button>&nbsp;&nbsp;\";");
printf("			strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide(); document.forms[0].locations_ga_default_fp_need_to_save.value=0;\\'>OK</button></div>\";");
printf("			Modalbox.show(strHTML,{title:\"Confirm default map selection...\"});");
printf("		}");
printf("		else if(currentlyLoadedFloorplan == thisMsgDefaultFloorplan){");	/* if the currently loaded floorplan is this message's default, then they don't want it to be default (but at least one needs to be), so offer them selection of floorplans to make default */
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>You've chosen to not make '\"+removeTrailingSpace(currentlyLoadedFloorplan)+\"' the default map for this message.</p>\";");
printf("			strHTML+=\"<p>Select which map you want to make the default for this message, instead:</p>\";");
printf("			for(var i=0; i<fpmoTable.length; i++){");
printf("				if( removeTrailingSpace(fpmoTable[i][fpmotiName]) == removeTrailingSpace(thisMsgDefaultFloorplan) ){");
printf("					strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' name=\\'radiogroup_defaultgamsgfloorplans\\' value=\\'\"+fpmoTable[i][fpmotiName]+\"\\'>\"+removeTrailingSpace(fpmoTable[i][fpmotiName])+\"</label>\";", cgi_icons);
printf("				}");
printf("				else{");
printf("					strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'radiogroup_defaultgamsgfloorplans\\' value=\\'\"+fpmoTable[i][fpmotiName]+\"\\'>\"+removeTrailingSpace(fpmoTable[i][fpmotiName])+\"</label>\";");
printf("				}");
printf("			}");
printf("			strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'gaDefaultMap_saveChange(\\\"\\\", true); Modalbox.hide();\\'>OK</button></div>\";");
printf("			Modalbox.show(strHTML,{title:\"Message requires a default map...\"});");
printf("		}");
printf("	}");
printf("}");
printf("</script>");
/* end handling of GA checkbox */

cgi_center_screen(0);
cgi_table_one_row_one_cell("border=0 cellpadding=0 cellspacing=0", "");

if(smbanner_locations)
	{
	/* dont show title when building a screen for smbanner */
	}
else
	{
	char fpPreTitle[92] = "<span id=\"fpPreTitle\">Device Map: </span><span id=\"fpPreTitlePost\">";

	/* currency is changed with checking persmissions on cgi_title */
	push_list = db_list_getcur();

//	cgi_title(present_floorplan_name_no_suffix, 0, 0, 0, FALSE);
	strcat(fpPreTitle, present_floorplan_name_no_suffix);
	cgi_title(fpPreTitle, 0, 0, 0, FALSE);
	strcat(fpPreTitle, "</span>");
	db_list_setcur(push_list);
	}

printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
printf("var locations_screen_refresh_timer = -1;");
printf("</SCRIPT>");

printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
printf("	mouseover_hold = new Image();");
printf("	mouseover_hold.src = '%s/map_invisible.gif';", cgi_icons);
printf("</SCRIPT>");

printf("<INPUT type=\"hidden\" name=\"locations_alert_status\" value=\"%d\"></INPUT>", locations_alert_status);
printf("<INPUT type=\"hidden\" name=\"save_width\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"save_height\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"save_image_type\" value=\"%d\"></INPUT>", FLOORPLAN_NONE);
printf("<INPUT type=\"hidden\" name=\"link_record\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"floor_command\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"mouse_last_left\" value=\"\" ></INPUT>");
printf("<INPUT type=\"hidden\" name=\"mouse_last_top\" value=\"\"></INPUT>");

add_JavascriptStrStr();
/* CR ADDED */
printf("<script src=\"%s/~silentm/javascripts/colorPicker.js\"></script>", cgi_base);
/* This is an HTML directive that only IE will see, that is necessary in order to patch IE into supporting the CANVAS element..... (Internet Explorer... don't let friends code for it!)  :p
 * Note: This should come before the related JavaScript below. 
 * Note: May need to check up on this later as new versions of IE come out... Also in support of potential SVG? */
printf("<!--[if IE]><script src=\"%s/~silentm/javascripts/excanvas.js\"></script><![endif]-->", cgi_base);
/* CR END */

printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
printf("var person_page_message = '';");

printf("var custom_message_mode = 0;");

printf("var map_over_width = 0;");
printf("var map_over_height = 0;");

printf("var new_image_selected = 1;");
printf("var move_name = 'background_image_div';");
printf("function NewImageMove(width, height){");
printf("	document.getElementById(move_name).style.left=width;");
printf("	document.getElementById(move_name).style.top=height;");
printf("	}");

printf("function NewTextSelected(){");
printf("	}");

printf("function NewImageSelected(type, gif){");
printf("	SetImage(type);");
printf("	document.getElementById('mouse_tracker_img').src=gif;");
printf("	document.getElementById('mouse_tracker_div').style.visibility='visible';");
printf("	return false;");
printf("	}");

printf("function ClearMultiIconSelection(){");
printf("	i = 0; ");
printf("	while(document.images[i])");
printf("		{");
printf("		if(document.images[i].src == '%s/map_sign_up_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_sign_up.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_sign_down_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_sign_down.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_sign_left_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_sign_left.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_sign_right_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_sign_right.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_up_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_up.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_down_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_down.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_left_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_left.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_all_call_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_all_call.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_all_call_emergency_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_all_call_emergency.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_right_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_right.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_group_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_group.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_speaker_group_up_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_speaker_group_up.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_pc_alert_connected_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_pc_alert_connected.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_pc_alert_not_connected_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_pc_alert_not_connected.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_mediaport_connected_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_mediaport_connected.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_mediaport_not_connected_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_mediaport_not_connected.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_phone_blinking.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_phone.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_person_list_blink.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_person_list.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_person_job_blink.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_person_job.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_person_blink.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_person.gif';", cgi_icons);
printf("		else if(document.images[i].src == '%s/map_video_camera.gif')", cgi_icons);
printf("			document.images[i].src = '%s/map_video_camera.gif';", cgi_icons);
printf("		i++;");
printf("		}");
printf("	}");

printf("function ShowMultiIconSelection(unique_name, record_number){");
printf("	var ret = 0;");
printf("	var new_gif = '';");
printf("	var cell_name = 'document.cell' + unique_name + '.src';");

printf("	if(eval('document.cell' + unique_name + '.src') == '%s/map_phone.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_phone_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_phone_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_up.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_sign_up_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_sign_up_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_down.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_sign_down_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_sign_down_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_left.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_sign_left_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_sign_left_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_right.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_sign_right_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_sign_right_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_group.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_group_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_group_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_group_up.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_group_up_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_group_up_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_up.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_up_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_up_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_down.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_down_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_down_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_left.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_left_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_left_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_right.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_right_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_right_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_all_call.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_all_call_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_all_call_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_all_call_emergency.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_speaker_all_call_emergency_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_speaker_all_call_emergency_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_list.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_person_list_blink.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_person_list_blink.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_job.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_person_job_blink.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_person_job_blink.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_person.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_person_blink.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_person_blink.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_video_camera.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_video_camera.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_video_camera.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_pc_alert_connected.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_pc_alert_connected_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_pc_alert_connected_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_pc_alert_not_connected.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_pc_alert_not_connected_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_pc_alert_not_connected_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_mediaport_connected.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_mediaport_connected_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_mediaport_connected_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");
printf("	else if(eval('document.cell' + unique_name + '.src') == '%s/map_mediaport_not_connected.gif')", cgi_icons);
printf("		{");
printf("		new_gif = '\"%s/map_mediaport_not_connected_blinking.gif\"';", cgi_icons);
printf("		eval(cell_name + ' = ' + new_gif);");
printf("		mouseover_hold.src='%s/map_mediaport_not_connected_blinking.gif';", cgi_icons);
printf("		ret=1;");
printf("		}");

/* see if we need to remove it from the list */
printf("	if(ret == 0)");
printf("		{");
printf("		var find_string = '';");

printf("		if(eval('document.cell' + unique_name + '.src') == '%s/map_phone_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_phone.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_up_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_sign_up.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_down_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_sign_down.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_left_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_sign_left.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_sign_right_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_sign_right.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_group_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_group.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_group_up_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_group_up.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_up_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_up.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_down_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_down.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_left_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_left.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_right_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_right.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_all_call_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_all_call.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_speaker_all_call_emergency_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_speaker_all_call_emergency.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_list_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person_list.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_job_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person_job.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_pc_alert_connected_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_pc_alert_connected.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_pc_alert_not_connected_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_pc_alert_not_connected.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_mediaport_connected_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_mediaport_connected.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_mediaport_not_connected_blinking.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_mediaport_not_connected.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_list_blink.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person_list.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_job_blink.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person_job.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_person_blink.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_person.gif'; ret = 0; }", cgi_icons);
printf("		else if(eval('document.cell' + unique_name + '.src') == '%s/map_video_camera.gif')", cgi_icons);
printf("			{mouseover_hold.src='%s/map_video_camera.gif'; ret = 0; }", cgi_icons);

			/* remove the selected record from our list */
//printf("		document.forms[0].fp_set_send_message.value=string;");
printf("		find_string = ' ' + record_number + ',';");
printf("		found = javascript_strstr(document.forms[0].fp_set_send_message.value, find_string);");
printf("		if(found >= 0)");
printf("			{");
printf("			var strh = document.forms[0].fp_set_send_message.value;");
printf("			var str1 = strh.substring(0, found);");
printf("			var str2 = strh.substring(found + find_string.length, strh.length);");
printf("			document.forms[0].fp_set_send_message.value=str1+str2;");
printf("			DecrementSelectionCount();");
printf("			}");
printf("		}");

printf("	return(ret);");
printf("	}");

printf("var person_sendto_pin='';");
printf("function ClearFirstPin(){");
printf("	person_sendto_pin='';");
printf("	}");

printf("function SetFirstPin(pin){");
printf("	if(person_sendto_pin == '')");
printf("		{");
printf("		person_sendto_pin=pin;");
printf("		}");
printf("	}");

printf("function SetSpeakerPhone(phone){");
printf("	if(document.forms[0].fp_speaker_phone.value == '')");
printf("		{");
printf("		document.forms[0].fp_speaker_phone.value=phone;");
printf("		}");
printf("	else if(phone != document.forms[0].fp_speaker_phone.value)");
printf("		{");
printf("		document.forms[0].fp_speaker_phone.value='MISMATCH';");
printf("		}");
printf("	}");

/* keep track of the speakers phone number */
/* if the phone number does not match then */
/* we need to switch to record mode */
printf("function GetSpeakerPhone(){");
printf("	return(document.forms[0].fp_speaker_phone.value);");
printf("	}");

printf("function ClearSpeakerPhone(){");
printf("	document.forms[0].fp_speaker_phone.value='';");
printf("	}");

/* record number and name is the span name */
printf("function MoveImageLocation(unique_name, record, name){");
printf("	if(cell_name != 0)");
printf("		{");
printf("		var cell_name = 'cell' + unique_name;");
printf("		document.getElementById('mouse_tracker_img').src=document.getElementById(cell_name).src;");
printf("		document.getElementById('mouse_tracker_div').style.visibility='visible';");
printf("		}");

printf("	if(GetMoveImageMode() == 0)");
printf("		{");
printf("		move_name=name;");						/* setup the move name */
printf("		document.forms[0].fp_move_record.value=record;");		/* save moving item info */
printf("		document.forms[0].save_image_type.value=\"1\";");		/* anything really */
printf("		CancelHelpText();");
printf("		}");
printf("	else");
printf("		{");
printf("		document.forms[0].save_width.value=map_over_width;");
printf("		document.forms[0].save_height.value=map_over_height;");
printf("		document.forms[0].delete_recno.value=record;");
printf("		document.forms[0].floor_command.value='MOVE';");
printf("		document.forms[0].submit();");
printf("		}");
printf("	}");

printf("function GetMoveImageMode(){");
printf("	return(parseInt(document.forms[0].fp_move_record.value));");
printf("	}");

/* Open a new window so the user can identify the name/id of the item they just placed on the map */
printf("function EditImageLocation(type, record_number){");
printf("	if(type == %d) return;", SELECTION_TYPE_NONE);
printf("	onload_auto_submit = \"%s\";", NEW_WINDOW_ONLOAD_DELAY_SUBMIT);
printf("	win = window.open('', 'SMSignSpeakerEdit', '%s,height=310,width=460,%s');", window_options, HTML_WINDOW_OPEN_LOCATION);	/* NOTE: had to enlarge this to make room for any potential modalboxes (like with placing a life safety device for support of graphical annunciator) */
CGI_CHECK_VALID_WINDOW_JAVASCRIPT;
printf("	str = '<HTML>';");
CGI_POP_WINDOW_STYLE;
printf("	str += '<BODY bgcolor=\"%s\" onLoad=\"' + onload_auto_submit + '\">';", main_bgcolor);
printf("	str += '<FORM action=%s method=post>';", cgi_server);
printf("	str += '<INPUT type=\"hidden\" name=\"%s\" value=\"%s\"></INPUT>';", cgi_identification_field, cgi_EncodeLoginPin(CurrentUserPin));
printf("	str += '<INPUT type=\"hidden\" name=\"loadEditSignSpeaker\" value=\"0\"></INPUT>';");
printf("	str += '<INPUT type=\"hidden\" name=\"type\" value=' + type + '></INPUT>';");
printf("	str += '<INPUT type=\"hidden\" name=\"record_number\" value=' + record_number + '></INPUT>';");
printf("	str += 'Loading Edit...';");
printf("	str += '</FORM>';");
printf("	str += '</BODY>';");
printf("	str += '</HTML>';");
printf("	win.document.write(str);");
printf("	win.document.close();");
printf("	SetEditItemMode(0);");
printf("	return false;");
printf("	}");

printf("function StoreImageLocation(event){");
printf("	if(GetImage() == %d)", SELECTION_TYPE_NONE);
printf("		{");
			/* nothing selected yet so just return */
printf("		}");
printf("	else if(GetMoveImageMode())");
printf("		{");
printf("		document.forms[0].save_width.value=document.forms[0].mouse_last_left.value;");
printf("		document.forms[0].save_height.value=document.forms[0].mouse_last_top.value;");
printf("		document.forms[0].delete_recno.value=" FORMAT_DBRECORD_STR ";", floor_plan_header_recno);
printf("		document.forms[0].floor_command.value='MOVE';");
printf("		document.forms[0].submit();");
printf("		}");
printf("	else");
printf("		{");
printf("		document.forms[0].save_width.value=document.forms[0].mouse_last_left.value;");
printf("		document.forms[0].save_height.value=document.forms[0].mouse_last_top.value;");
printf("		document.forms[0].save_image_type.value=document.forms[0].save_image_type.value;");
printf("		document.forms[0].floor_command.value='SAVE';");
printf("		document.forms[0].submit();");
printf("		}");
printf("	return false;");
printf("	}");

printf("function DeleteMap_do(){");
printf("	document.forms[0].delete_recno.value=" FORMAT_DBRECORD_STR ";", floor_plan_header_recno);
printf("	document.forms[0].floor_command.value='DELETEMAP';");
//printf("	document.forms[0].main_floor_plan_name.value=\"\";");		/* setting this to empty will cause the first map found to get deleted, not the desired one! */
printf("	document.forms[0].floor_plan_header_recno.value=\"\";");	/* this might be the only one (of these six related fields) that needs to be cleared, but won't hurt to clear these others for now - better safe than sorry */
printf("	document.forms[0].map_name.value=\"\";");
printf("	document.forms[0].locations_ga_default_floorplan.value=\"\";");
printf("	document.forms[0].routeFloorplanName_full.value=\"\";");
printf("	document.forms[0].routeFloorplanName.value=\"\";");
printf("	document.forms[0].submit();");
printf("	return false;");
printf("}");
printf("function DisassociateDeleteMap_do(){");
printf("	document.forms[0].delete_recno.value=" FORMAT_DBRECORD_STR ";", floor_plan_header_recno);
printf("	document.forms[0].floor_command.value='DISASSOCIATEMAP_DELETE';");	/* new way: not actually deleting, just disassociating from df_locations_maps_visible_for_tab */
//printf("	document.forms[0].main_floor_plan_name.value=\"\";");
printf("	document.forms[0].floor_plan_header_recno.value=\"\";");	/* this might be the only one (of these six related fields) that needs to be cleared, but won't hurt to clear these others for now - better safe than sorry */
printf("	document.forms[0].map_name.value=\"\";");
printf("	document.forms[0].locations_ga_default_floorplan.value=\"\";");
printf("	document.forms[0].routeFloorplanName_full.value=\"\";");
printf("	document.forms[0].routeFloorplanName.value=\"\";");
printf("	document.forms[0].submit();");
printf("	return false;");
printf("}");
printf("function DisassociateMap_do(){");
printf("	document.forms[0].delete_recno.value=" FORMAT_DBRECORD_STR ";", floor_plan_header_recno);
printf("	document.forms[0].floor_command.value='DISASSOCIATEMAP';");
printf("	document.forms[0].main_floor_plan_name.value=\"\";");
printf("	document.forms[0].floor_plan_header_recno.value=\"\";");	/* this might be the only one (of these six related fields) that needs to be cleared, but won't hurt to clear these others for now - better safe than sorry */
printf("	document.forms[0].map_name.value=\"\";");
printf("	document.forms[0].locations_ga_default_floorplan.value=\"\";");
printf("	document.forms[0].routeFloorplanName_full.value=\"\";");
printf("	document.forms[0].routeFloorplanName.value=\"\";");
printf("	document.forms[0].submit();");
printf("	return false;");
printf("}");

/* called when they click the 'delete map' button to delete this floorplan */
/* DEV-NOTE, CR 8/1/11: This no long actually deletes a map, but rather just disassociates it from the message template (even the 'fake' msg that now represents the locations tab)
 *			At this time, there are no plans from Kevin to actually do map-delete operations from the list database. */
/* DEV-NOTE, CR 6/4/12: It is now determined that there should be some potentially valid situations for deleting a map, even though it presents challenges to multi-user stability. To resolve this as best as possible, 
 * 			we will check whether a map has any devices placed on it; if it does, we will only disassociate; if it has no devices, it is ASSUMED safe to delete from the list db. The example of this would 
 * 			be if they uploaded a too-large map, and then quickly deleted it to re-upload a smaller version with the same name. */
printf("function DeleteMap(){");
printf("	var arrActiveDeviceNamesFound = [];");
printf("	var j = 0;");
printf("	for(var i=0; i<devicesTable.length; i++){");
printf("		if(parseInt(devicesTable[i][dtiIsLsd]) == 1 ");
printf("			&& parseInt(devicesTable[i][dtiActiveState]) == 1){");
printf("			arrActiveDeviceNamesFound[j] = removeTrailingSpace(devicesTable[i][dtiName]);");
printf("			j++;");
printf("		}");
printf("	}");
printf("	var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
		/* If this is not in the context of a message (as indicated by not having a valid message template record number in the form), then allow the map to be deleted from the system... */
printf("	if(parseInt(document.forms[0].me_msg_template_recno.value) == 0");
printf("		|| removeTrailingSpace(document.forms[0].me_msg_template_recno.value) == \"\"){");
printf("		if(arrActiveDeviceNamesFound.length == 1){");
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>This map contains a safety device that is marking it as active.</p>\";");
printf("			strHTML+=\"<p>If you delete this map, the safety device, \"+arrActiveDeviceNamesFound[0]+\", won't be active for any other maps.</p>\";");
printf("			strHTML+=\"<p>Click OK to delete, or Cancel to not delete the map.</p>\";");
printf("		}");
printf("		else if(arrActiveDeviceNamesFound.length > 1){");
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>This map contains multiple safety devices that are marking it as active.</p>\";");
printf("			strHTML+=\"<p>If you delete this map, the following safety devices will be affected:</p>\";");
printf("			strHTML+=\"<ul style=\\'max-height:55px; overflow:auto;\\'>\";");
printf("			for(var k=0; k<arrActiveDeviceNamesFound.length; k++){");
printf("				strHTML+=\"<li>&bull;&nbsp;\"+arrActiveDeviceNamesFound[k]+\"</li>\";");
printf("			}");
printf("			strHTML+=\"</ul>\";");
//printf("			strHTML+=\"<p><em>In-Depth:<br>If ever a safety device, which is placed on multiple maps, goes into an active/alarm state, a graphical annunciator would need to know which map to show on its screen. The user pre-determines this by marking a particular map as active for a given safety device. If you try to delete a map with this marked relationship to a safety device, the system will no longer know which map to display on the GA, if the safety device becomes active.</em></p>\";");
printf("			strHTML+=\"<p>Click OK to delete, or Cancel to not delete the map.</p>\";");
printf("		}");
printf("		else{");
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>You have asked to delete the present map (and any items it contains). Click OK to confirm or Cancel to not delete the map.</p>\";");
printf("		}");
printf("		strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'>\";");
printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide();return false;\\'>Cancel</button>&nbsp;\";");
printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'DisassociateDeleteMap_do();\\'>OK</button></div>\";");
printf("		Modalbox.show(strHTML,{title:\"Confirm Map Delete...\"});");
printf("	}");
		/* else, it looks like we're in the scope of a particular message, so the map should be disassociated from the message instead of deleted from the system... */
printf("	else{");
/* since locations tab is now treated essentially as a message (as far as map association goes), let's discern between them further */
if(flag_locations_tab_or_msg == FLAG_LOCATIONS_TAB)
	{
			/* Renewed functionality (6/4/12): actually delete a map from the list database, if there are no devices placed on it */
	printf("	if(parseInt(devicesTable.length) == 0){");
	printf("		strHTML+=\"<p class=\\'mb_mainmessage\\'>You are trying to permanently delete this map from the Locations tab.</p>\";");
	printf("		strHTML+=\"<p>Click OK to confirm, or Cancel to keep this map with the Locations tab.</p>\";");
	printf("		strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'>\";");
	printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide();return false;\\'>Cancel</button>&nbsp;\";");
	printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'DeleteMap_do();\\'>OK</button></div>\";");
	printf("		Modalbox.show(strHTML,{title:\"Confirm Locations Screen Map Deletion...\"});");
	printf("	}");
	printf("	else{");
	printf("		strHTML+=\"<p class=\\'mb_mainmessage\\'>You are trying to remove this map from the Locations tab. (you can still add it back again later)</p>\";");
	printf("		strHTML+=\"<p>Click OK to confirm, or Cancel to keep this map with the Locations tab.</p>\";");
	printf("		strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'>\";");
	printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide();return false;\\'>Cancel</button>&nbsp;\";");
	printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'DisassociateMap_do();\\'>OK</button></div>\";");
	printf("		Modalbox.show(strHTML,{title:\"Confirm Locations Screen Map Removal...\"});");
	printf("	}");
	}
else
	{
	printf("	strHTML+=\"<p class=\\'mb_mainmessage\\'>You are trying to delete this map from a message.</p>\";");
	printf("	strHTML+=\"<p>Deleting this map from this screen will not remove it completely from the system, but rather just dis-associate it from this message. \";");
	printf("	strHTML+=\"<p>Click OK to confirm, or Cancel to keep this map associated with the message.</p>\";");
	printf("	strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'>\";");
	printf("	strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide();return false;\\'>Cancel</button>&nbsp;\";");
	printf("	strHTML+=\"<button class=\\'mb_button\\' onclick=\\'DisassociateMap_do();\\'>OK</button></div>\";");
	printf("	Modalbox.show(strHTML,{title:\"Confirm Map-Message Disassociation...\"});");
	}
printf("	}");
printf("}");

printf("function CreateLaunchButton(){");
printf("	onload_auto_submit = \"%s\";", NEW_WINDOW_ONLOAD_DELAY_SUBMIT);
printf("	win = window.open('', 'SMButton', '%s,height=100,width=400,%s');", window_options, HTML_WINDOW_OPEN_LOCATION);
CGI_CHECK_VALID_WINDOW_JAVASCRIPT;
printf("	str = '<HTML>';");
CGI_POP_WINDOW_STYLE;
printf("	str += '<BODY bgcolor=\"%s\" onLoad=\"' + onload_auto_submit + '\">';", main_bgcolor);
printf("	str += '<FORM action=%s method=post>';", cgi_server);
printf("	str += '<INPUT type=\"hidden\" name=\"%s\" value=\"%s\"></INPUT>';", cgi_identification_field, cgi_EncodeLoginPin(CurrentUserPin));
printf("	str += '<INPUT type=\"hidden\" name=\"loadLaunchButton\" value=\"\"></INPUT>';");
printf("	str += '<INPUT type=\"hidden\" name=\"button_record_str\" value=\"' + document.forms[0].fp_set_send_message.value + '\"></INPUT>';");
printf("	str += '</FORM>';");
printf("	str += '</BODY>';");
printf("	str += '</HTML>';");
printf("	win.document.write(str);");
printf("	win.document.close();");
printf("	return;");
printf("	}");

/* this was originally the DeleteImageLocation function */
printf("function DeleteImageLocation_do(record_number){");
printf("		document.forms[0].floor_command.value='DELETE';");
printf("		document.forms[0].delete_recno.value=record_number;");
printf("		document.forms[0].submit();");
printf("		return false;");
printf("}");

/* called when they click on an item/icon with the intention of deleting it from this floorplan */
printf("function DeleteImageLocation(record_number){");
printf("	CancelHelpText();");
		/* find out if the item they're deleting happens to be a life safety device that is marking this map as active or not... */
printf("	var activeDeviceFound = false;");						/* initialize a flag (true condition will hold the device's name/id) */
printf("	for(var i=0; i<devicesTable.length; i++){");					/* for each device item on this map... */
printf("		if(parseInt(devicesTable[i][dtiRecNo]) == parseInt(record_number) ");		/* if we find the one they want to delete, */
printf("		&& parseInt(devicesTable[i][dtiIsLsd]) == 1 ");					/* and it is a life safety device, */
printf("		&& parseInt(devicesTable[i][dtiActiveState]) == 1){");				/* and it is marking the map as active, then... */
printf("			activeDeviceFound = devicesTable[i][dtiName];");				/* set our flag */
printf("			break;");									/* break out of the loop since we found what we need */
printf("		}");
printf("	}");
		/* if the flag was not set, then continue with original routine to go ahead and delete this item... */
printf("	if(!activeDeviceFound){");
printf("		DeleteImageLocation_do(record_number);");	/* call to the function containing the original routine, above */
printf("	}");
		/* else they are deleting that type of device, so prompt them... */
printf("	else{");
printf("		var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("		strHTML+=\"<p class=\\'mb_mainmessage\\'>The item you're trying to delete represents a safety device that is marking this map as active.</p>\";");
printf("		strHTML+=\"<p>Deleting this item will result in no other instances of \"+removeTrailingSpace(activeDeviceFound)+\" being active on any maps.</p>\";");
printf("		strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'>\";");
printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'Modalbox.hide();return false;\\'>Cancel</button>&nbsp;\";");
printf("		strHTML+=\"<button class=\\'mb_button\\' onclick=\\'DeleteImageLocation_do(\"+record_number+\");\\'>OK</button></div>\";");
printf("		Modalbox.show(strHTML,{title:\"Item is Marked Active...\"});");
printf("	}");
printf("}");

printf("function IncrementSelectionCount(){");
printf("	document.forms[0].fp_set_send_message_count.value=parseInt(document.forms[0].fp_set_send_message_count.value)+1;");
printf("	}");

printf("function DecrementSelectionCount(){");
printf("	document.forms[0].fp_set_send_message_count.value=parseInt(document.forms[0].fp_set_send_message_count.value)-1;");
printf("	}");

printf("function ClearSelectionCount(){");
printf("	document.forms[0].fp_set_send_message_count.value=\"0\";");
printf("	}");

printf("function GetSelectionCount(){");
printf("	return(document.forms[0].fp_set_send_message_count.value);");
printf("	}");

printf("function Redo(){");
printf("	document.forms[0].fp_set_send_message.value=document.forms[0].fp_redo_send_message.value;");
printf("	document.forms[0].fp_call_mode.value=document.forms[0].fp_call_mode_redo.value;");
printf("	document.forms[0].fp_selection_type.value=document.forms[0].fp_selection_type_redo.value;");
printf("	return false;");
printf("	}");

printf("function SetRedo(){");
printf("	document.forms[0].fp_redo_send_message.value=document.forms[0].fp_set_send_message.value;");
printf("	document.forms[0].fp_call_mode_redo.value=document.forms[0].fp_call_mode.value;");
printf("	document.forms[0].fp_selection_type_redo.value=document.forms[0].fp_selection_type.value;");
printf("	return false;");
printf("	}");

printf("function MapLink(record){");
printf("	document.forms[0].link_record.value=record;");
printf("	document.forms[0].floor_command.value='LINK';");
printf("	document.forms[0].submit();");
printf("	}");

printf("function CheckSomethingIsSelected(){");
printf("	if(document.forms[0].fp_set_send_message.value == '')");
printf("		{");
printf("		Redo();");							/* set the redo configuration */
printf("		if(document.forms[0].fp_set_send_message.value == '')");	/* if nothing still selected then warn user */
printf("			{");
printf("			alert('Click on signs or speakers below before attempting to send a message.');");
printf("			return(1);");
printf("			}");
printf("		}");
printf("	return(0);");
printf("	}");

printf("function SetCustomMessageMode(){");
printf("		custom_message_mode=1;");
printf("		return;");
printf("		}");

/* need to pop up the selection screen */
printf("function LaunchFloorPlanMessage(){");
printf("	if(CheckSomethingIsSelected())");
printf("		{");
printf("		return;");
printf("		}");

/* selection has signs or PC Alert so dont prompt for any selection - go right into launch text */
printf("	if(GetSelectionType() & %d || GetSelectionType() & %d)", SELECTION_TYPE_SIGNS, SELECTION_TYPE_PC_ALERT);
printf("		{");
printf("		SetCallMode(%d);", CALL_MODE_TEXT);		/* signs or pc alert selected so always default to text */
printf("		LaunchFloorPlanMessageContinue();");
printf("		return false;");
printf("		}");

printf("	win = window.open('', 'SMSelection', '%s,height=200,width=200,%s');", window_options, HTML_WINDOW_OPEN_LOCATION);
CGI_CHECK_VALID_WINDOW_JAVASCRIPT;
printf("	str = '<HTML>';");
CGI_POP_WINDOW_STYLE;
printf("	str += '<TITLE>Selection</TITLE>';");
printf("	str += '<BODY bgcolor=\"%s\" onload=\"self.focus(); if(document.forms[0].selection_submit && document.forms[0].selection_submit.value > 0) { if (opener) { opener.LaunchFloorPlanMessageContinue(); self.close(); } } return true;\">';", main_bgcolor);
printf("	str += '<FORM action=%s method=post>';", cgi_server);
printf("	str += '<FONT face=\"Arial,Helvetica\" size=\"2\" color=\"black\">';");
printf("	if(GetSelectionType() == %d)", (SELECTION_TYPE_SIGNS | SELECTION_TYPE_SPEAKERS));
printf("		{");
printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"2\" onclick=\"if(opener) opener.SetCallMode(%d); \" checked></INPUT>Type and Speak';", CALL_MODE_TEXT);
printf("		SetCallMode(%d);", CALL_MODE_TEXT);
printf("		}");
printf("	else if(GetSelectionType() == %d)", SELECTION_TYPE_SIGNS);
printf("		{");
/* signs only cannot speak live or record */
//printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"0\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Speak Live';", CALL_MODE_CALL);
//printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"1\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Record and Play';", CALL_MODE_VOICE_MESSAGE);

printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"2\" onclick=\"if(opener) opener.SetCallMode(%d); \" checked></INPUT>Type and Speak';", CALL_MODE_TEXT);
printf("		SetCallMode(%d);", CALL_MODE_TEXT);		/* only signs selected default to text */

printf("		}");
//printf("	else if(GetSelectionCount() == 1 && GetSelectionType() == %d)", SELECTION_TYPE_SPEAKERS);
printf("	else if(GetSelectionType() == %d)", SELECTION_TYPE_SPEAKERS);
printf("		{");
			map_speakers_message_recno = find_record_in_db_bann(silentm_default_directory, bb_valid_types[BBT_LF].string, df_locations_map_speakers);

			if(map_speakers_message_recno == 0)
				{
				show_ok_button = FALSE;
				printf("str += '<FONT face=\"Arial,Helvetica\">The message %s was not found.</FONT>';", df_locations_map_speakers);
				}
			else if(db_bann->dbb_pa_delivery_mode == PA_DELIVERY_MODE_CHOOSE)
				{
				printf("str += '<INPUT type=\"hidden\" name=\"selection_submit\" value=\"0\"></INPUT>';");
				printf("str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"0\" onclick=\"if(opener) opener.SetCallMode(%d); \" checked></INPUT>Speak Live';", CALL_MODE_CALL);

				if(configuration_lookup_integer(DB_PATH_LOCATION, "MICROPHONE_ACTIVE", 0))
					{
					if(cgi_user_is_on_console())
						{
						printf("str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"3\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Speak Microphone';", CALL_MODE_MICROPHONE);
						}
					}
	
				printf("str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"1\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Record and Play';", CALL_MODE_VOICE_MESSAGE);
				printf("str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"2\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Type and Speak';", CALL_MODE_TEXT);
				printf("SetCallMode(%d);", CALL_MODE_CALL);
				}
			else if(db_bann->dbb_pa_delivery_mode == PA_DELIVERY_MODE_SPEAK_LIVE)
				{
				printf("str += '<INPUT type=\"hidden\" name=\"selection_submit\" value=\"1\"></INPUT>';");
				printf("str += '<BR><INPUT type=\"hidden\" name=\"floor_plan_option\" value=\"0\" onclick=\"if(opener) opener.SetCallMode(%d); \" checked></INPUT>Speak Live';", CALL_MODE_CALL);
				printf("SetCallMode(%d);", CALL_MODE_CALL);
				}
			else if(db_bann->dbb_pa_delivery_mode == PA_DELIVERY_MODE_RECORD_AND_PLAY)
				{
				printf("str += '<INPUT type=\"hidden\" name=\"selection_submit\" value=\"1\"></INPUT>';");
				printf("str += '<BR><INPUT type=\"hidden\" name=\"floor_plan_option\" value=\"1\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Record and Play';", CALL_MODE_VOICE_MESSAGE);
				printf("SetCallMode(%d);", CALL_MODE_VOICE_MESSAGE);
				}
			else
				{
				printf("str += '<INPUT type=\"hidden\" name=\"selection_submit\" value=\"1\"></INPUT>';");
				printf("str += '<BR><INPUT type=\"hidden\" name=\"floor_plan_option\" value=\"2\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Type and Speak';", CALL_MODE_TEXT);
				printf("SetCallMode(%d);", CALL_MODE_TEXT);
				}
printf("		}");
printf("	else");
printf("		{");
printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"0\" onclick=\"if(opener) opener.SetCallMode(%d); \" checked></INPUT>Speak Live';", CALL_MODE_CALL);

			if(configuration_lookup_integer(DB_PATH_LOCATION, "MICROPHONE_ACTIVE", 0))
				{
				if(cgi_user_is_on_console())
					{
					printf("	str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"3\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Speak Microphone';", CALL_MODE_MICROPHONE);
					}
				}
			
printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"1\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Record and Play';", CALL_MODE_VOICE_MESSAGE);
printf("		str += '<BR><INPUT type=\"radio\" name=\"floor_plan_option\" value=\"2\" onclick=\"if(opener) opener.SetCallMode(%d); \"></INPUT>Type and Speak';", CALL_MODE_TEXT);
printf("		SetCallMode(%d);", CALL_MODE_CALL);
printf("		}");
printf("	str += '<BR>';");
printf("	str += '<P>';");
printf("	str += '<CENTER>';");
if(show_ok_button) 
	{
	printf("str += '<INPUT type=\"button\" value=\"OK\" onclick=\"if(opener) { opener.LaunchFloorPlanMessageContinue(); self.close(); }\"></INPUT>';");
	}

printf("	str += '&nbsp;';");
printf("	str += '<INPUT type=\"button\" value=\"Cancel\" onClick=\"if(opener) { opener.LaunchCancel(); } self.close();\"></INPUT>';");
printf("	str += '</CENTER>';");
printf("	str += '</FONT>';");
printf("	str += '</FORM>';");
printf("	str += '</BODY>';");
printf("	str += '</HTML>';");
printf("	win.document.write(str);");
printf("	win.document.close();");
printf("	return false;");
printf("	}");

printf("function LaunchCustomMessage(){");
printf("	SetRedo();");
printf("	CreatePersonalPage('%s', document.forms[0].custom_message.value, document.forms[0].custom_directory.value, '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, CurrentUserPin, "");
printf("	LaunchCancel();");
printf("	}");

printf("function LaunchFloorPlanMessageContinue(){");
printf("	SetRedo();");
printf("	if(custom_message_mode)");
printf("		{");
printf("		custom_message_mode=0;");
printf("		LaunchSelectMessage();");
printf("		return;");				/* need to RETURN here as cannot do the LaunchCancel() below yet */
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_BUTTON);
printf("		{");
printf("		CreateLaunchButton();");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_MESSAGES);
printf("		{");
printf("		CreatePersonalPage('%s', document.forms[0].set_launch_message.value, document.forms[0].set_launch_directory.value, '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, CurrentUserPin, "");
printf("		}");
printf("	else if(GetCallMode() == %d)", CALL_MODE_CALL);
printf("		{");
printf("		if(GetSelectionType() & %d)", SELECTION_TYPE_SIGNS);
printf("			{");
				/* force sign message */
printf("			CreatePersonalPage('%s', '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, df_locations_map_signs, "", CurrentUserPin, "");
printf("			}");
printf("		else if(GetSelectionType() & %d)", SELECTION_TYPE_MEDIAPORT);
printf("			{");
printf("			CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_mediaports, "", CurrentUserPin, "");
printf("			}");
printf("		else if(GetSelectionType() & %d)", SELECTION_TYPE_PC_ALERT);
printf("			{");
printf("			CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_signs, "", CurrentUserPin, "");
printf("			}");
printf("		else if(GetSelectionCount() > 1 && GetSelectionType() & %d)", SELECTION_TYPE_PHONES);
printf("			{");
				/* force record voice and deliver as cannot do multi phones */
printf("			CreateMapCommentCall('%s', document.forms[0].fp_set_send_message.value);", df_locations_map_record_voice);
printf("			}");
printf("		else if((%d == 0) && GetSelectionType() & %d)", live_intercom_access, SELECTION_TYPE_SPEAKERS);
printf("			{");
				/* no live access to speakers force recording then deliver */
printf("			CreateMapCommentCall('%s', document.forms[0].fp_set_send_message.value);", df_locations_map_record_voice);
printf("			}");
printf("		else if(GetSpeakerPhone() == 'MISMATCH')");
printf("			{");
				/* speaker dial number did not match so force record mode as cannot connect live to different systems */
printf("			CreateMapCommentCall('%s', document.forms[0].fp_set_send_message.value);", df_locations_map_record_voice);
printf("			}");
printf("		else");
printf("			{");
				/* connect live and speak */
printf("			CreateMapCommentCall('%s', document.forms[0].fp_set_send_message.value);", df_locations_map_call);
printf("			}");
printf("		}");
printf("	else if(GetCallMode() == %d)", CALL_MODE_VOICE_MESSAGE);
printf("		{");
printf("		if(GetSelectionType() & %d)", SELECTION_TYPE_SIGNS);
printf("			{");
				/* force sign message */
printf("			CreatePersonalPage('%s', '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, df_locations_map_signs, "", CurrentUserPin, "");
printf("			}");
printf("		else if(GetSelectionType() & %d)", SELECTION_TYPE_MEDIAPORT);
printf("			{");
printf("			CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_mediaports, "", CurrentUserPin, "");
printf("			}");
printf("		else if(GetSelectionType() & %d)", SELECTION_TYPE_PC_ALERT);
printf("			{");
printf("			CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_signs, "", CurrentUserPin, "");
printf("			}");
printf("		else");
printf("			{");
				/* record voice and deliver */
printf("			CreateMapCommentCall('%s', document.forms[0].fp_set_send_message.value);", df_locations_map_record_voice);
printf("			}");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_MEDIAPORT);
printf("		{");
printf("		CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_mediaports, "", CurrentUserPin, "");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_PC_ALERT);
printf("		{");
printf("		CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_signs, "", CurrentUserPin, "");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_SIGNS);
printf("		{");
printf("		CreatePersonalPage('%s', '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, df_locations_map_signs, "", CurrentUserPin, "");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_PHONES);
printf("		{");
printf("		CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_speakers, "", CurrentUserPin, "");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_PERSON);
printf("		{");
printf("		CreatePersonalPage(person_sendto_pin, '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", df_locations_map_person, "", CurrentUserPin, "");
printf("		}");
printf("	else if(GetSelectionType() & %d)", SELECTION_TYPE_SPEAKERS);
printf("		{");
printf("		CreatePersonalPage('%s', '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, df_locations_map_speakers, "", CurrentUserPin, "");
printf("		}");
printf("	else");
printf("		{");
printf("		CreatePersonalPage('%s', '%s', '%s', '%s', 3, '', '', '%s', document.forms[0].fp_set_send_message.value, GetCallMode());", CurrentUserPin, df_locations_map_signs_and_speakers, "", CurrentUserPin, "");
printf("		}");

printf("	LaunchCancel();");
printf("	}");

printf("function LaunchSelectMessage(){");
printf("	if(CheckSomethingIsSelected())");
printf("		{");
printf("		return;");
printf("		}");

printf("	CreateMsgNameChoiceList(document.forms[0].custom_directory.value, 'custom_directory', 'custom_message', '', %d, 'LaunchCustomMessage', 1, '');", BBT_ANY);
printf("	}");

printf("function LaunchCancel(){");
printf("	document.forms[0].fp_set_send_message.value='';");
printf("	ClearMultiIconSelection();");
printf("	SetSelectionType(%d);", SELECTION_TYPE_NONE);
printf("	ClearSpeakerPhone();");
printf("	ClearFirstPin();");
printf("	ClearSelectionCount();");
printf("	}");

printf("function MapReplay(){");
printf("	document.forms[0].floor_command.value='REPLAY';");
printf("	document.forms[0].submit();");
printf("	}");

printf("function GetDeleteMode(){");
printf("	return(parseInt(document.forms[0].fp_delete_mode.value));");
printf("	}");

printf("function SetDeleteMode(mode){");
printf("	document.forms[0].fp_delete_mode.value=mode;");
printf("	}");

	/* for getting which lsd/floorplan that is active for a GA type message */
printf("function GetShowMapsMode(){");
printf("	return(parseInt(document.forms[0].fp_showmaps_mode.value));");
printf("	}");

	/* for choosing which lsd/floorplan to make active for a GA type message */
printf("function SetShowMapsMode(mode){");
printf("	document.forms[0].fp_showmaps_mode.value=mode;");
printf("	}");

printf("function GetCallMode(){");
printf("	return(parseInt(document.forms[0].fp_call_mode.value));");
printf("	}");

printf("function SetCallMode(mode){");
printf("	document.forms[0].fp_call_mode.value=mode;");
printf("	}");

	/* edit item mode is editing an idividual icon */
printf("function GetEditItemMode(){");
printf("	return(parseInt(document.forms[0].fp_edit_item_mode.value));");
printf("	}");

	/* edit item mode is editing an idividual icon */
printf("function SetEditItemMode(mode){");
printf("	if(mode == 0) document.forms[0].fp_move_record.value=\"0\";");		/* reset the record to move */
printf("	document.forms[0].fp_edit_item_mode.value=mode;");
printf("	}");

	/* edit mode is entering edit mode */
printf("function GetEditMode(){");
printf("	return(parseInt(document.forms[0].fp_edit_mode.value));");
printf("	}");

	/* edit mode is entering edit mode */
printf("function SetEditMode(mode){");
printf("	document.forms[0].fp_edit_mode.value=mode;");
printf("	}");

printf("function GetSelectionType(){");
printf("	return(parseInt(document.forms[0].fp_selection_type.value));");
printf("	}");

printf("function SetSelectionType(mode){");
printf("	if(mode == 0) document.forms[0].fp_selection_type.value=mode;");
printf("	else document.forms[0].fp_selection_type.value|=mode;");
printf("	}");

printf("function SetImage(type){");
printf("	document.forms[0].save_image_type.value=type;");
printf("	}");

printf("function GetImage(type){");
printf("	return(parseInt(document.forms[0].save_image_type.value));");
printf("	}");

printf("function OneClickIgnore(record_number){");
printf("	}");

printf("function OneClickSendMessage(record_number){");
printf("	var string = '';");
printf("	string = record_number + ',' + document.forms[0].fp_set_send_message.value;");
printf("	document.forms[0].fp_set_send_message.value=string;");
printf("	LaunchFloorPlanMessageContinue();");
printf("	}");

printf("function SetSendMessage(record_number){");
printf("	var string = '';");
printf("	string = ' ' + record_number + ',' + document.forms[0].fp_set_send_message.value;");
printf("	document.forms[0].fp_set_send_message.value=string;");
printf("	IncrementSelectionCount();");
printf("	}");

/* These next several tests/blocks/sections take care of parsing the values out of the posted data, as generated by CreateSessionWindowCloseLoginWindow in smcgi.c  -- do not move! */
printf("\nvar autoEnterRouteMode;\n");
submission_to_add_route = cgi_parsed_lookup_integer("submissionToAddRoute=", 0);
submission_to_delete_route = cgi_parsed_lookup_integer("submissionToDeleteRoute=", 0);
submission_to_edit_route = cgi_parsed_lookup_integer("submissionToEditRoute=", 0);

if(cgi_parsed_lookup_exist("routeMediaportOrigin="))
	{
	cgi_parsed_lookup("routeMediaportOrigin=", route_mediaport_origin, sizeof(route_mediaport_origin));
	}
else
	{
	strcpy(route_mediaport_origin, "");
/*	strcpy(state_change_message, "Route NOT Saved (invalid device name)");*/
	}
if(cgi_parsed_lookup_exist("routeMediaportOrigin_old="))
	{
	cgi_parsed_lookup("routeMediaportOrigin_old=", route_mediaport_origin_old, sizeof(route_mediaport_origin_old));
	}

if(cgi_parsed_lookup_exist("routeFloorplanName="))
	{
	cgi_parsed_lookup("routeFloorplanName=", route_floorplan_name, sizeof(route_floorplan_name));
	}
else
	{
	strcpy(route_floorplan_name, "NULL");
/*	strcpy(state_change_message, "Route NOT Saved (invalid floorplan name)");*/
	}
if(cgi_parsed_lookup_exist("routeFloorplanName_old="))
	{
	cgi_parsed_lookup("routeFloorplanName_old=", route_floorplan_name_old, sizeof(route_floorplan_name_old));
	}

route_active_flag = cgi_parsed_lookup_integer("routeActiveFlag=", ROUTE_ACTIVE_FLAG_ON);  /* if changing, be sure this default number matches specified/matching route thicknesses in the JS function thickChooser below! */
route_active_flag_old = cgi_parsed_lookup_integer("routeActiveFlag_old=", ROUTE_ACTIVE_FLAG_ON);  /* NOTE: may not be necessary? */

route_thickness = cgi_parsed_lookup_integer("routeThickness=", ROUTE_THICKNESS_NORMAL);  /* if changing, be sure this default number matches specified/matching route thicknesses in the JS function thickChooser below! */
route_thickness_old = cgi_parsed_lookup_integer("routeThickness_old=", ROUTE_THICKNESS_NORMAL);  /* NOTE: may not be necessary? */

if(cgi_parsed_lookup_exist("routePathColor="))
	{
	cgi_parsed_lookup("routePathColor=", route_path_color, sizeof(route_path_color));
	if(strcmp(remove_trailing_space(route_path_color), "") == 0)  /* safety to make sure we don't save blank field to file */
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("loadFloorPlan() WARNING - routePathColor was submitted blank or detected as blank for some reason... default color being substituted so file can't break");
		strcpy(route_path_color, ROUTE_COLOR_DEFAULT);
		}
	}
else
	{
	strcpy(route_path_color, "#ee0000"); /* set default color to red if no data exists */
	}
if(cgi_parsed_lookup_exist("routePathColor_old="))
	{
	cgi_parsed_lookup("routePathColor_old=", route_path_color_old, sizeof(route_path_color_old));
	}

if(cgi_parsed_lookup_exist("routeCoordsStr="))
	{
	cgi_parsed_lookup("routeCoordsStr=", route_coordinates_string, sizeof(route_coordinates_string));
	remove_trailing_space(route_coordinates_string);
	}
if(cgi_parsed_lookup_exist("routeCoordsStr_old="))
	{
	cgi_parsed_lookup("routeCoordsStr_old=", route_coordinates_string_old, sizeof(route_coordinates_string_old));
	remove_trailing_space(route_coordinates_string_old);
	}

if(cgi_parsed_lookup_exist("routeDtsec="))
	{
	cgi_parsed_lookup("routeDtsec=", route_dtsec, sizeof(route_dtsec));
	}

if(cgi_parsed_lookup_exist("routeUser="))
	{
	cgi_parsed_lookup("routeUser=", route_user, sizeof(route_user));
	}


if(cgi_parsed_lookup_exist("stateChangeMessage="))
	{
	cgi_parsed_lookup("stateChangeMessage=", state_change_message, sizeof(state_change_message));
	remove_trailing_space(state_change_message);
	}

if(strcmp(state_change_message, "Page Reloaded") == 0)
	{
	printf("autoEnterRouteMode=1;");
	}

if(cgi_parsed_lookup_exist("routeLastSubmitReason="))
	{
	cgi_parsed_lookup("routeLastSubmitReason=", route_last_submit_reason, sizeof(route_last_submit_reason));
	remove_trailing_space(route_last_submit_reason);
	}

if(cgi_parsed_lookup_exist("routeLastUserIntention="))
	{
	cgi_parsed_lookup("routeLastUserIntention=", route_last_user_intention, sizeof(route_last_user_intention));
	remove_trailing_space(route_last_user_intention);
	}

route_index_at_submit_time = cgi_parsed_lookup_integer("routeIndexAtSubmitTime=", 0);
if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("loadFloorPlan() routeIndexAtSubmitTime = %d", route_index_at_submit_time);

if(cgi_parsed_lookup_exist("selectedActiveFloorplan="))
	{
	cgi_parsed_lookup("selectedActiveFloorplan=", selected_active_floorplan, sizeof(selected_active_floorplan));
	}

/* If requested, save new information to text file (/home/silentm/record/routes/[msgDir].[msgName].routes) (acts as our routes database) */
if(submission_to_add_route == 1
	&& submission_to_delete_route == 0
	&& submission_to_edit_route == 0
	&& strcmp(route_mediaport_origin, "") != 0)
	{
	do_add_route();
	if(strcmp(remove_trailing_space(selected_active_floorplan), "") != 0)		/* if something has been submitted in this field, then that means we need to do an update to the route-record file (as determined by client script before this) */
		{
		change_active_status_for_all_routes_for(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_ON);		/* activate the specified floorplan, for this device */
		change_active_status_for_all_routes_except_floorplan(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_OFF);	/* deactivate all floorplans other than that specified, for this device */
		}
	}
else if(submission_to_add_route == 1			/* else, do an error catch due to blank mediaport-origin (saving a blank one will corrupt the file) - Note that this is a redundancy measure because the client also does this check before submitting the form */
	&& submission_to_delete_route == 0
	&& submission_to_edit_route == 0
	&& strcmp(route_mediaport_origin, "") == 0)
	{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("loadFloorPlan() ERROR: Add new route operation by '%s' for message '%s' FAILED due to blank route_mediaport_origin...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));
	}

/* If requested, delete existing information from text file */
if(submission_to_delete_route == 1
	&& submission_to_add_route == 0
	&& submission_to_edit_route == 0)
	{
	do_delete_route();
	if(strcmp(remove_trailing_space(selected_active_floorplan), "") != 0)		/* if something has been submitted in this field, then that means we need to do an update to the route-record file (as determined by client script before this) */
		{
		change_active_status_for_all_routes_for(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_ON);		/* activate the specified floorplan, for this device */
		change_active_status_for_all_routes_except_floorplan(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_OFF);	/* deactivate all floorplans other than that specified, for this device */
		}
	}

/* If requested, edit existing information in text file */
//printf("window.alert(\"DEBUGGING:\\n\\nsubmission_to_edit_route = %d\\n\\nroute_mediaport_origin_old = %s\\nroute_coordinates_string_old = %s\\nroute_path_color_old = %s\\nroute_path_color_old = %s\\n\\nroute_thickness_old = %d\\nroute_thickness_old = %d\");", submission_to_edit_route, route_mediaport_origin_old, remove_trailing_space(route_coordinates_string_old), route_path_color, route_path_color_old, route_thickness, route_thickness_old);
if(submission_to_edit_route == 1			/* this is a unique flag combination that should only be the case if we are submitting a change-active-floorplan command */
	&& strcmp(remove_trailing_space(selected_active_floorplan), "") != 0)
	{
	change_active_status_for_all_routes_for(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_ON);		/* activate the specified floorplan, for this device */
	change_active_status_for_all_routes_except_floorplan(route_mediaport_origin, selected_active_floorplan, ROUTE_ACTIVE_FLAG_OFF);	/* deactivate all floorplans other than that specified, for this device */
	}
else if(submission_to_edit_route == 1
	&& submission_to_add_route == 0
	&& submission_to_delete_route == 0
	&& strcmp(route_mediaport_origin, "") != 0)
	{
	do_edit_route(route_mediaport_origin_old, route_mediaport_origin, route_floorplan_name, route_active_flag_old, route_active_flag, route_thickness_old, route_thickness, route_path_color_old, route_path_color, route_coordinates_string_old, route_coordinates_string);
	}
else if(submission_to_add_route == 1			/* else, do an error catch due to blank mediaport-origin (saving a blank one will corrupt the file) - Note that this is a redundancy measure because the client also does this check before submitting the form */
	&& submission_to_delete_route == 0
	&& submission_to_edit_route == 0
	&& strcmp(route_mediaport_origin, "") == 0)
	{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("loadFloorPlan() ERROR: Edit existing route operation by '%s' for message '%s' FAILED due to blank route_mediaport_origin...", remove_trailing_space(CurrentUserPin), remove_trailing_space(route_msg_name));
//		strcpy(state_change_message, "ERROR: Edited Route NOT Saved");
	}

remove_trailing_space(state_change_message);

/* This is the majority of the JavaScript overhead necessary for the route-drawing and creation process. */
/* NOTE: Some of this code, if changed, will also lead to needing a corresponding change in smcgi_xmlhttp.c - be sure to check that things will match, if updating! */
/* NOTE: This code can be harder to read, because it has been purposly condensed (removed white spaces) to help transfer to and render on the client more efficiently, so be careful. */
printf("</script>");	/* temporarily exit the script so we can load other JS */
printf("<script src=\"%s/~silentm/javascripts/modalbox/lib/prototype.js\"></script>", cgi_base);
printf("<script src=\"%s/~silentm/javascripts/modalbox/lib/scriptaculous.js\"></script>", cgi_base);
printf("<script src=\"%s/~silentm/javascripts/modalbox/lib/effects.js\"></script>", cgi_base);
printf("<script src=\"%s/~silentm/javascripts/modalbox/modalbox.js\"></script>", cgi_base);
printf("<link rel=\"stylesheet\" href=\"%s/~silentm/javascripts/modalbox/modalbox.css\" type=\"text/css\" media=\"screen\" />", cgi_base);
printf("<script type=\"text/javascript\">");	/* re-enter script */

/* setup globals */
printf("var routePathWidth=%d;\n", route_thickness);	/* define path thickness - just for initial starting point for now */
printf("var snapEnabled=false; snapDistance=10;\n");	/* define snap (i.e. straight-line-assist) attributes -- distance is number of pixels deviated from the x or y axis */
printf("var canvasPosX=canvasPosY=0;\n");		/* set coordinate plane tare to zero for both dimensions, X and Y */
printf("var previ=-1; var curri=0;\n");			/* setup increment counters that may be used */
printf("var coordinates=new Array(2);\n");		/* initialize array to hold a single point-coordinate pair (two elements representing X and Y - this is a 2D array) - this is used primarily for storing position when a click is registered */
printf("var routePoints=new Array();\n");		/* initialize array to hold all of a specific route's coordinate pairs (dynamic number of elements representing points along a path - this is a multidimensional array: each element contains a 2D array representing a coord-pair X,Y) */
printf("var canvasContainer,canvas,canvasContext;\n");	/* define canvas DOM/HTML-element stuff */
printf("var showRouteClickCounter=0;\n");		/* counter for keeping track of how many times they click on a particular LCD -- for toggling through each potential route an LCD may have */
printf("var showRoutePreviousDeviceClicked=null;\n");	/* global for keeping track of the last LCD icon they clicked */
printf("var routesDrawn=0;\n");				/* global for keeping track of the number of routes drawn (mainly by show/more functions) */
printf("var tempGlobalArrForSpecificLCD;\n");		/* set a temporary global variable that we can insert an array into for holding route info across function calls */
printf("var lcdIconImgRegistry=[];\n");			/* array for storing info about what LCD icons have been rendered on screen when the page is loaded (used for being able to go back through and update them if they have associated routes or not) */
printf("var iconRegistryArrayLength=0;\n");		/* global counter for working through that array across function calls */
printf("var arrPreExistingFloorplans=[];\n");		/* global array for holding any pre-existing floorplans that may already be in the route record file */
printf("var temp_arrLsdMaps = [];");
printf("var maxPoints = %d;", (int)(sizeof(route_coordinates_string)/10));

/* Looks through the main devicesTable for the given record number and returns the type if found, otherwise returns false */
printf("function returnDeviceIDForRecNo(recno){");
printf("	for(var i=0; i<devicesTable.length; i++){");			/* for each of the devices in the main devicesTable array (should be all devices visible on this map screen)... */
printf("		if(recno==devicesTable[i][dtiRecNo]){");			/* if our item is found, then... */
printf("			return devicesTable[i][dtiName];");			/* return its type */
printf("		}");
printf("	}");
printf("	console.warn(\"returnDeviceIDForRecNo(): Could not find deviceid for recno '\"+recno+\"' in the devicesTable.\");");
printf("	return "";");						/* else we finished our loop above without returning anything, so return false */
printf("}\n");

printf("function returnTypeForRecNo(recno){");
printf("	for(var i=0; i<devicesTable.length; i++){");			/* for each of the devices in the main devicesTable array (should be all devices visible on this map screen)... */
printf("		if(recno==devicesTable[i][dtiRecNo]){");			/* if our item is found, then... */
printf("			return devicesTable[i][dtiType];");				/* return its type */
printf("		}");
printf("	}");
printf("	console.warn(\"returnDeviceIDForRecNo(): Could not find device type for recno '\"+recno+\"' in the devicesTable.\");");
printf("	return false;");						/* else we finished our loop above without returning anything, so return false */
printf("}\n");

/* Traverses the DOM tree consisting of ALL device icons, looking for ANY null entries that would indicate an unidentified/unsaved device placement (or otherwise, as coded in each unique case) 
 * The reasoning for doing it this way was to more accurately look through exactly what the user would see on the screen... */
/* NOTE: if adding types to its case structure, be sure to add the appropriate hidden fields to each cell/icon, right after the </A> tag..... TEMPLATE NEXT LINE: */
//	printf("<input type=\"hidden\" id=\"hiddenfieldcell%d\" name=\"hiddenfieldcell%d\" value=\"%s\" />", unique_name, unique_name, db_hard->dhc_deviceid);  /* this will be used by XXXXX() function to determine if there are any unidentified/unsaved XXXX devices placed on the map */
printf("function isNullDevicePlaced(deviceType){");
printf("	var deviceDomParent = document.getElementById(\"main_image\");");					/* the DOM common parent (container) of the device icons */
printf("	var strSpanID, intType;");
printf("	switch(deviceType){");											/* in the case of... */
printf("		case \"LCD\":");											/* an LCD */
printf("			for(var i=0; i<deviceDomParent.childNodes.length; i++){");						/* loop through the child nodes (should be mostly span elements, aka 'cells', for containing all stuff about icons) */
printf("				if(deviceDomParent.childNodes[i].nodeName.toUpperCase() == 'SPAN'){");					/* if child is a span element, then we're getting warmer, so continue... */
printf("					strSpanID = deviceDomParent.childNodes[i].id;");						/* get this span's ID (which contains the record number) */
printf("					if(returnTypeForRecNo(parseInt(strSpanID.replace(\"image\",\"\")))==%d){", FLOORPLAN_MEDIAPORT);/* if this span's ID indicates it is a mediaport, let's continue... */
printf("						for(var ii=0; ii<deviceDomParent.childNodes[i].childNodes.length; ii++){");			/* loop through the child's child nodes (should be mostly anchor elements and our hidden field elements) */
printf("							if(deviceDomParent.childNodes[i].childNodes[ii].nodeName.toUpperCase() == 'INPUT'){");		/* if this child is an input element, we're getting hot now... */
printf("								if(deviceDomParent.childNodes[i].childNodes[ii].getAttribute('type')=='hidden' ");		/* if that input element is hidden, getting really really hot, */
printf("								&& !deviceDomParent.childNodes[i].childNodes[ii].getAttribute('value')){");			/* AND it also has an empty value, then BINGO... */
printf("									return true;");											/* return true */
printf("								}");
printf("							}");
printf("						}");
printf("					}");
printf("				}");
printf("			}");
printf("			break;");
printf("		case \"LSD\":");											/* a life safety device */
printf("			for(var i=0; i<deviceDomParent.childNodes.length; i++){");						/* loop through the child nodes (should be mostly span elements, aka 'cells', for containing all stuff about icons) */
printf("				if(deviceDomParent.childNodes[i].nodeName.toUpperCase() == 'SPAN'){");					/* if child is a span element, then we're getting warmer, so continue... */
printf("					strSpanID = deviceDomParent.childNodes[i].id;");						/* get this span's ID (which contains the record number) */
printf("					intType = returnTypeForRecNo(parseInt(strSpanID.replace(\"image\",\"\")));");
printf("					if(intType==%d ", FLOORPLAN_PULL_STATION);/* if this span's ID indicates it is a life-safety device, let's continue... */
printf("					|| intType==%d ", FLOORPLAN_WATER_FLOW);
printf("					|| intType==%d ", FLOORPLAN_DUCT_DETECTOR);
printf("					|| intType==%d ", FLOORPLAN_SMOKE_DETECTOR);
printf("					|| intType==%d ", FLOORPLAN_MOTION_DETECTOR);
printf("					|| intType==%d ", FLOORPLAN_SOUND_METER);
printf("					|| intType==%d){", FLOORPLAN_DOOR_OPEN);
printf("						for(var ii=0; ii<deviceDomParent.childNodes[i].childNodes.length; ii++){");				/* loop through the child's child nodes (should be mostly anchor elements and our hidden field elements) */
printf("							if(deviceDomParent.childNodes[i].childNodes[ii].nodeName.toUpperCase() == 'INPUT'){");			/* if this child is an input element, we're getting hot now... */
printf("								if(deviceDomParent.childNodes[i].childNodes[ii].getAttribute('type')=='hidden' ");			/* if that input element is hidden, getting really really hot, */
printf("								&& !deviceDomParent.childNodes[i].childNodes[ii].getAttribute('value')){");				/* AND it also has an empty value, then BINGO... */
printf("									return true;");												/* return true */
printf("								}");
printf("							}");
printf("						}");
printf("					}");
printf("				}");
printf("			}");
printf("			break;");
printf("		default:");
printf("			break;");
printf("	}");
printf("	return false;");
printf("}\n");

/* Returns the number of different maps that the specified life-safety device exists on */
printf("function numberOfMapsThatExistFor(strDeviceID){");
printf("	return 2;");  /* just needs to be > 1 for now... may enot eventually need this function? */
printf("}\n");

/* Returns an array of the devicesTable array containing only life-safety-devices */
printf("function extractOnlyLifeSafetyDevices(){");
printf("	var arr;");
printf("	if(devicesTable!=null){");
printf("		arr=[];");								/* initialize a new array with local scope that we will later return */
printf("		var j=0;");								/* increment counter that will be used for populating the local array */
printf("		for(var i=0;i<devicesTable.length;i++){");				/* loop through each of the main devices table (created earlier by C-written javascript) */
printf("			if(parseInt(devicesTable[i][dtiIsLsd])==1){");				/* if we find a match of the specified LCD, then we need to extract this route */
printf("				arr[j]=devicesTable[i].slice(0);");					/* copy the record's elements by value */
printf("				j++;");									/* increment the counter to prepare for any additional nested arrays, if there happen to be multiple routes for a given mediaport */
printf("			}");
printf("		}");
printf("	}");
printf("	else{");
printf("		arr=false;");
printf("	}");
printf("	return arr;");								/* return the local array to the whatever called this function */
printf("}\n");

/* Gets called whenever the 'SHOW MAPS' button is clicked (only available if the message was selected as a GRAPHICAL ANNUNCIATOR type) */
printf("function SetLsdMode(mode,msgName_local,msgDir_local){");
//printf("	window.alert(\"now setting document.forms[0].lsdMode to '\"+mode+\"'\");");
printf("	document.forms[0].lsdMode.value = mode;");
printf("}\n");

/* Returns boolean value indicating whether or not they have entered into life-safety device mode (by clicking 'SHOW MAPS') */
printf("function GetLsdMode(){");
printf("	var lsdMode_local = parseInt(document.forms[0].lsdMode.value);");
printf("	if(lsdMode_local==0){");
printf("		return false;");
printf("	}");
printf("	else if(lsdMode_local==1){");
printf("		return true;");
printf("	}");
printf("	else{");
printf("		return document.forms[0].lsdMode.value;");
printf("	}");
printf("}\n");

/* process the change active map for a given device request */
printf("function processSubmitRequest_changeLsdActive(form_selectionList){");
printf("	var arrRadioGroup = form_selectionList;");
printf("	var selectedValue;");
printf("	var almiRecNo = 3; var almiDeviceID = 0;");
printf("	for(var i=0; i<arrRadioGroup.length; i++){");
printf("		if(arrRadioGroup[i].checked){");
printf("			selectedValue = arrRadioGroup[i].value;");
printf("			break;");
printf("		}");
printf("	}");
printf("	if(selectedValue != document.forms[0].activeLsdSelected_originallyActiveItem.value){\n");		/* if they selected a different one to make active... */
printf("		for(var i=0; i<temp_arrLsdMaps.length; i++){");								/* go through the ajax-returned array to determine the device name that matches the chosen device recno (need to post this in the form for deactivate... function) */
printf("			if(parseInt(temp_arrLsdMaps[i][almiRecNo]) == parseInt(selectedValue)){");
printf("				document.forms[0].lsdnametodeactivate.value = temp_arrLsdMaps[i][almiDeviceID];");
printf("				break;");
printf("			}");
printf("		}");
printf("		document.forms[0].loadEditSignSpeakerAfterChangeActiveMap.value = \"1\";\n");				/* trigger smcgi to run deactivateAllFloorplansInSupportOfLifeSafetyDevice() and activateItemByRecNo() */
printf("		document.forms[0].needtoresetstatus.value = \"1\";\n");
printf("		document.forms[0].record_number.value = selectedValue;");				/* floorplan record number */
//printf("window.alert(\"going to submit command to rewrite db:\\nloadEditSignSpeakerAfterChangeActiveMap = \"+document.forms[0].loadEditSignSpeakerAfterChangeActiveMap.value+\"\\nrecord_number = \"+document.forms[0].record_number.value);");
//printf("window.alert(document.forms[0].lsdnametodeactivate.value);");
printf("		document.forms[0].submit();\n");
printf("	}");
printf("	else{");												/* else, for some reason, they chose to make the same one active, so just return them to regular mode */
printf("		SetLsdMode(false);");
printf("		Modalbox.hide();");
printf("		return false;");
printf("	}");
printf("}\n");

/* Test if the provided string matches one of the floorplans in the client-side array */
printf("function CheckStringIsValidFloorplan(str){");
printf("	str = removeTrailingSpace(str);");
printf("	if(fpTable != null){");
printf("		for(var i=0; i<fpTable.length; i++){");
printf("			if(removeTrailingSpace(fpTable[i][fptiName]) == str){");
printf("				return true;");
printf("			}");
printf("		}");
printf("	}");
printf("	return false;");
printf("}\n");

/* Gets called whenever a life safety device is clicked while lsdMode is true */
/* (will initiate an AJAX request, and then pop a Modalbox with content from what is returned via the AJAX) */
printf("function clickLsd(deviceClicked_recno, deviceClicked_name){");
printf("	if(GetLsdMode()){");
//printf("window.alert(document.forms[0].stateChangeMessage.value);");
			/* if a device item is unidentified, handle refresh or prompt user to identify it... */
//printf("		if(isNullDevicePlaced(\"LSD\")){");						/* if a null device was placed, then let's try to refresh the page to see if we can bring a possibly-existing name into the current state... */
printf("		if(!deviceClicked_name){");
printf("			if(document.forms[0].stateChangeMessage.value==\"GA Page Reloaded 0\" || document.forms[0].stateChangeMessage.value==\"GA Page Reloaded\"){");
					/* null device exists and a reload loop has been detected... so clear the reload flag and leave next steps up to user */
printf("				if(document.forms[0].stateChangeMessage.value==\"GA Page Reloaded 0\"){document.forms[0].stateChangeMessage.value=\"GA Page Reloaded\";}else{document.forms[0].stateChangeMessage.value=\"\";}");				/* must clear this or an infinite submit loop will happen */
printf("				var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("				strHTML+=\"<p class=\\'mb_mainmessage\\'>This device must be identified first!</p>\";");
printf("				strHTML+=\"<p>To accomplish this, follow these steps: Click OK below, Find the non-named device by hovering your mouse over the safety types of icons until you find one without a name, and then EDIT it to identify its name.</p>\";");
printf("				strHTML+=\"<p style=\\'font-size:0.8em;\\'>(If the EDIT button doesn't work for you, then make sure you don't have any active pop-up blockers or windows covering the pop-up window)</p>\";");
printf("				strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'Modalbox.hide()\\'>OK</button></div>\";");
printf("				Modalbox.show(strHTML,{title:\"Life-safety device item not identified...\"});");
printf("			}");
printf("			else{");
					/* null device exists and a reload loop has NOT been detected... so submit a reload request (they maybe named the device, but just forgot to hit the DONE button) */
printf("				reloadGaPage();");
printf("			}");
printf("		}");
			/* else we're good to continue...
			 * Steps:
			 * - get all device items, across all floorplans
			 * - find all matching deviceid's (names) and return an array of unique floorplans
			 * - using that array, pop a modalbox allowing them to select the floorplan they want to change active to
			 * - that will then be able to call a function to submit the form (also need to reset all to 0 and then reset whichever one they chose to 1?) */
printf("		else{");
				/* use ajax to get data from the server (these functions should be local to this block) */
printf("			var objXML = false;");
printf("			function ajax_open(){");
/* thank you, internet exploder */
switch(cgi_detect_browser())
        {
        case BROWSER_EXPLORER:  
                printf("try {");
                printf("        objXML = new ActiveXObject('Msxm12.XMLHTTP');");
                printf("    } catch (e) {");
                printf("try {");
                printf("        objXML = new ActiveXObject('Microsoft.XMLHTTP');");
                printf("    } catch (E) {");
                printf("        objXML = false;");
                printf("    }");
                printf(" }");
                break;

        case BROWSER_GECKO:
	case BROWSER_OPERA:
        case BROWSER_NETSCAPE:
                printf(" objXML = new XMLHttpRequest();");
                break;
        }
printf("				if(objXML){");
printf("					objXML.onreadystatechange=ajax_callback;");							// set the function to call once the AJAX has something from the server
printf("					objXML.open(\"POST\", \"/~silentm/bin/smajax%s.cgi\", true);", TimeShareCompanyNameDotGet());			// must be relative to same server or get errors
printf("					objXML.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");");
printf("					objXML.send(\"loadGetLifeSafetyDeviceDataBasedOnDeviceID=1&lsd=\"+deviceClicked_name);");	// post necessary data to server... flag must correspond to a flag of the same name in smajax.c or 
printf("				}");
printf("			}\n");
printf("			function ajax_callback(){");
printf("				if(objXML.readyState == 4){");
printf("					if(objXML.status == 200){");
printf("						eval(objXML.responseText);");
printf("						if(arrLsdMaps.length>0){");
printf("							var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("							strHTML+=\"<p class=\\'mb_mainmessage\\'>Select the map that you want to make active for the safety device, \"+removeTrailingSpace(deviceClicked_name)+\":</p>\";");
printf("							strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup2\\' value=\\'0\\' />(none)</label>\";");
printf("							if(removeTrailingSpace(arrLsdMaps[0][almiDeviceID])==removeTrailingSpace(deviceClicked_name) && arrLsdMaps[0][almiActiveFlag]==1 && CheckStringIsValidFloorplan(arrLsdMaps[0][almiFloorplanName])){");
printf("								strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src=\\'%s/activeScroll.gif\\' /><input type=\\'radio\\' checked=\\'checked\\' name=\\'floorplanselectiongroup2\\' value=\\'\"+arrLsdMaps[0][almiRecNo]+\"\\' />\"+arrLsdMaps[0][almiFloorplanName]+\"</label>\";", cgi_icons);
printf("								document.forms[0].activeLsdSelected_originallyActiveItem.value = arrLsdMaps[0][almiRecNo];");
printf("							}");
printf("							else if(removeTrailingSpace(arrLsdMaps[0][almiDeviceID])==removeTrailingSpace(deviceClicked_name) && CheckStringIsValidFloorplan(arrLsdMaps[0][almiFloorplanName])){");
printf("								strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup2\\' value=\\'\"+arrLsdMaps[0][almiRecNo]+\"\\' />\"+arrLsdMaps[0][almiFloorplanName]+\"</label>\";");
printf("							}");
printf("							for(var i=1; i<arrLsdMaps.length; i++){");
printf("								if(removeTrailingSpace(arrLsdMaps[i][almiDeviceID])==removeTrailingSpace(deviceClicked_name) && arrLsdMaps[i][almiActiveFlag]==1 && CheckStringIsValidFloorplan(arrLsdMaps[i][almiFloorplanName])){");
printf("									strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src=\\'%s/activeScroll.gif\\' /><input type=\\'radio\\' checked=\\'checked\\' name=\\'floorplanselectiongroup2\\' value=\\'\"+arrLsdMaps[i][almiRecNo]+\"\\' />\"+arrLsdMaps[i][almiFloorplanName]+\"</label>\";", cgi_icons);
printf("									document.forms[0].activeLsdSelected_originallyActiveItem.value = arrLsdMaps[i][almiRecNo];");
printf("								}");
printf("								else if(removeTrailingSpace(arrLsdMaps[i][almiDeviceID])==removeTrailingSpace(deviceClicked_name) && CheckStringIsValidFloorplan(arrLsdMaps[i][almiFloorplanName])){");
printf("									strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup2\\' value=\\'\"+arrLsdMaps[i][almiRecNo]+\"\\' />\"+arrLsdMaps[i][almiFloorplanName]+\"</label>\";");
printf("								}");
printf("							}");
								/* copy the ajax-returned array into the global array (by value?) so it can then be used by any other functions (like processSubmitRequest_changeLsdActive) */
printf("							var j=0;");								/* increment counter that will be used for populating the local array */
printf("							for(var i=0; i<arrLsdMaps.length; i++){");					/* loop through each of the main routes table (created earlier by C-written javascript) */
printf("								temp_arrLsdMaps[j] = arrLsdMaps[i].slice(0);");					/* copy the record's elements by value */
printf("								j++;");										/* increment the counter to prepare for any additional nested arrays, if there happen to be multiple routes for a given mediaport */
printf("							}");

printf("							strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'processSubmitRequest_changeLsdActive(document.getElementsByName(\\\"floorplanselectiongroup2\\\"));\\'>OK</button></div>\";");
printf("							Modalbox.show(strHTML,{title:\"Select active map for item...\"});");
printf("						}");
/* maybe, remember to set lsd-mode back to 0 when done, somehow? */
printf("					}");
printf("				}");
printf("			}\n");
printf("			ajax_open();");
printf("		}");
printf("	}");
printf("}\n");

/* Gets called when ROUTES button is clicked. */
printf("function SetRouteMode(mode,msgNameB,msgDirB){");
printf("	if(mode==1){");  /* if intention is to enter route-mode, then pull the included message information into the global JS scope, as given by CGI via this function call */
printf("		if(isNullDevicePlaced(\"LCD\")){");						/* if a null device was placed, then let's try to refresh the page to see if we can bring a possibly-existing name into the current state... */
printf("			if(document.forms[0].stateChangeMessage.value!=\"Page Reloaded\"){");
					/* null device exists and a reload loop has NOT been detected... so submit a reload request (they maybe named the device, but just forgot to hit the DONE button) */
printf("				reloadRoutesPage();");
printf("			}");
printf("			else{");
					/* null device exists and a reload loop has been detected... so clear the reload flag and leave next steps up to user */
printf("				document.forms[0].stateChangeMessage.value=\"\";");				/* must clear this or an infinite submit loop will happen */
printf("				var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("				strHTML+=\"<p class=\\'mb_mainmessage\\'>All LCD's must be identified before the routing manager can be accessed.</p>\";");
printf("				strHTML+=\"<p>To accomplish this, follow these steps: Click OK below, Find the non-named device by hovering your mouse over the LCD icons until you find one without a name, and then EDIT it to identify its name.</p>\";");
printf("				strHTML+=\"<p style=\\'font-size:0.8em;\\'>(If the EDIT button doesn't work for you, then make sure you don't have any active pop-up blockers or windows covering the pop-up window)</p>\";");
printf("				strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'Modalbox.hide()\\'>OK</button></div>\";");
printf("				Modalbox.show(strHTML,{title:\"An LCD's name has not been set...\"});");
printf("			}");
printf("		}");
printf("		else{");
printf("			if(msgDirB && msgNameB){");
printf("				document.forms[0].msgName.value=msgNameB;");
printf("				document.forms[0].msgDir.value=msgDirB;");
printf("			}");
printf("			changeElementStateById(\"fpEditToolbar\",\"visibility\",\"hidden\");");		/* hide the original non-routing-related toolbar */
printf("			changeElementStateById(\"mapcanvasHeader\",\"visibility\",\"visible\");");	/* show the routing-related toolbar in the original's place */
printf("			changeElementStateById(\"routewindowlet\",\"visibility\",\"visible\");");	/* show the routing windowlet */
printf("			changeElementStateById(\"btn_addRoute\",\"visibility\",\"visible\");");		/* show the add route button */
printf("			if(routesTable!=null){");/* if there is a routing file and it contains routes, then show the show-route button */
printf("				if(routesTable.length>0){");
printf("					changeElementStateById(\"btn_showRoute\",\"visibility\",\"visible\");");
printf("				}");
printf("			}");
printf("			changeElementStateById(\"btn_addRouteImage\",\"visibility\",\"visible\");");	/* show the add route button */
printf("			changeElementStateById(\"btn_routeHelp\",\"visibility\",\"visible\");");	/* show the help button */
printf("			changeElementStateById(\"btn_cancelRoute\",\"visibility\",\"visible\");");	/* show the cancel-x button */
printf("			colorChooser(\"updateButtons\");");						/* udpate color buttons with color currently in the form data */
printf("			routePathWidth=parseInt(document.forms[0].routeThickness.value);");		/* set the global var with this route's thickness */
printf("			thickChooser(\"updateButtons\",routePathWidth,\"%s\");", cgi_icons);		/* update the toolbar buttons to display the active route thickness */
printf("			document.getElementById(\"fpPreTitle\").innerHTML=\"Route Map: \";");
printf("			document.getElementById(\"chooseDifferentMapChoiceButton\").style.display=\"none\";");  /* hide the map selection button */
printf("		}");
printf("	}");
printf("	else{");
printf("		document.forms[0].showRouteMode.value=0;");
printf("		document.forms[0].addRouteMode.value=0;");
printf("	}");
printf("}\n");

/* Set the add-route mode (more accurately, a click on the ADD ROUTE button), and do appropriate actions, depending on current state of things. */
printf("function SetAddRouteMode(){");
printf("	var selectedLCD=document.forms[0].routeMediaportOrigin.value;");					/* define a local variable to contain the selected mediaport (if one is so chosen) */
printf("	selectedLCD=selectedLCD.replace(/\\s+$/, '');");							/* strip off all trailing space characters, since this will not interact with the DB/file and doesn't have to be exact, but just pretty - also so we can do some test conditions below */
printf("	if(selectedLCD==''){var lcdSelected=false;}else{var lcdSelected=true;}");				/* parse a boolean value for this */
printf("	var initiateAddRouteMode=false;");									/* initialize flag that will be used to determine whether to proceed into the add-route mode */
printf("	if(!GetAddRouteMode() && !GetShowRouteMode() && !lcdSelected && !GetSubmissionToEditRoute()){");	/* nothing has been done yet and this is the first time they've clicked ADD ROUTE... */
printf("		initiateAddRouteMode=true;");										/* flag to enter add route mode */
printf("	}");
printf("	else if(!GetAddRouteMode() && !GetShowRouteMode() && (lcdSelected || GetSubmissionToEditRoute())){");	/* invalid condition (lcd or edit-flag should not be set if show-route mode is false)... */
printf("		if(lcdSelected){document.forms[0].routeMediaportOrigin.value=\"\";}");					/* if needed, correct the invalid value */
printf("		if(GetSubmissionToEditRoute()){document.forms[0].submissionToEditRoute.value=0;}");			/* if needed, correct the invalid value */
printf("		initiateAddRouteMode=true;");										/* flag to enter add route mode now that things should be set right */
printf("	}");
printf("	else if(!GetAddRouteMode() && GetShowRouteMode() && !lcdSelected && !GetSubmissionToEditRoute()){");	/* they've entered show-route mode, and no changes have been made, nor LCD selected (in other words, they've only clicked SHOW ROUTE and done nothing else)... */
printf("		initiateAddRouteMode=true;");										/* flag to enter add route mode */
printf("	}");
printf("	else if(!GetAddRouteMode() && GetShowRouteMode() && lcdSelected && !GetSubmissionToEditRoute()){");	/* they've entered show-route mode, selected an LCD, but no changes have been made to its route... */
printf("		initiateAddRouteMode=true;");										/* flag to enter add route mode */
printf("	}");
printf("	else if(!GetAddRouteMode() && GetShowRouteMode() && lcdSelected && GetSubmissionToEditRoute()){");	/* they've entered show-route mode, selected an LCD, and unsaved changes have been made to its route... */
printf("		if(routePoints.length>1){");										/* if more than the initial point exists (in other words, at least a line segment), then auto save it first */
printf("			document.forms[0].routeLastUserIntention.value=\"EnterAddRouteMode\";");
printf("			document.forms[0].routeLastSubmitReason.value=\"SaveExistingViaAutoSave\";");
printf("			finishRoutePreCheck();");
printf("		}");
printf("		else{initiateAddRouteMode=true;}");									/* else just reset into add mode since they don't have a valid path yet anyway */
printf("	}");
printf("	else if(GetAddRouteMode() && !GetShowRouteMode() && !lcdSelected){");					/* they've already entered add-route mode, but haven't yet clicked an LCD (so, probably a repeat click)... */
printf("	}");
printf("	else if(GetAddRouteMode() && !GetShowRouteMode() && lcdSelected && !GetSubmissionToAddRoute()){");	/* they've already entered add-route mode, and also clicked an LCD (but only clicked it and have done nothing else)... */
printf("		initiateAddRouteMode=true;");
printf("	}");
printf("	else if(GetAddRouteMode() && !GetShowRouteMode() && lcdSelected && GetSubmissionToAddRoute()){");	/* they've already entered add-route mode, and also clicked an LCD, and presumably started creating an unsaved route... */
printf("		if(routePoints.length>1){");										/* if more than the initial point exists (in other words, at least a line segment), then auto save it first */
printf("			document.forms[0].routeLastUserIntention.value=\"EnterAddRouteMode\";");				/* set flag */
printf("			document.forms[0].routeLastSubmitReason.value=\"SaveNewViaAutoSave\";");				/* set flag */
printf("			finishRoutePreCheck();");											/* submit form */
printf("		}");
printf("		else{initiateAddRouteMode=true;}");									/* else just reset into add mode since they don't have a valid path yet anyway */
printf("	}");
printf("	else{");												/* else... */
printf("		initiateAddRouteMode=true;");										/* flag to enter add route mode */
printf("	}");
printf("	if(initiateAddRouteMode){");										/* if flag indicates we are good to proceed into standard add-route mode... */
printf("		document.forms[0].routeMediaportOrigin.value=\"\";");							/* clear any LCD selection */
printf("		clearCanvas();");											/* make sure the canvas is cleared of any possibly-shown routes */
printf("		disableCanvas();");											/* disable the canvas so it will be possible to click an lcd icon */
printf("		clearArray();");											/* clear out the routePoints array */
printf("		document.getElementById(\"btn_showRouteImage\").src=\"%s/showRoute.gif\";", cgi_icons);			/* reset SHOW ROUTE button to normal */
printf("		document.forms[0].addRouteMode.value=1;");								/* programatically turn on add-route mode */
printf("		document.forms[0].showRouteMode.value=0;");								/* to avoid the edit clause from taking place after submission... because register click sets edit flag true if showRouteMode equals 1 */
printf("		changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"hidden\");");			/* hide the UNDO button, just in case it was visible (it may be shown later once valid to do so) */
printf("		changeElementStateById(\"btn_routeThin\",\"visibility\",\"visible\");");				/* show the thin button */
printf("		changeElementStateById(\"btn_routeNormal\",\"visibility\",\"visible\");");				/* show the normal button */
printf("		changeElementStateById(\"btn_routeThick\",\"visibility\",\"visible\");");				/* show the thick button */
printf("		changeElementStateById(\"btn_colorChooser\",\"visibility\",\"visible\");");				/* show the color chooser button */
printf("		changeElementStateById(\"btn_straighten\",\"visibility\",\"visible\");");				/* show the STRAIGHTEN button */
printf("		changeElementStateById(\"btn_finishRoute\",\"visibility\",\"hidden\");");				/* hide the SAVE ROUTE button, just in case it was visible (it may be shown later once valid to do so) */
printf("		changeElementStateById(\"btn_deleteRoute\",\"visibility\",\"hidden\");");				/* hide the DELETE ROUTE button, just in case it was visible (it may be shown later once valid to do so) */
printf("	}");
printf("}\n");

/* Set the show-route mode (more accurately, a click on the SHOW ROUTE button), and do appropriate actions, depending on current state of things. */
printf("function SetShowRouteMode(){");
printf("	var initiateShowRouteMode=false;");											/* initialize flag that will be used to determine whether to proceed into the show-route mode */
printf("	var numberOfRegisteredPoints=parseInt(routePoints.length);");								/* set a local variable to the number of elements in the routePoints array (to get how many points are active) */
printf("	if(isNaN(numberOfRegisteredPoints)){numberOfRegisteredPoints=0;}");							/* just in case we got something weird, set to 0 so we can test reliably down below */
printf("	if(!GetAddRouteMode() && !GetShowRouteMode() && !GetSubmissionToEditRoute() && numberOfRegisteredPoints==0){");		/* clicking show from pure state... */
printf("		initiateShowRouteMode=true;");												/* set flag to enter show-route mode */
printf("	}");
printf("	else if(!GetAddRouteMode() && !GetShowRouteMode() && !GetSubmissionToEditRoute() && numberOfRegisteredPoints>0){");	/* clicking show when a newly-added (or just-changed?) route has been submitted and is currently showing */
printf("		document.forms[0].showRouteMode.value=1;");										/* programatically turn on show-route mode */
printf("	}");
printf("	else if(GetAddRouteMode() && !GetShowRouteMode() && !GetSubmissionToEditRoute() && numberOfRegisteredPoints>0){");	/* switching from add to show, but with an new unsaved route... */
printf("		if(routePoints.length>1){");												/* if more than the initial point exists (in other words, at least a line segment), then auto save it first */
printf("			document.forms[0].routeLastUserIntention.value=\"EnterShowRouteMode\";");						/* set flag */
printf("			document.forms[0].routeLastSubmitReason.value=\"SaveNewViaAutoSave\";");						/* set flag */
printf("			finishRoutePreCheck();");													/* submit form */
printf("		}");
printf("		else{initiateShowRouteMode=true;}");											/* else just reset into show mode since they don't have a valid path yet anyway */
printf("	}");
printf("	else if(GetAddRouteMode() && !GetShowRouteMode() && !GetSubmissionToEditRoute()){");					/* simply switching from add to show... */
printf("		document.getElementById(\"btn_addRouteImage\").src=\"%s/map_route_add.gif\";", cgi_icons);				/* undepress ADD ROUTE button */
printf("		document.forms[0].addRouteMode.value=0;");										/* programatically turn off add-route mode */
printf("		document.forms[0].submissionToAddRoute.value=0;");									/* turn off flag */
printf("		initiateShowRouteMode=true;");												/* set flag to enter show-route mode */
printf("	}");
printf("	else if(GetAddRouteMode() && GetShowRouteMode()){");									/* invalid state - both modes should not be on at the same time (this test condition should not become true)... */
printf("		document.forms[0].showRouteMode.value=0;");
printf("		initiateShowRouteMode=true;");
printf("	}");
printf("	else if(GetAddRouteMode() && GetSubmissionToEditRoute()){");								/* invalid state? - submit-to-edit flag should not be set when they were in add mode?... */
printf("		document.forms[0].submissiontToEditRoute.value=0;");
printf("		initiateShowRouteMode=true;");
printf("	}");
printf("	else if(GetShowRouteMode() && GetSubmissionToEditRoute()){");								/* they were in the middle of editing an existing route when they clicked SHOW ROUTE... */
printf("		if(routePoints.length>1){");												/* if more than the initial point exists (in other words, at least a line segment), then auto save it first */
printf("			document.forms[0].routeLastUserIntention.value=\"EnterShowRouteMode\";");
printf("			document.forms[0].routeLastSubmitReason.value=\"SaveExistingViaAutoSave\";");
printf("			finishRoutePreCheck();");
printf("		}");
printf("		else{initiateShowRouteMode=true;}");											/* else just reset into show mode since they don't have a valid path yet anyway */
printf("	}");
printf("	else if(GetShowRouteMode() && numberOfRegisteredPoints>0 && !GetSubmissionToEditRoute()){");				/* clicking show while in show-route mode, with an existing route already shown, but no changes made to it... */
printf("		initiateShowRouteMode=true;");												/* set flag to enter show-route mode */
printf("		clearArray();");													/* clear array since we're starting anew */
printf("	}");
printf("	else if(GetShowRouteMode()){");												/* clicking show while in show-route mode, but with no existing route already shown... (likely user error or clicking show twice in a row) */
//printf("		document.getElementById(\"btn_showRouteImage\").src=\"%s/showRoute.gif\";", cgi_icons);					/* un-depress the show route button to indicate that the show-route mode is now off */
//printf("		document.forms[0].showRouteMode.value=0;");										/* turn off show-route mode */
//printf("		disableCanvas();");													/* disable the canvas so normal clicking is allowed again */
//printf("		clearArray();");													/* clear the route array since they may be recreating it with a new route later */
printf("		initiateShowRouteMode=true;");												/* set flag to enter show-route mode */
printf("	}");
printf("	else{");														/* else all other cases... */
printf("		initiateShowRouteMode=true;");												/* set flag to enter show-route mode */
printf("	}");
printf("	if(initiateShowRouteMode){");												/* if flag indicates that we are good to proceed into standard show-route mode... */
printf("		document.forms[0].showRouteMode.value=1;");										/* programatically turn on show-route mode */
printf("		clearCanvas();");													/* clear the canvas so nothing shows if it's re-enabled later */
printf("		disableCanvas();");													/* disable the canvas so normal clicking can be allowed (in case an LCD selection needs to be made) */
printf("		clearAllHiddenFields();");												/* there should be no need for any route form data to exist at this point */
printf("		changeElementStateById(\"btn_maps\",\"visibility\",\"hidden\");");
printf("	}");
printf("}\n");

/* Returns a boolean indicating add-route mode */
printf("function GetAddRouteMode(){");
printf("	if(isNaN(parseInt(document.forms[0].addRouteMode.value))){return false;}");
printf("	else if(parseInt(document.forms[0].addRouteMode.value)==0){return false;}");
printf("	else{return true};");
printf("}\n");
/* Returns an boolean indicating show-route mode */
printf("function GetShowRouteMode(){");
printf("	if(isNaN(parseInt(document.forms[0].showRouteMode.value))){return false;}");
printf("	else if(parseInt(document.forms[0].showRouteMode.value)==0){return false;}");
printf("	else{return true;}");
printf("}\n");

/* Returns a boolean indicating submission-to-edit flag status */
printf("function GetSubmissionToEditRoute(){");
printf("	var haveEdited=parseInt(document.forms[0].submissionToEditRoute.value);");	/* set a local variable with the state of the submission flag, to avoid repeated DOM calls to check it */
printf("	if(isNaN(parseInt(document.forms[0].submissionToEditRoute.value))){return false;}");
printf("	else if(parseInt(document.forms[0].submissionToEditRoute.value)==0){return false;}");
printf("	else{return true;}");
printf("}\n");
/* Returns a boolean indicating submission-to-add flag status */
printf("function GetSubmissionToAddRoute(){");
printf("	var goingToAdd=parseInt(document.forms[0].submissionToAddRoute.value);");
printf("	if(isNaN(parseInt(document.forms[0].submissionToAddRoute.value))){return false;}");
printf("	else if(parseInt(document.forms[0].submissionToAddRoute.value)==0){return false;}");
printf("	else{return true;}");
printf("}\n");

/* Setup and initialize the canvas element specified. NOTE: There must be a canvas element already rendered on the client, first, before this is called! (can't work with something that doesn't exist yet) */
printf("function initializeCanvas(idOfCanvas){");
printf("        canvas=document.getElementById(idOfCanvas);");
/* DEV-NOTE: IE9 beta, as of Dec 2010 completely breaks the canvas AND the VML that is getting patched here... will beta be fixed? Will need to keep an eye on this. Probably need feature detection instead of browser detection, also. */
/* DEV-NOTE: IE9 RC-1, as of Feb 11, 2011 now appears to work. Still need to keep an eye on this as it develops toward official release. */
printf("	if(navigator.appName==\"Microsoft Internet Explorer\"){");	/* need special considerations for IE, as usual */
//printf("		if(parseInt(navigator.appVersion)<5){");			/* if IE version is less than 9 (IE's rendering engine, called trident) has a version numbering system that is different than common), then patch VML/canvas */
printf("			G_vmlCanvasManager.initElement(canvas);");
//printf("		}");
//printf("		else{");
//printf("			window.alert(\"Internet Explorer 9 doesn't yet support route administration.\\nNow closing...\");");
//printf("			top.close();");
//printf("		}");
printf("	}");
/* printf("window.alert(parseInt(navigator.appVersion));"); */
printf("	canvasContainer=document.getElementById(\"background_image_div\");");
printf("        canvas.width=parseInt(canvasContainer.style.width);");
printf("        canvas.height=parseInt(canvasContainer.style.height);");
printf("	canvas.style.cursor=\"pointer\";");
printf("        canvasContext=canvas.getContext(\"2d\");");
printf("}\n");

/* Need to enable the canvas in preparation for drawing on it, once they click an LCD icon after clicking ADD. LCD icon element will point to this if GetAddRouteMode() returns 1. */
printf("function clickLCDtoAdd(deviceClicked){");
//printf("	if(deviceClicked==\"\"){");
//printf("		var strHTML=\"<img src=\\'%s/dialog-info.png\\' style=\\'float:left; margin-right:20px;\\' />\";", cgi_icons);
//printf("		strHTML+=\"<p style=\\'font-weight:bold;\\'>You must identify and save the name for the LCD that was clicked.</p>\";");
//printf("		strHTML+=\"<p>When you place a new LCD icon onto a map, a pop-up window appears and you must identify the name that the icon represents. Before you can add a route to that device, its name must be correctly entered into the system.</p>\";");
//printf("		strHTML+=\"<p>Click the appropriate situation below for your case...</p>\";");
//printf("		strHTML+=\"<p style=\\'text-align:left;\\'><a href=\\'#\\' onclick=\\'Modalbox.hide();cancelRoute();SetEditItemMode(1);\\' style=\\'font-weight:bold;\\'>I forgot to or didn't identify a name for this LCD device</a><br />(you will automatically be returned to the device manager screen so you can click the LCD to identify its name)</p>\";");
//printf("		strHTML+=\"<p style=\\'text-align:left;\\'><a href=\\'#\\' onclick=\\'Modalbox.hide();cancelRoute();SetEditItemMode(1);\\' style=\\'font-weight:bold;\\'>Identify the name for this LCD device</a><br />(you will automatically be returned to the device manager screen so you can click the LCD to identify its name)</p>\";");
//printf("		strHTML+=\"<p style=\\'text-align:left;\\'><a href=\\'#\\' onclick=\\'Modalbox.hide();reloadRoutesPage();\\' style=\\'font-weight:bold;\\'>I identified a name for this LCD, but did not click the 'done' button before entering the route manager</a><br />(this screen will refresh so the name can be loaded properly)</p>\";");
//printf("		strHTML+=\"<p style=\\'text-align:left;\\'><a href=\\'#\\' onclick=\\'Modalbox.hide();reloadRoutesPage();\\' style=\\'font-weight:bold;\\'>I'm not sure</a></p>\";");
//printf("		Modalbox.show(strHTML,{title:\"This LCD's name has not been set...\"});");
//printf("	}");
//printf("	else{");
printf("		if(document.forms[0].routeThickness.value==\"\"){");					/* if thickness is not specified... (they will need some thickness to begin drawing a line!) */
printf("			if(document.forms[0].routeThickness_old.value==\"\"){");				/* if a recent thickness is not available, then... */
printf("				document.forms[0].routeThickness.value=\"%d\";", ROUTE_THICKNESS_NORMAL);		/* assign the default normal thickness */
printf("			}");
printf("			else{");										/* else... */
printf("				document.forms[0].routeThickness.value=document.forms[0].routeThickness_old.value;");	/* assign the most recent thickness available */
printf("			}");
printf("			routePathWidth=parseInt(document.forms[0].routeThickness.value);");			/* set the global var with this thickness */
printf("			thickChooser(\"updateButtons\",routePathWidth,\"%s\");", cgi_icons);			/* update the toolbar buttons */
printf("		}");
printf("		if(document.forms[0].routePathColor.value==\"\"){");					/* if color is not specified... (they will need some color to begin drawing a line!) */
printf("			if(document.forms[0].routePathColor_old.value==\"\"){");				/* if a recent color is not available, then... */
printf("				document.forms[0].routePathColor.value=\"%s\";", ROUTE_COLOR_DEFAULT);			/* assign the default color */
printf("			}");
printf("			else{");										/* else... */
printf("				document.forms[0].routePathColor.value=document.forms[0].routePathColor_old.value;");	/* assign the most recent color available */
printf("			}");
printf("			colorChooser(\"updateButtons\");");							/* update colored buttons with this color */
printf("		}");
printf("		unpopRouteHelpMouse();");
printf("		enableCanvas(deviceClicked);");
//printf("	}");
printf("}\n");

/* Handle what happens when they click an LCD icon (after clicking SHOW or if editing). LCD icon element will point to this if GetShowRouteMode() returns 1.
 *   clickLCDtoShow(str deviceClicked, [int routeIndex])  */
printf("function clickLCDtoShow(deviceClicked,routeIndex){");
printf("	if(deviceClicked==\"\"){deviceClicked=\"the device you clicked\"}");
printf("	var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("	strHTML+=\"<p class=\\'mb_mainmessage\\'>This LCD has no Routes on this Map.</p>\";");
printf("	strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'Modalbox.hide();\\'>OK</button></div>\";");
printf("	routesDrawn=0;");									/* fixes more click being eaten bug */
printf("	if(!routeIndex){routeIndex=0;}");
printf("	unpopRouteHelpMouse();");
printf("	clearCanvas();");									/* clear the canvas so nothing shows if it's re-enabled later */
printf("	canvas.style.display=\"block\";");							/* display the canvas so we can use it to draw routes */
printf("	var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(deviceClicked,\"%s\");", present_floorplan_name_no_suffix_with_space);		/* get all routes info for this specific device that was clicked and store it locally to this function so we can use it next... */
printf("	var numberOfRoutes=arrThisLCDsRouteInfo.length;");					/* get the number of routes for this device (corresponds to the length of this array, naturally) */
printf("	if(numberOfRoutes==0){");								/* if no routes exist for the LCD that was clicked... */
printf("		disableCanvas();");									/* disable the canvas layer so they will be able to click another lcd */
printf("		Modalbox.show(strHTML,{title:\"Notice...\"});");
printf("	}");
printf("	else if(numberOfRoutes==1){");								/* else if only one route exists for the LCD that was clicked... */
printf("		updateAllHiddenFieldsWithRouteInfo(deviceClicked,arrThisLCDsRouteInfo[0][rtiFloorplanName],arrThisLCDsRouteInfo[0][rtiActiveFlag],arrThisLCDsRouteInfo[0][rtiRouteThickness],arrThisLCDsRouteInfo[0][rtiRouteColor],arrThisLCDsRouteInfo[0][rtiCoords]);");
printf("		colorChooser(\"updateButtons\");");							/* udpate color buttons with color currently in the form data */
printf("		routePoints=delimitedStringToCoordPairArray(arrThisLCDsRouteInfo[0][rtiCoords]);");	/* set the global array with this route's coordinate pairs (needed in order to edit) */
printf("		routePathWidth=parseInt(document.forms[0].routeThickness.value);");			/* set the global var with this route's thickness */
printf("		thickChooser(\"updateButtons\",routePathWidth,\"%s\");", cgi_icons);			/* update the toolbar buttons to display the active route thickness */
printf("		drawCanvasFromPointsInArrayForMediaport(routePoints);");				/* draw the route */
printf("		changeElementStateById(\"btn_moreRoutes\",\"visibility\",\"hidden\");");		/* hide more button (in case it was previously shown for some reason) */
printf("		changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"visible\");");	/* show undo button (and be sure to show save route button after first click of this button) */
printf("		changeElementStateById(\"btn_routeThin\",\"visibility\",\"visible\");");		/* show the thin button */
printf("		changeElementStateById(\"btn_routeNormal\",\"visibility\",\"visible\");");		/* show the normal button */
printf("		changeElementStateById(\"btn_routeThick\",\"visibility\",\"visible\");");		/* show the thick button */
printf("		changeElementStateById(\"btn_colorChooser\",\"visibility\",\"visible\");");		/* show color selection list (and be sure to show save route button after first change of this */
printf("		changeElementStateById(\"btn_straighten\",\"visibility\",\"visible\");");		/* show straighten button */
printf("		changeElementStateById(\"btn_deleteRoute\",\"visibility\",\"visible\");");		/* show delete route button */
printf("	}");
printf("	else{");										/* else there should be more than one route that exists for the LCD that was clicked - just draw the first one we come across... */
printf("		updateAllHiddenFieldsWithRouteInfo(deviceClicked,arrThisLCDsRouteInfo[routeIndex][rtiFloorplanName],arrThisLCDsRouteInfo[routeIndex][rtiActiveFlag],arrThisLCDsRouteInfo[routeIndex][rtiRouteThickness],arrThisLCDsRouteInfo[routeIndex][rtiRouteColor],arrThisLCDsRouteInfo[routeIndex][rtiCoords]);");
printf("		colorChooser(\"updateButtons\");");							/* udpate color buttons with color currently in the form data */
printf("		routePoints=delimitedStringToCoordPairArray(arrThisLCDsRouteInfo[routeIndex][rtiCoords]);");/* set the global array with the first route's coordinate pairs (needed in order to edit) */
printf("		routePathWidth=parseInt(document.forms[0].routeThickness.value);");			/* set the global var with this route's thickness */
printf("		thickChooser(\"updateButtons\",routePathWidth,\"%s\");", cgi_icons);			/* update the toolbar buttons to display the active route thickness */
printf("		drawCanvasFromPointsInArrayForMediaport(routePoints);");				/* draw the first route */
printf("		routesDrawn++;");									/* increment the global (so we can work outside of this function) counter to keep track of how many routes we have drawn */
printf("		tempGlobalArrForSpecificLCD=arrThisLCDsRouteInfo;");					/* copy the LCD-specific route array to the temporary global container, so the more-routes function can work with it */
printf("		changeElementStateById(\"btn_moreRoutes\",\"visibility\",\"visible\");");		/* show more button (each click should somehow loop through and draw each route) */
printf("		changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"visible\");");	/* show undo button (and be sure to show save route button after first click of this button) */
printf("		changeElementStateById(\"btn_routeThin\",\"visibility\",\"visible\");");		/* show the thin button */
printf("		changeElementStateById(\"btn_routeNormal\",\"visibility\",\"visible\");");		/* show the normal button */
printf("		changeElementStateById(\"btn_routeThick\",\"visibility\",\"visible\");");		/* show the thick button */
printf("		changeElementStateById(\"btn_colorChooser\",\"visibility\",\"visible\");");		/* show color selection list (and be sure to show save route button after first change of this */
printf("		changeElementStateById(\"btn_straighten\",\"visibility\",\"visible\");");		/* show straighten button */
printf("		changeElementStateById(\"btn_deleteRoute\",\"visibility\",\"visible\");");		/* show delete route button */
printf("	}");
printf("	var arrThisLCDsRoutesFloorplans=extractAllRouteInfoForSpecificLCD(deviceClicked);");
printf("	var arrThisLCDsRoutesFloorplans=extractFloorplansFromArray(arrThisLCDsRoutesFloorplans,false);");/* get the unique floorplans for this device's routes */
printf("	if(numberOfRoutes!=0 && arrThisLCDsRoutesFloorplans.length>1){");			/* if there is more than one floorplan on record for this LCD, then... */
//printf("	if(arrThisLCDsRoutesFloorplans.length>1){");			/* if there is more than one floorplan on record for this LCD, then... */
printf("		changeElementStateById(\"btn_maps\",\"visibility\",\"visible\");");			/* make the map button visible (for selecting active/inactive floorplans) */
printf("	}");
printf("	else{");
printf("		changeElementStateById(\"btn_maps\",\"visibility\",\"hidden\");");
printf("	}");
printf("}\n");

/* Handle what happens when they click the MORE button. */
printf("function clickMoreRoutes(){");
printf("	var selectedLCD=document.forms[0].routeMediaportOrigin.value;");		/* define a local variable to contain the selected mediaport (if one is so chosen) */
printf("	selectedLCD=selectedLCD.replace(/\\s+$/, '');");				/* strip off all trailing space characters, since this will not interact with the DB/file and doesn't have to be exact, but just pretty - also so we can do some test conditions below */
printf("	if(!selectedLCD){");								/* if they have not clicked an LCD to select it currently, then assume we're using the previously selected LCD (this supports a persistent origin LCD whenever clicking MORE) */
printf("		clickLCDtoShow(document.forms[0].routeMediaportOrigin_old.value);");		/* reactivate the previous selected LCD */
printf("		selectedLCD=document.forms[0].routeMediaportOrigin.value;");			/* re-define the local variable to contain the last-selected mediaport */
printf("		selectedLCD=selectedLCD.replace(/\\s+$/, '');");				/* strip off all trailing space characters, since this will not interact with the DB/file and doesn't have to be exact, but just pretty - also so we can do some test conditions below */
printf("		routesDrawn--; if(routesDrawn<0){routesDrawn=tempGlobalArrForSpecificLCD.length;}");/* decrement and manage the counter */
printf("	}");
printf("	if(routesDrawn>=tempGlobalArrForSpecificLCD.length){");				/* if at end of the existing routes... */
printf("		routesDrawn=0;");								/* then go back around to the first route for this device */
printf("	}");
printf("	document.forms[0].routeIndexAtSubmitTime.value=routesDrawn;");			/* flag what route we are on here, so we can know what the next route should be after/if the page comes back from form submission */
printf("	if(GetShowRouteMode()==1 && selectedLCD!='' && GetSubmissionToEditRoute()){");	/* if they were in show-route mode, with an LCD selected, with edits made, and then they click more, then we need to automatically save the changes first, before continuing to show the next */
printf("		document.forms[0].routeLastUserIntention.value=\"ShowNextRoute\";");					/* flag what the user intends to do, so the page can know once we come back from form submission */
printf("		document.forms[0].routeLastSubmitReason.value=\"SaveExistingViaAutoSave\";");				/* flag why this form will now be submitted */
printf("		finishRoutePreCheck();");											/* submit the form (post-submission logic will take care of what to do, depending on the above flags) */
printf("	}");
//printf("	else if(GetAddRouteMode() && selectedLCD!='' && GetSubmissionToAddRoute()){");	/* else if they were in add-route mode, with an LCD selected, with edits made, and then they click more, then we need to automatically save new route first, before continuing to show the next */
printf("	else if(GetAddRouteMode()){");							/* else if they were adding a new route and clicked more */
printf("		document.forms[0].routeLastUserIntention.value=\"ShowNextRoute\";");					/* flag what the user intends to do, so the page can know once we come back from form submission */
printf("		document.forms[0].routeLastSubmitReason.value=\"SaveNewViaAutoSave\";");				/* flag why this form will now be submitted */
printf("		finishRoutePreCheck();");											/* submit the form (post-submission logic will take care of what to do, depending on the above flags) */
printf("	}");
printf("	else{");									/* else there are no changes to save... */
printf("		clearCanvas();");											/* clear the canvas */
printf("		updateAllHiddenFieldsWithRouteInfo(false,tempGlobalArrForSpecificLCD[routesDrawn][rtiFloorplanName],tempGlobalArrForSpecificLCD[routesDrawn][rtiActiveFlag],tempGlobalArrForSpecificLCD[routesDrawn][rtiRouteThickness],tempGlobalArrForSpecificLCD[routesDrawn][rtiRouteColor],tempGlobalArrForSpecificLCD[routesDrawn][rtiCoords]);");
printf("		colorChooser(\"updateButtons\");");									/* udpate color buttons with color currently in the form data */
printf("		thickChooser(\"updateButtons\",parseInt(document.forms[0].routeThickness.value),\"%s\");", cgi_icons);	/* update the toolbar buttons to display the active route thickness */
printf("		routePoints=delimitedStringToCoordPairArray(tempGlobalArrForSpecificLCD[routesDrawn][rtiCoords]);");	/* set the global array with the route's coordinate pairs (needed in order to edit) */
printf("		drawCanvasFromPointsInArrayForMediaport(routePoints);");						/* draw the route using the points just ascertained */
printf("		routesDrawn++;");											/* increment our counter */
printf("	}");
printf("}\n");

/* Handle what happens when they click the MAPS button (designed to be able to change active/inactive maps for a particular LCD) */
/* PSEUDO...
- Using the current LCD device from the current form state (whose field value for routeMediaportOrigin should be updated whenever they click an LCD to show)...
- Get a listing of all floorplans for that LCD (active or not... doesn't matter, just all of them -- as determined and returned by various JS functions)
- Present that listing for the user to select which one is active (also show which one is currently active)
*/
printf("function clickMapsButton(){");
printf("	var currentDevice = document.forms[0].routeMediaportOrigin.value;");						/* locally store the current route's display device (should be updated each time click LCD to show) */
printf("	var currentFloorplan = document.forms[0].routeFloorplanName.value;");						/* locally store the current route's floorplan name */
printf("	var arrRoutesByLCD = extractAllRouteInfoForSpecificLCD(currentDevice);");					/* get all pre-existing route records that match the specified LCD device */
printf("	var arrFloorplansByLCD = extractFloorplansFromArray(arrRoutesByLCD,currentFloorplan);");			/* get all unique floorplan names for the current LCD */
printf("	var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);			/* begin creating a string of HTML to pass along to a Modalbox... */
printf("	strHTML+=\"<p class=\\'mb_mainmessage\\'>Select the map you wish to display on the LCD</p><p>\";");
printf("	strHTML+=\"<p><div class=\\'mb_choiceheading\\'>Current Map:</div>\";");
printf("	if(parseInt(isFloorplanActiveForLCD(arrFloorplansByLCD[0],currentDevice))==1){");				/* if this floorplan is currently active, then show active thumb and make it default active */
printf("		strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' id=\\'fpsgchecked\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCD[0]+\"\\' onchange=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' />\"+removeTrailingSpace(arrFloorplansByLCD[0])+\"</label>\";", cgi_icons);
printf("		strHTML+=\"<script type=\\text/javascript\\'>document.forms[0].selectedActiveFloorplan.value=\\'\"+arrFloorplansByLCD[0]+\"\\';<\\/script>\";");
printf("	}");
printf("	else{");													/* else not active, so just show normal radio selection */
printf("		strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCD[0]+\"\\' onchange=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' />\"+removeTrailingSpace(arrFloorplansByLCD[0])+\"</label>\";");
printf("	}");
printf("	strHTML+=\"</p><p><div class=\\'mb_choiceheading\\'>Other Maps:</div>\";");
printf("	for(var i=1;i<arrFloorplansByLCD.length;i++){");
printf("		if(parseInt(isFloorplanActiveForLCD(arrFloorplansByLCD[i],currentDevice))==1){");
printf("			strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' id=\\'fpsgchecked\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCD[i]+\"\\' onchange=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' />\"+removeTrailingSpace(arrFloorplansByLCD[i])+\"</label>\";", cgi_icons);
printf("			strHTML+=\"<script type=\\'text/javascript\\'>document.forms[0].selectedActiveFloorplan.value=\\'\"+arrFloorplansByLCD[i]+\"\\';<\\/script>\";");
printf("		}");
printf("		else{");
printf("			strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCD[i]+\"\\' onchange=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' />\"+removeTrailingSpace(arrFloorplansByLCD[i])+\"</label>\";");
printf("		}");
printf("	}");
printf("	strHTML+=\"</p><script type=\\'text/javascript\\'>document.getElementById(\\\"fpsgchecked\\\").focus();<\\/script>\";");
printf("	strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'if(parseInt(isFloorplanActiveForLCD(document.forms[0].selectedActiveFloorplan.value,document.forms[0].routeMediaportOrigin.value))!=1){finishRoute();}else{Modalbox.hide();return false;}\\'>OK</button></div>\";"); /* button style is to keep IE from displaying long button */
//printf("	strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'window.alert(document.forms[0].selectedActiveFloorplan.value);return false;if(parseInt(isFloorplanActiveForLCD(document.forms[0].selectedActiveFloorplan.value,document.forms[0].routeMediaportOrigin.value))!=1){finishRoute();}else{Modalbox.hide();return false;}\\'>OK</button></div>\";"); /* button style is to keep IE from displaying long button */
printf("	Modalbox.show(strHTML,{title:\"Other maps have routes defined for this LCD...\"});");
printf("}\n");

/* Enact (submit) a change of the active floorplan for the device and floorplan currently in form state */
printf("function changeActiveFloorplan(){");
printf("	document.forms[0].submissionToEditRoute.value=\"1\";");		/* enable the edit flag (will be used in special combination with selectedActiveFloorplan to enact an active-flag change) */
printf("	document.forms[0].submissionToDeleteRoute.value=\"0\";");	/* just make sure delete flag isn't set, because that operation has its own function - you would not delete through a call to this function, but rather deleteRoute() */
printf("	document.forms[0].submissionToAddRoute.value=\"0\";");		/* set the flag that will add a whole new route, no questions asked - (if needed, they can delete the route that is now going to be added, later) */
printf("	document.forms[0].submit();");
printf("}\n");

/* Get the number of routes (records) that exist for the specified routes-type array */
/*	function countNumberOfRoutesInArray(arr arrRecords, str "DEVICE" | "FLOORPLAN", str strFilter) */
printf("function countNumberOfRoutesInArray(arrRecords,strFilterType,strFilter){");
printf("	if(arrRecords){");
printf("		if(strFilter){");
printf("			var counter = 0;");
printf("			if(strFilterType==\"DEVICE\"){");
printf("				for(var i=0;i<arrRecords.length;i++){");
printf("					if(removeTrailingSpace(arrRecords[i][rtiDeviceName])==removeTrailingSpace(\"\"+strFilter+\"\")){");
printf("						counter++;");
printf("					}");
printf("				}");
printf("				return counter;");
printf("			}");
printf("			else if(strFilterType==\"FLOORPLAN\"){");
printf("				for(var i=0;i<arrRecords.length;i++){");
printf("					if(removeTrailingSpace(arrRecords[i][rtiFloorplanName])==removeTrailingSpace(\"\"+strFilter+\"\")){");
printf("						counter++;");
printf("					}");
printf("				}");
printf("				return counter;");
printf("			}");
printf("			else{");
printf("				return false;");
printf("			}");
printf("		}");
printf("		else{");
printf("			return false;");
printf("		}");
printf("	}");
printf("	else{");
printf("		return false;");
printf("	}");
printf("}\n");

/* Check the main routesTable array for the floorplan to see if it is active for the specified device */
printf("function isFloorplanActiveForLCD(strFP,strLCD){");
printf("	if(strFP && strLCD){");
printf("		for(var i=0;i<routesTable.length;i++){");
printf("			if(removeTrailingSpace(routesTable[i][rtiFloorplanName])==removeTrailingSpace(strFP)");
printf("				&& removeTrailingSpace(routesTable[i][rtiDeviceName])==removeTrailingSpace(strLCD)){");
printf("				return routesTable[i][rtiActiveFlag];");	/* returns either a 0 or a 1 (as consistent with the record data for rtiActiveFlag) */
printf("			}");
printf("		}");
printf("		return false;");	/* should only happen if the floorplan was not found */
printf("	}");
printf("	else{");
printf("		return null;");		/* should only happen if a floorplan was not specified as an argument to this function */
printf("	}");
printf("}\n");

/* Return a string with all trailing spaces removed */
printf("function removeTrailingSpace(str){");
printf("	str = \"\"+str+\"\";");
//printf("	return str.replace(/\\s/g,'');");  /* would remove ALL spaces, not just trailing */
printf("	return str.replace(/\\s+$/g,'');");
printf("}\n");

/* Updates the hidden form fields that contain route data... both sets, old and new, alike.
 *   updateAllHiddenFieldsWithRouteInfo([routeMediaportOrigin | false], routeFloorplanName, routeActiveFlag, routeThickness, routePathColor, routeCoordsStr)  */
printf("function updateAllHiddenFieldsWithRouteInfo(routeMediaportOrigin,routeFloorplanName,routeActiveFlag,routeThickness,routePathColor,routeCoordsStr){");
printf("	if(routeMediaportOrigin){document.forms[0].routeMediaportOrigin.value=document.forms[0].routeMediaportOrigin_old.value=routeMediaportOrigin;}");
printf("	document.forms[0].routeThickness.value=document.forms[0].routeThickness_old.value=routeThickness;");
printf("	thickChooser(\"updateButtons\",routeThickness,\"%s\");", cgi_icons);			/* update the toolbar buttons to display the active route thickness */
printf("	document.forms[0].routePathColor.value=document.forms[0].routePathColor_old.value=routePathColor;");
printf("	document.forms[0].routeCoordsStr.value=document.forms[0].routeCoordsStr_old.value=routeCoordsStr.replace(/\\s+$/, '');");
//printf("	document.forms[0].routeFloorplanName.value=\"%s\";", present_floorplan_name_no_suffix);		/* DEV-NOTE: Static version... will need to change to getting this dynamically, once you implement ability to change floorplans */
printf("	document.forms[0].routeFloorplanName.value=document.forms[0].routeFloorplanName_old.value=routeFloorplanName;");
printf("	document.forms[0].routeActiveFlag.value=document.forms[0].routeActiveFlag_old.value=routeActiveFlag;");
printf("}\n");
/* Updates the hidden form fields that contain old/original route data... so we know the original data if they are making changes of any kind.
 *   updateOldHiddenFieldsWithRouteInfo([routeMediaportOrigin | false], routeThickness, routePathColor, routeCoordsStr)  */
printf("function updateOldHiddenFieldsWithRouteInfo(routeMediaportOrigin,routeThickness,routePathColor,routeCoordsStr){");
printf("	if(routeMediaportOrigin){document.forms[0].routeMediaportOrigin_old.value=routeMediaportOrigin;}");
printf("	document.forms[0].routeThickness_old.value=routeThickness;");
printf("	document.forms[0].routePathColor_old.value=routePathColor;");
printf("	document.forms[0].routeCoordsStr_old.value=routeCoordsStr.replace(/\\s+$/, '');");
//printf("	document.forms[0].routeFloorplanName_old.value=\"%s\";", present_floorplan_name_no_suffix);		/* DEV-NOTE: Static version... will need to change to getting this dynamically, once you implement ability to change floorplans */
printf("}\n");
/* Clears the hidden form fields and resets defaults*/
printf("function clearAllHiddenFields(){");
printf("	document.forms[0].routeThickness.value=document.forms[0].routeThickness_old.value=\"\";");
printf("	document.forms[0].routePathColor.value=document.forms[0].routePathColor_old.value=\"\";");
printf("	document.forms[0].routeCoordsStr.value=document.forms[0].routeCoordsStr_old.value=\"\";");
printf("}\n");
/* Resets all form fields and JavaScript variables to freshly-loaded, default states */
printf("function resetAllFormAndJavascriptVariables(){");
printf("	routePathWidth=%d;", route_thickness);	/* define path thickness - just for initial starting point for now */
printf("	snapEnabled=false;");			/* define snap (i.e. straight-line-assist) attributes -- distance is number of pixels deviated from the x or y axis */
printf("	previ=-1; curri=0;");			/* setup increment counters that may be used */
printf("	coordinates=new Array(2);");		/* initialize array to hold a single point-coordinate pair (two elements representing X and Y - this is a 2D array) - this is used primarily for storing position when a click is registered */
printf("	routePoints=new Array();");		/* initialize array to hold all of a specific route's coordinate pairs (dynamic number of elements representing points along a path - this is a multidimensional array: each element contains a 2D array representing a coord-pair X,Y) */
printf("	showRouteClickCounter=0;");		/* counter for keeping track of how many times they click on a particular LCD -- for toggling through each potential route an LCD may have */
printf("	showRoutePreviousDeviceClicked=null;");	/* global for keeping track of the last LCD icon they clicked */
printf("	routesDrawn=0;");			/* global for keeping track of the number of routes drawn (mainly by show/more functions) */
printf("	tempGlobalArrForSpecificLCD;");		/* set a temporary global variable that we can insert an array into for holding route info across function calls */
printf("	document.forms[0].routeDtsec.value=\"%s\";", route_dtsec); 							/* NOTE: could be buggy since we're only obtaining this at document creation-time */
printf("	document.forms[0].routeUser.value=\"%s\";", CurrentUserPin);
printf("	document.forms[0].fp_route_need_auto_submit_to_enter_edit_mode.value=\"0\";");					/* initial flag value of 0 changes to 1 once we programatically click the edit map button so we can avoid an ininite form submission loop */
printf("	document.forms[0].showRouteMode.value=\"0\";");									/* initial flag value of 0 indicates that the page won't initially be in show-route mode - this changes to 1 once user chooses otherwise, by clicking the show route button*/
printf("	document.forms[0].addRouteMode.value=\"0\";");									/* initial flag value of 0 indicates that the page won't initially be in add-route mode - this changes to 1 once user chooses otherwise, by clicking the add route button*/
printf("	document.forms[0].submissionToAddRoute.value=\"\";");								/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("	document.forms[0].submissionToDeleteRoute.value=\"\";");							/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("	document.forms[0].submissionToEditRoute.value=\"\";");								/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("	document.forms[0].msgName.value=\"%s\";", route_msg_name);							/* earlier, we obtained the submitted data from msg editor form, now let's setup cross-state capability by storing that data in a persistent form field */
printf("	document.forms[0].msgDir.value=\"%s\";", route_msg_directory);							/* earlier, we obtained the submitted data from msg editor form, now let's setup cross-state capability by storing that data in a persistent form field */
printf("	document.forms[0].routeMediaportOrigin.value=\"%s\";", route_mediaport_origin);					/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("	document.forms[0].routeThickness.value=\"%d\";", route_thickness);						/* value initially set to whatever happens to be in cgi state - if nothing, then this will be whatever default is set in the parsed integer line above (6 at dev time) */
printf("	document.forms[0].routePathColor.value=\"%s\";", remove_trailing_white_space(route_path_color));		/* value initially set to whatever happens to be in cgi state - if nothing, then this will be whatever default is set in the parsed value line above (#ee0000 at dev time) */
printf("	document.forms[0].routeCoordsStr.value=\"%s\";", remove_trailing_white_space(route_coordinates_string));	/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("	document.forms[0].routeMediaportOrigin_old.value=\"%s\";", route_mediaport_origin_old);				/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("	document.forms[0].routeThickness_old.value=\"%d\";", route_thickness_old);					/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("	document.forms[0].routePathColor_old.value=\"%s\";", remove_trailing_white_space(route_path_color_old));	/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("	document.forms[0].routeCoordsStr_old.value=\"%s\";", remove_trailing_white_space(route_coordinates_string_old));/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("	document.forms[0].autoEnterRouteMode.value=\"0\";");								/* used for determining whether the JS should automatically "enter" route mode, i.e. showing the routing toolbar in lieu of standard tool */
printf("	document.forms[0].stateChangeMessage.value=\"%s\";", state_change_message);					/* used for passing along the message popped-up whenever some change to the route took place (save, delete, etc.) */
printf("	document.forms[0].routeLastSubmitReason.value=\"\";");
printf("	document.forms[0].routeLastUserIntention.value=\"\";");
printf("	document.forms[0].routeIndexAtSubmitTime.value=\"0\";");
printf("}\n");

/* Enables (shows) the canvas layer and any other appropriate functionalities that are associated with doing so. Needs the LCD device name. (this is called with an onclick event for any given LCD - likely only after ADD ROUTE is clicked) */
printf("function enableCanvas(deviceClicked){");
printf("	document.forms[0].routeMediaportOrigin.value=\"\"+deviceClicked+\"\";");	/* set the hidden form field with the LCD device name what they clicked */
printf("	canvas.style.display=\"block\";");						/* display the canvas with its natural state of display */
printf("        obtainCanvasAbsolutePosition();");						/* calculate the canvas offset */
/*printf("	changeElementStateById(\"btn_showRoute\",\"visibility\",\"hidden\");");*/		/* hide the SHOW ROUTE button since it serves no purpose at this time */
printf("	changeElementStateById(\"btn_colorChooser\",\"visibility\",\"visible\");");	/* show the color choice button */
printf("	changeElementStateById(\"btn_straighten\",\"visibility\",\"visible\");");	/* show the straight-line-assist button */
printf("	return true;");									/* since this is called in-line, need to return something, just to make the browser engine happy */
printf("}\n");

/* Disables (hides) the canvas layer, without clearing it. */
printf("function disableCanvas(){");
printf("	document.forms[0].routeMediaportOrigin.value=\"\";");
printf("	canvas.style.display=\"none\";");
printf("}\n");

/* Bubbles up through the DOM nodes (canvas' parent elements), adding their starting pixels to obtain the canvas' absolute position on the page, and set the global variables with the calculated offset. */
printf("function obtainCanvasAbsolutePosition(){");
printf("        var canvasLocal=canvas;");
printf("        var curleft=curtop=0;");
printf("        if(canvas.offsetParent){");
printf("                do{curleft+=canvasLocal.offsetLeft; curtop+=canvasLocal.offsetTop;} while(canvasLocal=canvasLocal.offsetParent);");
printf("                canvasPosX=curleft; canvasPosY=curtop;");
printf("	}");
printf("}\n");

/* Uses a click event on the active canvas element to ascertain the coordinates of the click, and then registers it in the array of points which is then used to draw the path */
printf("function registerClick(e){");
printf("	if(GetShowRouteMode()){");											/* if they want to edit the path of an existing route that has been manually shown (NOTE: routePoints, curri, and previ will need to have been set already!) */
printf("		document.forms[0].submissionToEditRoute.value=1;");								/* set the appropriate save flag, so CGI knows what to do after a form submission */
printf("		document.forms[0].submissionToAddRoute.value=0;");								/* set the appropriate save flag, so CGI knows what to do after a form submission */
printf("	}");
printf("	if(GetAddRouteMode()){");											/* if they want to add a new route (NOTE: routePoints, curri, and previ will need to have been set already!) */
printf("		document.forms[0].submissionToEditRoute.value=0;");								/* set the appropriate save flag, so CGI knows what to do after a form submission */
printf("		document.forms[0].submissionToAddRoute.value=1;");								/* set the appropriate save flag, so CGI knows what to do after a form submission */
printf("	}");
printf("	changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"visible\");");				/* show the undo button */
printf("	obtainCanvasAbsolutePosition();");										/* need this to recalcuate the offset, otherwise the points won't be where they click */
printf("       	obtainClickCoordinates(e);");											/* set the click coordinates */
printf("       	if(curri==0){");												/* if this is the first point (starting point)... */
printf("       		routePoints[curri]=new Array(2);");										/* nest a new array into the existing routePoints array, in preparation for holding this additional set of click coordinates */
printf("       		routePoints[curri][0]=coordinates[0]-canvasPosX;");								/* store the x position */
printf("       		routePoints[curri][1]=coordinates[1]-canvasPosY;");								/* store the y position */
printf("       		if(snapEnabled && curri>0){applySnapTo();}");									/* if at least one line segment exists at this point, and they desire straight-line-assist, then apply snap calculation to these click coordinates */
printf("		drawPoint(routePoints[curri][0],routePoints[curri][1]);");							/* then draw the point of origin for this route */
printf("		changeElementStateById(\"btn_finishRoute\",\"visibility\",\"hidden\");");					/* disable the save button since this route is now ineligible to be saved */
printf("       		curri++; previ++;");
printf("	}");
printf("	else if(curri >= maxPoints){");
printf("		window.alert(\"Maximum number of points reached\\n(\"+curri+\" out of \"+maxPoints+\")\");");
printf("	}");
printf("       	else{");													/* else this is not the first point... */
printf("       		routePoints[curri]=new Array(2);");										/* nest a new array into the existing routePoints array, in preparation for holding this additional set of click coordinates */
printf("       		routePoints[curri][0]=coordinates[0]-canvasPosX;");								/* store the x position */
printf("       		routePoints[curri][1]=coordinates[1]-canvasPosY;");								/* store the y position */
printf("       		if(snapEnabled && curri>0){applySnapTo();}");									/* if at least one line segment exists at this point, and they desire straight-line-assist, then apply snap calculation to these click coordinates */
printf("		changeElementStateById(\"btn_finishRoute\",\"visibility\",\"visible\");");					/* so enable the save button since this route is now eligible to be saved */
printf("		drawLineSegment(routePoints[previ][0],routePoints[previ][1],routePoints[curri][0],routePoints[curri][1]);");	/* draw this line segment */
printf("		drawLineSegmentConnectorPoint(routePoints[previ][0],routePoints[previ][1]);");					/* draw the point that will visually connect this line segment to the previous one */
printf("		drawArrowShape(routePoints[previ][0],routePoints[previ][1],routePoints[curri][0],routePoints[curri][1]);");	/* draw the triangle shaped arrowhead pointing with the vector of this line */
printf("       		curri++; previ++;");
printf("	}");
printf("}\n");

/* Get the coordinates of an onClick event (which is passed as a parameter. This takes into account different browsers' ways of handling event coordinates (thanks again, Microsoft) */
printf("function obtainClickCoordinates(e){");
printf("        if(!e){var e=window.event;}");
printf("        if(e.pageX || e.pageY){coordinates[0]=e.pageX; coordinates[1]=e.pageY;}");
printf("        else if(navigator.appName!=\"Microsoft Internet Explorer\" && (e.clientX || e.clientY)){coordinates[0]=e.clientX+document.body.scrollLeft+document.documentElement.scrollLeft; coordinates[1]=e.clientY+document.body.scrollTop+documentElement.scrollTop;}");
printf("        else if(navigator.appName==\"Microsoft Internet Explorer\" && (e.clientX || e.clientY)){coordinates[0]=e.clientX+document.body.scrollLeft; coordinates[1]=e.clientY+document.body.scrollTop;}");  /* (I hate IE) documentElement is undefine */
printf("}\n");

printf("function applySnapTo(){"); /* provides the straight-line-assist functionality */
printf("        if(Math.abs(routePoints[curri][0]-routePoints[previ][0])<=snapDistance){routePoints[curri][0]=routePoints[previ][0];}");
printf("        if(Math.abs(routePoints[curri][1]-routePoints[previ][1])<=snapDistance){routePoints[curri][1]=routePoints[previ][1];}");
printf("}\n");

/* Draw a starting point (the one with the bullseye effect) at the specified coordinates. */
printf("function drawPoint(x,y){");
printf("	x=parseInt(x); y=parseInt(y);");					/* parse the integers, just in case they persisted as a string from the file-data load */
printf("	var radius=(parseInt(document.forms[0].routeThickness.value)+2)/2;");	/* calculate starting point radius (needed by canvas arc method), scaled to the desired route thickness */
printf("	var color=document.forms[0].routePathColor.value;");			/* set a local variable for the color, to avoid repeated DOM calls */
printf("	canvasContext.beginPath();");						/* first, let's lay down a large circle of the chosen color for effect */
printf("		canvasContext.arc(x,y,radius+4,0,Math.PI*2,true);");			/* specify an arc */
printf("		canvasContext.closePath();");						/* close the arc path */
printf("		canvasContext.fillStyle=color;");					/* specify the color to fill the shape with */
printf("		canvasContext.fill();");						/* fill the shape with the specified color */
printf("	canvasContext.beginPath();");						/* then, let's lay down a smaller white circle on top of that */
printf("		canvasContext.arc(x,y,radius+2,0,Math.PI*2,true);");			/* specify an arc */
printf("		canvasContext.closePath();");						/* close the arc path */
printf("		canvasContext.fillStyle=\"#ffffff\";");					/* specify the color to fill the shape with */
printf("		canvasContext.fill();");						/* fill the shape with the specified color */
printf("	canvasContext.beginPath();");						/* finally, let's lay down an even smaller circle on the top, with the chosen color... all this provides a bullseye efect */
printf("		canvasContext.arc(x,y,radius-1,0,Math.PI*2,true);");			/* specify an arc */
printf("		canvasContext.closePath();");						/* close the arc path */
printf("		canvasContext.fillStyle=color;");					/* specify the color to fill the shape with */
printf("		canvasContext.fill();");						/* fill the shape with the specified color */
printf("}\n");

/* Draw a single line segment between the specified coordinates. */
printf("function drawLineSegment(x1,y1,x2,y2){");
printf("	var thickness=parseInt(document.forms[0].routeThickness.value);");	/* set a local variable for the thickness, to avoid repeated DOM calls */
printf("	canvasContext.beginPath();");						/* first, let's lay down a larger white line that will provide an outline effect, so the path will be visible on any color background */
printf("		canvasContext.moveTo(x1,y1); canvasContext.lineTo(x2,y2);");		/* beginning at the starting point, specify a line to the end point */
printf("		canvasContext.strokeStyle=\"#ffffff\";");				/* specify a white line color (the outline) */
printf("		canvasContext.lineWidth=thickness+3;");					/* specify the desired line thickness (bigger than what is specified, so help provide outline effect) */
printf("		canvasContext.lineCap=\"round\";");					/* specify what the ends of the line segment should look like */
printf("		canvasContext.stroke();");						/* draw the line, per these specifications */
printf("	canvasContext.beginPath();");						/* draw the smaller colored line on top of the white line just drawn */
printf("		canvasContext.moveTo(x1,y1); canvasContext.lineTo(x2,y2);");		/* beginning at the starting point, specify a line to the end point */
printf("		canvasContext.strokeStyle=document.forms[0].routePathColor.value;");	/* specify the desired line color */
printf("		canvasContext.lineWidth=thickness;");					/* specify the desired line thickness */
printf("		canvasContext.lineCap=\"round\";");					/* specify what the ends of the line segment should look like */
printf("		canvasContext.stroke();");						/* draw the line, per these specifications */
printf("}\n");

/* Draw a connector point at the specified coordinates. */
printf("function drawLineSegmentConnectorPoint(x1,y1){");
printf("	var radius=(parseInt(document.forms[0].routeThickness.value)+3)/2;");	/* set a local variable for the radius (needed by the canvas arc method), scaled to the desired route thickness */
printf("	var color=document.forms[0].routePathColor.value;");			/* set a local variable for the color, to avoid repeated DOM calls */
printf("	canvasContext.beginPath();");						/* then, lay down a circle to cover the white overlap (of the line segment) as best as we can, so it stands out but isn't too big */
printf("		canvasContext.arc(x1,y1,Math.round(radius),0,Math.PI*2,true);");	/* specify an arc */
printf("		canvasContext.closePath();");						/* close the arc path */
printf("		canvasContext.fillStyle=color;");					/* specify the color to fill the shape with */
printf("		canvasContext.fill();");						/* fill the shape with the specified color */
printf("}\n");

/* Draw the arrow-head (triangle) shape at the end of a line segment. Complicated because need to calculate vectors, account for line length, etc.
 *  drawArrowShape(beginningX, beginningY, endingX, endingY, [bool]) 
 *  NOTE! Do NOT mess with the advanced geometric-algebraic formulas used in calculating coordinate points of the arrowhead's corners (ax,ay,cx,cy) */
printf("function drawArrowShape(x1,y1,x2,y2,shortLineAware){");
printf("	x1=parseInt(x1); y1=parseInt(y1); x2=parseInt(x2); y2=parseInt(y2);");			/* just in case the parameters get passed as something other than a calcuable integer */
printf("	var lengthOfLine=Math.sqrt(Math.pow(Math.abs(x2-x1),2)+Math.pow(Math.abs(y2-y1),2));");	/* set a local variable with the calculated line segment length */
printf("	var thickness=parseInt(document.forms[0].routeThickness.value);");			/* set a local variable with the desired route thickness (in pixels) */
printf("	var arrowShapeLength=Math.round(thickness+(thickness*2.5));");				/* set a local variable with the calculated length of the arrow-head (scaled to the desired route thickness) */
printf("	if(shortLineAware){var shortLineFactor=1.5;}else{var shortLineFactor=-1;}");		/* set a local variable with a factor used to determine whether an arrowhead is drawn or not (won't draw it if the line is too short to accomodate an arrowhead) */
printf("	if(lengthOfLine>arrowShapeLength*shortLineFactor){");					/* if length of the line drawn is sufficient enough to draw an arrow head... */
printf("		var arrowShapeWidth,arrowBeginX,arrowBeginY,m,b,r,ax,ay,cx,cy;");			/* define local variables */
printf("		arrowShapeWidth=Math.round(thickness+(thickness*2.5));");				/* set local variable with the calculated width of the arrowhead */
printf("		if(y1==y2){");										/* if horizontal line was drawn (i.e. our slope is 0), need to treat differently since vertical bisector calculations will result in undefined slope (divide by zero) */
printf("			if(x2>x1){arrowBeginX=x2-arrowShapeLength;}");						/* if vectoring right, find the coordinate x-point on our line segment at which the back of our arrow should bisect */
printf("			else{arrowBeginX=x2+arrowShapeLength;}");						/* else vectoring left, find the coordinate x-point on our line segment at which the back of our arrow should bisect */
printf("			arrowBeginY=y2;");									/* find the coordinate y-point on our line segment at which the back of our arrow should bisect */
printf("			ax=arrowBeginX;");									/* point-A will begin a specified distance from our endpoint, in the x direction */
printf("			ay=Math.round(arrowBeginY-(arrowShapeWidth/2));");					/* point-A will begin a specified distance, equal to half the width of our arrowhead, in the y direction opposite of point-C */
printf("			cx=arrowBeginX;");									/* point-C will be a specified distance from our endpoint, in the x direction */
printf("			cy=Math.round(arrowBeginY+(arrowShapeWidth/2));");					/* point-C will be a specified distance, equal to half the width of our arrowhead, in the y direction opposite of point-A */
printf("		}");
printf("		else{");										/* else anything but horizontal line was drawn (our slope might undefined, resulting in an automatically calculated zero-slope perpendicular line -- or for other line, our slope will naturally be a real number) */
printf("			arrowBeginX=Math.round(x2-(x2-x1)*(arrowShapeLength/lengthOfLine));");			/* find the coordinate x-point on our line segment at which the back of our arrow should bisect */
printf("			arrowBeginY=Math.round(y2-(y2-y1)*(arrowShapeLength/lengthOfLine));");			/* find the coordinate y-point on our line segment at which the back of our arrow should bisect */
printf("			m=-1/((y2-y1)/(x2-x1));");								/* find the slope of the line that will perpendicularly bisect our line segment (it will be inverse of what was drawn) */
printf("			b=Math.round(arrowBeginY-m*arrowBeginX);");						/* find the y intercept of the line that would perpendicularly bisect our line segment */
printf("			r=Math.sqrt(Math.pow(arrowShapeLength,2)+Math.pow(arrowShapeWidth/2,2));");		/* this represents radius of a circle drawn from the line segment's endpoint, and is a function of the desired arrow size (as related to route thickness) */
printf("			ax=Math.round(((m*y2+x2-m*b)/(Math.pow(m,2)+1))+((Math.sqrt(Math.pow(r,2)-Math.pow((y2-m*x2-b)/(Math.sqrt(1+Math.pow(m,2))),2)))/(Math.sqrt(1+Math.pow(m,2)))));");/* use formula for solving a line that intersects a circle in two points */
printf("			ay=Math.round(((Math.pow(m,2)*y2+m*x2+b)/(Math.pow(m,2)+1))+((m*Math.sqrt(Math.pow(r,2)-Math.pow((y2-m*x2-b)/Math.sqrt(1+Math.pow(m,2)),2)))/(Math.sqrt(1+Math.pow(m,2)))));");/* use formula for solving a line that intersects a circle in two points */
printf("			cx=Math.round(((m*y2+x2-m*b)/(Math.pow(m,2)+1))-((Math.sqrt(Math.pow(r,2)-Math.pow((y2-m*x2-b)/(Math.sqrt(1+Math.pow(m,2))),2)))/(Math.sqrt(1+Math.pow(m,2)))));");/* use formula for solving a line that intersects a circle in two points */
printf("			cy=Math.round(((Math.pow(m,2)*y2+m*x2+b)/(Math.pow(m,2)+1))-((m*Math.sqrt(Math.pow(r,2)-Math.pow((y2-m*x2-b)/Math.sqrt(1+Math.pow(m,2)),2)))/(Math.sqrt(1+Math.pow(m,2)))));");/* use formula for solving a line that intersects a circle in two points */
printf("		}");
printf("		canvasContext.beginPath();");
printf("			canvasContext.moveTo(arrowBeginX,arrowBeginY);");				/* starting at A-prime (intersection of arrowhead's back edge and route's line segment... */
printf("			canvasContext.lineTo(ax,ay);");								/* draw line to A (defined as a point that is one-half the width of the arrow shape, in a perpendicular direction to the line segment that was drawn) */
printf("			canvasContext.lineTo(x2,y2);");								/* draw line to B (pointing point of the arrow) */
printf("			canvasContext.lineTo(cx,cy);");								/* draw line to C */
printf("			canvasContext.lineTo(arrowBeginX,arrowBeginY);");					/* draw line back to A-prime */
printf("			canvasContext.fillStyle=document.forms[0].routePathColor.value;");			/* specify the color to fill the arrowhead with */
printf("			canvasContext.strokeStyle=\"#ffffff\";");						/* specify the color of the arrowhead's outline */
printf("			canvasContext.lineWidth=1;");								/* specify the thickness of the arrowhead's outline */
printf("			canvasContext.fill(); canvasContext.stroke(); canvasContext.closePath();");		/* fill the arrowhead, outline it, and close the outline */
printf("	}");
printf("}\n");

/* Clear a rectangular area of the canvas, with the same location and size as its canvas -- effectively visually erasing anything and everything that might be drawn on any portion of the canvas. */
printf("function clearCanvas(){");
printf("        canvasContext.clearRect(0,0,canvas.width,canvas.height);");
printf("}\n");

/* Clear the multidimensional array that contains the main set of coordinate points for the route-path */
printf("function clearArray(){");
printf("        routePoints.length=0;");
printf("	curri=0; previ=-1;");
printf("}\n");

/* Undo the most recent/last line segment of routes in progress of being drawn, or existing routes. Need to clear the canvas and then redraw it based on the array of points for the route, minus the most recent/last point. */
printf("function undoLastSegment(){");
printf("	if(GetShowRouteMode()==1){");
printf("		document.forms[0].submissionToEditRoute.value=1;");
printf("		changeElementStateById(\"btn_finishRoute\",\"visibility\",\"visible\");");
printf("	}");
printf("        var numberOfPoints=routePoints.length;");
printf("        if(numberOfPoints==0){window.alert(\"You have not placed any points yet, so nothing to undo!\\n\\nPlease either click to place a starting point, or\\ncancel by clicking the red X next to the 'help' button.\");}"); /* user-error catching */
printf("        else if(numberOfPoints==1){undoStartingPoint();}");
printf("        else{");
printf("		if(numberOfPoints==2){document.getElementById(\"btn_finishRoute\").style.visibility=\"hidden\";}");
printf("                routePoints=routePoints.slice(0,numberOfPoints-1);");
printf("                curri--; previ--;");
printf("                clearCanvas();");
printf("		drawCanvasFromPointsInArray(routePoints);");
printf("	}");
printf("}\n");

/* Used whenever there is only one coordinate pair for a route (the starting point), and they click the undo button. Just need to clear the point and the array - no redrawing necessary */
printf("function undoStartingPoint(){");
printf("	if(GetShowRouteMode()==1){");
printf("		document.forms[0].submissionToEditRoute.value=1;");
printf("		changeElementStateById(\"btn_finishRoute\",\"visibility\",\"visible\");");
printf("	}");
printf("	clearCanvas();");
printf("	clearArray();");
printf("}\n");

printf("function cancelRoute(){");  /* on cancel, we are exiting route mode altogether */
printf("	clearCanvas(); clearArray();");
printf("	canvas.style.display=\"none\";");
printf("	SetRouteMode(0,null,null);");
printf("	changeElementStateById(\"btn_showRoute\",\"visibility\",\"hidden\");");
printf("	document.getElementById(\"btn_showRouteImage\").src=\"%s/showRoute.gif\";", cgi_icons);
printf("	changeElementStateById(\"btn_moreRoutes\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_maps\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"hidden\");");
printf("	document.getElementById(\"btn_addRouteImage\").src=\"%s/map_route_add.gif\";", cgi_icons);
printf("	changeElementStateById(\"btn_addRouteImage\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_addRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeThin\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeNormal\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeThick\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_colorChooser\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_straighten\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_finishRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_deleteRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeHelp\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_cancelRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"routewindowlet\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"mapcanvasHeader\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"fpEditToolbar\",\"visibility\",\"visible\");");
printf("	document.getElementById(\"fpPreTitle\").innerHTML=\"Device Map: \";");
printf("	document.getElementById(\"chooseDifferentMapChoiceButton\").style.display=\"\";");  /* hide the map selection button */
printf("	resetAllFormAndJavascriptVariables();");
printf("}\n");

/* When user clicks btn_reloadRoutes, the page will reload (re-read the routes file), but should come back displaying the last-visible route, and also be showing route toolbar */
printf("function reloadRoutesPage(){");
printf("	changeElementStateById(\"btn_showRoute\",\"visibility\",\"hidden\");");
printf("	document.getElementById(\"btn_showRouteImage\").src=\"%s/showRoute.gif\";", cgi_icons);
printf("	changeElementStateById(\"btn_moreRoutes\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_maps\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_undoLastRouteClick\",\"visibility\",\"hidden\");");
printf("	document.getElementById(\"btn_addRouteImage\").src=\"%s/map_route_add.gif\";", cgi_icons);
printf("	changeElementStateById(\"btn_addRouteImage\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_addRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeThin\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeNormal\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeThick\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_colorChooser\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_straighten\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_finishRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_deleteRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_routeHelp\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"btn_cancelRoute\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"routewindowlet\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"mapcanvasHeader\",\"visibility\",\"hidden\");");
printf("	changeElementStateById(\"fpEditToolbar\",\"visibility\",\"visible\");");
printf("	document.forms[0].submissionToAddRoute.value=0;");
printf("	document.forms[0].submissionToDeleteRoute.value=0;");
printf("	document.forms[0].submissionToEditRoute.value=0;");
printf("	document.forms[0].stateChangeMessage.value=\"Page Reloaded\";");
printf("	document.forms[0].submit();");
printf("}\n");

/* The GA page will reload (re-read the '' file), and should come back displaying the GA devices page */
printf("function reloadGaPage(){");
printf("	document.forms[0].lsdMode.value=0;");
printf("	document.forms[0].submissionToAddRoute.value=0;");
printf("	document.forms[0].submissionToDeleteRoute.value=0;");
printf("	document.forms[0].submissionToEditRoute.value=0;");
printf("	document.forms[0].stateChangeMessage.value=\"GA Page Reloaded\";");
printf("	document.forms[0].submit();");
printf("}\n");

/* Does pre-checks of everything before initiating a save submission */
printf("function finishRoutePreCheck(){");
printf("	if(document.forms[0].routeMediaportOrigin.value){");
printf("		var thisLCDsRoutes = extractAllRouteInfoForSpecificLCD(document.forms[0].routeMediaportOrigin.value);");

			/* if there are no records associated with this device, in this message contect, then just finish */
printf("		if(thisLCDsRoutes.length==0){");
printf("			finishRoute();");
printf("		}");

			/* a conflict would exist if this pending route were to be saved... so they need to choose an active one before continuing */
printf("		else if(multipleMapDeviceConflictWouldExist(document.forms[0].routeMediaportOrigin.value)){");
printf("			var str;");
printf("			var currentDevice = document.forms[0].routeMediaportOrigin.value;");						/* locally store the current route's display device (should be updated each time click LCD to show) */
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>Select the map you wish to display on the LCD:</p><p>\";");
				/* first display the currently visible floorplan (data gotten from current form state) */
printf("			if(parseInt(isFloorplanActiveForLCD(document.forms[0].routeFloorplanName.value,currentDevice))==1){");
printf("				strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' name=\\'floorplanselectiongroup\\' value=\\'\"+document.forms[0].routeFloorplanName.value+\"\\' />\"+document.forms[0].routeFloorplanName.value+\"</label>\";", cgi_icons);
printf("			}");
printf("			else{");
printf("				strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' name=\\'floorplanselectiongroup\\' value=\\'\"+document.forms[0].routeFloorplanName.value+\"\\' />\"+document.forms[0].routeFloorplanName.value+\"</label>\";");
printf("			}");
				/* then display other available floorplans (data gotten from global unique floorplans array) */
printf("			for(var i=0;i<arrPreExistingFloorplans.length;i++){");
printf("				str=removeTrailingSpace(arrPreExistingFloorplans[i]);");
printf("				if(removeTrailingSpace(document.forms[0].routeFloorplanName.value)!=str){");		/* if this iteration's floorplan is different than the currently loaded floorplan */
printf("					if(parseInt(isFloorplanActiveForLCD(arrPreExistingFloorplans[i],currentDevice))==1){");
printf("						strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrPreExistingFloorplans[i]+\"\\' />\"+str+\"</label>\";", cgi_icons);
printf("					}");
printf("					else{");
printf("						strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrPreExistingFloorplans[i]+\"\\' />\"+str+\"</label>\";");
printf("					}");
printf("				}");
printf("			}");
printf("			strHTML+=\"</p><div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'finishRoute();\\'>OK</button></div>\";"); /* button style is to keep IE from displaying long button */
printf("			Modalbox.show(strHTML,{title:\"Other maps have routes defined for this LCD...\"});");
printf("		}");

			/* if they're trying to submit without a route actually defined */
printf("		else if(routePoints.length<2){");
printf("			var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>A route cannot be saved if it contains 0 or 1 points.</p>\";");
printf("			strHTML+=\"<p>Please be sure your route is complete before continuing.</p>\";");
printf("			strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'Modalbox.hide();\\'>OK</button></div>\";");
printf("			Modalbox.show(strHTML,{title:\"Invalid route submitted...\"});");
printf("		}");
//printf("		else if(parseInt(document.forms[0].routeActiveFlag.value)==0){");
//printf("			var strHTML=\"<img src=\\'%s/dialog-warning.png\\' style=\\'float:left; margin-right:20px;\\' />\";", cgi_icons);
//printf("			strHTML+=\"<p style=\\'font-weight:bold;\\'>This map may be inactive.</p>\";");
//printf("			strHTML+=\"<p>Select the map you wish to activate for display on the LCD:</p>\";");
//printf("			strHTML+=\"<label style=\\'display:block;\\'><input type=\\'radio\\' checked=\\'checked\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;document.getElementById(\\\"contbtn01\\\").innerHTML=\\\"And Activate \"+document.forms[0].routeFloorplanName.value+\"\\\"\\' name=\\'floorplanselectiongroup\\' value=\\'\"+document.forms[0].routeFloorplanName.value+\"\\' />\"+document.forms[0].routeFloorplanName.value+\" <em>(current map)</em></label>\";");
//printf("			var str;");
//printf("			for(var i=0;i<arrPreExistingFloorplans.length;i++){");
//printf("				str=removeTrailingSpace(arrPreExistingFloorplans[i]);");
//printf("				if(document.forms[0].routeFloorplanName.value!=str){");
//printf("					strHTML+=\"<label style=\\'display:block;\\'><input type=\\'radio\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;document.getElementById(\\\"contbtn01\\\").innerHTML=\\\"But Activate \"+str+\"\\\"\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrPreExistingFloorplans[i]+\"\\' />\"+str+\"</label>\";");
//printf("				}");
//printf("			}");
//printf("			strHTML+=\"</p><div align=\\'center\\'><button style=\\'padding:0 .4em; width:auto; overflow:visible;\\' onclick=\\'finishRoute();\\'>Save This Route <span id=\\'contbtn01\\'>And Activate \"+document.forms[0].routeFloorplanName.value+\"</span></button>&nbsp;\";"); /* button style is to keep IE from displaying long button */
//printf("			strHTML+=\"<button onclick=\\'Modalbox.hide();return false;\\'>Cancel</button></div>\";");
//printf("			Modalbox.show(strHTML,{title:\"Potential conflict detected...\"});");
//printf("		}");
printf("		else{");
printf("			finishRoute();");
printf("		}");
printf("	}");
printf("	else{");
printf("		var strHTML_warn=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("		strHTML_warn+=\"<p class=\\'mb_mainmessage\\'>Warning, no LCD name was detected for this route!</p>\";");
printf("		strHTML_warn+=\"<p>You must specify an LCD display device to show this route on.</p>\";");
printf("		strHTML_warn+=\"<p>If you did specify an LCD, but you just placed it as a new LCD on the map, before creating this route, you may have forgotten to click 'DONE'</p>\";");
printf("		strHTML_warn+=\"<p>Please make sure you have either selected an LCD first, or clicked 'DONE' before continuing with route creation.</p>\";");
printf("		strHTML_warn+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'Modalbox.hide();\\'>OK</button></div>\";");
printf("		Modalbox.show(strHTML_warn,{title:\"No LCD Detected!\"});");
printf("	}");
printf("}\n");
/* Initiates a save route action (upon adding new or editing existing / newly-added) */
printf("function finishRoute(){");
printf("	document.forms[0].submissionToDeleteRoute.value=\"0\";");	/* just make sure delete flag isn't set, because that operation has its own function - you would not delete through a call to this function, but rather deleteRoute() */
printf("	if(GetSubmissionToEditRoute()){");				/* if we've set our edit flag because we made a change somewhere, then... */
printf("		document.forms[0].submissionToAddRoute.value=\"0\";");		/* ensure we don't set the flag that would add a new route */
printf("	}");
printf("	else{");							/* else in all other cases... (shouldn't happen, but just a bit of redundant logic for safe-measure */
printf("		document.forms[0].submissionToAddRoute.value=\"1\";");		/* set the flag that will add a whole new route, no questions asked - (if needed, they can delete the route that is now going to be added, later) */
printf("	}");
printf("	document.forms[0].routeCoordsStr.value=\"\"+coordPairArrayToDelimitedString(routePoints)+\"\";"); /* most of our route data should already be set in the form (either manually or by default), except for the array-bound coordinates-set, so set them into the form */
printf("	var str=document.forms[0].routeFloorplanName.value;");
printf("	if(str.replace(/\\s/g,'')==\"NULL\"){document.forms[0].routeFloorplanName.value=\"%s\";}", present_floorplan_name_no_suffix);
printf("	if(arrPreExistingFloorplans.length==1 && parseInt(document.forms[0].routeActiveFlag.value)==0){document.forms[0].routeActiveFlag.value=\"1\";}");	/* to fix adding an inactive route if a previously showing inactive happened */
printf("	document.forms[0].submit();");
printf("}\n");

/* Check if multiple map/device conflicts would exist for the specified device, with the current form route-data (in other words, what would be submitted), and return boolean response
 * (A conflict is defined as when, in the same message (.route file), an LCD exists in multiple record-lines along 
 *  with multiple different floorplans, while the active flag is true for more than one of those .) */
/* PSEUDO
- get existing records that match the pending device 
- out of those, get the ones that match the pending floorplan (and we only care about the active ones)
*/
printf("function multipleMapDeviceConflictWouldExist(lcdDevice){");
printf("	if(routesTable!=null){");									/* if there are existing routes (as evidenced in the main routesTable array actually containing stuff) */
printf("		var arrAllExistingRecordsByLCD = extractAllRouteInfoForSpecificLCD(lcdDevice);");		/* get an array of existing routes that are associated with the pending route's device */
printf("		arrPreExistingFloorplans = arrFloorplans = extractFloorplansFromArray(arrAllExistingRecordsByLCD,false);");/* (legacy but required to set a global?) */
printf("		if(arrAllExistingRecordsByLCD.length>0){");							/* if there are existing records... */
printf("			for(var i=0; i<arrAllExistingRecordsByLCD.length; i++){");					/* loop through them... */
printf("				if(parseInt(arrAllExistingRecordsByLCD[i][rtiActiveFlag])==1");					/* if this iteration's record is marked as active AND its floorplan is different than the pending-save, then... */
printf("					&& removeTrailingSpace(arrAllExistingRecordsByLCD[i][rtiFloorplanName])!=removeTrailingSpace(document.forms[0].routeFloorplanName.value)){");
printf("					return true;");											/* return true (should exit this loop and function altogether) */
printf("				}");
printf("			}");
printf("			return false;");										/* loop has completed without exiting (by means of returning true), so we should return false since no routes were found to be active (highly unlikely) */
printf("		}");
printf("		else{");											/* else there were no existing records in that array, so... */
printf("			return false;");										/* return false */
printf("		}");
printf("	}");
printf("	else{");											/* else there is something not quite right with existing routes (or no existing routes), so... */
printf("		return false;");										/* return false */
printf("	}");
printf("}\n");

/* Processes the array argument given to it (must be in routesTable format), and returns a simple array of unique floorplan names that are contained therein (array indexes of the argument must match the convention found in routesTable main array) 
 * 	The optional argument, specificOfMultipleFloorplans, can be provided to force its inclusion at the top of the array (this function will not duplicate it, if it's also found in the passed array)... this only works if there are multiple floorplans! */
/* 	function extractFloorplansFromArray(arr routeArray, [str floorplanName]) -- optional value must be set to false if not using */
printf("function extractFloorplansFromArray(arr,specificOfMultipleFloorplans){");
printf("	if(arr){");											/* if an argument was provided to this function, then... */
printf("		var arrFloorplans = [];");
printf("		var j = 0;");											/* setup a counter for creating the new array */
printf("		if(arr.length==1){");										/* if there is only one element (record) in the array, then... */
printf("			arrFloorplans[j]=arr[0][rtiFloorplanName];");							/* return the floorplan name extracted from it */
printf("		}");
printf("		else if(arr.length>1){");									/* else if there are multiple elements (records) in the array, then... */
printf("			if(specificOfMultipleFloorplans){");								/* if the optional argument was provided, then... */
printf("				arrFloorplans[j] = specificOfMultipleFloorplans;");						/* inject it as the first element (top of the list) */
printf("				j++;");												/* increment this array's counter */
printf("			}");
printf("			var strSpecificOfMultipleFloorplans = \"\";");							/* put the argument into an assured string-type of variable, so we can apply string methods ('replace' method to strip trailing spaces for comparison) */
printf("			strSpecificOfMultipleFloorplans+=specificOfMultipleFloorplans;");
printf("			strSpecificOfMultipleFloorplans = strSpecificOfMultipleFloorplans.replace(/\\s/g,'');");	/* apply the replace method to strip off all trailing spaces */
printf("			var str2 = \"\";");										/* create a string-type variable (again, so we can use string methods) for the array's floorplan */
printf("			for(var i=0;i<arr.length;i++){");									/* for each element of the array passed to this function... */
printf("				str2 = arr[i][rtiFloorplanName];");								/* save this element's floorplan in a variable for this iteration, so we can use string methods */
printf("				if(str2.replace(/\\s/g,'')!=strSpecificOfMultipleFloorplans){");				/* if this element's floorplan is different than that specified in the argument (even if no arg provided, the following will happen), then... */
printf("					if(i==0){");											/* if this is the first element, then... */
printf("						arrFloorplans[j]=arr[i][rtiFloorplanName];");							/* extract the floorplan name to get things started */
printf("						j++;");												/* increment this array's counter */
printf("					}");
printf("					else{");											/* else, for everything after the first record... */
printf("						if(arrFloorplans[j-1]!=arr[i][rtiFloorplanName]){");						/* if the last record's floorplan is different than this record's floorplan, then... */
printf("							arrFloorplans[j]=arr[i][rtiFloorplanName];");							/* extract this floorplan name */
printf("							j++;");												/* increment this array's counter */
printf("						}");
printf("					}");
printf("				}");
printf("			}");
printf("		}");
printf("		return arrFloorplans;");									/* return the simple array that was just generated */
printf("	}");
printf("	else{");											/* else, no argument was provided, so... */
printf("		return false;");										/* return false */
printf("	}");
printf("}\n");

/* Initiates a delete route action */
printf("function deleteRoute(doDelete){");
printf("	if(!doDelete){");													/* if this is a virgin call of this function (no argument provided), then do the default necessary checks (which will then recursively call this function if needed) */
printf("		var arrRoutesByLCD = extractAllRouteInfoForSpecificLCD(document.forms[0].routeMediaportOrigin.value);");		/* get an array of routes (all floorplans) for the LCD of the route that is being desired to delete */
printf("		var arrFloorplansByLCD = extractFloorplansFromArray(arrRoutesByLCD,false);");						/* get an array of unique floorplans that exist for that LCD */
printf("		var routesByLCD = arrRoutesByLCD.length;");										/* get the number of routes that exist for the LCD */
printf("		var floorplansByLCD = arrFloorplansByLCD.length;");									/* get the number of floorplans that exist for the LCD */
printf("		var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";", cgi_icons);
printf("		if(routesByLCD == 1){");												/* if only one route exists, then of course they are deleting the ONLY route for its device, so we need verification from the user */
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>This is the only route that exists for \"+document.forms[0].routeMediaportOrigin.value+\"! Are you sure you want to delete this route?</p><div align=\\'center\\'>\";");
printf("			strHTML+=\"<input type=\\'button\\' value=\\'Yes, delete it anyway\\' onclick=\\'Modalbox.hide();deleteRoute(true);\\' /> or \";");
printf("			strHTML+=\"<input type=\\'button\\' value=\\'No, cancel delete\\' onclick=\\'Modalbox.hide();return false;\\' /></div>\";");
printf("		}");
printf("		else if(routesByLCD > 1 && floorplansByLCD == 1){");									/* else if more than one route exists for this device, but only one floorplan, then... (all routes are the same floorplan, so delete should be okay) */
printf("			strHTML+=\"<p class=\\'mb_mainmessage\\'>Are you sure you want to delete this route?</p><div align=\\'center\\'>\";");
printf("			strHTML+=\"<input type=\\'button\\' value=\\'Yes, delete it\\' onclick=\\'Modalbox.hide();deleteRoute(true);\\' /> or \";");
printf("			strHTML+=\"<input type=\\'button\\' value=\\'No, cancel delete\\' onclick=\\'Modalbox.hide();return false;\\' /></div>\";");
printf("		}");
printf("		else{");														/* else if more than one route exists for this device, and also more than one floorplan, then... (need to check if this route's delete would result in no routes remaining active) */
printf("			var deletingRouteFloorplan = document.forms[0].routeFloorplanName.value;");
printf("			var deletingRouteActiveFlag = parseInt(document.forms[0].routeActiveFlag.value);");
printf("			var arrRoutesByLCDFP = extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");/* get an array of routes for the floorplan that's associated with the route they're trying to delete */
printf("			var routesByLCDFP = arrRoutesByLCDFP.length;");										/* get the number of routes that exist for the LCD on the floorplan that's associated with the route they're trying to delete */
//printf("			if(deletingRouteActiveFlag==1 && routesByLCDFP==1){");									/* if the route that they're trying to delete is active, and it's the only one for its LCD on this floorplan, then... (they want to delete the only active route for the floorplan) */
//printf("				var arrFloorplansByLCDexceptThisFP=[];");
//printf("				var str=\"\";");													/* explicitly declare a string-type variable */
//printf("				str+=document.forms[0].routeFloorplanName.value;");									/* get the pending route-delete's floorplan */
//printf("				str=str.replace(/\\s/g,'');");												/* strip all trailing spaces so we can compare */
//printf("				var str2 = \"\";");													/* explicitly declare a string-type variable */
//printf("				var j=0;");
//printf("				for(var i=0;i<floorplansByLCD;i++){");											/* for each floorplan that exists for this device... */
//printf("					str2 = arrFloorplansByLCD[i];");											/* save this floorplan in a variable for this iteration, so we can use string methods */
//printf("					str2 = str2.replace(/\\s/g,'');");											/* strip all trailing spaces, using the replace method, so we can compare */
//printf("					if(str!=str2){");													/* if this floorplan is different than the one of the route they're trying to delete, copy it to the array */
//printf("						arrFloorplansByLCDexceptThisFP[j]=arrFloorplansByLCD[i];");
//printf("						j++;");
//printf("					}");
//printf("				}");															/* (now we have an array of floorplans that excludes this route's floorplan - can be used in a selection group) */
//printf("				if(floorplansByLCD==2){");												/* if there are only two total floorplans for this LCD, then... (the other floorplan needs to be made active) */
//printf("					strHTML+=\"<p class=\\'mb_mainmessage\\'>You are trying to delete the only remaining active route for this LCD!</p>\";");
//printf("					strHTML+=\"<p>However, other inactive route(s) already exist for this LCD, on another floorplan (\"+str2+\"). If you delete this route, that floorplan will be activated in its place.</p>\";");
//printf("					strHTML+=\"<div align=\\'center\\'><button style=\\'padding:0 .4em;width:auto;overflow:visible;\\' onclick=\\'document.forms[0].selectedActiveFloorplan.value=\\\"\"+arrFloorplansByLCDexceptThisFP[0]+\"\\\";deleteRoute(true);\\'>Delete This Route and Activate \"+arrFloorplansByLCDexceptThisFP[0]+\"</button>&nbsp;\";");
//printf("					strHTML+=\"<button onclick=\\'Modalbox.hide();return false;\\'>OK</button></div>\";");
//printf("					strHTML+=\"<button onclick=\\'Modalbox.hide();return false;\\'>Cancel</button></div>\";");
//printf("				}");
//printf("				else{");														/* else there must be multiple floorplans for this LCD, then... (one of the other floorplans needs to be made active) */
//printf("					strHTML+=\"<p class=\\'mb_mainmessage\\'>You are trying to delete the only remaining active route for this LCD! However, inactive routes already exist for this LCD, on other floorplans.</p>\";");
//printf("					strHTML+=\"<p>To delete this route, you need to identify which routes will become active in its place, by selecting a map below:</p><p>\";");
//printf("					strHTML+=\"<label style=\\'display:block;\\'><input type=\\'radio\\' checked=\\'checked\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;document.getElementById(\\\"contbtn02\\\").innerHTML=\\\"\"+arrFloorplansByLCDexceptThisFP[0]+\"\\\"\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCDexceptThisFP[0]+\"\\' />\"+arrFloorplansByLCDexceptThisFP[0]+\"</label>\";");
//printf("					for(var i=1;i<arrFloorplansByLCDexceptThisFP.length;i++){");
//printf("						strHTML+=\"<label style=\\'display:block;\\'><input type=\\'radio\\' onfocus=\\'document.forms[0].selectedActiveFloorplan.value=this.value;document.getElementById(\\\"contbtn02\\\").innerHTML=\\\"\"+arrFloorplansByLCDexceptThisFP[i]+\"\\\";\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrFloorplansByLCDexceptThisFP[i]+\"\\' />\"+arrFloorplansByLCDexceptThisFP[i]+\"</label>\";");
//printf("					}");
//printf("					strHTML+=\"</p><div align=\\'center\\'><button style=\\'padding:0 .4em;width:auto;overflow:visible;\\' onclick=\\'deleteRoute(true);\\'>Delete This Route and Activate <span id=\\'contbtn02\\'>\"+arrFloorplansByLCDexceptThisFP[0]+\"</span></button>&nbsp;\";");
//printf("					strHTML+=\"<button onclick=\\'Modalbox.hide();return false;\\'>Cancel</button></div>\";");
//printf("				}");
//printf("			}");
//printf("			else{");														/* else the route that they're trying to delete isn't active, so we don't care if its deleted or not */
printf("				strHTML+=\"<p class=\\'mb_mainmessage\\'>Are you sure you want to delete this route?</p><div align=\\'center\\'>\";");
printf("				strHTML+=\"<input type=\\'button\\' value=\\'Yes, delete it\\' onclick=\\'Modalbox.hide();deleteRoute(true);\\' /> or \";");
printf("				strHTML+=\"<input type=\\'button\\' value=\\'No, cancel delete\\' onclick=\\'Modalbox.hide();return false;\\' /></div>\";");
//printf("			}");
printf("		}");
printf("		Modalbox.show(strHTML,{title:\"Please confirm...\"});");
printf("	}");
printf("	else if(doDelete){");													/* else if this function was called explicity to do a delete (by being passed true), then... */
printf("		document.forms[0].submissionToAddRoute.value=\"0\";");									/* make sure add flag is not set */
printf("		document.forms[0].submissionToEditRoute.value=\"0\";");									/* make sure edit flag is not set */
printf("		document.forms[0].submissionToDeleteRoute.value=\"1\";");								/* enable the delete flag, so CGI can do what it needs to after form submission */
printf("		document.forms[0].submit();");												/* submit the form */
printf("	}");
printf("	else{");														/* else they cancelled, so... */
printf("		return false;");													/* do not do anything */
printf("	}");
printf("}\n");

/* Converts an array of multiple coordinate pairs to a delimited string */
printf("function coordPairArrayToDelimitedString(arr){");
printf("	var delimXY=\"_\";");				/* specify the desired X-Y coordinate delimiter (between the x and y that make up a single coordinate pair) */
printf("	var delimCP=\"-\";");				/* specify the desired coordinate pair delimiter (between the coordinate pairs) */
printf("	var strCoords=\"\";");  
printf("	for(var i=0;i<arr.length;i++){");			/* for each coordinate pair in the array... */
printf("		strCoords+=arr[i][0]+delimXY+arr[i][1];");	/* append its x and y values to the string */
printf("		strCoords=strCoords.replace(/\\s+$/, '');");	/* strip off all trailing space characters, since this will not interact with the DB/file and doesn't have to be exact, but just pretty - also so we can do some test conditions below */
printf("		if(i<arr.length-1){strCoords+=delimCP;}");	/* if we are not at the final pair in the array, append a pair delimiter in preparation for the next pair to be appended */
printf("	}");
printf("	return strCoords;");
printf("}\n");

/* Converts a string representing a route path's multiple points to a multidimensional array */
printf("function delimitedStringToCoordPairArray(coordString){");
printf("	var delimXY=\"_\";");					/* specify the X-Y coordinate delimiter used (between the x and y that make up a single coordinate pair) */
printf("	var delimCP=\"-\";");					/* specify the coordinate pair delimiter used (between the coordinate pairs) */
printf("	var arrRoutePoints=new Array();");			/* create a place in the first dimension for our soon-to-be-nested array */
printf("	var arrCoordPairs=coordString.split(delimCP);");	/* gives us an array of coordinate pairs... now need to split those up */
printf("	for(var i=0;i<arrCoordPairs.length;i++){");			/* for each of those pairs, split them into an array that's nested into a master array that we'll return */
printf("		arrRoutePoints[i]=arrCoordPairs[i].split(delimXY);");	/* nest an array that represents a single coordinate pair */
printf("	}");
printf("	return arrRoutePoints;");
printf("}\n");

/* Toggle the straight-line-assist mode and button representing that mode */
printf("function toggleSnap(gifPath){");
printf("	var objStraightenField=document.getElementById(\"btn_straightenField\");");
printf("        if(objStraightenField.value==\"true\"){");
printf("		snapEnabled=false;");
printf("		objStraightenField.value=\"false\";");
printf("		document.getElementById(\"btn_straightenImage\").src=gifPath+\"/straighten0.gif\";");
printf("	}");
printf("        else{");
printf("		snapEnabled=true;");
printf("		objStraightenField.value=\"true\";");
printf("		document.getElementById(\"btn_straightenImage\").src=gifPath+\"/straighten1.gif\";");
printf("	}");
printf("}\n");

/* Should be called whenever thickness is changed or specified (either by the user or other functions). It will update the buttons as well as route data in the form, and even redraw the line to represent specified thickness */
printf("function thickChooser(operation,choice,gifPath){");
printf("	if(operation==\"click\"){");
printf("		if(choice==\"thin\"){");
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin_selected.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick.gif\";");
printf("			document.forms[0].routeThickness.value=\"%d\";", ROUTE_THICKNESS_THIN);
printf("		}");
printf("		else if(choice==\"thick\"){");
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick_selected.gif\";");
printf("			document.forms[0].routeThickness.value=\"%d\";", ROUTE_THICKNESS_THICK);
printf("		}");
printf("		else{");
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal_selected.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick.gif\";");
printf("			document.forms[0].routeThickness.value=\"%d\";", ROUTE_THICKNESS_NORMAL);
printf("		}");
printf("		clearCanvas();");
printf("		routePathWidth=parseInt(document.forms[0].routeThickness.value);");
printf("		drawCanvasFromPointsInArrayForMediaport(routePoints);");
printf("		var numberOfRegisteredPoints=parseInt(routePoints.length);");								/* set a local variable to the number of elements in the routePoints array (to get how many points are active) */
printf("		if(isNaN(numberOfRegisteredPoints)){numberOfRegisteredPoints=0;}");							/* just in case we got something weird, set to 0 so we can test reliably down below */
printf("		if(GetAddRouteMode()!=1 && (GetShowRouteMode()==1 || numberOfRegisteredPoints>0)){document.forms[0].submissionToEditRoute.value=1;changeElementStateById(\"btn_finishRoute\",\"visibility\",\"visible\");}");
printf("	}");
printf("	if(operation==\"updateButtons\"){");
printf("		choice=parseInt(choice);");
printf("		if(choice==%d){", ROUTE_THICKNESS_THIN);
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin_selected.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick.gif\";");
printf("		}");
printf("		else if(choice==%d){", ROUTE_THICKNESS_THICK);
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick_selected.gif\";");
printf("		}");
printf("		else{");
printf("			document.getElementById(\"btn_routeThinImage\").src=gifPath+\"/routeThin.gif\";");
printf("			document.getElementById(\"btn_routeNormalImage\").src=gifPath+\"/routeNormal_selected.gif\";");
printf("			document.getElementById(\"btn_routeThickImage\").src=gifPath+\"/routeThick.gif\";");
printf("		}");
printf("	}");
printf("}\n");

/* Should be called whenever color is changed or specified (either by the user or other functions). It will update the buttons as well as route data in the form, and even redraw the line to represent specified color
 *  colorChooser([operation], [color])
 *  (default action with no arguments will redraw the route with existing form data) */
printf("function colorChooser(operation,color){");
printf("	var selectedColor;");
printf("	if(color){");											/* if color argument is provided, then set a local variable with it and update the form data */
printf("		selectedColor=color;");										/* set the local variable to the value passed through the argument */
printf("		document.forms[0].routePathColor.value=selectedColor;");					/* set the form data with that value, also */
printf("	}");
printf("	else{");											/* else no color argument is provided, so set the local variable to whatever color is in the form data */
printf("		selectedColor=document.forms[0].routePathColor.value;");					/* set the local variable to the value currently in the form data */
printf("	};");
printf("	if(operation==\"updateButtons\"){");								/* if operation specified was just to update the buttons (color has been specified outside of this function somehow), then only update the buttons */
printf("		changeElementStateById(\"btn_colorChooser\",\"backgroundColor\",selectedColor);");		/* update the color button */
printf("		changeElementStateById(\"btn_routeThinImage\",\"backgroundColor\",selectedColor);");		/* update this thickness button */
printf("		changeElementStateById(\"btn_routeNormalImage\",\"backgroundColor\",selectedColor);");		/* update this thickness button */
printf("		changeElementStateById(\"btn_routeThickImage\",\"backgroundColor\",selectedColor);");		/* update this thickness button */
printf("	}");
printf("	else{");											/* else operation defaults to an assumed click on the color button (to select a color), so update the buttons to reflect the chosen color and redraw the route, etc. */
printf("		changeElementStateById(\"btn_colorChooser\",\"backgroundColor\",selectedColor);");		/* update the color button */
printf("		changeElementStateById(\"btn_routeThinImage\",\"backgroundColor\",selectedColor);");		/* update this thickness button */
printf("		changeElementStateById(\"btn_routeNormalImage\",\"backgroundColor\",selectedColor);");		/* update this thickness button */
printf("		changeElementStateById(\"btn_routeThickImage\",\"backgroundColor\",selectedColor);");		/* update thsi thickness button */
printf("		clearCanvas();");										/* clear the canvas in preparation for redrawing the route with the chosen new color */
printf("		drawCanvasFromPointsInArrayForMediaport(routePoints);");					/* redraw the route with the chosen new color */
printf("		var numberOfRegisteredPoints=parseInt(routePoints.length);");					/* set a local variable to the number of elements in the routePoints array (to get how many points are active) */
printf("		if(isNaN(numberOfRegisteredPoints)){numberOfRegisteredPoints=0;}");				/* just in case we got something weird, set to 0 so we can test reliably down below */
printf("		if(!GetAddRouteMode() && (GetShowRouteMode()==1 || numberOfRegisteredPoints>0)){");		/* if color was updated for an already existing route, then assume this is an edit, so we need to let them save */
printf("			document.forms[0].submissionToEditRoute.value=1;");						/* enable the submit-to-edit flag */
printf("			changeElementStateById(\"btn_finishRoute\",\"visibility\",\"visible\");");			/* ensure we are showing the SAVE ROUTE button (they made an edit, so need to be able to save this change) */
printf("		}");
printf("	}");
printf("}\n");

/* Draw single route for display on a user's screen while creating and editing routes - displays them as discrete line segments that are helpfu lin understanding the construction of a route */
printf("function drawCanvasFromPointsInArray(pointsArray){");
printf("	for(var i=0;i<pointsArray.length;i++){");						/* looping through each coordinate pair that makes up this route... */
printf("		if(i==0){drawPoint(pointsArray[i][0],pointsArray[i][1]);}");			/* if at the first point, draw the starting point */
printf("		else {");									/* else we are beyond the starting point, so... */
printf("			drawLineSegment(pointsArray[i-1][0],pointsArray[i-1][1],pointsArray[i][0],pointsArray[i][1]);");	/* draw the line segment from the previous point to this current one */
printf("			drawLineSegmentConnectorPoint(pointsArray[i-1][0],pointsArray[i-1][1]);");				/* draw the point (at the beginning of the segment) that will visually connect this line segment to the previous one */
printf("			drawArrowShape(pointsArray[i-1][0],pointsArray[i-1][1],pointsArray[i][0],pointsArray[i][1]);");		/* draw the triangle shaped arrowhead (the function automatically vectors depending on start/end points) pointing with the vector of this line */
printf("		}");
printf("	}");
//printf("	canvas.style.width=null;");							/* gecko-neutral hack for IE, otherwise canvas breaks for this window-session */
//printf("	setTimeout(\"canvas.style.width=canvas.parentNode.style.width\",100);");	/* gecko-neutral hack for IE, otherwise canvas breaks for this window-session */
printf("}\n");

/* Draw single route for display on a MediaPort (also for a prettier display on the user's screen - alternative to the individual line-segments view) */
printf("function drawCanvasFromPointsInArrayForMediaport(pointsArray){");
printf("	var thickness=parseInt(document.forms[0].routeThickness.value);");
printf("	for(var i=0;i<pointsArray.length;i++){");	/* looping through each coordinate pair that makes up this route, draw our base white route first (so we can provide a white outline effect) */
printf("		if(i==0){");								/* if we are at the beginning of our route... */
printf("			drawPoint(pointsArray[i][0],pointsArray[i][1]);");			/* draw point of origin */
printf("			canvasContext.strokeStyle=\"#ffffff\";");				/* specify a color of white (color of the outline) */
printf("			canvasContext.lineWidth=thickness+3;");					/* specify a width that is larger than the desired, to allow for an outline effect */
printf("			canvasContext.lineCap=\"round\";");					/* specify the line end-cap style */
printf("			canvasContext.lineJoin=\"round\";");					/* specify the line join style */
printf("			canvasContext.beginPath();");						/* begin the path */
printf("			canvasContext.moveTo(pointsArray[i][0],pointsArray[i][1]);");		/* start at first point */
printf("		}");
printf("		else if(i==pointsArray.length-1) {");					/* if we are at the end of our route... */
printf("			canvasContext.lineTo(pointsArray[i][0],pointsArray[i][1]);");		/* continue along route to this final point */
printf("			canvasContext.stroke();");						/* actually draw the line that we set-out up until now */
printf("			canvasContext.closePath();");						/* close the path */
printf("		}");
printf("		else {");								/* if we are in the middle of our route... */
printf("			canvasContext.lineTo(pointsArray[i][0],pointsArray[i][1]);");		/* continue along route to this next point */
printf("		}");
printf("	}");
printf("	for(var i=0;i<pointsArray.length;i++){");	/* looping through each coordinate pair that makes up this route, draw a route of the user-specified color and width on top of that, to finish the outline effect */
printf("		if(i==0){");								/* if we are at the beginning of our route... */
printf("			canvasContext.strokeStyle=document.forms[0].routePathColor.value;");	/* specify a color of white (color of the outline) */
printf("			canvasContext.lineWidth=thickness;");					/* specify a width that is larger than the desired, to allow for an outline effect */
printf("			canvasContext.lineCap=\"round\";");					/* specify the line end-cap style */
printf("			canvasContext.lineJoin=\"round\";");					/* specify the line join style */
printf("			canvasContext.beginPath();");						/* begin the path */
printf("			canvasContext.moveTo(pointsArray[i][0],pointsArray[i][1]);");		/* start at first point */
printf("		}");
printf("		else if(i==pointsArray.length-1){");					/* if we are at the end of our route... */
printf("			canvasContext.lineTo(pointsArray[i][0],pointsArray[i][1]);");		/* continue along route to this final point */
printf("			canvasContext.stroke();");						/* actually draw the line that we set-out up until now */
printf("			canvasContext.closePath();");						/* close the path */
printf("			drawArrowShape(pointsArray[i-1][0],pointsArray[i-1][1],pointsArray[i][0],pointsArray[i][1]);");/* draw the arrowhead at the end of the route... */
printf("		}");
printf("		else{");								/* if we are in the middle of our route... */
printf("			canvasContext.lineTo(pointsArray[i][0],pointsArray[i][1]);");		/* continue along route to this next point */
printf("		}");
printf("	}");
printf("	curri=pointsArray.length; previ=curri-1;");
printf("	canvas.style.width=null;");							/* gecko-neutral hack for IE, otherwise canvas breaks for this window-session */
printf("	setTimeout(\"canvas.style.width=canvas.parentNode.style.width\",100);");	/* gecko-neutral hack for IE, otherwise canvas breaks for this window-session */
printf("}\n");

/* Extracts from the main routesTable array and returns a multi-dimensional array containing route data for a specified LCD and message */
printf("function extractAllRouteInfoForSpecificLCD(lcdDeviceName){");
printf("	var arr;");
printf("	if(routesTable!=null){");
printf("		arr=[];");								/* initialize a new array with local scope that we will later return */
printf("		var j=0;");								/* increment counter that will be used for populating the local array */
printf("		for(var i=0;i<routesTable.length;i++){");					/* loop through each of the main routes table (created earlier by C-written javascript) */
printf("			if(routesTable[i][rtiDeviceName]==lcdDeviceName){");			/* if we find a match of the specified LCD, then we need to extract this route */
printf("				arr[j]=routesTable[i].slice(0);");					/* copy the record's elements by value */
printf("				j++;");									/* increment the counter to prepare for any additional nested arrays, if there happen to be multiple routes for a given mediaport */
printf("			}");
printf("		}");
printf("	}");
printf("	else{");
printf("		arr=false;");
printf("	}");
printf("	return arr;");								/* return the local array to the whatever called this function */
printf("}\n");
/* Extracts from the main routesTable array and returns a multi-dimensional array containing route data for a specified LCD, floorplan and message */
printf("function extractAllRouteInfoForSpecificLCDwithFloorplan(lcdDeviceName,floorplanName){");
printf("	var arr;");
printf("	if(routesTable!=null){");
printf("		arr=[];");								/* initialize a new array with local scope that we will later return */
printf("		var j=0;");								/* increment counter that will be used for populating the local array */
printf("		for(var i=0;i<routesTable.length;i++){");					/* loop through each of the main routes table (created earlier by C-written javascript) */
printf("			if(routesTable[i][rtiDeviceName]==lcdDeviceName){");			/* if we find a match of the specified LCD, then we may need to extract this route if.... */
printf("				if(routesTable[i][rtiFloorplanName]==floorplanName){");			/* if we find a match of the specified floorplan, then we definitely need to extract this route */
printf("					arr[j]=routesTable[i].slice(0);");					/* copy the record's elements by value */
printf("					j++;");									/* increment the counter to prepare for any additional nested arrays, if there happen to be multiple routes for a given mediaport */
printf("				}");
printf("			}");
printf("		}");
printf("	}");
printf("	else{");
printf("		arr=false;");
printf("	}");
printf("	return arr;");								/* return the local array to the whatever called this function */
printf("}\n");

/* Extracts an array listing the different floorplans/maps that are routed */
printf("function extractAllFloorplansFromRoutes(){");
printf("	var arr;");
printf("	if(routesTable!=null){");
printf("		arr=[];");						/* initialize a new array with local scope that we will later return */
printf("		var j=0;");
printf("		for(var i=0;i<routesTable.length;i++){");			/* for each route record... */
printf("			if(i==0){");						/* if this is the first record... */
printf("				arr[j]=routesTable[i][rtiFloorplanName];");		/* then extract the floorplan name to get things started */
printf("			}");
printf("			else{");						/* else... */
printf("				if(routesTable[i][rtiFloorplanName]!=arr[j]){");	/* if this next record's floorplan is different than the last one extracted... */
printf("					j++; arr[j]=routesTable[i][rtiFloorplanName];");	/* then extract this new different one */
printf("				}");
printf("			}");
printf("		}");
printf("	}");
printf("	else{");
printf("		arr=false;");
printf("	}");
printf("	return arr;");					/* return the local array to the whatever called this function */
printf("}\n");

/* Called by a MediaPort's browser when displaying a routes message, to make it flash. */
printf("function makeRouteFlash(){");
printf("	if(canvas.style.display==\"block\"){");			/* if canvas is currently showing, then... */
printf("		canvas.style.display=\"none\";");			/* hide the canvas layer */
printf("		setTimeout(\"makeRouteFlash()\",450);");		/* wait this long before recursion to display it again (this is the time it is invisible for) */
printf("	}");
printf("	else{");						/* else cavas must not be showing, so... */
printf("		canvas.style.display=\"block\";");			/* show the canvas layer */
printf("		setTimeout(\"makeRouteFlash()\",850);");		/* wait this long before recursion to hide it again (this is the time it is visible for) */
printf("	}");
printf("}\n");

/* Just making it a bit easier to work with DOM */
printf("function changeElementStateById(elementID,elementAttribute,elementState){");
printf("	switch(elementAttribute){");
printf("		case \"visibility\":");
printf("			document.getElementById(elementID).style.visibility=elementState;");
printf("			break;");
printf("		case \"display\":");
printf("			document.getElementById(elementID).style.display=elementState;");
printf("			break;");
printf("		case \"backgroundColor\":");
printf("			document.getElementById(elementID).style.backgroundColor=elementState;");
printf("			break;");
printf("	}");
printf("}\n");

/* These functions popup and take down help tooltip div layers. */
printf("function popRouteHelp(helpText){");
printf("	var thediv=document.getElementById(\"routingTooltipWrapper\");");
printf("	thediv.innerHTML=helpText;");
printf("	thediv.style.display=\"block\";");
printf("}");
printf("function unpopRouteHelp(){");
printf("	var thediv=document.getElementById(\"routingTooltipWrapper\");");
printf("	thediv.innerHTML=\"\";");
printf("	thediv.style.display=\"none\";");
printf("}\n");
printf("function popRouteHelpMouse(thisElement,helpText){");
printf("	var thediv=document.getElementById(\"routingTooltipWrapperMouse\");");
printf("        var curleft=curtop=0;");
printf("	var tooltipOffsetX=35;");
printf("	var tooltipOffsetY=-30;");
printf("        if(thisElement.offsetParent){");
printf("                do{curleft+=thisElement.offsetLeft; curtop+=thisElement.offsetTop;} while(thisElement=thisElement.offsetParent);");
printf("	}");
printf("	thediv.innerHTML=helpText;");
printf("	var strLeft=parseInt(curleft)+tooltipOffsetX+\"px\";");
printf("	var strTop=parseInt(curtop)+tooltipOffsetY+\"px\";");
printf("	thediv.style.left=strLeft;");
printf("	thediv.style.top=strTop;");
printf("	thediv.style.zIndex=3;");
printf("	thediv.style.display=\"block\";");
printf("}\n");
printf("function unpopRouteHelpMouse(){");
printf("	var thediv=document.getElementById(\"routingTooltipWrapperMouse\");");
printf("	thediv.innerHTML=\"\";");
printf("	thediv.style.display=\"none\";");
printf("	thediv.style.left=\"0px\";");
printf("	thediv.style.top=\"0px\";");
printf("}\n");

/* Gets called by a standalone script whenever an LCD icon img is loaded - sole purpose is to register it into an array. */
printf("function registerLCDiconImg(cellid,deviceid){");
printf("	lcdIconImgRegistry[iconRegistryArrayLength]=[];");
printf("	lcdIconImgRegistry[iconRegistryArrayLength][0]=cellid;");
printf("	lcdIconImgRegistry[iconRegistryArrayLength][1]=deviceid;");
printf("	iconRegistryArrayLength++;");
printf("}\n");

/* This function gets called after page is finished loading. Its purpose is to go through the registered-icon array and change the img src of any LCD icon that matches an LCD that has associated routes, so we know which LCD's have routes just by looking at them */
printf("function reviewLCDiconImgState(){");
printf("	var lcdIconImgRegistryLength=lcdIconImgRegistry.length;");
printf("	if(routesTable==null){");
printf("		var routesTableLength=0;");
printf("	}");
printf("	else{");
printf("		var routesTableLength=routesTable.length;");
printf("	}");
printf("	var intThisIterationActive;");
printf("	for(var i=0;i<lcdIconImgRegistryLength;i++){");							/* for each of the registered lcd icon imgs (as populated into the lcdIconImgRegistry array at page runtime)... */
printf("		for(var ii=0;ii<routesTableLength;ii++){");							/* look through the master routesTable array for a matching deviceid and currently loaded floorplan... */
printf("			if(lcdIconImgRegistry[i][1]==routesTable[ii][rtiDeviceName]){");				/* if registered icon's deviceid matches that of an entry in the routesTable, then there is routing data for the icon, so... */
printf("				if(routesTable[ii][rtiFloorplanName]==\"%s\"){", present_floorplan_name_no_suffix_with_space);	/* next, check if the route entry's floorplan matches whatever is supposed to be currently showing on screen */
printf("					if(document.getElementById(lcdIconImgRegistry[i][0]).src==\"%s/map_mediaport_not_connected.gif\" && parseInt(routesTable[ii][rtiActiveFlag])==1){", cgi_icons);
printf("						document.getElementById(lcdIconImgRegistry[i][0]).src=\"%s/map_mediaport_with_route_not_connected.gif\";", cgi_icons);
printf("					}");
printf("					else if(document.getElementById(lcdIconImgRegistry[i][0]).src==\"%s/map_mediaport_not_connected.gif\" && parseInt(routesTable[ii][rtiActiveFlag])==0){", cgi_icons);
printf("						document.getElementById(lcdIconImgRegistry[i][0]).src=\"%s/map_mediaport_map_off_with_route_not_connected.gif\";", cgi_icons);
printf("					}");
printf("					else if(parseInt(routesTable[ii][rtiActiveFlag])==0){");
printf("						document.getElementById(lcdIconImgRegistry[i][0]).src=\"%s/map_mediaport_map_off_with_route_connected.gif\";", cgi_icons);
printf("					}");
printf("					else{");
printf("						document.getElementById(lcdIconImgRegistry[i][0]).src=\"%s/map_mediaport_with_route_connected.gif\";", cgi_icons);
printf("					}");
printf("				}");
printf("			}");
printf("		}");
printf("	}");
printf("}\n");

/* used by AJAX routines to create iconset based on a particular floorplan */
printf("var postAjaxCreateIcon_strAnchorOnClick;");		/* will need to populate this global strings before calling the function below! */
printf("var postAjaxCreateIcon_strAnchorOnMouseover;");		/* will need to populate this global strings before calling the function below! */			
printf("var postAjaxCreateIcon_strAnchorOnMouseout;");		/* will need to populate this global strings before calling the function below! */
printf("function postAjaxCreateIcon(strSpanID,strSpanTop,strSpanLeft,strCellID,arrAnchorTagContent){");
printf("	var strAnchorTag = \"<a onclick=\\\"\"+postAjaxCreateIcon_strAnchorOnClick+\"\\\" onmouseover=\\\"\"+postAjaxCreateIcon_strAnchorOnMouseover+\"\\\" onmouseout=\\\"\"+postAjaxCreateIcon_strAnchorOnMouseout+\"\\\" href=\\\"javascript: OperationComplete()\\\">\";");
printf("	if(arrAnchorTagContent[0]==\"text\"){");					/* note: look in smcgi_xmlhttp.c in the callback function to see how this array is created */
printf("		var strAnchorTagContent = \"<font face='Arial,Helvetica' size='2'>\"+arrAnchorTagContent[1]+\"</font>\";");
printf("	}");
printf("	else{");
printf("		var strAnchorTagContent = \"<img id=\\\"cell\"+strCellID+\"\\\" src=\\\"%s/\"+arrAnchorTagContent[1]+\"\\\" name=\\\"cell\"+strCellID+\"\\\">\";", cgi_icons);
printf("	}");
printf("	var strScriptTag = \"\";");								
printf("	var spanTag = document.createElement(\"span\");");					/* create a dynamically added html span element and give it a handle called spanTag */
printf("	spanTag.id = \"image\"+strSpanID+\"\";");						/* give an id to the span element */
printf("	spanTag.style.position = \"absolute\";");						/* make the span have an absolute positioning (so we can apply top and left positions) */
printf("	spanTag.style.top = \"\"+strSpanTop+\"\";");						/* position it the specified distance down from the top axis */
printf("	spanTag.style.left = \"\"+strSpanLeft+\"\";");						/* position it the specified distance to the right of the left axis */
printf("	spanTag.style.visibility = \"visible\";");						/* make it visible */
printf("	spanTag.innerHTML = strAnchorTag + strAnchorTagContent + strScriptTag +\"</a>\";");	/* construct what goes inside the span element */
printf("	document.getElementById(\"main_image\").appendChild(spanTag);");			/* add the completed html as a child to the main_image element */
printf("}\n");

/* used by AJAX routines to determine what filename of what graphic to render -- based on an array passed as an argument from smajax.c */
printf("function getIconFilenameFromType(arrIconset_local){");
printf("	var type = parseInt(arrIconset_local[0]);");
printf("	var deviceState = parseInt(arrIconset_local[5]);");
printf("	switch(type){");
printf("		case %d:", FLOORPLAN_NONE);
printf("			return \"map_invisible.gif\";");
printf("		case %d:", FLOORPLAN_SIGN_UP);
printf("			return \"map_sign_up.gif\";");
printf("		case %d:", FLOORPLAN_SIGN_DOWN);
printf("			return \"map_sign_down.gif\";");
printf("		case %d:", FLOORPLAN_SIGN_LEFT);
printf("			return \"map_sign_left.gif\";");
printf("		case %d:", FLOORPLAN_SIGN_RIGHT);
printf("			return \"map_sign_right.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_UP);
printf("			return \"map_speaker_up.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_DOWN);
printf("			return \"map_speaker_down.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_LEFT);
printf("			return \"map_speaker_left.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_RIGHT);
printf("			return \"map_speaker_right.gif\";");
printf("		case %d:", FLOORPLAN_LINK_UP);
printf("			return \"map_link_up.gif\";");
printf("		case %d:", FLOORPLAN_LINK_DOWN);
printf("			return \"map_link_down.gif\";");
printf("		case %d:", FLOORPLAN_LINK_LEFT);
printf("			return \"map_link_left.gif\";");
printf("		case %d:", FLOORPLAN_LINK_RIGHT);
printf("			return \"map_link_right.gif\";");
printf("		case %d:", FLOORPLAN_PC_ALERT);
printf("			return \"map_pc_alert_connected.gif\";");
printf("		case %d:", FLOORPLAN_PHONE);
printf("			return \"map_phone.gif\";");
printf("		case %d:", FLOORPLAN_MESSAGE);
printf("			return \"map_msg.gif\";");
printf("		case %d:", FLOORPLAN_PERSON);
printf("			return \"map_person.gif\";");
printf("		case %d:", FLOORPLAN_BUTTON);
printf("			return \"map_button_blink_inactive.gif\";");
printf("		case %d:", FLOORPLAN_ZOOM_IN);
printf("			return \"map_zoom_in.gif\";");
printf("		case %d:", FLOORPLAN_ZOOM_OUT);
printf("			return \"map_zoom_out.gif\";");
printf("		case %d:", FLOORPLAN_TEXT);
printf("			return \"text\";");
printf("		case %d:", FLOORPLAN_SPEAKER_GROUP);
printf("			return \"map_speaker_group.gif\";");
printf("		case %d:", FLOORPLAN_TEXT_RED);
printf("			return \"map_letter_t_red.gif\";");
printf("		case %d:", FLOORPLAN_TEXT_BLUE);
printf("			return \"map_letter_t_blue.gif\";");
printf("		case %d:", FLOORPLAN_TEXT_YELLOW);
printf("			return \"map_letter_t_yellow.gif\";");
printf("		case %d:", FLOORPLAN_TEXT_GREEN);
printf("			return \"map_letter_t_green.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_GROUP_UP);
printf("			return \"map_speaker_group_up.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_ALL_CALL);
printf("			return \"map_speaker_all_call.gif\";");
printf("		case %d:", FLOORPLAN_SPEAKER_ALL_CALL_EMERGENCY);
printf("			return \"map_speaker_all_call_emergency.gif\";");
printf("		case %d:", FLOORPLAN_CAMERA);
printf("			return \"map_video_camera.gif\";");
printf("		case %d:", FLOORPLAN_PULL_STATION);
printf("			if(deviceState == %d){return \"map_pull_station_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_pull_station_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_pull_station_idle.gif\";}");
printf("		case %d:", FLOORPLAN_WATER_FLOW);
printf("			if(deviceState == %d){return \"map_water_flow_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_water_flow_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_water_flow_idle.gif\";}");
printf("		case %d:", FLOORPLAN_SMOKE_DETECTOR);
printf("			if(deviceState == %d){return \"map_smoke_detector_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_smoke_detector_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_smoke_detector_idle.gif\";}");
printf("		case %d:", FLOORPLAN_MOTION_DETECTOR);
printf("			if(deviceState == %d){return \"map_motion_detector_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_motion_detector_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_motion_detector_idle.gif\";}");
printf("		case %d:", FLOORPLAN_SOUND_METER);
printf("			if(deviceState == %d){return \"map_sound_meter_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_sound_meter_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_sound_meter_idle.gif\";}");
printf("		case %d:", FLOORPLAN_DOOR_OPEN);
printf("			if(deviceState == %d){return \"map_door_open_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_door_open_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_door_open_idle.gif\";}");
printf("		case %d:", FLOORPLAN_FIRE_EXTINGUISHER);
printf("			return \"map_fire_extinguisher.gif\";");
printf("		case %d:", FLOORPLAN_MEDIAPORT);
printf("			return \"map_mediaport_connected.gif\";");
printf("		case %d:", FLOORPLAN_DUCT_DETECTOR);
printf("			if(deviceState == %d){return \"map_duct_detector_alarming.gif\";}", DEVICE_STATE_ALARM);
printf("			else if(deviceState == %d){return \"map_duct_detector_trouble.gif\";}", DEVICE_STATE_TROUBLE);
printf("			else{return \"map_duct_detector_idle.gif\";}");
printf("		default:");
printf("			return false;");
printf("	}");
printf("}\n");

/* CR END */

printf("</SCRIPT>");

printf("<CENTER>");

if(smbanner_locations)
	{
	if(notjustspace(locations_text, sizeof(locations_text)))
		{
		printf("<div align=\"center\" id=\"fpGAText\">");

		/* show any message from smbanner for the locations screen */
		printf("%s", locations_text);
		printf("<IMG SRC=\"%s/map_spacer.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
		printf("</div>");
		}

	printf("<div style=\"display: none; visibility: hidden;\" align=\"center\" id=\"fpEditToolbar\">"); /* div tag id added to isolate the original (pre-routing development) toolbar so it can be hidden temporarily in lieu of showing the route toolbar */
	}
else
	{
	printf("<div align=\"center\" id=\"fpEditToolbar\">"); /* div tag id added to isolate the original (pre-routing development) toolbar so it can be hidden temporarily in lieu of showing the route toolbar */
	}

printf("<IMG SRC=\"%s/map_spacer.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);

if(smbanner_locations)
	{
	}
else if(fp_edit_mode)
	{
	if(DistributionFindListName(db_list->dli_who_can_modify)
		&& (DistributionPersonInListName(CurrentUserPin, db_list->dli_who_can_modify) == DL_EXCLUDE))
		{
		/* no modify access to this floor plan */
		}
	else
		{
		/* not display this if in route-mode-available mode */
		if((db_sysp->dsy_signs_entered > 0  || db_sysp->dsy_acu_and_ccu_entered > 0)
			&& fp_route_need_auto_submit_to_enter_edit_mode == 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new sign.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_signs').style.visibility='visible'; return false;} \">");
			printf("<IMG SRC=\"%s/map_sign_up.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}
	
		if(db_sysp->dsy_mediaports_entered > 0
			|| db_sysp->dsy_acu_and_ccu_entered
			|| db_sysp->dsy_mediaports_graphical_annunciator_devices_entered > 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new MediaPort.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_mediaport_connected.gif'); return false;} \">", FLOORPLAN_MEDIAPORT, cgi_icons);
			printf("<IMG SRC=\"%s/map_mediaport_connected.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}
	
		/* not display this if in route-mode-available mode */
		if(db_sysp->dsy_pc_alerts_entered > 0 
			&& fp_route_need_auto_submit_to_enter_edit_mode == 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new PC Alert.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_pc_alert_connected.gif'); return false;} \">", FLOORPLAN_PC_ALERT, cgi_icons);
			printf("<IMG SRC=\"%s/map_pc_alert_connected.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}
	
		/* not display this if in route-mode-available mode */
		if(fp_route_need_auto_submit_to_enter_edit_mode == 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='visible'; return false;} \">");
			printf("<IMG SRC=\"%s/map_speaker_right.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
		
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new phone.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_phone.gif'); return false;} \">", FLOORPLAN_PHONE, cgi_icons);
			printf("<IMG SRC=\"%s/map_phone.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
		
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new message.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_msg.gif'); return false;} \">", FLOORPLAN_MESSAGE, cgi_icons);
			printf("<IMG SRC=\"%s/map_msg.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
		
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place text.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_texts').style.visibility='visible'; return false;} \">");
			printf("<IMG SRC=\"%s/map_letter_t_blue.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
		
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a person.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_person.gif'); return false;} \">", FLOORPLAN_PERSON, cgi_icons);
			printf("<IMG SRC=\"%s/map_person.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
		
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a camera.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_video_camera.gif'); return false;} \">", FLOORPLAN_CAMERA, cgi_icons);
			printf("<IMG SRC=\"%s/map_video_camera.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}
		/* CR END */
	
		/* CR ADDED - modified to not display this if in route-mode-available mode */
		//if(db_sysp->dsy_wireless_buttons_entered > 0)
		if(db_sysp->dsy_wireless_buttons_entered > 0 
			&& fp_route_need_auto_submit_to_enter_edit_mode == 0)
		/* CR END */
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a wireless button.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_button_blink_inactive.gif'); return false;} \">", FLOORPLAN_BUTTON, cgi_icons);
			printf("<IMG SRC=\"%s/map_button_blink_inactive.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}

		/* CR ADDED - modified to not display this if in route-mode-available mode */
		if(fp_route_need_auto_submit_to_enter_edit_mode == 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new up link.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='visible'; return false;} \">");
			printf("<IMG SRC=\"%s/map_zoom_in_blinking.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);
			}
		/* CR END */
	
		/* if no devices entered then dont show the icons */
		if(db_sysp->dsy_mediaports_graphical_annunciator_devices_entered > 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a pull station.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_pull_station_idle.gif'); return false;} \">", FLOORPLAN_PULL_STATION, cgi_icons);
			printf("<IMG SRC=\"%s/map_pull_station_idle.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);
	
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a water flow valve.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_water_flow_idle.gif'); return false;} \">", FLOORPLAN_WATER_FLOW, cgi_icons);
			printf("<IMG SRC=\"%s/map_water_flow_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);

			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a smoke detector.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_smoke_detector_idle.gif'); return false;} \">", FLOORPLAN_SMOKE_DETECTOR, cgi_icons);
			printf("<IMG SRC=\"%s/map_smoke_detector_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);

			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a motion detector.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_motion_detector_idle.gif'); return false;} \">", FLOORPLAN_MOTION_DETECTOR, cgi_icons);
			printf("<IMG SRC=\"%s/map_motion_detector_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);

			if(db_sysp->dsy_sound_meters_entered)
				{
				printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a sound meter.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_sound_meter_idle.gif'); return false;} \">", FLOORPLAN_SOUND_METER, cgi_icons);
				printf("<IMG SRC=\"%s/map_sound_meter_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
				printf("</A>");
				html_space(1);
				}

			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a duct detector.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_duct_detector_alarming.gif'); return false;} \">", FLOORPLAN_DUCT_DETECTOR, cgi_icons);
			printf("<IMG SRC=\"%s/map_duct_detector_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);

			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a door open.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_door_open_alarming.gif'); return false;} \">", FLOORPLAN_DOOR_OPEN, cgi_icons);
			printf("<IMG SRC=\"%s/map_door_open_alarming.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			html_space(1);
			}

		/* CR ADDED - modified to not display this if in route-mode-available mode */
		if(fp_route_need_auto_submit_to_enter_edit_mode == 0)
			{
			printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a fire extinguisher.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {NewImageSelected(%d, '%s/map_fire_extinguisher.gif'); return false;} \">", FLOORPLAN_FIRE_EXTINGUISHER, cgi_icons);
			printf("<IMG SRC=\"%s/map_fire_extinguisher_static.gif\" hspace=%d  align=\"%s\">", cgi_icons, hspace, alignment);
			printf("</A>");
			}
		/* CR END */
		html_space(1);

		cgi_button_img("map_edit.gif", "SetEditItemMode(1);", "SetHelpText(0, 0, 'To edit the information for an icon on the map, click here first then click on the icon to edit.'); return true;", "CancelHelpText();", alignment);
		html_space(1);

		cgi_button_img("map_delete.gif", "SetDeleteMode(1);", "SetHelpText(0, 0, 'To delete icons on the map, click here first then click on the icon to delete.'); return true;", "CancelHelpText();", alignment);
		html_space(1);
		}
	
	cgi_button_img("map_done.gif", "SetEditMode(0); document.forms[0].submit(); return false;", "SetHelpText(0, 0, 'Click here when you are done editing the map.'); return true;", "CancelHelpText();", alignment);
	html_space(1);

//	html_space(5);
       	if(db_staff_service_class_valid(CurrentUserSecurClass) == SECURITY_MGR)
		{

		/* If there is a floorplan actually loaded AND we need route mode to be available, let's provide the 'ROUTE MGR' or 'SHOW MAPS' button in addition to the other usual controls. */
		if(floor_plan_header_recno > 0 
			&& assumed_need_route_mode == 1)
			{

			/* for these types of messages, show the routes manager button */
			if(atoi(msg_alert_status) == BB_ALERT_STATUS_EMERGENCY
			 || atoi(msg_alert_status) == BB_ALERT_STATUS_SHELTER 
			 || atoi(msg_alert_status) == BB_ALERT_STATUS_EVACUATE)
				{
				char buffer[35+MSG_NAME_LENGTH+DIRECTORY_LENGTH];  /* 35 is equal to the number of characters in the JavaScript syntax in the next line */

				sprintf(buffer, "SetRouteMode(1, '%s', '%s');", route_msg_name, route_msg_directory);

				html_space(2);

				//cgi_button_img("routeMgrBtn.gif", buffer, "SetHelpText(0, 0, 'Click here to work with routes for this message.'); return true;", "CancelHelpText();", alignment);
				cgi_button_img("routeMgrBtn.gif", buffer, "popRouteHelpMouse(this,'Display route manager screen');setTimeout('unpopRouteHelpMouse()',1500);", "CancelHelpText();", alignment);

				html_space(3);
				}

			/* for this type of message, show the show maps button */
			else if (atoi(msg_alert_status) == BB_ALERT_STATUS_GRAPHICAL_ANNUNCIATOR)
				{
				char buffer[37+MSG_NAME_LENGTH+DIRECTORY_LENGTH];  /* 37 is equal to the number of characters in the JavaScript syntax in the next line */

				sprintf(buffer, "SetLsdMode(true, '%s', '%s');", route_msg_name, route_msg_directory);
				cgi_button_img("deviceMaps.gif", buffer, "popRouteHelpMouse(this,'Choose active map for a safety device');setTimeout('unpopRouteHelpMouse()',1500);", "CancelHelpText();", alignment);

				html_space(2);

				/* allow the user to specify whether the currently visible map is the default for this message, should a GA type message be manually launched (or some method other than an alarming device launching it) */
				printf("<label style=\"border:0px solid gray; font-size:11px; padding:1px; *position:relative; *top:10px;\">GA Default Map:&nbsp;");
				BannerOptions(me_msg_template_recno, DB_ISAM_READ);			/* get default-flag data for this message */
				if(strcmp(db_bann_message_options->mo_ga_default_locations, cgi_form_states.main_floor_plan_name) == 0)	/* if the record matches the currently loaded map, then check the box to indicate that this map is the default */
					{
					//printf("<input type=\"checkbox\" checked=\"checked\" id=\"locations_ga_default_floorplan\" name=\"locations_ga_default_floorplan\" value=\"%s\" onchange=\"gaDefaultFloorplanCheckbox_onChange()\" align=\"bottom\" style=\"width:13px; height:13px; padding:0; margin:0; vertical-align:middle; position:relative; top:-1px; *overflow:hidden;\">", cgi_form_states.main_floor_plan_name);
					printf("<input type=\"checkbox\" checked=\"checked\" id=\"locations_ga_default_floorplan\" name=\"locations_ga_default_floorplan\" value=\"%s\" onclick=\"gaDefaultFloorplanCheckbox_onChange()\" align=\"bottom\" style=\"width:13px; height:13px; padding:0; margin:0; vertical-align:middle; position:relative; top:-1px; *overflow:hidden;\">", cgi_form_states.main_floor_plan_name);
					}
				else								/* else, don't check the box */
					{
					//printf("<input type=\"checkbox\" id=\"locations_ga_default_floorplan\" name=\"locations_ga_default_floorplan\" value=\"%s\" align=\"bottom\" onchange=\"gaDefaultFloorplanCheckbox_onChange()\" style=\"width:13px; height:13px; padding:0; margin:0; vertical-align:middle; position:relative; top:-1px; *overflow:hidden;\">", cgi_form_states.main_floor_plan_name);
					printf("<input type=\"checkbox\" id=\"locations_ga_default_floorplan\" name=\"locations_ga_default_floorplan\" value=\"%s\" align=\"bottom\" onclick=\"gaDefaultFloorplanCheckbox_onChange()\" style=\"width:13px; height:13px; padding:0; margin:0; vertical-align:middle; position:relative; top:-1px; *overflow:hidden;\">", cgi_form_states.main_floor_plan_name);
					}
				printf("</label>");

				html_space(2);
				}

			}

		/*(Disabled at request of Kevin...)
		cgi_button_img("map_route.gif", "window.alert('NOTE!\\nRouting functionality is not available directly from\\nthis screen (Locations tab).\\n\\nTo work with routes, please follow these steps:\\n 1. Edit an existing message\\n 2. Make sure a Media Type of LCD Location Map\\n     is defined (save if necessary)\\n 3. Click the Edit Map Features button');", "SetHelpText(0, 0, 'Click for instructions.'); return true;", "CancelHelpText();", alignment);
		html_space(3);
		*/

		cgi_button_img("map_new.gif", "if(HasFormBeenLoaded(1)) {CreateNewMap(0);} return false;", "SetHelpText(0, 0, 'Click here to import a new gif picture.'); return true;", "CancelHelpText();", alignment);
		html_space(1);
		if(floor_plan_header_recno > 0)
			{
			cgi_button_img("map_delete_map.gif", "DeleteMap();", "SetHelpText(0, 0, 'Click here to delete this map.'); return true;", "CancelHelpText();", alignment);
			html_space(1);
			}

		}

	/* CR ADDED - modified to not show this button if we are in route-mode-available mode*/
	if(floor_plan_header_recno)
//		&& fp_route_need_auto_submit_to_enter_edit_mode==0)
	/* CR END */
		{
		char buffer[100];

		if(DistributionFindListName(db_list->dli_who_can_modify)
			&& (DistributionPersonInListName(CurrentUserPin, db_list->dli_who_can_modify) == DL_EXCLUDE))
			{
			/* no access */
			}
		else
			{
			sprintf(buffer, "CreateNewMap(" FORMAT_DBRECORD_STR ")", floor_plan_header_recno);
			cgi_button_img("map_controls.gif", buffer, "SetHelpText(0, 0, 'Click here to change the access or control for this map.'); return true;", "CancelHelpText();", alignment);
			html_space(1);
			}
		}

	if(atoi(msg_alert_status) == BB_ALERT_STATUS_EMERGENCY
		|| atoi(msg_alert_status) == BB_ALERT_STATUS_SHELTER 
		|| atoi(msg_alert_status) == BB_ALERT_STATUS_EVACUATE)
		{
		printf("<A href=\"javascript: OperationComplete();\" onclick=\"CreateHelpDirect('routing_manager_help.htm');\" >Help</A>");
		}
	else if(atoi(msg_alert_status) == BB_ALERT_STATUS_GRAPHICAL_ANNUNCIATOR)
		{
		printf("<A href=\"javascript: OperationComplete();\" onclick=\"CreateHelpDirect('graphical_annunciator_help.htm');\" >Help</A>");
		}
	}
else
	{
	char phone_number[100] = "";

	if(db_staff_pin_valid(CurrentUserPin))
		{
		if(VoiceDecodeCallAt(phone_number, sizeof(phone_number), NULL, db_staf->dss_call_at))
			{
			if(SiteConnectionFind(db_staf->dss_site, db_staf->dss_site))
				{
				int slen;
				remove_trailing_white_space(phone_number);
				slen = strlen(phone_number);
	
				lm_st_delstr(phone_number, slen - atoi(db_conn->dco_number_of_digits_to_dial));
				}
			}
		}

	printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Initiate your message.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) { CancelHelpText(); LaunchFloorPlanMessage(); } return false; \">");
	printf("<IMG name='map_go' SRC=\"%s/map_go.gif\" hspace=0  align=\"%s\">", cgi_icons, alignment);
	printf("</A>");
	html_space(1);
	html_space(additional_space);

	printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Cancel your selections.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) { CancelHelpText(); LaunchCancel(); } return false; \">");
	printf("<IMG name='map_cancel' SRC=\"%s/map_cancel.gif\" hspace=0  align=\"%s\">", cgi_icons, alignment);
	printf("</A>");
	html_space(1);
	html_space(additional_space);

	printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Select a message to send.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) { CancelHelpText(); SetCustomMessageMode(); LaunchFloorPlanMessage(); } return false; \">");
	printf("<IMG name='map_message_select' SRC=\"%s/map_message_select.gif\" hspace=0  align=\"%s\">", cgi_icons, alignment);
	printf("</A>");
	html_space(1);
	html_space(additional_space);

	cgi_button_img("map_replay.gif", "MapReplay(); document.forms[0].submit(); return false;", "SetHelpText(0, 0, 'Replay your last active PA message.'); return true;", "CancelHelpText();", alignment);
	html_space(1);
	}

/* CR ADDED */
	printf("</div>");  /* to end the fpEditToolbar div */

/* A DIV element containing all of the route-creation controls.
 * This is always written to the page as hidden, and made visible whenever appropriate. */
	printf("<div id=\"routingTooltipWrapperMouse\" style=\"background-color:#ffff99; position:absolute; left:0px; top:0px; padding:2px; text-align:center; display:none;\"></div>");
	if(smbanner_locations)
		{
		/* dont show for GA */
		printf("<div id=\"mapcanvasHeader\" style=\"display: none; visibility:hidden;\">");
		}
	else
		{
		printf("<div id=\"mapcanvasHeader\" style=\"visibility:hidden; margin-top:-36px; margin-bottom:5px;\">");
		}

	printf("	<div id=\"routingTooltipWrapper\" style=\"background-color:#ffff99; position:absolute; left:43%%; top:5px; padding:2px; text-align:center; display:none; font-weight:bold;\"></div>");
	/*printf("	<div id=\"routingTooltipWrapperMouse\" style=\"background-color:#ffff99; position:absolute; left:0px; top:0px; padding:2px; text-align:center; display:none;\"></div>");*/
	printf("	<div id=\"routewindowlet\">");
	printf("		<div id=\"\" style=\"background-color:%s; padding:2px;\">", tableheader_bgcolor);
	printf("			<span style=\"font-weight:bold;\">Message: </span>");
	printf("			<span>%s</span>", remove_trailing_space(route_msg_name));
	printf("		</div>");
	printf("		<div style=\"background-color:%s; padding-left:3px; padding-right:3px; padding-bottom:3px; padding-top:5px;\">", background_faint_gradient_color2);
	printf("			<span id=\"btn_addRoute\" style=\"margin-right:20px; visibility:hidden;\"><a id=\"btn_addRouteAnchor\" href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Click here then click on an LCD below to add a new route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"SetAddRouteMode();this.blur();return false\"><img id=\"btn_addRouteImage\" src=\"%s/map_route_add.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_showRoute\" style=\"visibility:hidden;\"><a id=\"btn_showRouteAnchor\" href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Click here then click on an LCD below to see its routes')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"SetShowRouteMode();this.blur();return false;\"><img id=\"btn_showRouteImage\" src=\"%s/showRoute.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_moreRoutes\" style=\"visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'View alternate routes')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"clickMoreRoutes();this.blur();return false;\"><img src=\"%s/more.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_maps\" style=\"margin-right:20px; visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'View and change which maps are active for the selected LCD')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"clickMapsButton();this.blur();return false;\"><img src=\"%s/maps_button.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_colorChooser\" style=\"margin-right:1px; visibility:hidden; border:1px solid gray; background-color:%s; vertical-align:top;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Choose a color for the route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"TCP.popup(document.forms[0].elements['routePathColor']);this.blur();\">&nbsp;&nbsp;&nbsp;&nbsp;</a></span>", ROUTE_COLOR_DEFAULT);
	printf("			<a id=\"btn_routeThin\" style=\"visibility:hidden;\" href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this,'Select a thin line for the route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"thickChooser('click','thin','%s');this.blur();return false;\"><img style=\"background-color:%s;\" id=\"btn_routeThinImage\" src=\"%s/routeThin.gif\"></a>", cgi_icons, ROUTE_COLOR_DEFAULT, cgi_icons);
	printf("			<a id=\"btn_routeNormal\" style=\"visibility:hidden;\" href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this,'Select an average line for the route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"thickChooser('click','normal','%s');this.blur();return false;\"><img style=\"background-color:%s; margin-left:-4px; margin-right:-4px;\" id=\"btn_routeNormalImage\" src=\"%s/routeNormal_selected.gif\"></a>", cgi_icons, ROUTE_COLOR_DEFAULT, cgi_icons);
	printf("			<a id=\"btn_routeThick\" style=\"visibility:hidden;\" href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this,'Select a thick line for the route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"thickChooser('click','thick','%s');this.blur();return false;\"><img style=\"background-color:%s;\" id=\"btn_routeThickImage\" src=\"%s/routeThick.gif\"></a>", cgi_icons, ROUTE_COLOR_DEFAULT, cgi_icons);
	printf("			<span id=\"btn_straighten\" style=\"margin-right:20px; visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Turn on/off snap-to horizontal and vertical')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"toggleSnap('%s');this.blur();\"><img id=\"btn_straightenImage\" src=\"%s/straighten0.gif\"></a><input type=\"hidden\" id=\"btn_straightenField\" name=\"btn_straightenField\" value=\"false\"></span>", cgi_icons, cgi_icons);
	printf("			<span id=\"btn_finishRoute\" style=\"margin-right:20px; visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Save this route.')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"finishRoutePreCheck();this.blur();return false;\"><img src=\"%s/saveRoute.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_undoLastRouteClick\" style=\"margin-right:10px; visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Undo the last part of the route')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"undoLastSegment();this.blur();return false;\"><img src=\"%s/undo.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_deleteRoute\" style=\"margin-right:20px; visibility:hidden;\"><a href=\"javascript:OperationComplete()\" onmouseover=\"popRouteHelpMouse(this.parentNode,'Delete this route.')\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"deleteRoute();this.blur();return false;\"><img src=\"%s/map_route_delete.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_routeHelp\"><a href=\"%s/~silentm/routing_manager_help.htm\" target=\"_blank\"><img src=\"%s/help2.gif\"></a></span>", cgi_base, cgi_icons);
	printf("			<span id=\"btn_reloadRoutes\"><a href=\"javascript:OperationComplete()\" onmouseover=\"if(GetSubmissionToEditRoute()||GetSubmissionToAddRoute()){popRouteHelpMouse(this.parentNode,'Refresh without saving');}else{popRouteHelpMouse(this.parentNode,'Refresh');}\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"reloadRoutesPage();this.blur();return false;\"><img src=\"%s/refresh.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_deviceMgr\"><a href=\"javascript:OperationComplete()\" onmouseover=\"if(GetSubmissionToEditRoute()||GetSubmissionToAddRoute()){popRouteHelpMouse(this.parentNode,'Return to device manager screen without saving');}else{popRouteHelpMouse(this.parentNode,'Return to device manager screen');}\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"cancelRoute();this.blur();return false;\"><img src=\"%s/deviceMgrBtn.gif\"></a></span>", cgi_icons);
	printf("			<span id=\"btn_cancelRoute\"><a href=\"javascript:OperationComplete()\" onmouseover=\"if(GetSubmissionToEditRoute()||GetSubmissionToAddRoute()){popRouteHelpMouse(this.parentNode,'Close window without saving');}else{popRouteHelpMouse(this.parentNode,'Close window');}\" onmouseout=\"unpopRouteHelpMouse()\" onclick=\"javascript:top.close()\"><img src=\"%s/cancel_red_x.gif\"></a></span>", cgi_icons);
	printf("		</div>");
	printf("		");
	printf("	</div>");
	printf("</div>");
/* CR END */

printf("</CENTER>");
/* CR ADDED comments to this since spacing is now different in this section
if(cgi_detect_browser() == BROWSER_GECKO)
	{
	printf("&nbsp;<BR>");
	}
*/

db_list_select(6);					/* select with type, name, and width, height (dli_decode_primitive) */
strcpy(db_list->res_id, res_id);
strcpysl(db_list->dli_name, present_floorplan_name_no_suffix, sizeof(db_list->dli_name));
strcpy(db_list->dli_decode_primitive, "");
db_list->dli_type = LIST_FLOORPLAN_MAP;
if(db_list_find() > 0
	&& !strcmp(db_list->res_id, res_id)
	&& !strcmp(db_list->dli_name, present_floorplan_name_no_suffix_with_space))
	{
	next_map = 1;
	}
else
	{
	next_map = 0;
	}

strcpy(compare_width_height, "");

printf("<CENTER>");

if(fp_edit_mode)
	{
	printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
//printf("      width = event.offsetX?(event.offsetX):event.pageX-document.getElementById('background_image_div').offsetLeft;");
//printf("	width -= GetRealPosition(document.getElementById('background_image_div'), 'x');");
//printf("      height = event.offsetY?(event.offsetY):event.pageY-document.getElementById('background_image_div').offsetTop;");
//printf("	height -= GetRealPosition(document.getElementById('background_image_div'), 'y');");

	if(cgi_detect_browser() == BROWSER_EXPLORER)
		{
		printf("function MoveMouseTrackerXY(ev) {");
		printf("	ev = ev || window.event;");
		/* move the graphic */
    		printf("	document.getElementById('mouse_tracker_div').style.left = ev.clientX + document.body.scrollLeft;");
		printf("	document.getElementById('mouse_tracker_div').style.top = ev.clientY + document.body.scrollTop;");
		/* remember the placement */
		printf("	document.forms[0].mouse_last_left.value = ev.clientX + document.body.scrollLeft - GetRealPosition(document.getElementById('background_image_div'), 'x');");
		printf("	document.forms[0].mouse_last_top.value  = ev.clientY + document.body.scrollTop -  GetRealPosition(document.getElementById('background_image_div'), 'y');");
		printf("	}");
		printf("document.onmousemove = MoveMouseTrackerXY;");
		printf("document.onmouseup = StoreImageLocation;");
		}
	else
		{
		printf("function MoveMouseTrackerXY(ev) {");
		/* move the graphic */
	    	printf("	document.getElementById('mouse_tracker_div').style.left = ev.pageX;");
		printf("	document.getElementById('mouse_tracker_div').style.top = ev.pageY;");
		/* remember the placement */
		printf("	document.forms[0].mouse_last_left.value = ev.pageX - GetRealPosition(document.getElementById('background_image_div'), 'x');"); 
		printf("	document.forms[0].mouse_last_top.value  = ev.pageY - GetRealPosition(document.getElementById('background_image_div'), 'y');");
		printf("	}");
		printf("addEventListener('mousemove', MoveMouseTrackerXY, false);");
		printf("addEventListener('mouseup', StoreImageLocation, false);");
		}

	printf("</SCRIPT>");
	}

/* used to track the mouse for new image placement */
printf("<DIV id=\"mouse_tracker_div\" style=\"position: absolute; visibility: hidden; z-index: 2;\">");
printf("<IMG id=\"mouse_tracker_img\"  SRC='%s/map_invisible.gif' style=\"position: relative; z-index: 2; \" >", cgi_icons);
printf("</DIV>");

if(floor_plan_header_recno > 0)
	{
	if(locations_alert_status >= 0)
		{
		/* show floor plan map for graphical annunciator (initially hidden) */
		printf("<SPAN id='main_image' style=\"position: absolute; visibility: visible; display: none;\">");
		}
	else
		{
		/* show floor plan map */
		printf("<SPAN id='main_image' style=\"position: absolute; visibility: visible;\">");
		}
	}
else
	{
	/* no floor plan map to show */
	printf("<SPAN id='main_image' style=\"position: absolute; visibility: hidden;\">");
	}

/* CR ADD -- modified to add extra test for routing or regular locations */
//if(fp_edit_mode)	/* if in edit mode (not for displaying on an actual device) then get floorplan name from banner */
//	{
	mn_snprintf(check_exist, sizeof(check_exist), "/home/silentm/public_html/floor_plans/%s", TimeShareCompanyNameDirectoryGet());
	if(SystemCheckFileExists(check_exist, present_floorplan_name) < 0)
		{
		printf("<FONT face=\"Arial,Helvetica\">GIF '%s' was not found.", present_floorplan_name);
		DIAGNOSTIC_LOG_2("GIF PATH=%s '%s' not found.", check_exist, present_floorplan_name);
		}
	else
		{
		printf("<center><DIV id=\"background_image_div\" style=\"background-image:url('%s/%s'); width:%dpx; height:%dpx;\" >", cgi_locations, present_floorplan_name, backwidth, backheight);
//		printf("<DIV id=\"background_image_div\" style=\"background-image:url('%s/%s');\" >", cgi_locations, present_floorplan_name);
		/* NOTE! Need to add a test condition for whether it's being generated for server or mediaport (don't need/want to draw canvas on mediaport?-- or just use canvas instead of SVG??) */
//		if(strcmp(CurrentUserSecurClass, security_class[SECURITY_KSK]) == 0
//			|| strcmp(CurrentUserSecurClass, "") == 0)
		if(fp_edit_mode == 0)		/* if not in edit mode (presumably to display on a mediaport) */
			{
//			/* printf("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" id=\"mapsvg\" style=\"position:relative; z-index:3; display:none;\"></svg>"); */  /* re-enable when developing future SVG animation ability */
//			printf("<canvas id=\"mapcanvas\" onclick=\"registerClick(event);\" style=\"position:relative; z-index:3; display:none;\"></canvas>");
			if(locations_alert_status == BB_ALERT_STATUS_EMERGENCY
				|| locations_alert_status == BB_ALERT_STATUS_SHELTER
				|| locations_alert_status == BB_ALERT_STATUS_EVACUATE)
				{
				/* printf("<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" id=\"mapsvg\" style=\"position:relative; z-index:3; display:none;\"></svg>"); */  /* re-enable when developing future SVG animation ability */
				printf("<canvas id=\"mapcanvas\" onclick=\"registerClick(event);\" style=\"position:relative; z-index:3; display:none;\"></canvas>");
				printf("<script type=\"text/javascript\">initializeCanvas(\"mapcanvas\");</script>");
				/* DEV-NOTE: if the routes file for this message does not contain a record with this device's name (ID), then nothing will show! -- may need future redundancy to fall-back on at least showing something (perhaps GA?) if that ever happens! */
				}
			else if(locations_alert_status == BB_ALERT_STATUS_GRAPHICAL_ANNUNCIATOR)
			/* DEV-NOTE: may also need to make sure that the device trying to show this is indeed/actually a GA device (do that in xmlhttp and/or ajax file?) */
				{

				/* get the currently alarming/active life-safety-device from the message stream */
				/* DEV-NOTE: What if the message doesn't contain data from stream? (i.e. no device in alarm that is triggering the message) */
				/* 		Then need to somehow get a list of ALL life safety devices and figure out which floorplan it is marked active with */
				char this_msg_active_lsd[DEVICEID_LENGTH];

				DBRECORD list_cur = db_list_getcur();

				LoadLaunchStreamNumberData(msg_name_recno, "#lifesafetyid", this_msg_active_lsd, sizeof(this_msg_active_lsd), NULL);

				db_list_setcur(list_cur);

				if(db_bann_setcur(msg_name_recno) > 0)
					{
					BannerOptions(db_bann->dbb_parent_record, DB_ISAM_READ);			/* initialize the structure so we can use it (or write to it to save the user's checkbox selection after they submit the form by clicking done) */
					}
				AJAX_SetYouAreHereBasedOnIP();
				printf("<script type=\"text/javascript\">");
				printf("var repositionMainImageDivRecursions0 = 0;");
				printf("function repositionMainImageDiv0(intRepositionToX){");
				printf("        if(repositionMainImageDivRecursions0 < 10){");
				printf("                document.getElementById(\"main_image\").style.left = \"\"+intRepositionToX+\"px\";");
				printf("                repositionMainImageDivRecursions0++;");
				printf("                setTimeout(\"repositionMainImageDiv0(\"+intRepositionToX+\")\",200);");                         /* just in case that didn't take effect, let's try again */
				printf("        }");
				printf("}");
				printf("function gaUpdateBackground(floorplanName){");
						/* determine filename */
				printf("	var floorplanFilename = removeTrailingSpace(floorplanName) + \".gif\";");
				//printf("window.alert(floorplanFilename);");
						/* update the style */
				printf("	bgImageDiv2.style.background = \"url('%s/\"+floorplanFilename+\"') no-repeat left top\";", cgi_locations);
						/* after changing the graphic, readjust the dimensions, using an array that should have already been created that contains all possible floorplans and their dimensions */
				printf("	var bgImageWidth, bgImageHeight;");                                                                                     /* declare variables to be used to store the dimensions */
				printf("	var bgDefImageWidth = parseInt(bgImageDiv2.style.width);");
				printf("	var bgDefImageHeight = parseInt(bgImageDiv2.style.height);");
				printf("	var centerx;");                                                                                                         /* declare variable to store our centering benchmark */
				printf("	var centerxDef;");                                                                                                              /* declare variable to store our centering benchmark */
				printf("	for(var iSizes=0; iSizes<fpTable.length; iSizes++){");                                                  /* for each of the floorplan records in the sizes array created at load-time... */
				printf("		if(removeTrailingSpace(fpTable[iSizes][fptiName]) == removeTrailingSpace(floorplanName)){");                              /* if the matching floorplan is found in the sizes array, then... */
				printf("			bgImageWidth = fpTable[iSizes][fptiWidth];");                                                                           /* get the active floorplan graphic's width */
				printf("			bgImageHeight = fpTable[iSizes][fptiHeight];");                                                                 /* get the active floorplan graphic's height */
				printf("			break;");                                                                                                               /* exit the loop since we got what we needed */
				printf("		}");
				printf("	}");
				printf("	bgImageDiv2.style.width = \"\" + bgImageWidth + \"px\";");                                                               /* resize width to match the updated floorplan graphic */
				printf("	bgImageDiv2.style.height = \"\" + bgImageHeight + \"px\";");                                                             /* resize height to match the updated floorplan graphic */
				printf("	centerx = (parseInt(window.innerWidth) - parseInt(bgImageWidth)) / 2;");                                                /* this will give us the margin needed on either side of the floorplan background graphic to center it */
				printf("	centerxDef = (parseInt(window.innerWidth) - parseInt(bgDefImageWidth)) / 2;");                                          /* this will give us the margin needed on either side of the floorplan background graphic to center it */
				printf("	if(centerx > 0){");                                                                                                     /* if the floorplan graphic fits inside the window's width, then... */
				printf("		repositionMainImageDiv0(centerx);");
				printf("	}");
				printf("	else{");                                                                                                                /* else the floorplan graphic is wider than the window, so... */
				printf("		repositionMainImageDiv0(0);");
				printf("	}");
						/* make visible */
				printf("	document.getElementById(\"background_image_div\").style.display = \"block\";");
				printf("	document.getElementById(\"background_image_div\").style.visibility = \"visible\";");
				printf("	document.getElementById(\"main_image\").style.display = \"block\";");
				printf("	document.getElementById(\"main_image\").style.visibility = \"visible\";");
				printf("}");
				printf("function PopulateItemSet_XMLHTTPStateChange(){");
				printf("	if(objXML_itemSet.readyState == 4){");
				printf("		if(objXML_itemSet.status == 200){");
//				printf("			document.write(objXML_itemSet.responseText);");
				printf("			eval(objXML_itemSet.responseText);");
//				printf("			window.alert(arrIconset);");
				printf("			if(arrIconset.length > 0){");                                                           /* if the interpereted response is a valid array containing a set of icon(s) for the floorplan, then continue... */
				printf("                                var strSpanID, strSpanTop, strSpanLeft, strCellID;");                                   /* initialize variables for passing along as arguments to postAjaxCreateIcon function */
				printf("                                var arrAnchorTagContent = [];");                                                        /* initialize array to contain icon info, for passing along as arguments to postAjaxCreateIcon function */
				printf("                                translation_id = [];");                                                                 /* reset translation table (since it was originally created for the default floorplan/iconset) */
				printf("                                translation_deviceid = [];");                                                           /* reset translation table (since it was originally created for the default floorplan/iconset) */
				printf("                                for(var i=0; i<arrIconset.length; i++){");                                              /* for each of the icons returned from the AJAX (for this particular active /floorplan) */
				                                                /* rebuild translation table arrays with the new data */
				printf("                                        translation_id[i] = \"cell\"+arrIconset[i][7];");                                       /* add in a new arbitrary cell id based on record number */
				printf("                                        translation_deviceid[i] = arrIconset[i][3];");                                          /* associate (by using same iteration number) the device id with the cell id */
				                                                /* set arguments that will be passed to the function */
				printf("                                        strSpanID = \"\"+arrIconset[i][7]+\"\";");                                                             /* specify a span ID based on the iteration number (similar to cell ID) */
				printf("                                        strSpanTop = \"\"+arrIconset[i][2]+\"\";");                                             /* specify the top position based on what was returned via AJAX */
				printf("                                        strSpanLeft = \"\"+arrIconset[i][1]+\"\";");                                            /* specify the left position based on what was returned via AJAX */
				printf("                                        strCellID = \"\"+arrIconset[i][7]+\"\";");                      
				                                                /* special case: for text don't need to show an image but rather a string of text */
				printf("                                        if(arrIconset[i][0]==%d){", FLOORPLAN_TEXT);
				printf("                                                arrAnchorTagContent[0] = \"text\";");                                                   /* specify text type for postAjaxCreateIcon argument */
				printf("                                                arrAnchorTagContent[1] = arrIconset[i][4];");                                           /* specify the string of text for postAjaxCreateIcon argument */
				printf("                                        }");
				                                                /* all other cases, show some other icon (ascertained by getIconFilenameFromType function, since can't use cgi_show_locations_item client side) */
				printf("                                        else{");
				printf("                                                arrAnchorTagContent[0] = \"image\";");                                                  /* specify image type for postAjaxCreateIcon argument */
				printf("                                                arrAnchorTagContent[1] = getIconFilenameFromType(arrIconset[i]);");                     /* specify image filename for postAjaxCreateIcon argument (function located in smcgi_locations to return filename based on type) */
				printf("                                        }");
				                                                /* set js globals that are needed by the function called after them... since displaying on mediaport, there's currently no need to make them interactive, so empty */
				printf("                                        postAjaxCreateIcon_strAnchorOnClick = \"\";");
				printf("                                        postAjaxCreateIcon_strAnchorOnMouseOver = \"\";");
				printf("                                        postAjaxCreateIcon_strAnchorOnMouseOut = \"\";");
				                                                /* call the function to actually create the icon on the screen */
				printf("                                        postAjaxCreateIcon(strSpanID,strSpanTop,strSpanLeft,strCellID,arrAnchorTagContent);");
				printf("				}");
				printf("			}");
				printf("		}");
				printf("	}");
				printf("}");
				printf("function PopulateItemSet_XMLHTTPOpen(strFloorplanName){");
				printf("	objXML_itemSet = new XMLHttpRequest();");
				printf("        if(objXML_itemSet){");
				printf("                objXML_itemSet.onreadystatechange=PopulateItemSet_XMLHTTPStateChange;");                     // specify the callback function
				printf("                objXML_itemSet.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());                 // must be relative to same server or get errors
				printf("                objXML_itemSet.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
				printf("                objXML_itemSet.send(\"loadSetIconsetBasedOnFloorplan=1&specifiedFloorplan=\"+strFloorplanName+\"\");");                                  // send the floorplan name to the ajax processor on the server
				printf("	}");
				printf("}");
				printf("</script>");

				/* if we were NOT able to get something from the message stream (supposedly meaning the message is NOT being launched by an alarming device), then...
				   get the ga-default map from the message options file, show the map background, and then populate its devices-iconset on top of it. */
				if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadFloorPlan(): Testing value for this_msg_active_lsd (this_msg_active_lsd = '%s')", this_msg_active_lsd);
				if(strcmp(this_msg_active_lsd, "") == 0)
					{
					if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadFloorPlan(): Message stream data does not seem to contain an active life safety device (this_msg_active_lsd = '%s')", this_msg_active_lsd);
					printf("<script type=\"text/javascript\">");
					printf("	var bgImageDiv2 = document.getElementById(\"background_image_div\");");
					printf("	gaUpdateBackground(\"%s\");", db_bann_message_options->mo_ga_default_locations);
					printf("	var objXML_itemSet = false;");
					printf("	PopulateItemSet_XMLHTTPOpen(\"%s\");", db_bann_message_options->mo_ga_default_locations);
					printf("</script>");
					}

				/* else we just need to load the message's default floorplan graphic (which should be in db_bann_message_options->mo_ga_default_locations, available via BannerOptions() ) */
				else
					{
					if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("smcgi_locations.c: loadFloorPlan(): Deviceid obtained from device in alarm = '%s'", this_msg_active_lsd);
				/* (Explanation: It's possible that the device data retrieved below won't have an active floorplan specified. In that case, fall back to the GA Default Map) */
					AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID(this_msg_active_lsd);						// load the javascript code containing the necessary AJAX functions that will be used next
					printf("<script type=\"text/javascript\">");
					printf("	var bgImageDiv2 = document.getElementById(\"background_image_div\");");
					printf("	var objXML_itemSet = false;");
					printf("	var strBackgroundImage = \"\";");
					printf("	function doFallback(){"); /* recursive function */
					printf("		var local_strBackgroundImage = \"\";");
					printf("		local_strBackgroundImage += document.getElementById(\"background_image_div\").style.backgroundImage;");	/* get the background image url value as a string */
					printf("		if(local_strBackgroundImage == \"\" || local_strBackgroundImage.indexOf(\"undefined.gif\") != -1 || bgImageDiv2.style.visibility != \"visible\"){");	/* if this string (or no string) is in the dom object, then we need to fallback */
									/* if flags indicate previous ajax is completed (see smcgi_xmlhttp), go ahead and try to update the background... for good measure, wait a bit and try again, just in case it didn't take */
					printf("			if(bool_GetLifeSafetyDeviceDataBasedOnDeviceID_mainIsComplete && bool_GetLifeSafetyDeviceDataBasedOnDeviceID_repositionIsComplete){");
					printf("				gaUpdateBackground(\"%s\");", db_bann_message_options->mo_ga_default_locations);
					printf("				PopulateItemSet_XMLHTTPOpen(\"%s\");", db_bann_message_options->mo_ga_default_locations);
					printf("				setTimeout('doFallback()', 500);");
					printf("			}");
									/* else wait a little bit and try again (keeps trying until flags above are ok) */
					printf("			else{");
					printf("				setTimeout('doFallback()', 500);");
					printf("			}");
					printf("		}");
								/* else exit recursion */
					printf("	}");
				/* first, call AJAX to get data about the device that is in active alarm, etc. (ideally getting floorplan name and active flag, but possible that won't be there) */
					printf("	GetLifeSafetyDeviceDataBasedOnDeviceID_XMLHTTPOpen();");					// initiate the AJAX in an attempt to get the necessary floorplan to show (should also show it, if found)
				/* then detect if no floorplan info found (or not properly set/shown), fall back to GA Default Map, but only once previous AJAX has completed (to prevent race) */
					printf("	strBackgroundImage += document.getElementById(\"background_image_div\").style.backgroundImage;");	/* get the background image url value as a string */
					printf("	if(strBackgroundImage==\"\" || strBackgroundImage.indexOf(\"undefined.gif\")!=-1 || bgImageDiv2.style.visibility!=\"visible\"){");		/* if this string (or no string) is in the dom object, then we need to fallback */
					printf("		doFallback();");	/* call recursive function */
					printf("	}");
					/* DEV-NOTE:  DO WE NEED AN ELSE STATEMENT HERE THAT WILL TAKE CARE OF gaUpdateBackground and PopulateItemSet_XMLHTTPOpen ??? */
					/* 8/28/12: tried and didn't seem to make any apparent difference in anything */
					//printf("	else {");
					//printf("		gaUpdateBackground();"); /* may not need to do this, consider using it if the map image isn't correctly showing on the GA */
					//printf("		PopulateItemSet_XMLHTTPOpen(\"%s\");", db_bann_message_options->mo_ga_default_locations);
					//printf("	}");

					printf("</script>");
					}
	

				}
			}
		else				/* else in interactive edit mode (presumably in someone's browser) */
			{
			printf("<canvas id=\"mapcanvas\" onclick=\"registerClick(event);\" style=\"position:relative; z-index:3; display:none;\"></canvas>");
			printf("<script type=\"text/javascript\">initializeCanvas(\"mapcanvas\");</script>");
			}
		printf("</DIV></center>");
		}
//	}
//else			/* else the page is being generated for display on a device at message launch, so get the floorplan name from the active floorplan (from data in the routesTable array) */
//if(fp_edit_mode == 0)
//	{
//	printf("<script type=\"text/javascript\">");
//	printf("	var arrTempFloorplans = extractFloorplansFromArray( extractAllRouteInfoForSpecificLCD(\"%s\") );", db_hard->dhc_deviceid);
//	printf("	for(var i; i<arrTempFloorplans.length; i++){");
//	printf("		if( isFloorplanActive(arrTempFloorplans[i]) ){");
//	printf("			");
//	printf("		}");
//	printf("	}");
//	printf("</script>");
//	}
/* CR END */

if(floor_plan_header_recno > 0)
	{
	char mediaport_translation[FILENAME_LENGTH];
	char visible_list[FILENAME_LENGTH];

	FILE *visible_fp = NULL;
	FILE *translation_fp = NULL;

	/* see if we need to build the emergency translation table to show "you are here" icons */
	mn_snprintf(mediaport_translation, sizeof(mediaport_translation), "/tmp/mediaport.%d.routes", mn_getpid());
	translation_fp = fopen(mediaport_translation, "w+");

	mn_snprintf(visible_list, sizeof(visible_list), "/tmp/mediaport_visible.%d.routes", mn_getpid());
	visible_fp = fopen(visible_list, "w+");

	while(next_map)
		{
		int top;
		int left;
	
		char *ptr;
	
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
	
		if(smbanner_locations)
			{
			/* DONT load initital icons - let AJAX load them based on IP */
			}	
		else
			{
			cgi_show_locations_item(fp_edit_mode, db_list_getcur(), width, height, left, top, translation_fp, visible_fp, locations_alert_status, smbanner_locations);
			}
	
		if(db_list_next() > 0
			&& !strcmp(db_list->res_id, res_id)
			&& !strcmp(db_list->dli_name, present_floorplan_name_no_suffix_with_space))
			{
			next_map = 1;
			}
		else
			{
			next_map = 0;
			}
		}

	if(visible_fp)
		{
		fclose(visible_fp);
		}

	if(translation_fp)
		{
		int ga_index = 0;
		int mediaport_fd;
		int read_count;

		char read_buffer[10000];

		fclose(translation_fp);

		/* add the translation to the form */
		printf("\n");
		printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >\n");
		printf("	var translation_id = [];\n");			/* array to hold the id names for mediaports */
		printf("	var translation_deviceid = [];\n");		/* array to hold the deviceid names for mediaports */

		mediaport_fd = open(mediaport_translation, O_RDONLY);
		if(mediaport_fd >= 0)
			{
			do
				{
				read_count = read(mediaport_fd, read_buffer, sizeof(read_buffer));
				if(read_count > 0)
					{
					read_buffer[read_count] = 0;
					puts(read_buffer);
					}
				}
			while(read_count > 0);
			}
		close(mediaport_fd);

		printf("	var mediaport_ga_deviceid = [];\n");
		db_hard_select(5);
		strcpy(db_hard->res_id, res_id);
		strcpy(db_hard->dhc_device_type, hc_valid_device[DEVICE_MEDIAPORT_LCD_GA].dv_name);
		strcpy(db_hard->dhc_node_name, "");
		strcpy(db_hard->dhc_comm_port, "");
		next = db_hard_find();
		while(next > 0
			&& !strcmp(db_hard->res_id, res_id)
			&& !strcmp(db_hard->dhc_device_type, hc_valid_device[DEVICE_MEDIAPORT_LCD_GA].dv_name))
			{
			printf(" mediaport_ga_deviceid[%d]=\"%s\";\n", ga_index, db_hard->dhc_deviceid);
			ga_index++;
			next = db_hard_next();
			}

		printf("\n");

		printf("	var visible_list = [];\n");			/* array to make visible for graphical annunciators */
		mediaport_fd = open(visible_list, O_RDONLY);
		if(mediaport_fd >= 0)
			{
			do
				{
				read_count = read(mediaport_fd, read_buffer, sizeof(read_buffer));
				if(read_count > 0)
					{
					read_buffer[read_count] = 0;
					puts(read_buffer);
					}
				}
			while(read_count > 0);

			printf(" visible_list[%d]=\"main_image\";\n", visible_list_index);				/* add the background image as a showable object */
			}
		close(mediaport_fd);
		
		printf("function ShowGraphicalAnnunciatorIcons() {");
		printf("	for(var i = 0; visible_list[i]; i++)");
		printf("		{");
		printf("		document.getElementById(visible_list[i]).style.display='block';");		/* values are none and block */
		printf("		}");
		printf("	}");

		printf("</SCRIPT>");

		unlink(mediaport_translation);
		unlink(visible_list);

		/* use an AJAX command to connect back to the server, which will grab the IP address of the mediaport, lookup what device id this unit is and return it */
		/* then we can set mediaport cell images to "you are here" */
		/* AJAX_SetYouAreHereBasedOnIP was updated with routing to also call other AJAX-related routines (smcgi_xmlhttp.c and smajax.c) - so this one call now does more than originally intended! */
		if(fp_edit_mode == 0 					/* only call AJAX if the screen is not being loaded from a user's browser */
			&& assumed_need_route_mode == 0)		/* also be sure to load it only once (since on client there is a double submission) */
			{
			AJAX_SetYouAreHereBasedOnIP();
			if(locations_alert_status != BB_ALERT_STATUS_GRAPHICAL_ANNUNCIATOR)
				{
				printf("<script type=\"text/javascript\">SetYouAreHere_XMLHTTPOpen();</script>"); /* call the function that was just loaded with the line above this */
				}
			}
		}
	}

if(fp_edit_mode)
	{
	int top = 0;
	int left = 0;
	int multi_hspace = 5;

	#ifdef NOT
	if(1)
		{
		printf("<SPAN ID='fine_adjust' style='position: absolute; visibility: hidden; left: %d; top: %d;' onclick=\"return false;\">", left, top);
		printf("<TABLE border=1 cellspacing=0 cellpadding=0 bgcolor=\"%s\"><TR><TD>", main_bgcolor);
		html_space(1);

		printf("<TABLE><TR>");
		printf("<TD><FONT face=\"Arial,Helvetica\" size=\"2\">Up</FONT></TD>");
		printf("<TD><FONT face=\"Arial,Helvetica\" size=\"2\">Down</FONT></TD>");
		printf("<TD><FONT face=\"Arial,Helvetica\" size=\"2\">Left</FONT></TD>");
		printf("<TD><FONT face=\"Arial,Helvetica\" size=\"2\">Right</FONT></TD>");
		printf("</TR></TABLE>");
		
		html_space(1);
		printf("</TD></TR></TABLE>");
		printf("</SPAN>");
		}
	#endif

	if(1)
		{
		/* put multi signs in their own span */
		printf("<SPAN ID='multi_signs' style='position: absolute; visibility: hidden; left: %d; top: %d;' onclick=\"return false;\">", left, top);
		printf("<TABLE border=1 cellspacing=0 cellpadding=0 bgcolor=\"%s\"><TR><TD nowrap>", main_bgcolor);
		html_space(1);

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new sign.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_signs').style.visibility='hidden'; NewImageSelected(%d, '%s/map_sign_up.gif'); return false;} \">", FLOORPLAN_SIGN_UP, cgi_icons);
		printf("<IMG SRC=\"%s/map_sign_up.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
		
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new sign.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_signs').style.visibility='hidden'; NewImageSelected(%d, '%s/map_sign_down.gif'); return false;} \">", FLOORPLAN_SIGN_DOWN, cgi_icons);
		printf("<IMG SRC=\"%s/map_sign_down.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new sign.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_signs').style.visibility='hidden'; NewImageSelected(%d, '%s/map_sign_left.gif'); return false;} \">", FLOORPLAN_SIGN_LEFT, cgi_icons);
		printf("<IMG SRC=\"%s/map_sign_left.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new sign.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_signs').style.visibility='hidden'; NewImageSelected(%d, '%s/map_sign_right.gif'); return false;} \">", FLOORPLAN_SIGN_RIGHT, cgi_icons);
		printf("<IMG SRC=\"%s/map_sign_right.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		html_space(1);
		printf("</TD></TR></TABLE>");
		printf("</SPAN>");
		}

	if(1)
		{
		/* put multi speakers in their own span */
		printf("<SPAN ID='multi_speakers' style='position: absolute; visibility: hidden; left: %d; top: %d;' onclick=\"return false;\">", left, top);
		printf("<TABLE border=1 cellspacing=0 cellpadding=0 bgcolor=\"%s\"><TR><TD nowrap nowrap>", main_bgcolor);
		html_space(1);

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_up.gif'); return false;} \">", FLOORPLAN_SPEAKER_UP, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_up.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_down.gif'); return false;} \">", FLOORPLAN_SPEAKER_DOWN, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_down.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_left.gif'); return false;} \">", FLOORPLAN_SPEAKER_LEFT, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_left.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_right.gif'); return false;} \">", FLOORPLAN_SPEAKER_RIGHT, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_right.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker group.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_group.gif'); return false;} \">", FLOORPLAN_SPEAKER_GROUP, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_group_small.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new speaker group.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_group.gif'); return false;} \">", FLOORPLAN_SPEAKER_GROUP_UP, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_group_up.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place an all call speaker group.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_all_call.gif'); return false;} \">", FLOORPLAN_SPEAKER_ALL_CALL, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_all_call.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place an emergency all call speaker group.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_speakers').style.visibility='hidden'; NewImageSelected(%d, '%s/map_speaker_all_call_emergency.gif'); return false;} \">", FLOORPLAN_SPEAKER_ALL_CALL_EMERGENCY, cgi_icons);
		printf("<IMG SRC=\"%s/map_speaker_all_call_emergency.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		html_space(1);
		printf("</TD></TR></TABLE>");
		printf("</SPAN>");
		}

	if(1)
		{
		/* put multi speakers in their own span */
		printf("<SPAN ID='multi_links' style='position: absolute; visibility: hidden; left: %d; top: %d;' onclick=\"return false;\">", left, top);
		printf("<TABLE border=1 cellspacing=0 cellpadding=0 bgcolor=\"%s\"><TR><TD nowrap>", main_bgcolor);
		html_space(1);

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new up link.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_link_up.gif'); return false;} \">", FLOORPLAN_LINK_UP, cgi_icons);
		printf("<IMG SRC=\"%s/map_link_up.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new down link.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_link_down.gif'); return false;} \">", FLOORPLAN_LINK_DOWN, cgi_icons);
		printf("<IMG SRC=\"%s/map_link_down.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new left link.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_link_left.gif'); return false;} \">", FLOORPLAN_LINK_LEFT, cgi_icons);
		printf("<IMG SRC=\"%s/map_link_left.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new right link.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_link_right.gif'); return false;} \">", FLOORPLAN_LINK_RIGHT, cgi_icons);
		printf("<IMG SRC=\"%s/map_link_right.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new zoom in.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_zoom_in_blinking.gif'); return false;} \">", FLOORPLAN_ZOOM_IN, cgi_icons);
		printf("<IMG SRC=\"%s/map_zoom_in_blinking.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");
	
		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place a new zoom out.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_links').style.visibility='hidden'; NewImageSelected(%d, '%s/map_zoom_out_blinking.gif'); return false;} \">", FLOORPLAN_ZOOM_OUT, cgi_icons);
		printf("<IMG SRC=\"%s/map_zoom_out_blinking.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, multi_hspace);
		printf("</A>");

		html_space(1);
		printf("</TD></TR></TABLE>");
		printf("</SPAN>");
		}

	if(1)
		{
		/* put multi texts in their own span */
		printf("<SPAN ID='multi_texts' style='position: absolute; visibility: hidden; left: %d; top: %d;' onclick=\"return false;\">", left, top);
		printf("<TABLE border=1 cellspacing=0 cellpadding=0 bgcolor=\"%s\"><TR><TD nowrap>", main_bgcolor);
		html_space(1);

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place text.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_texts').style.visibility='hidden'; NewImageSelected(%d, '%s/map_letter_t_blue.gif'); return false;} \">", FLOORPLAN_TEXT_BLUE, cgi_icons);
		printf("<IMG SRC=\"%s/map_letter_t_blue.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place text.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_texts').style.visibility='hidden'; NewImageSelected(%d, '%s/map_letter_t_red.gif'); return false;} \">", FLOORPLAN_TEXT_RED, cgi_icons);
		printf("<IMG SRC=\"%s/map_letter_t_red.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place text.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_texts').style.visibility='hidden'; NewImageSelected(%d, '%s/map_letter_t_green.gif'); return false;} \">", FLOORPLAN_TEXT_GREEN, cgi_icons);
		printf("<IMG SRC=\"%s/map_letter_t_green.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, hspace);
		printf("</A>");

		printf("<A HREF=\"javascript: OperationComplete()\" onMouseOver=\"SetHelpText(0, 0, 'Click to place text.'); return true;\" onMouseOut=\"CancelHelpText();\" onClick=\"if(HasFormBeenLoaded(1)) {document.getElementById('multi_texts').style.visibility='hidden'; NewImageSelected(%d, '%s/map_letter_t_yellow.gif'); return false;} \">", FLOORPLAN_TEXT_YELLOW, cgi_icons);
		printf("<IMG SRC=\"%s/map_letter_t_yellow.gif\" hspace=%d  align=\"ABSBOTTOM\">", cgi_icons, hspace);
		printf("</A>");

		html_space(1);
		printf("</TD></TR></TABLE>");
		printf("</SPAN>");
		}	
	}

printf("</SPAN>");

if(floor_plan_header_recno == 0)
	{
	printf("<FONT color=\"black\">No location to show.</FONT>");
	printf("<script>document.getElementById(\"fpPreTitlePost\").innerHTML=\"\";</script>");
	}

printf("</CENTER>");

cgi_table_one_row_one_cell_end();
cgi_center_screen(1);

printf("<INPUT type=\"hidden\" name=\"loadFloorPlan\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"%s\" value=\"%s\"></INPUT>", cgi_identification_field, cgi_EncodeLoginPin(CurrentUserPin));
printf("<INPUT type=\"hidden\" name=\"map_name\" value=\"%s\"></INPUT>", present_floorplan_name);
printf("<INPUT type=\"hidden\" name=\"change_map\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"fp_move_record\" value=\"0\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"fp_call_mode\" value=\"%d\"></INPUT>", fp_call_mode);
printf("<INPUT type=\"hidden\" name=\"fp_call_mode_redo\" value=\"%d\"></INPUT>", fp_call_mode_redo);
printf("<INPUT type=\"hidden\" name=\"fp_edit_mode\" value=\"%d\"></INPUT>", fp_edit_mode);
printf("<INPUT type=\"hidden\" name=\"fp_edit_item_mode\" value=\"%d\"></INPUT>", 0);
printf("<INPUT type=\"hidden\" name=\"fp_selection_type\" value=\"%d\"></INPUT>", fp_selection_type);
printf("<INPUT type=\"hidden\" name=\"fp_selection_type_redo\" value=\"%d\"></INPUT>", fp_selection_type_redo);
printf("<INPUT type=\"hidden\" name=\"fp_speaker_phone\" value=\"%s\"></INPUT>", fp_speaker_phone);
printf("<INPUT type=\"hidden\" name=\"fp_delete_mode\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"fp_showmaps_mode\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"fp_set_send_message\" value=\"%s\"></INPUT>", floor_plan_record_numbers);
printf("<INPUT type=\"hidden\" name=\"fp_set_send_message_count\" value=\"%d\"></INPUT>", fp_set_send_message_count);
printf("<INPUT type=\"hidden\" name=\"fp_redo_send_message\" value=\"%s\"></INPUT>", floor_plan_record_numbers_for_redo);
printf("<INPUT type=\"hidden\" name=\"floor_plan_header_recno\" value=\"" FORMAT_DBRECORD_STR "\"></INPUT>", floor_plan_header_recno);
printf("<INPUT type=\"hidden\" name=\"delete_recno\" value=\"0\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"set_launch_message\" value=\"\"></INPUT>");
printf("<INPUT type=\"hidden\" name=\"set_launch_directory\" value=\"\"></INPUT>");
/* CR ADDED...
 * Routing-related form fields: */
printf("<input type=\"hidden\" name=\"routeDtsec\" value=\"%s\"></input>", route_dtsec); /* NOTE: could be buggy since we're only obtaining this at document creation-time */
printf("<input type=\"hidden\" name=\"routeUser\" value=\"%s\"></input>", CurrentUserPin);
printf("<INPUT type=\"hidden\" name=\"fp_route_need_auto_submit_to_enter_edit_mode\" value=\"0\"></INPUT>\n");		/* initial flag value of 0 changes to 1 once we programatically click the edit map button so we can avoid an ininite form submission loop */
printf("<INPUT type=\"hidden\" name=\"showRouteMode\" value=\"0\"></INPUT>\n");						/* initial flag value of 0 indicates that the page won't initially be in show-route mode - this changes to 1 once user chooses otherwise, by clicking the show route button*/
printf("<INPUT type=\"hidden\" name=\"addRouteMode\" value=\"0\"></INPUT>\n");						/* initial flag value of 0 indicates that the page won't initially be in add-route mode - this changes to 1 once user chooses otherwise, by clicking the add route button*/
printf("<input type=\"hidden\" name=\"submissionToAddRoute\" value=\"\"></input>\n");					/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("<input type=\"hidden\" name=\"submissionToDeleteRoute\" value=\"\"></input>\n");				/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("<input type=\"hidden\" name=\"submissionToEditRoute\" value=\"\"></input>\n");					/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("<input type=\"hidden\" name=\"msgName\" value=\"%s\"></input>\n", route_msg_name);				/* earlier, we obtained the submitted data from msg editor form, now let's setup cross-state capability by storing that data in a persistent form field */
printf("<input type=\"hidden\" name=\"msgDir\" value=\"%s\"></input>\n", route_msg_directory);				/* earlier, we obtained the submitted data from msg editor form, now let's setup cross-state capability by storing that data in a persistent form field */
printf("<input type=\"hidden\" name=\"routeMediaportOrigin\" value=\"%s\"></input>\n", route_mediaport_origin);		/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeFloorplanName\" value=\"%s\"></input>\n", route_floorplan_name);		/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeActiveFlag\" value=\"%d\"></input>\n", route_active_flag);			/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeThickness\" value=\"%d\"></input>\n", route_thickness);			/* value initially set to whatever happens to be in cgi state - if nothing, then this will be whatever default is set in the parsed integer line above (6 at dev time) */
printf("<input type=\"hidden\" name=\"routePathColor\" value=\"%s\"></input>\n", remove_trailing_white_space(route_path_color));/* value initially set to whatever happens to be in cgi state - if nothing, then this will be whatever default is set in the parsed value line above (#ee0000 at dev time) */
printf("<input type=\"hidden\" name=\"routeCoordsStr\" value=\"%s\"></input>\n", remove_trailing_white_space(route_coordinates_string));/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeMediaportOrigin_old\" value=\"%s\"></input>\n", route_mediaport_origin_old);	/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("<input type=\"hidden\" name=\"routeFloorplanName_old\" value=\"%s\"></input>\n", route_floorplan_name_old);	/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeActiveFlag_old\" value=\"%d\"></input>\n", route_active_flag_old);		/* value initially set to whatever happens to be in cgi state - if nothing, then this will be empty (always on initial page load) - updated via DOM */
printf("<input type=\"hidden\" name=\"routeThickness_old\" value=\"%d\"></input>\n", route_thickness_old);		/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("<input type=\"hidden\" name=\"routePathColor_old\" value=\"%s\"></input>\n", remove_trailing_white_space(route_path_color_old));/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("<input type=\"hidden\" name=\"routeCoordsStr_old\" value=\"%s\"></input>\n", remove_trailing_white_space(route_coordinates_string_old));/* old should only be original route information - we only care about it if doing an edit operation - will be empty if not applicable - once route selected, this will populate via DOM */
printf("<input type=\"hidden\" name=\"autoEnterRouteMode\" value=\"0\"></input>\n");					/* used for determining whether the JS should automatically "enter" route mode, i.e. showing the routing toolbar in lieu of standard tool */
printf("<input type=\"hidden\" name=\"stateChangeMessage\" value=\"%s\"></input>\n", state_change_message);		/* used for passing along the message popped-up whenever some change to the route took place (save, delete, etc.) */
printf("<input type=\"hidden\" name=\"routeLastUserIntention\" value=\"%s\"></input>\n", route_last_user_intention);	/* used for passing along the last action performed, so we can continue with the user's desired action after a form submission */
printf("<input type=\"hidden\" name=\"routeLastSubmitReason\" value=\"%s\"></input>\n", route_last_submit_reason);	/* used for passing along the reason for the last submit, so we can process cross-state things */
printf("<input type=\"hidden\" name=\"routeIndexAtSubmitTime\" value=\"%d\"></input>\n", route_index_at_submit_time);	/* used for passing along the last active index of the route displayed, so a more click can continue on after form submission */
printf("<input type=\"hidden\" name=\"selectedActiveFloorplan\" value=\"\"></input>\n");				/* used for passing along the chosen active floorplan, to avoid multiple floorplans per single device/message */
printf("<input type=\"hidden\" name=\"msgAlertStatus\" value=\"%s\"></input>\n", msg_alert_status);			/* used for passing along the message alert status (evacuation, shelter, graphical annunciator, etc.) */
printf("<input type=\"hidden\" name=\"lsdMode\" value=\"0\"></input>\n");						/* this flag modified by JavaScript before form submission... needs immediate server-side, runtime parsing to be effective */
printf("<input type=\"hidden\" name=\"routeFloorplanName_full\" value=\"%s\"></input>\n", present_floorplan_name_no_suffix_with_space);
printf("<input type=\"hidden\" name=\"activeLsdSelected_originallyActiveFloorplanName\" value=\"\"></input>\n");	/* for storing the currently active device/floorplan -- populated from smcgi_xmlhttp.c */
printf("<input type=\"hidden\" name=\"loadEditSignSpeakerAfterChangeActiveMap\" value=\"0\"></input>\n");		/* used to flag cgi to update the database due to a change active LSD submission */
printf("<input type=\"hidden\" name=\"record_number\" value=\"\"></input>\n");				/* used to pass along the chosen floorplan recno that they want to be active for a given LSD */
printf("<input type=\"hidden\" name=\"me_msg_template_recno\" value=\""FORMAT_DBRECORD_STR"\"></input>\n", me_msg_template_recno);		/* pass along the msg template record number from the message definition screen, through the second submission */
printf("<input type=\"hidden\" name=\"flag_locations_tab_or_msg\" value=\"%d\"></input>\n", flag_locations_tab_or_msg);	/* pass along the msg template record number from the message definition screen, through the second submission */
printf("<input type=\"hidden\" name=\"activeLsdSelected_originallyActiveItem\" value=\"\"></input>\n");			/* may be populated by clickLsd whenever the Modalbox pops to allow them to change what LSD is active... used to detect if a change is to be made or not (contains item record number) */
printf("<input type=\"hidden\" name=\"locations_ga_default_fp_need_to_save\" value=\"\"></input>\n");			/* flag for saving the currently showing map as the default for this message */
printf("<input type=\"hidden\" name=\"locations_ga_default_floorplan\" value=\"%s\"></input>\n", present_floorplan_name_no_suffix_with_space);		/* field for storing floorplan name to make default */

printf("<script type=\"text/javascript\">if(document.forms[0].stateChangeMessage.value!=\"\"){popRouteHelp(document.forms[0].stateChangeMessage.value);setTimeout(\"unpopRouteHelp()\",6000);}</script>\n");
printf("<script type=\"text/javascript\">if(autoEnterRouteMode==1){document.forms[0].autoEnterRouteMode.value=1;}</script>\n");
if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("loadFloorPlan() auto_enter_route_mode = %d", auto_enter_route_mode);

/* if the last form submission flagged us to update a LSD item's active floorplan, then do the updates */
if(load_edit_sign_speaker_after_change_active_map)
	{
	deactivateAllFloorplansInSupportOfLifeSafetyDevice();
	activateItemByRecNo();
	}

/* If last form submission indicated to show the routing toolbar (autoEnterRouteMode was true)... (then we must assume the user is trying to do something within route-mode and should automatically take them back into it, rather than DEVICE MGR mode)
 * So... everything that needs to be done automatically, in post-processing, (like auto-drawing routes) should be within this block - most likely within an appropriate nested test condition for what last happened (state_change_message, etc.) */
printf("<script type=\"text/javascript\">if(document.forms[0].stateChangeMessage.value==\"GA Page Reloaded\"){SetLsdMode(true); clickLsd();}</script>");
if(auto_enter_route_mode == 1)
	{
	/* Most of the following test conditions check flags to determine what it was that the user was trying to do */
	printf("<script type=\"text/javascript\">");
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("loadFloorPlan() auto_enter_route_mode is true... so, directing client to call SetRouteMode(1,'%s','%s')", route_msg_name, route_msg_directory);
	printf("SetRouteMode(1,'%s','%s');", route_msg_name, route_msg_directory);	/* virtually click the ROUTE MGR button to enable the routing toolbar */
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1(" (test condition flag: state_change_message = \"%s\")", state_change_message);
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1(" (test condition flag: route_last_user_intention = \"%s\")", route_last_user_intention);
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1(" (test condition flag: route_last_submit_reason = \"%s\")", route_last_submit_reason);
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1(" (test condition flag: route_mediaport_origin = \"%s\")", remove_trailing_space(route_mediaport_origin));
	/* If the ROUTE MGR page was reloaded by clicking the refresh icon, and no routes were initially loaded or showing on-screen, then we don't need to draw any routes or show formatting buttons */
	if(strcmp(state_change_message, "Page Reloaded") == 0
		&& strcmp(remove_trailing_space(route_mediaport_origin), "") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("  Test Condition Result: Page reloaded without a route on-screen first.");
		}
	else if(strcmp(state_change_message, "New Route Saved") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: New route added... Now drawing that route on %s's screen.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=arrThisLCDsRouteInfo.length-1;");									/* this represents the most recent/latest/last route for the device */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated/new route based on that new data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		}
	/* Else if an existing route was just manually saved, or the refresh icon clicked while a route was on-screen, then draw that route in a way that they'll be able to directly work with it further (i.e. load arrays, show buttons, etc.) */
	else if(strcmp(state_change_message, "Route Saved") == 0
		|| strcmp(state_change_message, "Page Reloaded") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Route saved or reloaded... Now drawing the latest route on %s's screen.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=parseInt(document.forms[0].routeIndexAtSubmitTime.value);");					/* get this locally to prevent repeated DOM interaction */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated/new route based on that new data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		}
	/* Else if an existing route's changes were just automatically saved because they clicked MORE (while those unsaved changes existed), then draw the next route just as a normal MORE click without the save-submission would have done */
	else if(strcmp(state_change_message, "Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "ShowNextRoute") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Existing route auto-saved because %s didn't save before clicking MORE... Now directing client to call SetShowRouteMode(), etc.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=parseInt(document.forms[0].routeIndexAtSubmitTime.value);");					/* get this locally to prevent repeated DOM interaction */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated route based on that data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		}
	/* Else if a new route was just automatically saved because the user clicked MORE (instead of manually saving first), then draw the next route just as a normal MORE click without the save-submission would have done */
	else if(strcmp(state_change_message, "New Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "ShowNextRoute") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Existing route auto-saved because %s didn't save before clicking MORE... Now directing client to call SetShowRouteMode(), etc.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=parseInt(document.forms[0].routeIndexAtSubmitTime.value);");					/* get this locally to prevent repeated DOM interaction */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated route based on that data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		}
	/* Else if a new route was just automatically saved because the user clicked ADD ROUTE, then don't draw anything, and instead just take them directly to add-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "New Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "EnterAddRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: New route auto-saved because %s didn't save before clicking ADD ROUTE...Now directing client to call SetAddRouteMode().", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastUserIntention.value=\"\";");
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");								/* need to clear this flag now, otherwise any subsequent manual-saves will be mistaken in this section as an auto-save, and the route won't display */
		printf("SetAddRouteMode();");
		}
	/* Else if a new route was just automatically saved because the user clicked SHOW ROUTE, then don't draw anything, and instead just take them directly to show-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "New Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "EnterShowRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: New route auto-saved because %s didn't save before clicking SHOW ROUTE... Now directing client to call SetShowRouteMode()", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		printf("SetShowRouteMode();");
		}
	/* Else if an existing route's changes were just automatically saved because the user clicked ADD ROUTE, then don't draw anything, and instead just take them directly to add-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "EnterAddRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Existing route auto-saved because %s didn't save before clicking ADD ROUTE... Now directing client to call SetAddRouteMode()", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		printf("SetAddRouteMode();");
		}
	/* Else if an existing route's changes were just automatically saved because the user clicked SHOW ROUTE, then don't draw anything, and instead just take them directly to show-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "Route Automatically Saved") == 0
		&& strcmp(route_last_user_intention, "EnterShowRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Existing route auto-saved because %s didn't save before clicking SHOW ROUTE... Now directing client to call SetShowRouteMode()", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		printf("SetShowRouteMode();");
		}
	/* Else if an existing route was just deleted because the user clicked DELETE ROUTE, then make sure the MORE button remains, and make it functional for the just-deleted route's LCD's route(s) -- like a shortcut for clicking SHOW and then the LCD, to help user be more efficient */
	else if(strcmp(state_change_message, "Route Deleted") == 0)
		{
		if(route_number_of_total_routes > 0)	/* if there is at least one route still existing after this delete, then... (call appropriate javascript to digest what device to show the route for, and what route to show belonging to that device) */
			{
			if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("  Test Condition Result: Existing route (of many) deleted because %s clicked DELETE ROUTE for \"%s\"... Now directing client to call SetShowRouteMode()", remove_trailing_space(CurrentUserPin), remove_trailing_white_space(route_coordinates_string));
			/* Load new data (from main array that was populated from the re-read file)... */
			printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
			printf("routesDrawn=parseInt(document.forms[0].routeIndexAtSubmitTime.value);");					/* get this locally to prevent repeated DOM interaction */
			printf("if(arrThisLCDsRouteInfo && routesDrawn!=\"\"){");								/* we already did a check at the C-level, but that is not floorplan-specific... so if there are more routes existing for this floorplan, then proceed... */
			printf("	routesDrawn=arrThisLCDsRouteInfo.length-1;");									/* reset this to one less than the array length (to fix out of bounds index bug) */
			printf("	document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
			printf("	document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
			printf("	document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
			printf("	routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
			/* Draw the updated route based on that data... */
			printf("	canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
			printf("	obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
			printf("	SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
			printf("	clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
			printf("	}");
			printf("document.forms[0].routeLastSubmitReason.value=\"\";");
			}
		else					/* else there's no point in doing anything, since no routes now exist, whatsoever */
			{
			if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_2("  Test Condition Result: The only Existing route was deleted because %s clicked DELETE ROUTE for \"%s\"", remove_trailing_space(CurrentUserPin), remove_trailing_white_space(route_coordinates_string));
			}
		}
	/* Else if a new route was not saved, because it has been detected as already existing in the file, and this is all a result of them clicking ADD ROUTE, then don't draw anything, and instead just take them directly to add-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "New Route Already Exists") == 0
		&& strcmp(route_last_user_intention, "EnterAddRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Auto-save new route did NOT occur because it already existed... %s clicked ADD ROUTE...Now directing client to call SetAddRouteMode().", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastUserIntention.value=\"\";");
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");								/* need to clear this flag now, otherwise any subsequent manual-saves will be mistaken in this section as an auto-save, and the route won't display */
		printf("SetAddRouteMode();");
		}
	/* Else if a new route was not saved, because it has been detected as already existing in the file, and this is all a result of them clicking SHOW ROUTE, then don't draw anything, and instead just take them directly to show-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "New Route Already Exists") == 0
		&& strcmp(route_last_user_intention, "EnterShowRouteMode") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Auto-save new route did NOT occur because it already existed... %s clicked SHOW ROUTE... Now directing client to call SetShowRouteMode()", remove_trailing_space(CurrentUserPin));
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		printf("SetShowRouteMode();");
		}
	/* Else if a new route was not saved, because it has been detected as already existing in the file, and this is all a result of them clicking MORE, then don't draw anything, and instead just take them directly to show-route mode (they will need to click an LCD to begin) */
	else if(strcmp(state_change_message, "New Route Already Exists") == 0
		&& strcmp(route_last_user_intention, "ShowNextRoute") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: Auto-save new route did NOT occur because it already existed... %s clicked MORE... Now directing client to call SetShowRouteMode(), etc.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=parseInt(document.forms[0].routeIndexAtSubmitTime.value);");					/* get this locally to prevent repeated DOM interaction */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated route based on that data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		printf("document.forms[0].routeLastSubmitReason.value=\"\";");
		}
	/* Else if a new route was not saved, because it has been detected as already existing in the file, and this is all a result of them clicking SAVE ROUTE, then draw that route in a way they can work with it */
	else if(strcmp(state_change_message, "New Route Already Exists") == 0)
		{
		if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("  Test Condition Result: New route added... Now drawing that route on %s's screen.", remove_trailing_space(CurrentUserPin));
		/* Save form-stored route data in preparation for getting new file-loaded data... */
		printf("updateOldHiddenFieldsWithRouteInfo(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeThickness.value,document.forms[0].routePathColor.value,document.forms[0].routeCoordsStr.value);");
		/* Load new data (from main array that was populated from the re-read file)... */
		printf("var arrThisLCDsRouteInfo=extractAllRouteInfoForSpecificLCDwithFloorplan(document.forms[0].routeMediaportOrigin.value,document.forms[0].routeFloorplanName.value);");	/* get all routes info for this specific device that was clicked and store it locally to this block so we can use it next... */
		printf("routesDrawn=arrThisLCDsRouteInfo.length-1;");									/* this represents the most recent/latest/last route for the device */
		printf("document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteThickness];");			/* update current route form data for thickness */
		printf("document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[routesDrawn][rtiRouteColor];");			/* update current route form data for color */
		printf("document.forms[0].routeCoordsStr.value=arrThisLCDsRouteInfo[routesDrawn][rtiCoords];");				/* update current route form data for the coordinates of the whole path */
		printf("routePoints=delimitedStringToCoordPairArray(document.forms[0].routeCoordsStr.value);");				/* populate the routePoints array so it's later possible for the route-path to be edited */
		/* Draw the updated/new route based on that new data... */
		printf("canvas.style.display=\"block\";");										/* display the canvas so we can use it to draw routes */
		printf("obtainCanvasAbsolutePosition();");										/* get the canvas element's pixel position on the page */
		printf("SetShowRouteMode();");												/* technically, we are showing a route - this also gives us interactivity */
		printf("clickLCDtoShow(document.forms[0].routeMediaportOrigin.value,routesDrawn);");					/* virtually click the LCD they were just working with before the page submission */
		}
	printf("</script>\n");
	}
/* If msg name/dir info is in-state, and we haven't already entered route mode, then automatically/programatically click the edit-map button */
if(assumed_need_route_mode == 1 
	&& fp_route_need_auto_submit_to_enter_edit_mode == 0)
	{
	if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG("loadFloorPlan() - It looks like user desires routing capability... Now setting fp_route_need_auto_submit_to_enter_edit_mode, Calling SetEditMode(1), and auto-submitting.");
	printf("<script type=\"text/javascript\">");
	printf("document.forms[0].fp_route_need_auto_submit_to_enter_edit_mode.value = 1;");	/* set the form-level route-mode flag - important otherwise we could have an infinitely looped submission! */
	printf("SetEditMode(1); setTimeout(\"document.forms[0].submit()\",100);");		/* programatically click the edit map button to take us directly into edit mode (need to wait a bit to avoid race condition when placing a new LCD icon that prevents naming popup) */
	printf("</script>\n");
	}
//if(fp_route_need_auto_submit_to_enter_edit_mode == 1)
//	{
//	printf("<script type=\"text/javascript\">");
//	printf("document.getElementById(\"chooseDifferentMapChoiceButton\").style.display=\"none\";");  /* hide the map selection button */
//	printf("</script>\n");
//	}
printf("<script type=\"text/javascript\">reviewLCDiconImgState();</script>\n");
printf("<script type=\"text/javascript\">document.forms[0].routeFloorplanName.value=\"%s\";</script>\n", present_floorplan_name_no_suffix);
printf("<script type=\"text/javascript\">document.forms[0].routeFloorplanName_full.value=\"%s\";</script>\n", present_floorplan_name_no_suffix_with_space);
/* CR END */

/* go get the current users directory to show messages from */
db_staff_pin_valid(CurrentUserPin);
printf("<INPUT type=\"hidden\" name=\"custom_directory\" value=\"%s\"></INPUT>", db_staf->dss_directory);
printf("<INPUT type=\"hidden\" name=\"custom_message\" value=\"\"></INPUT>");

/* We need to center the span image */
if(cgi_detect_browser() == BROWSER_EXPLORER)
	{
	printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
	printf("function SetSpanLocation(){");
	printf("var centerx = (document.body.clientWidth - %d)/2;", backwidth);
	printf("if(centerx > 0) document.getElementById('main_image').style.left=centerx;");
	printf("else document.getElementById('main_image').style.left=10;");
	printf("}");
	printf("</SCRIPT>");
	}
else
	{
	printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
	printf("function SetSpanLocation(){");
	printf("var centerx = (window.innerWidth - %d)/2;", backwidth);
	printf("if(centerx > 0) document.getElementById('main_image').style.left=centerx;");
	printf("else document.getElementById('main_image').style.left=10;");
	printf("}");
	printf("</SCRIPT>");
	}

printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
printf("%s", other);
printf("SetSpanLocation();");
printf("</SCRIPT>");

/* set screen refresh timer every minute */
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");
printf("function LocationsScreenTimerReset() {");
if(locations_screen_refresh_timer_default > 0)
	{
	printf("	if(locations_screen_refresh_timer >= 0)");
	printf("		{");
	printf("		clearTimeout(locations_screen_refresh_timer);");
	printf("		locations_screen_refresh_timer = -1;");
	printf("		}");
	
	printf("	if(%d || %d || %d)", fp_edit_mode, notjustspace(route_msg_name_fromDB, MSG_NAME_LENGTH), notjustspace(route_msg_directory_fromDB, DIRECTORY_LENGTH));
	printf("		{");
				/* no refresh for this case */
	printf("		locations_screen_refresh_timer = -1;");
	printf("		}");
	printf("	else");
	printf("		{");
				/* not editing the map or showing on a MediaPort so allow refresh */
	printf("		locations_screen_refresh_timer = setTimeout('document.forms[0].submit();', %d*60*1000); ", locations_screen_refresh_timer_default);
	printf("		}");
	printf("	}");
	
	printf("LocationsScreenTimerReset();");
	printf("</SCRIPT>");
	}
else
	{
	printf("	}");
	printf("</SCRIPT>");
	}

/* The following will periodically update the device icons' status on a user's location screen (for user-browser sessions, only - not GA or MediaPort)
 * Rather than risk hitting the database too hard, we're utilizing Apache by having the client read 1-byte text files on the server (the byte is an integer representing the device state)
 * The text files are created and updated by a function call to BannerLifeSafetyDeviceStateUpdate(void)
 * The algorithm should basically go like this: client should periodically read the server's text file(s) that match the on-screen device(s), and loop through the DOM to update any icons as necessary.
 */
#define USE_LOCATIONS_BACKGROUND_UPDATE 1
#ifdef USE_LOCATIONS_BACKGROUND_UPDATE
if(assumed_need_route_mode == FALSE
	&& configuration_lookup_integer(DB_PATH_LOCATION, "SMCGI_LOCATIONS_LIFE_SAFETY_AUTOMATIC", 1))
	{
	/* only used on actual locations tab not routes or anything */
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\">");
printf("var objXML_deviceState = false;");
printf("function deviceUpdateState_xmlHttp_open(deviceid, deviceType, strImgDomID) {");
printf("	if(typeof deviceid===\"undefined\"){");
printf("		console.error(\"deviceUpdateState_xmlHttp_open(): Required argument, 'deviceid' was not supplied. Aborting.\");");
printf("		return false;");
printf("	}");
printf("	if(deviceid.length==0){");
printf("		console.warn(\"deviceUpdateState_xmlHttp_open(): The 'deviceid' argument is empty. Perhaps devicesTable got invalid data? Aborting.\");");
printf("		return false;");
printf("	}");
		/* first, use browser-feature detection to determine method that can be used to instantiate a XHR object */
printf("	if(window.XMLHttpRequest) {");							// if this is a modern, standards-compliant browser...
printf("		try {objXML_deviceState = new XMLHttpRequest();}");				// try instantiating an XHR object
printf("		catch(e) {objXML_deviceState = false;}");					// if that didn't work for some reason, set flag
printf("	}");
printf("	else if(window.ActiveXObject) {");						// else if this is the dreaded IE, we have extra work to do (of course), first...
printf("		try {objXML_deviceState = new ActiveXObject(\"Msxml2.XMLHTTP\");}");		// try to use v2
printf("		catch(e) {");									// if that didn't work (perhaps because client is an older IE), try another version
printf("			try {objXML_deviceState = new ActiveXObject(\"Microsoft.XMLHTTP\");}");		// try to use older version
printf("			catch(e) {objXML_deviceState = false;}");					// if that didn't work, just give up... IE sucks
printf("		}");
printf("	}");
		/* now, continue doing ajax stuff or not */
printf("	if(objXML_deviceState!=false) {");						// if the AJAX object was created successfully...
printf("		objXML_deviceState.onreadystatechange=function (){deviceUpdateState_xmlHttp_callback(deviceType, strImgDomID);};");	// specify callback function (this construct allows us to pass argments to it, since server file is dumb)
printf("		objXML_deviceState.open('GET', '/~silentm/device_state/%s'+deviceid+'.txt?'+(new Date()).getTime(), false);", TimeShareCompanyNameDirectoryGet());		// must be relative to same server or get errors
printf("		objXML_deviceState.setRequestHeader('Pragma','no-cache');");								// helps ensure browser won't use cache and get old state data
printf("		objXML_deviceState.setRequestHeader('Cache-Control','no-store, no-cache, must-revalidate, post-check=0');");		// helps ensure browser won't use cache and get old state data
printf("		objXML_deviceState.setRequestHeader('Expires',0);");									// helps ensure browser won't use cache and get old state data
printf("		objXML_deviceState.setRequestHeader('Last-Modified',new Date(0));");							// helps ensure browser won't use cache and get old state data
printf("		objXML_deviceState.setRequestHeader('If-Modified-Since',new Date(0));");						// helps ensure browser won't use cache and get old state data
printf("		objXML_deviceState.send(null);");											// send the floorplan name to the ajax processor on the server
printf("	}");
printf("	else{"); // else their browser is not up to the task
printf("		console.warn(\"deviceUpdateState_xmlHttp_open(): This browser cannot perform asynchronous HTTP requests.\");");
printf("	}");
printf("}");
printf("function deviceUpdateState_xmlHttp_callback(deviceType, strImgDomID) {");
printf("	if(objXML_deviceState.readyState == 4 && objXML_deviceState.status == 200){");
printf("		var arrIconset_local = [];");								/* required as an input to the getIconFilenameFromType function call that may be made a bit later */
printf("		arrIconset_local[0] = deviceType;");
printf("		arrIconset_local[5] = objXML_deviceState.responseText;");
printf("		var node_img = document.getElementById(strImgDomID);");
printf("		if(typeof node_img!==\"undefined\"){");
printf("			node_img.src = \"%s/\"+getIconFilenameFromType(arrIconset_local);", cgi_icons);
//printf("			node_img.src = \"%s/\"+getIconFilenameFromType(arrIconset_local) + \"?datenow=\" + Date.now();", cgi_icons);
printf("		}else{");
printf("			console.error(\"deviceUpdateState_xmlHttp_callback(): The node with id '\"+strImgDomID+\"' is undefined.\");");
printf("		}");
printf("	}");
printf("}");
printf("function LocationsScreenMonitorStates() {");
printf("    try{");
printf("	var deviceDomParent = document.getElementById(\"main_image\");");					/* the DOM common parent (container) of the device icons */
printf("	var strSpanID, intType, strImgDomID, strDeviceID, DeviceRecno;");
printf("	for(var i=0; i<deviceDomParent.childNodes.length; i++){");						/* loop through the child nodes (should be mostly span elements, aka 'cells', for containing all stuff about icons) */
printf("		if(deviceDomParent.childNodes[i].nodeName.toUpperCase() == 'SPAN'){");					/* if child is a span element, then this could contain a device icon, so continue... */
printf("			strSpanID = deviceDomParent.childNodes[i].id;");						/* get this span's ID (which contains the record number as 'image[recno]') */
printf("			intType = returnTypeForRecNo(parseInt(strSpanID.replace(\"image\",\"\")));");			/* parse out the recno from the ID and figure out which type of device this is */
printf("			if(intType==%d ", FLOORPLAN_PULL_STATION);							/* if this span's ID indicates it's any type of life-safety device, let's continue... */
printf("			|| intType==%d ", FLOORPLAN_WATER_FLOW);
printf("			|| intType==%d ", FLOORPLAN_DUCT_DETECTOR);
printf("			|| intType==%d ", FLOORPLAN_SMOKE_DETECTOR);
printf("			|| intType==%d ", FLOORPLAN_MOTION_DETECTOR);
printf("			|| intType==%d ", FLOORPLAN_SOUND_METER);
printf("			|| intType==%d){", FLOORPLAN_DOOR_OPEN);
printf("				for(var ii=0; ii<deviceDomParent.childNodes[i].childNodes.length; ii++){");			/* loop through the span's child nodes (should be mostly anchor and hidden field elements) */
printf("					if(deviceDomParent.childNodes[i].childNodes[ii].nodeName.toUpperCase() == 'A'){");		/* if this child is an anchor element, we're getting close to the IMG (icon) now... */
printf("						for(var iii=0; iii<deviceDomParent.childNodes[i].childNodes[ii].childNodes.length; iii++){");	/* loop through the anchor's child nodes, even though there should just be an IMG (just to be safe) */
printf("							if(deviceDomParent.childNodes[i].childNodes[ii].childNodes[iii].nodeName.toUpperCase() == 'IMG'){");/* this should be the device icon's element, so do what you need to... */
printf("								strImgDomID = deviceDomParent.childNodes[i].childNodes[ii].childNodes[iii].id;");		
printf("								DeviceRecno = strImgDomID.replace(\"cell\",\"\");");						/* parse record number drop cell name */
printf("								strDeviceID = returnDeviceIDForRecNo(DeviceRecno);");						/* parse record number drop cell name */
printf("								deviceUpdateState_xmlHttp_open(strDeviceID, intType, strImgDomID);");				/* call AJAX function to check for and initiate update if needed */
printf("							}");
printf("						}");
printf("					}");
printf("				}");
printf("			}");
printf("		}");
printf("	}");
printf("    }catch(err){");
printf("	console.error(\"LocationsScreenMonitorStates(): \"+err.message);");
printf("    }");
printf("    setTimeout('LocationsScreenMonitorStates();', parseInt(%d)); ", configuration_lookup_integer(DB_PATH_LOCATION, "SMCGI_LOCATIONS_LIFE_SAFETY_INTERVAL", 10*1000));
printf("}");
printf("setTimeout('LocationsScreenMonitorStates();', 5*1000);");			/* start after five seconds updating the screen icons or states */
printf("</SCRIPT>");
	}
#endif

main_volatile_parameters_write(TRUE);

add_ScreenPopMessage("", "");
add_HelpTextLayer(TRUE, FALSE);

printf("</FORM>");
printf("</BODY>");
printf("</HTML>");
}
