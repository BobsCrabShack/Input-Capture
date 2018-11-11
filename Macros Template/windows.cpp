#include "Keys.h"
#include "RawInp.h"
#include "SimInp.h"
#include "Timer.h"
#include "InputHandler.h"
#include "CheckKey.h"
#include "RecordList.h"
#include "KeyComboRec.h"
#include "StringHashMap.h"
#include "IgnoreKeys.h"
#include "File.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <time.h>
#include <memory>
#include <assert.h>
#include <string>
#pragma comment(lib, "Winmm.lib")

TCHAR szWindowClass[] = _T("Macros");
TCHAR szTitle[] = _T("Macros");
UINT screenWidth = 600;
UINT screenHeight = 400;

HINSTANCE hInst;
HWND hWnd;
HWND textDisp;

const TCHAR Directory[] = _T("Records");

std::unique_ptr<RawInp> rawInput;

KeyComboRec comboRec;
RecordList recordList;
Ignorekeys ignoreKeys;
StringHashMap outStrings;

COLORREF bckClr = RGB(255, 255, 255);
COLORREF colorKey = bckClr;
BYTE alpha = 255;
HPEN hp = NULL;
HBRUSH hb = NULL;
HBRUSH clrb = NULL;
HFONT hf = NULL;
RECT winRect;

void MouseBIProc(const RAWMOUSE& mouse, DWORD delay);
void KbdBIProc(const RAWKEYBOARD& kbd, DWORD delay);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK KbdInputProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MouseInputProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL; //LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL; //LoadIcon(wcex.hInstance, IDI_APPLICATION);


	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), szTitle, NULL);
		return 1;
	}

	hInst = hInstance;

	RECT rc;
	GetWindowRect(GetDesktopWindow(), &rc);
	screenWidth = rc.right - rc.left;
	screenHeight = rc.bottom - rc.top;

	winRect.left = 0;
	winRect.right = screenWidth + winRect.left;
	winRect.top = 0;
	winRect.bottom = screenHeight + winRect.top;

	hWnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
		szWindowClass,
		szTitle,
		WS_POPUPWINDOW,
		0, 0,
		screenWidth, screenHeight,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	colorKey = GetSysColor(COLOR_WINDOW);

	SetLayeredWindowAttributes(hWnd, colorKey, alpha, LWA_COLORKEY | LWA_ALPHA);

	if (!hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), szTitle, NULL);
		return 1;
	}

	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);

	//EnableWindow(hWnd, FALSE);

	MSG msg = {};
	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (hf)
		DeleteFont(hf); 

	if (hp)
		DeletePen(hp);

	if (hb)
		DeleteBrush(hb);

	if (clrb)
		DeleteBrush(clrb);

	UnregisterClass(szWindowClass, hInst);

	return (int)msg.wParam;
}

void MouseBIProc(const RAWMOUSE& mouse, DWORD delay)
{
	if (recordList.IsRecording())
	{
		if (delay != 0)
		{
			InputData* ptr = recordList.GetBack();
			if (ptr)
			{
				if (auto dat = std::get_if<DelayData>(ptr))
					dat->AddDelay(delay);
				else
					recordList.AddEventToRecord<DelayData>(delay);
			}
		}

		if (mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			if ((bool)mouse.lLastX || (bool)mouse.lLastY)
				recordList.AddEventToRecord<MouseMoveData>(mouse.lLastX, mouse.lLastY, false);
		}
		else if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			recordList.AddEventToRecord<MouseMoveData>(mouse.lLastX, mouse.lLastY, true);
		}

		if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			recordList.AddEventToRecord<MouseClickData>(true, true, false, false);
		}
		else if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
		{
			recordList.AddEventToRecord<MouseClickData>(false, true, false, false);
		}

		if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
		{
			recordList.AddEventToRecord<MouseClickData>(true, false, true, false);
		}
		else if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
		{
			recordList.AddEventToRecord<MouseClickData>(false, false, true, false);
		}


		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			recordList.AddEventToRecord<MouseScrollData>((short)mouse.usButtonData / WHEEL_DELTA);
		}


		else if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
		{
			recordList.AddEventToRecord<MouseClickData>(true, false, false, true);
		}
		else if (mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
		{
			recordList.AddEventToRecord<MouseClickData>(false, false, false, true);
		}

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) //side button 1
		{
			recordList.AddEventToRecord<MouseXClickData>(true, true, false);
		}
		else if (mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
		{
			recordList.AddEventToRecord<MouseXClickData>(false, true, false);
		}

		if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)//side button 2
		{
			recordList.AddEventToRecord<MouseXClickData>(true, false, true);
		}
		else if (mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
		{
			recordList.AddEventToRecord<MouseXClickData>(false, false, true);
		}

		//inputTimer.StartWatch();
	}
}

void KbdBIProc(const RAWKEYBOARD& kbd, DWORD delay)
{
	//inputTimer.StopWatch();
	//const double time = inputTimer.GetTimeMilli();
	const int previousRecord = recordList.GetCurrentRecord();

	if (comboRec.GetRecordType() == KeyComboRec::RECORDING)
	{
		if (kbd.Message == WM_KEYDOWN)
		{
			comboRec.AddVKey(kbd.VKey);
			return;
		}
		else if (comboRec.HasRecorded())
		{
			comboRec.Stop();
			if (!recordList.AddRecord(comboRec.GetVKeys()))
			{
				comboRec.StartRecording();
				return;
			}

			outStrings.Lock();
			outStrings.RemoveStringNL(_T("Adding Record... waiting for key combination"));
			
			outStrings.RemoveStringNL(_T("Current Record = ") + std::to_string(previousRecord));

			outStrings.AddStringNL(_T("Current Record = ") + std::to_string(recordList.GetCurrentRecord()));
			outStrings.Unlock();

			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
			return;
		}
	}
	else if (comboRec.GetRecordType() == KeyComboRec::DELETING)
	{
		if (kbd.Message == WM_KEYDOWN)
		{
			comboRec.AddVKey(kbd.VKey);
			return;
		}
		else if (comboRec.HasRecorded())
		{
			comboRec.Stop();

			if (!recordList.DeleteRecord(comboRec.GetVKeys()))
			{
				return;
			}

			outStrings.Lock();
			outStrings.RemoveStringNL(_T("Deleting Record... waiting for key combination"));

			outStrings.RemoveStringNL(_T("Current Record = ") + std::to_string(previousRecord));

			outStrings.AddStringNL(_T("Current Record = ") + std::to_string(recordList.GetCurrentRecord()));
			outStrings.Unlock();

			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
			return;
		}
	}

	// Select Record
	if (CheckKey::VKComboDown(kbd, { VK_CONTROL, VK_F1 }))
	{
		if (recordList.IsRecording())
		{
			recordList.Save();

			outStrings.RemoveString(_T("Recording...."));
			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		}
		else if(recordList.GetCurrentRecord() != RecordList::INVALIDRECORD)
		{
			ignoreKeys.SetKeys({ { VK_CONTROL, WM_KEYUP, true }, { VK_F1, WM_KEYUP, true } });
			//Set starting mouse position
			POINT pt;
			GetCursorPos(&pt);
			int mx = (pt.x * USHRT_MAX) / screenWidth;
			int my = (pt.y * USHRT_MAX) / screenHeight;
			outStrings.AddString(_T("Recording...."));
			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

			recordList.StartRecording();
			//recordList.AddEventToRecord<MouseMoveData>(mx, my, true);

			//inputTimer.StartWatch();
		}
		return;
	}

	// Simulate Record
	if (CheckKey::VKComboDown(kbd, { VK_CONTROL, VK_F2 }))
	{
		if (recordList.HasRecorded() && !recordList.IsRecording())
		{
			outStrings.AddString(_T("Simulating Record..."));
			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

			recordList.SimulateRecord();

			outStrings.RemoveString(_T("Simulating Record..."));
			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		}
		return;
	}

	// Exit program
	if (CheckKey::VKComboDown(kbd, { VK_CONTROL, VK_ESCAPE })) // for some reason a VK_ESCAPE with WM_KEYDOWN is not triggered when ctrl is pressed...
	{
		if (!(recordList.IsRecording() || recordList.IsSimulating()))
		{
			PostMessage(::hWnd, WM_CLOSE, NULL, NULL);
		}
		return;
	}

	// Add Record
	if (CheckKey::VKComboDown(kbd, { VK_CONTROL, VK_MENU, keys.CharToVirtualKey(_T('A')) }))
	{
		outStrings.AddString(_T("Adding Record... waiting for key combination"));
		RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

		comboRec.StartRecording();
		return;
	}

	// Delete record
	if (CheckKey::VKComboDown(kbd, { VK_CONTROL, VK_MENU, keys.CharToVirtualKey(_T('D')) }))
	{
		outStrings.AddString(_T("Deleting Record... waiting for key combination"));
		RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);

		comboRec.StartDeleting();
		return;
	}

	if (recordList.SelectRecord(kbd) != -1)
	{
		if (previousRecord != recordList.GetCurrentRecord())
		{
			outStrings.Lock();
			if (previousRecord != -1)
				outStrings.RemoveStringNL(_T("Current Record = ") + std::to_string(previousRecord));

			outStrings.AddStringNL(_T("Current Record = ") + std::to_string(recordList.GetCurrentRecord()));
			outStrings.Unlock();

			RedrawWindow(::hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		}
	}

	if (recordList.IsRecording())
	{
		if (!ignoreKeys.KeyIgnored(kbd))
		{
			if (delay != 0)
			{
				InputData* ptr = recordList.GetBack();
				if (ptr)
				{
					if (auto dat = std::get_if<DelayData>(ptr))
						dat->AddDelay(delay);
					else
						recordList.AddEventToRecord<DelayData>(delay);
				}
			}
			recordList.AddEventToRecord<KbdData>(kbd.MakeCode, kbd.Flags == RI_KEY_MAKE, true);
		}

		//inputTimer.Reset();
		//inputTimer.StartWatch();
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_CREATE:
	{
		TIMECAPS tc = {};
		timeGetDevCaps(&tc, sizeof(TIMECAPS));

		MMRESULT res = timeBeginPeriod(tc.wPeriodMin);
		assert(res == TIMERR_NOERROR);

		if (!File::Exists(Directory))
			File::CreateFolder(Directory);
		File::SetCurDirectory(Directory);

		recordList.Initialize("");
		
		rawInput = std::make_unique<RawInp>(hInst, MouseBIProc, KbdBIProc);

		outStrings.AddString("| SELECT/TOGGLE_REC - CTRL + F1 | SIM -  CTRL + F2 | ADD - CTRL + MENU + A | DEL - CTRL + MENU + D | EXIT - CTRL + ESC |");

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC h = BeginPaint(hWnd, &ps);

		if (!hf)
			hf = CreateFont(-MulDiv(12, GetDeviceCaps(h, LOGPIXELSY), 72), 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_MODERN, _T("Arial"));
		SelectFont(h, hf);

		if (!hp)
			hp = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		SelectPen(h, hp);

		if (!hb)
			hb = CreateSolidBrush(bckClr);

		if (!clrb)
			clrb = CreateSolidBrush(RGB(255, 0, 255));

		SelectBrush(h, hb);

		SetBkColor(h, bckClr);
		SetTextColor(h, RGB(0, 0, 0));
		SetTextCharacterExtra(h, 1);

		FillRect(h, &ps.rcPaint, hb);

		int yPos = 60;

		/*std::string outStr = _T("Recording...");
		if (recordList.IsRecording())
		{
			TextOut(h, 0, yPos, outStr.c_str(), outStr.size());
			yPos += 30;
		}

		const int record = recordList.GetCurrentRecord();
		outStr = (record != -1) ? std::string(_T("Current Record = ")) + std::to_string(record) : _T("Invalid Record");
		TextOut(h, 0, yPos, outStr.c_str(), outStr.size());
		yPos += 30;

		if (recordList.IsSimulating())
		{
			outStr = _T("Simulating...");
			TextOut(h, 0, yPos, outStr.c_str(), outStr.size());
			yPos += 30;
		}*/

		auto& strings = outStrings.GetOutStrings();
		outStrings.Lock();
		for (auto& it : strings)
		{
			TextOut(h, 0, yPos, it.first.c_str(), it.first.size());
			yPos += 30;
		}
		outStrings.Unlock();

		SelectBrush(h, clrb);

		EndPaint(hWnd, &ps);
	}	break;

	case WM_DESTROY:
		timeEndPeriod(1);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
