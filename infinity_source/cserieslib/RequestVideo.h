/*------------------------------------------------------------------------------
#
#	MacOS™ Sample Code
#
#	Written by: Eric Anderson
#	 AppleLink: ERIC3
#
#	Display Manager sample code
#
#	RequestVideo
#
#	RequestVideo.h	-	C Header
#
#	Copyright © 1995 Apple Computer, Inc.
#	All rights reserved.
#
#	Versions:	1.0					5/4/95
#
#	To Do:		1)	Support a minimal form of gravitate since Display Manager 1.0 does
#					not do it automatically for us in the DMSetDisplayMode call. Display
#					Manager 2.0 does this work for us. Gravitate is the code which will
#					reposition all of the displays into a non-disjoint region after one
#					or more displays changes its timing mode (screen resolution).
#
#
#	Components:	PlayVideo.c			May 4, 1995
#				RequestVideo.c		May 4, 1995
#				RequestVideo.h		May 4, 1995
#
#	RequestVideo demonstrates the usage of the Display Manager introduced
#	with the PowerMacs and integrated into the system under System 7.5. With
#	the RequestVideo sample code library, developers will be able to explore
#	the Display Manager API by changing bit depth and screen resolution on
#	multisync displays on built-in, NuBus, and PCI based video. Display Manager 1.0
#	is built into the Systems included with the first PowerMacs up through System 7.5.
#	Display Manager 2.0 is included with the release of the new PCI based PowerMacs,
#	and will be included in post 7.5 System Software releases.
#
#	It is a good idea to reset the screen(s) to the original setting before exit
#	since the call to RVSetVideoAsScreenPrefs() may not do the right thing under
#	Display Manager 1.0 with certain video drivers.
#
#		OSErr RVRequestVideoSetting(VideoRequestRecPtr requestRecPtr);
#		OSErr RVSetVideoRequest (VideoRequestRecPtr requestRecPtr);
#		OSErr RVSetVideoAsScreenPrefs (void);
#
#
------------------------------------------------------------------------------*/

/************************************************************************************************

------------------------------
External routine descriptions:

OSErr RVRequestVideoSetting(VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoRequest (VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoAsScreenPrefs (void);


OSErr RVSetVideoRequest(VideoRequestRecPtr requestRecPtr);

RVSetVideoRequest is a wrapper around the Display Manger call DMSetDisplayMode
which is used to set the desired timing (screen resolution) and bit depth. Given
a request record that has been filled in with the RVRequestVideoSetting call,
RVSetVideoRequest verifies that a least a display mode (timing) and depth mode
(bit depth) are present and calls DMSetDisplayMode. If we get a
kDMDriverNotDisplayMgrAwareErr from the Display Manager, we must use the Quickdraw
SetDepth call to set the bit depth using the desired depth mode. In the error
case (when devices are not supported by the Display Manager for example), we can
not set the display mode, but can only set the bit depth on the desired display.
Under both the Display Manager 1.0 and 2.0, the VDSwitchInfoRec is used to set the
desired timing and bit depth. As a fall back, the Display Manager will use the
display and depth mode settings which are also passed into the DMSetDisplayMode
call. Before making the DMSetDisplayMode call, we set the GDevice to monochrome
if we want 1-bit mode or color for all other bit depths using the SetDeviceAttribute
call. This monochrome/color logic can be changed if you want to set 8 bit monochrome
for example. An error of -1 is returned from this call if the Display Manager is
not installed, or if an error from SetDepth is generated.
IMPORTANT: WARNING WHEN CHANGING THE SCREEN RESOLUTION
IMPORTANT:       IN A MULTI-MONITOR ENVIRONMENT
IMPORTANT: Monitor gravitation (repositioning) is not suported under
IMPORTANT: Display Manager 1.0, and there is not currently any code
IMPORTANT: written in this sample library to provide even minimal
IMPORTANT: gravitate functionality. Sample gravitate code will be
IMPORTANT: released as soon as I get to it. Until that time, changing
IMPORTANT: the video settings on multi-monitor systems under the Display
IMPORTANT: Manager 1.0 will result in only the bit depth being changed.


OSErr RVSetVideoAsScreenPrefs (void);

RVSetVideoAsScreenPrefs is a wrapper around the newly made public Display Manger
call DMUseScreenPrefs. This call is implemented under both Display Manager 1.0
and 2.0. Under Display Manager 1.0, it is a good idea to reset the screen(s) to
the original setting before you exit your application since the call to
DMUseScreenPrefs may not do the right thing with certain video drivers. An error
of -1 is returned from this call if the Display Manager is not installed.


OSErr RVRequestVideoSetting(VideoRequestRecPtr requestRecPtr);

RVRequestVideoSetting will iterate through all the active display GDevices
looking for a display that bets matches the requested settings passed in via
the VideoRequestRecPtr. If the user specified a particular GDevice to search
(by passing a GDevice in requestRec.screenDevice) then only that GDevice is
searched for a best fit of the requested video settings. Two different methods
of searching are used depending on the version of the Display Manager installed.
For systems with Display Manager 1.0, a search is done using the Slot Manager
via the GetRequestTheDM1Way routine. GetRequestTheDM1Way looks at all the slot
resources associated with the specified GDevice to determine the best timing and
bit depth fit for the users request. Under the Display Manager 2.0, the
GetRequestTheDM2Way routine searches the specified GDevice using new Display
Manger 2.0 calls to search for video modes of both slot and PCI based video devices.


------------------------------
Internal Routine descriptions:

void GetRequestTheDM1Way (	VideoRequestRecPtr requestRecPtr,
					GDHandle walkDevice);

void GetRequestTheDM2Way (	VideoRequestRecPtr requestRecPtr,
					GDHandle walkDevice,
					DMDisplayModeListIteratorUPP myModeIteratorProc,
					DMListIndexType theDisplayModeCount,
					DMListType *theDisplayModeList);

pascal void ModeListIterator(	void *userData,
					DMListIndexType itemIndex,
					DMDisplayModeListEntryPtr displaymodeInfo);

Boolean FindBestMatch (		VideoRequestRecPtr requestRecPtr,
					short bitDepth,
					unsigned long horizontal,
					unsigned long vertical);

void GravitateMonitors (void);



void GetRequestTheDM1Way (	VideoRequestRecPtr requestRecPtr,
					GDHandle walkDevice);

GetRequestTheDM1Way is passed the request record and the current GDevice to
search. Using the GDevice, GetRequestTheDM1Way first gets all the necessary slot
information (spSlot, spID, spDrvrHW, etc.) to begin the search of the device.
A search of all valid timings and all valid bit depths associated for each
timing is then made. For each valid timing/depth mode available, a call to the
FindBestMatch routine is made and the request record is updated as appropriate.
GetRequestTheDM1Way not called if Display Manager 2.0 is present since Display
Manager 2.0 can search both Slot Manager and PCI based video devices, and
returns much more detailed information. In this case, GetRequestTheDM2Way is
called which uses a much different search algorithm.

void GetRequestTheDM2Way (	VideoRequestRecPtr requestRecPtr,
					GDHandle walkDevice,
					DMDisplayModeListIteratorUPP myModeIteratorProc,
					DMListIndexType theDisplayModeCount,
					DMListType *theDisplayModeList);

GetRequestTheDM2Way is used if the Display Manager 2.0 is installed. This routine
can search both slot Manager and PCI based video drivers, and thus is used over
GetRequestTheDM1Way if possible. This routine is set up by the API call
RVRequestVideoSetting which initializes the DMDisplayModeListIteratorUPP,
DMListIndexType and DMListType values for this call. GetRequestTheDM2Way makes
use the new Display Manager 2.0 call DMGetIndexedDisplayModeFromList to iterate
through the <theDisplayModeList> for a count of <theDisplayModeCount> available
timings supported  by the specified GDevice. Before returning from the Display
Manager routine DMGetIndexedDisplayModeFromList, a call back routine ( ModeListIterator() )
is called which copies the Display Manager data into our own data record. The
specific data we look at is the bit depth, depth mode, timing resolution (horizontal
and vertical), the timing mode, and the VDSwitchInfoRec. The call back routine
then returns to the Display Manager which then returns back to GetRequestTheDM2Way.
Once we have all the bit depths for a given timing mode, GetRequestTheDM2Way
then loops through all the depths searching for the best fit to the users video
request. This search continues until there are no more timing modes available for
the GDevice, at which point it returns to RVRequestVideoSetting with a possible
better fit to the users request.

pascal void ModeListIterator(	void *userData,
					DMListIndexType itemIndex,
					DMDisplayModeListEntryPtr displaymodeInfo);

ModeListIterator is the sister routine of GetRequestTheDM2Way. This routine is
a call back routine called by the Display Manager during a call to
DMGetIndexedDisplayModeFromList, and is called with all the associated information
about a particular timing mode for a display. The information copied to the <userData>
record is the bit depth, depth mode, timing resolution (horizontal and vertical),
the timing mode, and the VDSwitchInfoRec.

Boolean FindBestMatch (	VideoRequestRecPtr requestRecPtr,
				short bitDepth,
				unsigned long horizontal,
				unsigned long vertical);

FindBestMatch is used to find the best fit of the requested video settings to
the available video settings. Given a request record, FindBestMatch uses the
<bitDepth>, <horizontal>, and <vertical> values to determine if they are better
or equal to the current match. Input values are set prior to calling
RVRequestVideoSetting. Output values are set within the call to RVRequestVideoSetting.
The video request record looks like the following:

struct VideoRequestRec	{
	GDHandle		screenDevice;
	short			reqBitDepth;
	short			availBitDepth;
	unsigned long	reqHorizontal;
	unsigned long	reqVertical;
	unsigned long	availHorizontal;
	unsigned long	availVertical;
	unsigned long	requestFlags;
	unsigned long	displayMode;
	unsigned long	depthMode;
	VDSwitchInfoRec	switchInfo;
};

where the fields are defined as follows:

screenDevice: Is an input/output field.
Input definition: The screenDevice field is used to specify two types of
searches. If the screenDevice is set to a active GDHandle, then only that
device will be looked at to match the requested video setting. For example,
if you pass in the value returned from GetMainDevice, the only the main
screen will be looked at for a possible match. If the screenDevice is set
to <nil>, then all active GDevices will be looked at to make the best
match of the requested video setting.
Output definition: the screenDevice field is set to the GDevice which is
capable of best matching the requested video setting.

reqBitDepth: Is an input field.
Input definition: The reqBitDepth field is the requested bit depth for the
new video setting. Appropriate values are 1, 2, 4, 8, 16, 32.

availBitDepth: Is an output field.
Output definition: The availBitDepth field is the bit depth that most closely
matches the requested bit depth.

reqHorizontal: Is an input field.
Input definition: The reqHorizontal field is the requested horizontal resolution
for the new video setting. To set to a 14" monitor, for example, a value of 640
would be appropriate.

reqVertical: Is an input field.
Input definition: The reqVertical field is the requested vertical resolution
for the new video setting. To set to a 14" monitor, for example, a value of 480
would be appropriate.

availHorizontal: Is an output field.
Output definition: The availHorizontal field is the horizontal resolution that
most closely matches the requested resolution.

availVertical: Is an output field.
Output definition: The availVertical field is the vertical resolution that most
closely matches the requested resolution.

displayMode: Is an output field.
Output definition: The displayMode is the value used by the system to produce
the horizontal/vertical resolution set in the availHorizontal/availVertical fields.

depthMode: Is an output field.
Output definition: The depthMode is the value used by the system to produce
the bit depth set in the availBitDepth field.

switchInfo: Is an output field.
Output definition: The switchInfo is a record used by the Display Manager 2.0
as an alternate way to set the displayMode/depthMode of a GDevice.

requestFlags: Is an input field.
Input definition: The requestFlags is a bit field that helps to shape the best
fit search of the new video setting. The requestFlags are defined as follows:

enum {
	kBitDepthPriorityBit	= 0,
	kAbsoluteRequestBit	= 1,
	kShallowDepthBit		= 2,
	kMaximizeResBit		= 3
};

kBitDepthPriorityBit: Should be set to force a best fit depth resolution over a
best fit screen resolution since there may be cases where the best screen resolution
match does not allow for the specified bit depth.

kAbsoluteRequestBit: Should be set to force an exact match of the requested video setting.

kShallowDepthBit: Should be set to match the bet depth less than or equal to the
requested bit depth since there may be cases where the available best fit bit depth
is greater than the requested depth.

kMaximizeResBit: Should be set to match the screen resolution equal to or greater
than the requested resolution since there may be cases where the available best
fit screen resolution is less than the requested resolution.

When the requestFlags is set to zero, the default search algorithm of the
FindBestMatch routine is used. This search is a combination of minimizing
horizontal and vertical deltas off of the requested values while at the same
time finding an equal or greater matching bit depth at. When any combination
of requestFlags bits are set, the search behaves in an appropriate fashion.
For example, to search with bit depth priority and maximize screen resolution,
requestFlags would be set to (1<<kBitDepthPriorityBit) + (1<<kMaximizeResBit).


void GravitateMonitors (void);

GravitateMonitors is not currently implemented. This code is to be called after
the Display Manager call DMSetDisplayMode only if Display Manager 1.0 is running
since Display Manager 2.0 does this work for us. The problem to be solved ocurs
only in multimonitor situations where it is possibe to create a disjoint desktop
region. For example, prior to the call to DMSetDisplayMode, two monitors are sitting
side by side. We then set the leftmost monitor to a smaller screen resolution which
forces the bottom right side of the monitor to be moved up and to the left. This
results is a disjoint desktop since the left side of the rightmost monitor is no
longer "attached" to the right side of the leftmost monitor. This problem can be
exagerated by the use of other monitors positioned in odd relative locations to each
other.

************************************************************************************************/

#include <QuickDraw.h>
#include <Video.h>

// requestFlags bit values in VideoRequestRec (use: 1<<kAbsoluteRequestBit)
enum {
	kBitDepthPriorityBit		= 0,	// Bit depth setting has priority over resolution
	kAbsoluteRequestBit			= 1,	// Available setting must match request
	kShallowDepthBit			= 2,	// Match bit depth less than or equal to request
	kMaximizeResBit				= 3		// Match screen resolution greater than or equal to request
};

// video request structure
struct VideoRequestRec	{
	GDHandle		screenDevice;		// <in/out>	nil will force search of best device, otherwise search this device only
	short			reqBitDepth;		// <in>		requested bit depth
	short			availBitDepth;		// <out>	available bit depth
	unsigned long	reqHorizontal;		// <in>		requested horizontal resolution
	unsigned long	reqVertical;		// <in>		requested vertical resolution
	unsigned long	availHorizontal;	// <out>	available horizontal resolution
	unsigned long	availVertical;		// <out>	available vertical resolution
	unsigned long	requestFlags;		// <in>		request flags
	unsigned long	displayMode;		// <out>	mode used to set the screen resolution
	unsigned long	depthMode;			// <out>	mode used to set the depth
	VDSwitchInfoRec	switchInfo;			// <out>	DM2.0 uses this rather than displayMode/depthMode combo
};
typedef struct VideoRequestRec VideoRequestRec;
typedef struct VideoRequestRec *VideoRequestRecPtr;

// Routine defines
OSErr RVRequestVideoSetting(VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoRequest (VideoRequestRecPtr requestRecPtr);
OSErr RVSetVideoAsScreenPrefs (void);

