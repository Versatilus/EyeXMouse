   /*
 * This is an adaptation of one of the SDK sample applications released by Tobii.
 * It allows for control of the mouse pointer through eye tracking if both eyes are being tracked.
 * It's only been tested with the 4C model of eye tracker.
 * The values of the variables below were chosen through trial and error. Your mileage may vary.
 * Most of the original sample code has been left intact, including comments.

 * Original code Copyright 2013-2014 Tobii Technology AB. All rights reserved.
 * Modifications Copyright 2017-2018 Eric Lewis Paulson (@Versatilus on GitHub)
 */

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <math.h>
#include "eyex\EyeX.h"

#pragma comment (lib, "Tobii.EyeX.Client.lib")

void HandleNewCoords(float x, float y);

float lastRawX = 0;
float lastRawY = 0;

BOOL g_BothEyesActive = FALSE;
#define HISTORY_LENGTH 4
float historyX[HISTORY_LENGTH];
float historyY[HISTORY_LENGTH];
float filteredX = 0;
float filteredY = 0;
const float FDeadZone = 20.0;  // pixels
const float g_SlowZone = 625; // pixels. This should probably be refactored to be a percentage of the screen. 
const float FSpeed = 0.25f; // how fast to move

// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "EyeXMouse Gaze Data";
static const TX_STRING EyePositionInteractorId = "EyeXMouse Eye Position Data";

// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;
static TX_HANDLE g_hGlobalEyePositionInteractorSnapshot = TX_EMPTY_HANDLE;

/*
 * Initializes g_hGlobalInteractorSnapshot with an interactor that has the Gaze Point behavior.
 */
BOOL InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_GAZEPOINTDATAPARAMS gazeParams = { TX_GAZEPOINTDATAMODE_UNFILTERED};
	// TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };
	TX_HANDLE hBehaviorWithoutParameters = TX_EMPTY_HANDLE;

	BOOL success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		EyePositionInteractorId,
		&g_hGlobalEyePositionInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateInteractorBehavior(hInteractor, &hBehaviorWithoutParameters, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK;

	txReleaseObject(&hInteractor);

	hInteractor = TX_EMPTY_HANDLE;
	success &= txCreateGlobalInteractorSnapshot(
		hContext,
		InteractorId,
		&g_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateGazePointDataBehavior(hInteractor, &gazeParams) == TX_RESULT_OK;

	txReleaseObject(&hInteractor);

	return success;
}

/*
 * Callback function invoked when a snapshot has been committed.
 */
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
	assert(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED);
}

/*
* Callback function invoked when the status of the connection to the EyeX Engine has changed.
*/
void TX_CALLCONVENTION OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	switch (connectionState) {
	case TX_CONNECTIONSTATE_CONNECTED: {
		BOOL success;
		printf("The connection state is now CONNECTED (We are connected to the EyeX Engine)\n");
		// commit the snapshot with the global interactor as soon as the connection to the engine is established.
		// (it cannot be done earlier because committing means "send to the engine".)
		success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
		success &= txCommitSnapshotAsync(g_hGlobalEyePositionInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
	}
		/*if (!success) {
			printf("Failed to initialize the data stream.\n");
		}
		else
		{
			printf("Waiting for gaze data to start streaming...\n");
		}
	}
		break;
	 
	case TX_CONNECTIONSTATE_DISCONNECTED:
		printf("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
		printf("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
		printf("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.\n");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		printf("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.\n");
		break;
	*/
	}
}


/*
 * Handles an event from the Gaze Point data stream.
 */
void OnGazeDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_GAZEPOINTDATAEVENTPARAMS gazeEventParams;
	
	if ((txGetGazePointDataEventParams(hGazeDataBehavior, &gazeEventParams) == TX_RESULT_OK)&& g_BothEyesActive) {
		HandleNewCoords((float)gazeEventParams.X, (float)gazeEventParams.Y);
	}
	
}
void OnEyePositionDataEvent(TX_HANDLE hEyePositionDataBehavior)
{
	TX_EYEPOSITIONDATAEVENTPARAMS eyePositionEventParams;

	if (txGetEyePositionDataEventParams(hEyePositionDataBehavior, &eyePositionEventParams) == TX_RESULT_OK) {
		g_BothEyesActive = eyePositionEventParams.HasLeftEyePosition && eyePositionEventParams.HasRightEyePosition;
	}

}

void HandleNewCoords(float x, float y)
{

	if (abs(lastRawX - x) + abs(lastRawY - y) > 50)
	{
		lastRawX = x;
		lastRawY = y;
		return;  //reject noise
	}

	float averageX = historyX[HISTORY_LENGTH - 1];
	float averageY = historyY[HISTORY_LENGTH - 1];
	for (int i = HISTORY_LENGTH - 2; i > -1; i--) {
		averageX += historyX[i];
		averageY += historyY[i];
		historyX[i + 1] = historyX[i];
		historyY[i + 1] = historyY[i];
	}
	if (!isnan(x + y) && !isinf(x + y)) {
		historyX[0] = x;
		historyY[0] = y;
		averageX = (averageX + x) * 1 / (HISTORY_LENGTH + 1);
		averageY = (averageY + y) * 1 / (HISTORY_LENGTH + 1);
		if (!isnan(averageX + averageY) && !isinf(averageX + averageY)) {
			historyX[HISTORY_LENGTH - 1] = averageX;
			historyY[HISTORY_LENGTH - 1] = averageY;
		}
	}
	float travelX = averageX - filteredX;
	float travelY = averageY - filteredY;

	float speedGradient = (travelX*travelX + travelY*travelY) / (g_SlowZone*g_SlowZone);
	float speed = FSpeed * speedGradient;
	


	if (abs(travelX) > FDeadZone){ filteredX += speed * travelX; }
	if (abs(travelY) > FDeadZone){ filteredY += speed * travelY; }

	SetCursorPos((int)filteredX, (int)filteredY);
}

/*
 * Callback function invoked when an event has been received from the EyeX Engine.
 */
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

    txGetAsyncDataContent(hAsyncData, &hEvent);
 
	// NOTE. Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));

	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK) {
		OnGazeDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}
	else if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_EYEPOSITIONDATA) == TX_RESULT_OK) {
		OnEyePositionDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
		//txReleaseContext(&hBehavior);
	}

	// NOTE since this is a very simple application with a single interactor and a single data stream, 
	// our event handling code can be very simple too. A more complex application would typically have to 
	// check for multiple behaviors and route events based on interactor IDs.

	txReleaseObject(&hEvent);
}

/*
 * Application entry point.
 */
int main(int argc, char* argv[])
{
	TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
	TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
	TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;
	BOOL success;
	POINT currentXY;


	if (GetCursorPos(&currentXY)) {
		for (int i = 0; i < HISTORY_LENGTH; i++) {
			historyX[i] = (float)currentXY.x;
			historyY[i] = (float)currentXY.y;
		}
		filteredX = lastRawX = (float)currentXY.x;
		filteredY = lastRawY = (float)currentXY.y;
	}


	// initialize and enable the context that is our link to the EyeX Engine.
	success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
	success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
	success &= InitializeGlobalInteractorSnapshot(hContext);
	success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged, NULL) == TX_RESULT_OK;
	success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
	success &= txEnableConnection(hContext) == TX_RESULT_OK;

	// let the events flow until a key is pressed.
	if (success) {

		//HIDE CONSOLE WINDOW
		ShowWindow(GetConsoleWindow(), SW_HIDE);
		_getch();

	}
	// disable and delete the context.
	txDisableConnection(hContext);
	txReleaseObject(&g_hGlobalInteractorSnapshot);
	txReleaseObject(&g_hGlobalEyePositionInteractorSnapshot);
	txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE);
	txReleaseContext(&hContext);

	return 0;
	
}
