/*********************************************************
**	smcgi_xmlhttp.c
**
**  Created by Chris Rider
**
*********************************************************/

#include <stdio.h>
#include <string.h>

/***** REDACTED *****/

void add_xml(void)
{
printf("<A HREF='javascript: OperationComplete()' onmouseover='return true;' onmouseout='' onclick='test_XMLHTTPOpen(); ' > CLICK </A>");

printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("var test_xmlhttp=false;");

printf("function test_XMLHTTPStateChange(){");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */
printf("	if(test_xmlhttp.readyState == 4)");
printf("		{");
printf("		if(test_xmlhttp.status == 200)");
printf("			{");
printf("			alert('RESPONSE: ' + test_xmlhttp.responseText);");
printf("			}");
printf("		}");
printf("	}");

printf("function test_XMLHTTPOpen(){");

switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	test_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	test_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	test_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" test_xmlhttp = new XMLHttpRequest();");
		break;
	}

printf("	if(test_xmlhttp)");
printf("		{");
printf("		test_xmlhttp.onreadystatechange=test_XMLHTTPStateChange;");
printf("		test_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("		test_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("		test_xmlhttp.send('loadCheckInbox=%s');", CurrentUserPin);
printf("		}");
printf("	}");
printf("</SCRIPT>");
}

/***** REDACTED *****/

/***********************************************
** void AJAX_CheckHardwareState(char *device_id_str)
**
**	Will update the hardware screen status icon without resubmitting the form.	
**
**	This function is part of show_main_icon_bar();
**
***********************************************/
void AJAX_CheckHardwareState(char *device_id_str)
{
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("var update_time=%d;", 1*1000);							// check inbox in five seconds initially
printf("var CheckHardwareState_xmlhttp=false;");

printf("function CheckHardwareStateSetTimer(){");
printf("	setTimeout('CheckHardwareState_XMLHTTPOpen();', update_time);");
printf("	update_time += 2000;");							// update every 2 seconds till 10 seconds
printf("	if(update_time > 10*1000)");
printf("		{");
printf("		update_time = 10*1000;");
printf("		}");
printf("	}");

printf("function CheckHardwareState_XMLHTTPStateChange(){");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */
printf("	if(CheckHardwareState_xmlhttp.readyState == 4)");
printf("		{");
printf("		if(CheckHardwareState_xmlhttp.status == 200)");
printf("			{");
printf("			HardwareUpdateStateConnectionImage(CheckHardwareState_xmlhttp.responseText);");
printf("			}");
printf("		}");
printf("	}");

printf("function CheckHardwareState_XMLHTTPOpen(){");

switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	CheckHardwareState_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	CheckHardwareState_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	CheckHardwareState_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" CheckHardwareState_xmlhttp = new XMLHttpRequest();");
		break;
	}

printf("	if(CheckHardwareState_xmlhttp)");
printf("		{");
printf("		CheckHardwareState_xmlhttp.onreadystatechange=CheckHardwareState_XMLHTTPStateChange;");
printf("		CheckHardwareState_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("		CheckHardwareState_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("		CheckHardwareState_xmlhttp.send('loadCheckHardwareState=%s');", device_id_str);
printf("		CheckHardwareStateSetTimer();");
printf("		}");
printf("	}");

printf("CheckHardwareStateSetTimer();");			/* start the timer the first time */
printf("</SCRIPT>");
}

/***********************************************
** void AJAX_CheckHardwareStatus(char *device_id_str)
**
**	Will update the hardware screen status icon without resubmitting the form.	
**
**	This function is part of show_main_icon_bar();
**
***********************************************/
void AJAX_CheckHardwareStatus(char *device_id_str)
{
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("var update_time=%d;", 1*1000);							// check inbox in five seconds initially
printf("var CheckHardwareStatus_xmlhttp=false;");

printf("function CheckHardwareStatusSetTimer(){");
printf("	setTimeout('CheckHardwareStatus_XMLHTTPOpen();', update_time);");
printf("	update_time += 2000;");							// update every 2 seconds till 10 seconds
printf("	if(update_time > 10*1000)");
printf("		{");
printf("		update_time = 10*1000;");
printf("		}");
printf("	}");

printf("function CheckHardwareStatus_XMLHTTPStatusChange(){");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */
printf("	if(CheckHardwareStatus_xmlhttp.readyState == 4)");
printf("		{");
printf("		if(CheckHardwareStatus_xmlhttp.status == 200)");
printf("			{");
printf("			HardwareUpdateStatusConnectionImage(CheckHardwareStatus_xmlhttp.responseText);");
printf("			}");
printf("		}");
printf("	}");

printf("function CheckHardwareStatus_XMLHTTPOpen(){");

switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	CheckHardwareStatus_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	CheckHardwareStatus_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	CheckHardwareStatus_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" CheckHardwareStatus_xmlhttp = new XMLHttpRequest();");
		break;
	}

printf("	if(CheckHardwareStatus_xmlhttp)");
printf("		{");
printf("		CheckHardwareStatus_xmlhttp.onreadystatechange=CheckHardwareStatus_XMLHTTPStatusChange;");
printf("		CheckHardwareStatus_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("		CheckHardwareStatus_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("		CheckHardwareStatus_xmlhttp.send('loadCheckHardwareStatus=%s');", device_id_str);
printf("		CheckHardwareStatusSetTimer();");
printf("		}");
printf("	}");

printf("CheckHardwareStatusSetTimer();");			/* start the timer the first time */
printf("</SCRIPT>");
}

/***********************************************
** void AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID_forUI(void)
**
**	(NOTE -- this is just like the function below, except it's intended for the user-interface, rather than a graphical annunciator display)
**
**	Will ask the server to return all applicable data for a given life-safety device.
**	That data should include floorplan, so the mediaport knows what to display...
**		And, that floorplan should be the one associated with the LSD that's in alarm/active state...
**		(if there are multiple floorplans for that device, also need to determine the active one, since the mediaport can only show one per message)
**
***********************************************/
void AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID_forUI(void)
{
/* begin writing script-code to the client */
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("function processSubmitRequest(){\n");
printf("	var arrRadioGroup = document.getElementsByName(\"floorplanselectiongroup\");\n");
printf("	var selectedValue;\n");
printf("	for(var i=0; i<arrRadioGroup.length; i++){\n");
printf("		if(arrRadioGroup[i].checked){\n");
printf("			selectedValue = arrRadioGroup[i].value;\n");
printf("			break;\n");
printf("		}\n");
printf("	}\n");
printf("	if(document.forms[0].activeLsdSelected_originallyActiveFloorplanName.value != selectedValue){\n");
printf("		if(document.forms[0].needtoresetstatus) document.forms[0].needtoresetstatus.value = \"1\";\n");		/* 7.31.2012: to fix browser console error */
printf("	}\n");
printf("	document.forms[0].submit();\n");
printf("}\n\n");

/* The call-back function that will get called once the AJAX returns something from the server... */
printf("var life_safety_device_xmlhttp = false;\n\n");
printf("function GetLifeSafetyDeviceDataBasedOnDeviceID_forUI_XMLHTTPStateChange(){\n\n");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */

		/* various supporting functions */
printf("        function removeTrailingSpace(str){");   //required by validation checks below
printf("                str = \"\"+str+\"\";");
printf("                return str.replace(/\\s+$/g,'');");
printf("        }\n");
		/* function returns indication of whether or not the device is active on another map or not */
printf("	function theDeviceIsActiveElsewhere(arr, deviceID){\n");		/* argument 'arr' must be arrLsdMaps formatted (smajax.c) and argument 'deviceID' must be life safety device id (with trailing spaces) */
printf("		if(arr.length>0){\n");							/* if the array is populated, then... */
printf("			for(var i=0; i<arr.length; i++){\n");					/* looping through each element of the array... */
printf("				if(arr[i][almiDeviceID]==deviceID){\n");				/* if this iteration's device matches what they're trying to identify the device as, then... */
printf("					if(parseInt(arr[i][almiActiveFlag])==1){\n");				/* if this iteration's active flag is active, then... */
printf("						return arr[i][almiFloorplanName];\n");					/* quit looping and return the floorplan that it's active on */
printf("					}\n");
printf("				}\n");
printf("			}\n");
printf("		}\n");
printf("		else{\n");
printf("			return false;\n");
printf("		}\n");
printf("	}\n\n");
printf("	function extractFloorplansForLsd(arr, deviceID){\n");			/* argument 'arr' must be arrLsdMaps formatted (smajax.c) and argument 'deviceID' must be life safety device id (with trailing spaces) */
printf("		var j=0;\n");
printf("		var arrRet = [];\n");
printf("		if(arr.length>0){\n");							/* if the array is populated, then... */
printf("			for(var i=0; i<arr.length; i++){\n");					/* looping through each element of the array... */
printf("				if(arr[i][almiDeviceID]==deviceID){\n");
printf("					arrRet[j] = arr[i].slice(0);\n");
printf("					j++;\n");
printf("				}\n");
printf("			}\n");
printf("		}\n");
printf("		return arrRet;\n");
printf("	}\n\n");

printf("	if(life_safety_device_xmlhttp.readyState == 4){\n");
printf("		if(life_safety_device_xmlhttp.status == 200){\n");
printf("			eval(life_safety_device_xmlhttp.responseText);\n");					/* eval (interperet) the AJAX response as proper javascript (so now we have arrLsdMaps to work with --- see smajax.c) */
				/* go through that array to determine if this device is already active or whatnot */
printf("			if(arrLsdMaps.length>0){\n");				/* if the ajax-returned array contains records, then... */
					/* check if the device is active elsewhere... if so, then user will need to choose which one to now make active... we will then need to save the device on the chosen map as active (and be sure to deactivate any others, if necessary) */
printf("				var activeOnFloorplan = theDeviceIsActiveElsewhere(arrLsdMaps,thisDeviceID);\n");
printf("				if(activeOnFloorplan && activeOnFloorplan != floorplan_name){\n");
printf("					submitNormalOrExtended = \"extended\";\n");
printf("					var arrLsdFloorplans = extractFloorplansForLsd(arrLsdMaps,thisDeviceID);\n");
printf("					var strHTML=\"<img src=\\'%s/dialog-info.png\\' class=\\'mb_mainicon\\' />\";\n", cgi_icons);
//printf("					strHTML+=\"<p class=\\'mb_mainmessage\\'>The device you identified is already active on another floorplan.</p>\";\n");				/* delete this line once sure the new wording is ok */
printf("					strHTML+=\"<p class=\\'mb_mainmessage\\'>\"+removeTrailingSpace(thisDeviceID)+\" has been associated with another active graphical annunciator map.</p>\";\n");	/* cleared up the wording */
//printf("					strHTML+=\"<p>Specify the floorplan that you want the graphical annunciator to display whenever this device goes into alarm:</p>\";\n");	/* delete this line once sure the new wording is ok */
printf("					strHTML+=\"<p>If this device goes into alarm, it may show up on a GA display, on a specific map (selected by default, below).\
								If desired, you may now update which map will be shown on the GA. If you're unsure what to do, you may just \
								wish to click OK without changing the map selection below.</p>\";\n");	/* cleared up the wording */
printf("					strHTML+=\"<div style=\\'max-height:70px; overflow-y:auto;\\'>\";");
printf("					strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup\\' value=\\'\"+floorplan_name+\"\\' />\"+floorplan_name+\"</label>\";\n");
printf("					for(var i=0; i<arrLsdFloorplans.length; i++){\n");
printf("						if(parseInt(arrLsdFloorplans[i][almiActiveFlag])==1){\n");
printf("							document.forms[0].activeLsdSelected_originallyActiveFloorplanName.value = arrLsdFloorplans[i][almiFloorplanName];\n");
printf("							strHTML+=\"<label class=\\'mb_radiowrapper\\'><img src='%s/activeScroll.gif' /><input type=\\'radio\\' checked=\\'checked\\' id=\\'fpsgchecked\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrLsdFloorplans[i][almiFloorplanName]+\"\\'>\"+arrLsdFloorplans[i][almiFloorplanName]+\"</label>\";\n", cgi_icons);
printf("						}\n");
printf("						else{\n");
printf("							strHTML+=\"<label class=\\'mb_radiowrapper\\' style=\\'padding-left:12px;\\'><input type=\\'radio\\' name=\\'floorplanselectiongroup\\' value=\\'\"+arrLsdFloorplans[i][almiFloorplanName]+\"\\'>\"+arrLsdFloorplans[i][almiFloorplanName]+\"</label>\";\n");
printf("						}\n");
printf("					}\n");
printf("					strHTML+=\"</div>\";");
printf("					strHTML+=\"<div align=\\'center\\' class=\\'mb_buttondiv\\'><button class=\\'mb_button\\' onclick=\\'processSubmitRequest();\\'>OK</button></div>\";\n");
//printf("					Modalbox.show(strHTML,{title:\"Device is already active elsewhere...\"});\n");
printf("					Modalbox.show(strHTML,{title:\"Review Graphical Annunciator Device-Map Association\"});\n");	/* cleared up wording */
printf("				}\n");
printf("			}\n");
printf("		}\n");
printf("	}\n");
printf("}\n\n");

/* The function that gets called to initiate the AJAX operation... */
printf("function GetLifeSafetyDeviceDataBasedOnDeviceID_forUI_XMLHTTPOpen(deviceID){\n");
switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	life_safety_device_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	life_safety_device_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	life_safety_device_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" life_safety_device_xmlhttp = new XMLHttpRequest();");
		break;
	}
printf("	if(life_safety_device_xmlhttp){\n");
printf("		life_safety_device_xmlhttp.onreadystatechange=GetLifeSafetyDeviceDataBasedOnDeviceID_forUI_XMLHTTPStateChange;\n");	// set the function to call once the AJAX has something from the server
printf("		life_safety_device_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);\n", TimeShareCompanyNameDotGet());
printf("		life_safety_device_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');\n");
printf("		life_safety_device_xmlhttp.send('loadGetLifeSafetyDeviceDataBasedOnDeviceID=1&lsd='+deviceID);\n");			// post necessary data to server... flag must correspond to a flag of the same name in smajax.c or nothing will happen
printf("	}\n");
printf("}\n");

printf("</SCRIPT>");
}

/***********************************************
** void AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID(char deviceid_local)
**
**	Will ask the server to return all applicable data for a given life-safety device.
**	That data should include floorplan, so the mediaport knows what to display...
**		And, that floorplan should be the one associated with the LSD that's in alarm/active state...
**		(if there are multiple floorplans for that device, also need to determine the active one, since the mediaport can only show one per message)
**
***********************************************/
void AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID(char deviceid_local[DEVICEID_LENGTH])
{
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("var life_safety_device_xmlhttp = false;");
printf("var repositionMainImageDivRecursions2 = 0;");
printf("var bool_GetLifeSafetyDeviceDataBasedOnDeviceID_mainIsComplete = false;");
printf("var bool_GetLifeSafetyDeviceDataBasedOnDeviceID_repositionIsComplete = false;");

/* Recursive function to keep trying to reset floorplan wrapper position until it takes */
printf("function repositionMainImageDiv2(intRepositionToX){");
printf("	if(repositionMainImageDivRecursions2 < 10){");
printf("		document.getElementById(\"main_image\").style.left = \"\"+intRepositionToX+\"px\";");
printf("		repositionMainImageDivRecursions2++;");
printf("		setTimeout(\"repositionMainImageDiv2(\"+intRepositionToX+\")\",200);");				/* just in case that didn't take effect, let's try again */
printf("	}");
printf("	else{");
printf("		bool_GetLifeSafetyDeviceDataBasedOnDeviceID_repositionIsComplete = true;");
printf("	}");
printf("}");

/* The call-back function that will get called once the AJAX returns something from the server... */
printf("function GetLifeSafetyDeviceDataBasedOnDeviceID_XMLHTTPStateChange(){");
//printf("window.alert('GetLifeSafetyDeviceDataBasedOnDeviceID_XMLHTTPStateChange() called');");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */
printf("	if(life_safety_device_xmlhttp.readyState == 4){");
printf("		if(life_safety_device_xmlhttp.status == 200){");
/*PSEUDO...
- Now have an array of life safety devices containing their floorplans and whether active or not
- Go through that array and find the record that has an active flag (for this to work, there can only be one?)
- Once that record is found, use its floorplan name as the one that will now be loaded as the background image ---- Done?!
END*/
				/* initialize variables */
printf("			var activeFloorplan;");
printf("			var activeFloorplanForThisGA_trimmed, activeFloorplanForThisGA_filename;");

				/* initialize DOM shorthands */
printf("			var mainImageDiv = document.getElementById(\"main_image\");");
printf("			var bgImageDiv = document.getElementById(\"background_image_div\");");

				/* evaluate the AJAX-returned array of devices */
//printf("window.alert(life_safety_device_xmlhttp.responseText);");
printf("			eval(life_safety_device_xmlhttp.responseText);");					/* eval (interperet) the AJAX response as proper javascript (so now we have arrLsdMaps to work with --- see smajax.c) */

				/* using that array data, get other relevant data needed in order to proceed */
printf("			if(arrLsdMaps!=null || arrLsdMaps.length>0){");						/* if an array was actually returned, then... */
printf("				for(var i=0; i<arrLsdMaps.length; i++){");						/* for each element of the AJAX-returned array... */
printf("					if(parseInt(arrLsdMaps[i][almiActiveFlag])==1){");					/* if the proper element indicates active (there should only be one for this to work), then... */
printf("						activeFloorplan = arrLsdMaps[i][almiFloorplanName];");					/* save the active floorplan name */
printf("					}");
printf("				}");
printf("			}");
printf("			else{");
printf("				console.warn(\"Floorplan data unavailable\");");
printf("				document.write(\"Floorplan data unavailable\");");
//printf("				window.alert(\"Floorplan data unavailable\");");
printf("			}");

				/* determine what the floorplan image filename is supposed to be */
printf("			activeFloorplanForThisGA_trimmed = removeTrailingSpace(activeFloorplan);");
printf("			activeFloorplanForThisGA_filename = activeFloorplanForThisGA_trimmed + \".gif\";");

				/* change the background image to the active floorplan */
printf("			bgImageDiv.style.background = \"url('%s/\"+activeFloorplanForThisGA_filename+\"') no-repeat scroll left top\";", cgi_locations);			/* change the background image from default-loaded to the properly active floorplan */

				/* after changing the graphic, readjust the dimensions, using an array that should have already been created that contains all possible floorplans and their dimensions */
printf("			var bgImageWidth, bgImageHeight;");											/* declare variables to be used to store the dimensions */
printf("			var bgDefImageWidth = parseInt(bgImageDiv.style.width);");
printf("			var bgDefImageHeight = parseInt(bgImageDiv.style.height);");
printf("			var centerx;");														/* declare variable to store our centering benchmark */
printf("			var centerxDef;");														/* declare variable to store our centering benchmark */
printf("			for(var iSizes=0; iSizes<fpTable.length; iSizes++){");							/* for each of the floorplan records in the sizes array created at load-time... */
printf("				if(removeTrailingSpace(fpTable[iSizes][fptiName]) == activeFloorplanForThisGA_trimmed){");				/* if the matching floorplan is found in the sizes array, then... */
printf("					bgImageWidth = fpTable[iSizes][fptiWidth];");										/* get the active floorplan graphic's width */
printf("					bgImageHeight = fpTable[iSizes][fptiHeight];");									/* get the active floorplan graphic's height */
printf("					break;");														/* exit the loop since we got what we needed */
printf("				}");
printf("			}");
printf("			bgImageDiv.style.width = \"\" + bgImageWidth + \"px\";");								/* resize width to match the updated floorplan graphic */
printf("			bgImageDiv.style.height = \"\" + bgImageHeight + \"px\";");								/* resize height to match the updated floorplan graphic */
printf("			centerx = (parseInt(window.innerWidth) - parseInt(bgImageWidth)) / 2;");						/* this will give us the margin needed on either side of the floorplan background graphic to center it */
printf("			centerxDef = (parseInt(window.innerWidth) - parseInt(bgDefImageWidth)) / 2;");						/* this will give us the margin needed on either side of the floorplan background graphic to center it */
printf("			if(centerx > 0){");													/* if the floorplan graphic fits inside the window's width, then... */
printf("				repositionMainImageDiv2(centerx);");
printf("			}");
printf("			else{");														/* else the floorplan graphic is wider than the window, so... */
printf("				repositionMainImageDiv2(0);");
printf("			}");
printf("			bgImageDiv.style.visibility = \"visible\";");
printf("			mainImageDiv.style.display = \"block\";");
//printf("window.alert(\"vis = \"+document.getElementById('background_image_div').style.visibility);");
//printf("window.alert(\"dis = \"+document.getElementById('background_image_div').style.display);");
//printf("window.alert(\"bgimage = \"+document.getElementById('background_image_div').style.background);");
printf("			bool_GetLifeSafetyDeviceDataBasedOnDeviceID_mainIsComplete = true;");

				/* get iconset and routes for the active floorplan, and draw them all */
//printf("window.alert(\"going to call xml open for floorplan: '\"+activeFloorplan+\"'\");");
printf("			if(activeFloorplanForThisGA_trimmed != \"\"){");
printf("				setTimeout(\"SetFloorplanIconset_XMLHTTPOpen('\"+activeFloorplan+\"')\",50);");							/* initiate an AJAX transaction that will load the proper iconset for this floorplan */
printf("			}");
printf("		}");
printf("	}");
printf("}");

/* The function that gets called to initiate the AJAX operation... */
printf("function GetLifeSafetyDeviceDataBasedOnDeviceID_XMLHTTPOpen(){");
printf("	life_safety_device_xmlhttp = new XMLHttpRequest();");
printf("	if(life_safety_device_xmlhttp){");
printf("		life_safety_device_xmlhttp.onreadystatechange=GetLifeSafetyDeviceDataBasedOnDeviceID_XMLHTTPStateChange;");		// set the function to call once the AJAX has something from the server
printf("		life_safety_device_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("		life_safety_device_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
if(DiagnosticCheck(DIAGNOSTIC_SMCGI_ROUTES)) DIAGNOSTIC_LOG_1("AJAX_GetLifeSafetyDeviceDataBasedOnDeviceID: deviceid = %s", deviceid_local);
printf("		life_safety_device_xmlhttp.send('loadGetLifeSafetyDeviceDataBasedOnDeviceID=1&lsd=%s');", deviceid_local);		// post necessary data to server... flag must correspond to a flag of the same name in smajax.c or nothing will happen
printf("	}");
printf("}");

printf("</SCRIPT>");
}

/***********************************************
** void AJAX_SetYouAreHereBasedOnIP(void)
**
**	Will ask the server what mediaport I am based on IP address submitted.
**	Then lookup in the translation_deviceid table and set the correct IMG
**	to the "you are here" mediaport icon.
**
**	Additional functionality added by CR in support of graphical annunciator:
**	- Ability to deal with routing's multiple floorplans (by getting device-icons data from DB, as well as getting routes data/active floorplan from the appropriate file in ~/record/routes/)
**	- Call JS functions necessary to determine that... as well as draw any needed routes
**	- Ability to deal specifically with GA's multiple floorplans (no routes), and showing only the floorplan that an active/alarming device is located on
**
***********************************************/
void AJAX_SetYouAreHereBasedOnIP(void)
{
printf("<SCRIPT language=\"JavaScript\" type=\"text/javascript\" >");

printf("var update_time=%d;", 1*1000);							// check inbox in five seconds initially
printf("var set_you_are_here_xmlhttp=false;");
printf("var set_floorplan_iconset_xmlhttp=false;");
printf("var repositionMainImageDivRecursions = 0;");
printf("var deviceNameResponse = -1;");

/* Recursive function to keep trying to reset floorplan wrapper position until it takes */
printf("function repositionMainImageDiv(intRepositionToX){");
printf("	if(repositionMainImageDivRecursions < 10){");
printf("		document.getElementById(\"main_image\").style.left = \"\"+intRepositionToX+\"px\";");
printf("		repositionMainImageDivRecursions++;");
printf("		setTimeout(\"repositionMainImageDiv(\"+intRepositionToX+\")\",200);");				/* just in case that didn't take effect, let's try again */
printf("	}");
printf("}");

printf("function SetYouAreHere_XMLHTTPStateChange(){");
//printf("window.alert(\"SetYouAreHere_XMLHTTPStateChange() called\");");
/* readystate 	0 - uninitialized */
/* 		1 - object been created but send has not been called */
/* 		2 - send method called but nothing back */
/* 		3 - some data has been received */
/* 		4 - all done look at responseBody (complete) and responseText */
printf("	if(set_you_are_here_xmlhttp.readyState == 4)");
printf("		{");
printf("		if(set_you_are_here_xmlhttp.status == 200)");
printf("			{");
				/* initialize DOM shorthands */
printf("			var mainImageDiv = document.getElementById(\"main_image\");");
printf("			var bgImageDiv = document.getElementById(\"background_image_div\");");

				/* grab the name of the device that is processing this code (from AJAX response) */
printf("			var deviceNameResponse = set_you_are_here_xmlhttp.responseText;");							/* note: GA and device are terms used interchangeably meaning the same thing! */

//printf("			window.alert('deviceNameResponse = '+parseInt(deviceNameResponse)+'\\nform_state value = '+parseInt(document.forms[0].form_state.value));");
/* DEV-NOTE: patch */
printf("			var patch_form_state;");
printf("			if(typeof document.forms[0].form_state === 'undefined') {");
printf("				patch_form_state = -1;");
printf("			}");
printf("			else {");
printf("				patch_form_state = parseInt(document.forms[0].form_state.value);");
printf("			}");
/* END PATCH */
//printf("			if(parseInt(deviceNameResponse) != -1 || parseInt(document.forms[0].form_state.value) != 9)");
printf("			if(parseInt(deviceNameResponse) != -1 || patch_form_state != 9)");
printf("				{");

					/* using the device name we got from AJAX, get other relevant data needed in order to proceed */
printf("				var arrThisGAsRouteInfo = extractAllRouteInfoForSpecificLCD(deviceNameResponse);");					/* get all route data for this device, as a 2D array (like a table, filtered only for this device) */
printf("				var arrThisGAsUniqueFloorplans = extractFloorplansFromArray(arrThisGAsRouteInfo);");					/* from that data, get a simple 1D array of all the unique (nothing repeated) floorplans possible for it (active and inactive) */
	
					/* determine the floorplan that is supposed to be active for this device */
printf("				var activeFloorplanForThisGA, activeFloorplanForThisGA_trimmed;");
printf("				for(var iUniqueFP=0; iUniqueFP<arrThisGAsUniqueFloorplans.length; iUniqueFP++){");					/* for each of this device's possible floorplans... (going to find the active one) */
printf("					if(isFloorplanActiveForLCD(\"\"+arrThisGAsUniqueFloorplans[iUniqueFP]+\"\",\"\"+deviceNameResponse+\"\") == 1){");	/* if this iteration's floorplan is the active one for this device...*/
printf("						activeFloorplanForThisGA = arrThisGAsUniqueFloorplans[iUniqueFP];");
printf("						activeFloorplanForThisGA_trimmed = removeTrailingSpace(activeFloorplanForThisGA);");
printf("						activeFloorplanForThisGA_filename = activeFloorplanForThisGA_trimmed + \".gif\";");
printf("						break;");
printf("					}");
printf("				}");

/* If, for some reason we couldn't determine an active floorplan, figure it out... */
/* (result of bug in which multiple mediaports, but no routes defined for one of them - it would show a black screen only) */
printf("				if(activeFloorplanForThisGA == undefined){");
//printf("					window.alert(\"undefined active floorplan\");");
printf("					");
printf("				}");
printf("				else{");

						/* we now have the floorplan that is supposed to be showing on this device, so update the background image to represent that */
printf("					bgImageDiv.style.background = \"url('%s/\"+activeFloorplanForThisGA_filename+\"') no-repeat scroll left top\";", cgi_locations);			/* change the background image from default-loaded to the properly active floorplan */

						/* after changing the graphic, readjust the dimensions, using an array that should have already been created that contains all possible floorplans and their dimensions */
printf("					var bgImageWidth, bgImageHeight;");											/* declare variables to be used to store the dimensions */
printf("					var bgDefImageWidth = parseInt(bgImageDiv.style.width);");
printf("					var bgDefImageHeight = parseInt(bgImageDiv.style.height);");
printf("					var centerx;");														/* declare variable to store our centering benchmark */
printf("					var centerxDef;");														/* declare variable to store our centering benchmark */
printf("					for(var iSizes=0; iSizes<arrFloorplanSizes.length; iSizes++){");							/* for each of the floorplan records in the sizes array created at load-time... */
printf("						if(removeTrailingSpace(arrFloorplanSizes[iSizes][0]) == activeFloorplanForThisGA_trimmed){");				/* if the matching floorplan is found in the sizes array, then... */
printf("							bgImageWidth = arrFloorplanSizes[iSizes][1];");										/* get the active floorplan graphic's width */
printf("							bgImageHeight = arrFloorplanSizes[iSizes][2];");									/* get the active floorplan graphic's height */
printf("							break;");														/* exit the loop since we got what we needed */
printf("						}");
printf("					}");
printf("					bgImageDiv.style.width = \"\" + bgImageWidth + \"px\";");								/* resize width to match the updated floorplan graphic */
printf("					bgImageDiv.style.height = \"\" + bgImageHeight + \"px\";");								/* resize height to match the updated floorplan graphic */
printf("					centerx = (parseInt(window.innerWidth) - parseInt(bgImageWidth)) / 2;");						/* this will give us the margin needed on either side of the floorplan background graphic to center it */
printf("					centerxDef = (parseInt(window.innerWidth) - parseInt(bgDefImageWidth)) / 2;");						/* this will give us the margin needed on either side of the floorplan background graphic to center it */
printf("					if(centerx > 0){");													/* if the floorplan graphic fits inside the window's width, then... */
printf("						repositionMainImageDiv(centerx);");
printf("					}");
printf("					else{");														/* else the floorplan graphic is wider than the window, so... */
printf("						repositionMainImageDiv(0);");
printf("					}");

printf("				}");

printf("				bgImageDiv.style.visibility = \"visible\";");
//printf("				mainImageDiv.style.display = \"block\";");
printf("				initializeCanvas(\"mapcanvas\");");											/* finally, reinitialize the canvas to match the updated dimensions (to ensure routes are properly positioned) */

					/* clear off any other floorplan's device icons */
printf("				for(var iDefaultVisibleIcons=0; visible_list[iDefaultVisibleIcons]; iDefaultVisibleIcons++){");				/* for each of the presumed already-up default icons, be sure to take them down */
printf("					if(visible_list[iDefaultVisibleIcons] != \"main_image\"){");								/* ignoring the element for the main_image wrapper ID... */
printf("						document.getElementById(visible_list[iDefaultVisibleIcons]).style.visibility = \"hidden\";");				/* hide all default icons, using their ID's */
printf("					}");
printf("				}");

					/* get iconset and routes for the active floorplan, and draw them all */
printf("				SetFloorplanIconset_XMLHTTPOpen(\"\"+activeFloorplanForThisGA+\"\");");							/* initiate an AJAX transaction that will load the proper iconset for this floorplan */
	
					/* check for showing the GA screen on GA devices */
printf("				for(var ib = 0; mediaport_ga_deviceid[ib]; ib++)");
printf("					{");
printf("					if(mediaport_ga_deviceid[ib] == deviceNameResponse) ");
printf("						{");
printf("						ShowGraphicalAnnunciatorIcons();");
printf("						break;");
printf("						}");
printf("					}");

					/* for each device on this floorplan */
printf("				for(var ic = 0; translation_deviceid[ic]; ic++)");
printf("					{");
						/* if the GA matches this iteration's device... */
printf("					if(translation_deviceid[ic] == deviceNameResponse) ");
printf("						{");
							/* if emergency type messages then change the graphic to "your are here" */
printf("						if(document.forms[0].locations_alert_status.value >= %d)", BB_ALERT_STATUS_EMERGENCY);
printf("							{");
printf("							document.getElementById(\"\"+translation_id[ic]+\"\").src=\"%s/map_you_are_here.gif\";", cgi_icons);
printf("							}");
printf("						}");
printf("					}");
printf("				}");//end for
printf("			}");//end if
printf("		}");
printf("	}");

printf("function SetYouAreHere_XMLHTTPOpen(){");
switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	set_you_are_here_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	set_you_are_here_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	set_you_are_here_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" set_you_are_here_xmlhttp = new XMLHttpRequest();");
		break;
	}
printf("	if(set_you_are_here_xmlhttp)");
printf("		{");
printf("		set_you_are_here_xmlhttp.onreadystatechange=SetYouAreHere_XMLHTTPStateChange;");
printf("		set_you_are_here_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', false);", TimeShareCompanyNameDotGet());
printf("		set_you_are_here_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("		set_you_are_here_xmlhttp.send('loadSetYouAreHereBasedOnIP=1');");
printf("		}");
printf("	}");

/* call-back function (what this page executes once the AJAX returns a response) */
/* dev-note: this code block is chopped all up due to debugging apparent firefox readyState bug.. keep around a couple versions in case need to revisit */
//printf("var setFloorplanIconsetRecursions = 0;");
printf("function SetFloorplanIconset_XMLHTTPStateChange(){");
//printf("window.alert('SetFloorplanIconset_XMLHTTPStateChange called (readyState = '+set_floorplan_iconset_xmlhttp.readyState+') (status = '+set_floorplan_iconset_xmlhttp.status+')');");
printf("	if(set_floorplan_iconset_xmlhttp.readyState == 4)");
printf("		{");
printf("		if(set_floorplan_iconset_xmlhttp.status == 200)");
printf("			{");
printf("			eval(set_floorplan_iconset_xmlhttp.responseText);");					/* eval (interperet) the AJAX response as proper javascript (so now we have arrIconset to work with --- see smajax.c) */
printf("			if(arrIconset.length > 0){");								/* if the interpereted response is a valid array containing a set of icon(s) for the floorplan, then continue... */
printf("				var strSpanID, strSpanTop, strSpanLeft, strCellID;");					/* initialize variables for passing along as arguments to postAjaxCreateIcon function */
printf("				var arrAnchorTagContent = [];");							/* initialize array to contain icon info, for passing along as arguments to postAjaxCreateIcon function */
printf("				translation_id = [];");									/* reset translation table (since it was originally created for the default floorplan/iconset) */
printf("				translation_deviceid = [];");								/* reset translation table (since it was originally created for the default floorplan/iconset) */
printf("				for(var i=0; i<arrIconset.length; i++){");						/* for each of the icons returned from the AJAX (for this particular active /floorplan) */
						/* rebuild translation table arrays with the new data */
printf("					translation_id[i] = \"cell\"+arrIconset[i][aisRecno];");							/* add in a new arbitrary cell id based on the iteration number */
printf("					translation_deviceid[i] = arrIconset[i][aisDeviceID];");				/* associate (by using same iteration number) the device id with the cell id */
						/* set arguments that will be passed to the function */
printf("					strSpanID = \"\"+arrIconset[i][aisRecno]+\"\";");								/* specify a span ID based on the iteration number (similar to cell ID) */
printf("					strSpanTop = \"\"+arrIconset[i][aisItemTop]+\"\";");					/* specify the top position based on what was returned via AJAX */
printf("					strSpanLeft = \"\"+arrIconset[i][aisItemLeft]+\"\";");					/* specify the left position based on what was returned via AJAX */
printf("					strCellID = \"\"+arrIconset[i][aisRecno]+\"\";");								/* specify a cell ID based on the iteration number (similar to cell ID - translation table) */
						/* if this icon represents the device that is displaying it, then show you-are-here image and draw route(s) */
printf("					if(removeTrailingSpace(arrIconset[i][aisDeviceID])==removeTrailingSpace(set_you_are_here_xmlhttp.responseText)){");
printf("						arrAnchorTagContent[0] = \"image\";");							/* specify image type for postAjaxCreateIcon argument */
printf("						arrAnchorTagContent[1] = \"map_you_are_here.gif\";");					/* specify image filename for postAjaxCreateIcon argument */
							/* if this is a proper device for doing so, draw the route(s) associated with it */
printf("						var intAlertStatus = parseInt(document.forms[0].locations_alert_status.value);");
printf("						if(intAlertStatus==%d || intAlertStatus==%d || intAlertStatus==%d){", BB_ALERT_STATUS_EMERGENCY, BB_ALERT_STATUS_SHELTER, BB_ALERT_STATUS_EVACUATE);
printf("							\ncanvas.style.display=\"block\";\n");  						/* canvas is already rendered and initialized by now, just only in a display state of none */
printf("							var arrThisLCDsRouteInfo = extractAllRouteInfoForSpecificLCD(arrIconset[i][aisDeviceID]);\n");	/* function exists in smcgi_locations.c */
printf("							if(arrThisLCDsRouteInfo.length>0){");
printf("								for(var ici=0; ici<arrThisLCDsRouteInfo.length; ici++){\n");				/* for each route for this lcd... */
printf("									if(parseInt(arrThisLCDsRouteInfo[ici][rtiActiveFlag])==1){\n");				/* if the route is marked active, then set attributes and draw the route */
printf("										document.forms[0].routeThickness.value=arrThisLCDsRouteInfo[ici][rtiRouteThickness];\n");
printf("										document.forms[0].routePathColor.value=arrThisLCDsRouteInfo[ici][rtiRouteColor];\n");
printf("										drawCanvasFromPointsInArrayForMediaport(delimitedStringToCoordPairArray(arrThisLCDsRouteInfo[ici][rtiCoords]));\n");
printf("									}\n");
printf("								}\n");
printf("								setTimeout(\"makeRouteFlash()\",2000);\n");						/* after drawing routes, call recursive function to hide/show canvas (function exists in smcgi_locations.c) */
printf("							}\n");
printf("							else{\n");										/* else... */
printf("								arrThisLCDsRouteInfo = null;\n");							/* no routes */
printf("							}\n");
printf("						}\n");
/* IN-DEV: to show active/alarming LSD's appropriate map (probably should go somewhere else, independent of this block) */
printf("						else if(intAlertStatus==%d){\n", BB_ALERT_STATUS_GRAPHICAL_ANNUNCIATOR);		/* if GA, need to do an AJAX to get an array of LSD's (which should contain floorplans - and, ideally, active flags) */
printf("							\n");
printf("						}\n");
/* end IN-DEV */
printf("					}");
						/* special case: for text don't need to show an image but rather a string of text */
printf("					else if(arrIconset[i][aisItemType]==%d){", FLOORPLAN_TEXT);
printf("						arrAnchorTagContent[0] = \"text\";");							/* specify text type for postAjaxCreateIcon argument */
printf("						arrAnchorTagContent[1] = arrIconset[i][aisText];");					/* specify the string of text for postAjaxCreateIcon argument */
printf("					}");
						/* all other cases, show some other icon (ascertained by getIconFilenameFromType function, since can't use cgi_show_locations_item client side) */
printf("					else{");
printf("						arrAnchorTagContent[0] = \"image\";");							/* specify image type for postAjaxCreateIcon argument */
printf("						arrAnchorTagContent[1] = getIconFilenameFromType(arrIconset[i]);");			/* specify image filename for postAjaxCreateIcon argument (function located in smcgi_locations to return filename based on type) */
printf("					}");
						/* set js globals that are needed by the function called after them... since displaying on mediaport, there's currently no need to make them interactive, so empty */
printf("					postAjaxCreateIcon_strAnchorOnClick = \"\";");
printf("					postAjaxCreateIcon_strAnchorOnMouseOver = \"\";");
printf("					postAjaxCreateIcon_strAnchorOnMouseOut = \"\";");
						/* call the function to actually create the icon on the screen */
printf("					postAjaxCreateIcon(strSpanID,strSpanTop,strSpanLeft,strCellID,arrAnchorTagContent);");
printf("				}");
printf("			}");
printf("			else{");
printf("				document.write(\"No map/icon data available for this message!\");");
printf("			}");
printf("		}");
printf("	}");
/* WARNING:
 * If you do any kind of writing to the client here, it will somehow cancel out the httpRequest object, resulting in it never getting to readyState 4 -- as of Feb 8, 2011, FireFox 3.6.~13
 */
//printf("	else{");
//printf("		document.write(\"Attempt #\"+setFloorplanIconsetRecursions+\" - AJAX ready-state or status is being checked or did not succeed (readyState=\"+set_floorplan_iconset_xmlhttp.readyState+\") (status=\"+set_floorplan_iconset_xmlhttp.status+\")<br>\");");
//printf("window.alert('SetFloorplanIconset_XMLHTTPStateChange called (readyState = '+set_floorplan_iconset_xmlhttp.readyState+') (status = '+set_floorplan_iconset_xmlhttp.status+')');");
//printf("		if(set_floorplan_iconset_xmlhttp.readyState<4 && setFloorplanIconsetRecursions<4){setFloorplanIconsetRecursions++; SetFloorplanIconset_XMLHTTPStateChange();}");
//printf("	}");
printf("}");

printf("function SetFloorplanIconset_XMLHTTPOpen(strFloorplanName){");
//printf("window.alert(\"SetFloorplanIconset_XMLHTTPOpen called for floorplan: '\"+strFloorplanName+\"'\");");
switch(cgi_detect_browser())
	{
	case BROWSER_EXPLORER:	
		printf("try {");
		printf("	set_floorplan_iconset_xmlhttp = new ActiveXObject('Msxm12.XMLHTTP');");
		printf("    } catch (e) {");
		printf("try {");
		printf("	set_floorplan_iconset_xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');");
		printf("    } catch (E) {");
		printf("      	set_floorplan_iconset_xmlhttp = false;");
		printf("    }");
		printf(" }");
		break;

	case BROWSER_GECKO:	
	case BROWSER_OPERA:
	case BROWSER_NETSCAPE:
		printf(" set_floorplan_iconset_xmlhttp = new XMLHttpRequest();");
		break;
	default:
		printf(" set_floorplan_iconset_xmlhttp = new XMLHttpRequest();");
		break;
	}
printf("	if(set_floorplan_iconset_xmlhttp)");
printf("		{");
printf("		set_floorplan_iconset_xmlhttp.onreadystatechange=SetFloorplanIconset_XMLHTTPStateChange;");			// specify the callback function
printf("		set_floorplan_iconset_xmlhttp.open('POST', '/~silentm/bin/smajax%s.cgi', true);", TimeShareCompanyNameDotGet());
printf("		set_floorplan_iconset_xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');");
printf("		set_floorplan_iconset_xmlhttp.send(\"loadSetIconsetBasedOnFloorplan=1&specifiedFloorplan=\"+strFloorplanName);");					// send the floorplan name to the ajax processor on the server
printf("		}");
printf("	}");

//printf("SetYouAreHere_XMLHTTPOpen();");
printf("</SCRIPT>");
}
