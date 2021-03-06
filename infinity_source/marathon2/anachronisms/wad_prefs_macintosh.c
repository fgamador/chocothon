/*

	macintosh_wad_prefs.c
	Tuesday, August 29, 1995 3:53:22 PM- rdm created.

*/

#include "macintosh_cseries.h"

#include "cseries.h"
#include <string.h>

#include "map.h"
#include "wad.h"
#include "game_errors.h"

#include "shell.h" // for refPREFERENCES_DIALOG only
#include "wad_prefs.h"

extern struct preferences_info *prefInfo;

/* ------------------ non portable code! */
static boolean handle_switch(DialogPtr dialog, struct preferences_dialog_data *funcs,
	void **prefs, short new_section, short old_section);
static pascal Boolean preferences_filter_proc(DialogPtr dialog, EventRecord *event, short *item_hit);

enum {
	dlogPREFERENCES_DIALOG= 4000,
	/* iOK */
	/* iCANCEL */
	iPREF_SECTION_POPUP= 3,
	NUMBER_VALID_PREF_ITEMS= 8
};
/* -------------- Preferences Dialog --------- */
/* Kick ass, take names */
boolean set_preferences(
	struct preferences_dialog_data *funcs,
	short count,
	void (*reload_function)(void))
{
	DialogPtr dialog;
	short index, current_pref_section, item_type, item_hit;
	Rect bounds;
	ControlHandle preferences_control;
	struct PopupPrivateData **privateHndl;
	MenuHandle mHandle;
	void *preferences;
	short new_value;
	ModalFilterUPP modal_proc;

	assert(count);

	dialog= myGetNewDialog(dlogPREFERENCES_DIALOG, NULL,(WindowPtr) -1, refPREFERENCES_DIALOG);
	assert(dialog);

	/* Setup the popup list.. */
	GetDItem(dialog, iPREF_SECTION_POPUP, &item_type,
		(Handle *) &preferences_control, &bounds);
	assert(preferences_control);
	privateHndl= (PopupPrivateData **) ((*preferences_control)->contrlData);
	mHandle= (*privateHndl)->mHandle;

	/* Append the preferences names to the popup. */
	for(index= 0; index<count; ++index)
	{
		AppendMenu(mHandle, "\p ");
		getpstr(temporary, funcs[index].resource_group, funcs[index].string_index);
		SetMenuItemText(mHandle, index+1, temporary);
	}

	/* Set our max value.. */
	SetCtlMax(preferences_control, index+1); /* +1 because menus are one based. */

	/* Set the current prefs section.. */
	current_pref_section= 0;
	handle_switch(dialog, funcs, &preferences, current_pref_section, NONE);
	SetCtlValue(preferences_control, current_pref_section+1);

	/* Show the dialog... */
	ShowWindow((WindowPtr) dialog);

	/* Create the modal proc. */
	modal_proc= NewModalFilterProc(preferences_filter_proc);
	assert(modal_proc);

	/* Setup the filter procedure */
	/* Note that this doesn't allow for cancelling.. */
	do {
		ModalDialog(modal_proc, &item_hit);

		switch(item_hit)
		{
			case iPREF_SECTION_POPUP:
				new_value= GetCtlValue(preferences_control)-1;
				if(new_value != current_pref_section)
				{
					if(handle_switch(dialog, funcs, &preferences,
						new_value, current_pref_section))
					{
						/* Changed it... */
						current_pref_section= new_value;
					}
				}
				break;

			case iOK:
				if(!funcs[current_pref_section].teardown_dialog_func(dialog,
					NUMBER_VALID_PREF_ITEMS, preferences))
				{
					/* We can't tear down yet.. */
					item_hit= 4000;
				}
				break;

			case iCANCEL:
				if(funcs[current_pref_section].teardown_dialog_func(dialog,
					NUMBER_VALID_PREF_ITEMS, preferences))
				{
					/* Reload the wadfile.. */
					reload_function();
				} else {
					/* Can't tear down yet... */
					item_hit= 4000;
				}
				break;

			default:
				funcs[current_pref_section].item_hit_func(dialog,
					NUMBER_VALID_PREF_ITEMS, preferences, item_hit);
				break;
		}
	} while (item_hit>iCANCEL);

	DisposeRoutineDescriptor(modal_proc);
	DisposeDialog(dialog);

	return item_hit==iOK;
}

/* ------------------ local code */
static boolean handle_switch(
	DialogPtr dialog,
	struct preferences_dialog_data *funcs,
	void **prefs,
	short new_section,
	short old_section)
{
	boolean able_to_switch= TRUE;

	/* Call the cleanup routines.. */
	if(old_section != NONE)
	{
		able_to_switch= funcs[old_section].teardown_dialog_func(dialog,
			NUMBER_VALID_PREF_ITEMS, *prefs);
		if(able_to_switch)
		{
			short number_items= CountDITL(dialog)-NUMBER_VALID_PREF_ITEMS;

			/* Remove the old ones. */
			ShortenDITL(dialog, number_items);
		}
	}

	if(able_to_switch)
	{
		Handle theDITL;

		/* Add the data from the dialog... */
		theDITL= GetResource('DITL', funcs[new_section].ditl_id);
		assert(theDITL);

		/* Append it.. */
		AppendDITL(dialog, theDITL, overlayDITL);

		/* Free the memory */
		ReleaseResource(theDITL);

		*prefs= funcs[new_section].get_data();

		/* Called on setup (initialize your fields) */
		funcs[new_section].setup_dialog_func(dialog, NUMBER_VALID_PREF_ITEMS,
			*prefs);
	}

	return able_to_switch;
}

static pascal Boolean preferences_filter_proc(
	DialogPtr dialog,
	EventRecord *event,
	short *item_hit)
{
	short item_type, value, new_value;
	ControlHandle preferences_control;
	Rect bounds;
	boolean handled= FALSE;

	GetDItem(dialog, iPREF_SECTION_POPUP, &item_type,
		(Handle *) &preferences_control, &bounds);
	new_value= value= GetCtlValue(preferences_control);

	switch(event->what)
	{
		case keyDown:
		case autoKey:
			switch(event->message & charCodeMask)
			{
				case kPAGE_UP:
					new_value= value-1;
					break;

				case kPAGE_DOWN:
					new_value= value+1;
					break;

				case kHOME:
					new_value= 1;
					break;

				case kEND:
					new_value= 45;
					break;
			}

			new_value= PIN(new_value, 1, GetCtlMax(preferences_control)-1);
			if(new_value != value)
			{
				SetCtlValue(preferences_control, new_value);
				*item_hit= iPREF_SECTION_POPUP;
				event->what= nullEvent;
				handled= TRUE;
			}
			break;
	}

	if(!handled)
	{
		handled= general_filter_proc(dialog, event, item_hit);
	}

	return handled;
}
