#include "tiling.h"
#include "error.h"
#include "debug.h"
#include <Windows.h>

HWND focusedWindow = 0;
HWND managed[256];
int numOfManagedWindows = 0;
int currentFocusedWindowIndex = 0;

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lparam)
{
	if (numOfManagedWindows > 255) {
		return FALSE;
	}

	if (!IsWindowVisible(hwnd) || IsHungAppWindow(hwnd)) {
		return TRUE;
	}

	WINDOWINFO winInfo;
	winInfo.cbSize = sizeof(WINDOWINFO);
	if (!GetWindowInfo(hwnd, &winInfo)) {
		return TRUE;
	}

	if (winInfo.dwStyle & WS_POPUP) {
		return TRUE;
	}

	if (!(winInfo.dwExStyle & 0x20000000)) {
		return TRUE;
	}

	if (GetWindowTextLengthW(hwnd) == 0) {
		return TRUE;
	}

	RECT clientRect;
	if (!GetClientRect(hwnd, &clientRect)) {
		return TRUE;
	}

	// Skip small windows to avoid bugs
	if (clientRect.right < 100 || clientRect.bottom < 100){
		return TRUE;
	}

	// temp
	char theTitle[256] = {0};
	GetWindowTextA(hwnd, theTitle, 256);
	DEBUG_PRINT("title: %s\n", theTitle);
	DEBUG_PRINT("style: %x\n", winInfo.dwExStyle);

	managed[numOfManagedWindows] = hwnd;
	numOfManagedWindows++;
	return TRUE;
}

void tileWindows()
{
	numOfManagedWindows = 0;

	if (focusedWindow == 0) {
		EnumChildWindows(GetDesktopWindow(), EnumChildProc, 0);
	} else {
		managed[numOfManagedWindows] = focusedWindow;
		numOfManagedWindows++;
	}

	if (numOfManagedWindows == 0) {
		return;
	}

	TileWindows(GetDesktopWindow(), MDITILE_VERTICAL | MDITILE_SKIPDISABLED, NULL, numOfManagedWindows, managed);
}

void toggleFocusedWindow(HWND hwnd)
{
	if (focusedWindow != 0) {
		focusedWindow = 0;
	} else {
		focusedWindow = hwnd;
	}

	tileWindows();
}

void focusNextWindow(bool goBack)
{
	currentFocusedWindowIndex += goBack ? -1 : 1;

	if (currentFocusedWindowIndex < 0) {
		currentFocusedWindowIndex = numOfManagedWindows - 1;
	} else if (currentFocusedWindowIndex == numOfManagedWindows) {
		currentFocusedWindowIndex = 0;
	}

	SwitchToThisWindow(managed[currentFocusedWindowIndex], FALSE);
}
