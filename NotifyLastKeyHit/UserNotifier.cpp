#include "UserNotifier.h"
#include "resource.h"
#include <ctime>
#include <Strsafe.h>

UserNotifier::UserNotifier() {
	ZeroMemory(&iconData, sizeof(iconData));
	lastNotifyAfter = Main::getInstance().getLastKeyHit();
}

UserNotifier::~UserNotifier() {
	Main::getInstance().writeLastKeyHitToIni();
	Shell_NotifyIcon(NIM_DELETE, &iconData);
}


LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

HWND UserNotifier::createWindow() {
	const wchar_t className[] = L"HiddenWindowClass";
	WNDCLASSEX wc;
	HWND hwnd;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		Main::getInstance().showError(L"Register hidden window class failed", GetLastError());
	}

	hwnd = CreateWindowEx(0, className, L"Hidden Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 10, 10, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (hwnd == NULL) {
		Main::getInstance().showError(L"Show hidden window failed", GetLastError());
	}

	return hwnd;
}

void UserNotifier::run() {
	iconData.cbSize = sizeof(iconData);
	iconData.hWnd = createWindow();
	iconData.uFlags = NIF_INFO | NIF_ICON | NIF_TIP;
	iconData.uID = 1;
	iconData.uVersion = NOTIFYICON_VERSION;
	iconData.dwInfoFlags = NIF_INFO;
	iconData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	if (iconData.hIcon == NULL) {
		Main::getInstance().showError(L"Load icon failed", GetLastError());
	}
	::StringCchCopy(iconData.szTip, 63, L"Notifies about last key hits");

	if (FALSE == Shell_NotifyIcon(NIM_ADD, &iconData)) {
		Main::getInstance().showError(L"Failed to set notification icon", GetLastError());
	}
	if (FALSE == Shell_NotifyIcon(NIM_SETVERSION, &iconData)) {
		Main::getInstance().showError(L"Failed to set notification version", GetLastError());
	}
	iconData.uTimeout = (UINT)std::chrono::duration_cast<std::chrono::milliseconds>(Main::getInstance().getHideNotificationAfter()).count();

	MSG msg = { 0 };
	uint8_t numIterations = 0;
	while (!Main::getInstance().isShutdownRequested()) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				Main::getInstance().setShutdownRequested();
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		numIterations++;
		if (numIterations > 60) {
			Main::getInstance().writeLastKeyHitToIni();
			numIterations = 0;
		}

		checkForNotification();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	Main::getInstance().writeLastKeyHitToIni();
}

void UserNotifier::checkForNotification() {
	std::chrono::system_clock::time_point currentNotifyAfter = Main::getInstance().getLastKeyHit();
	std::chrono::system_clock::duration diff = currentNotifyAfter - lastNotifyAfter;
	if (diff >= Main::getInstance().getNotifyAfterTimeWithoutKeyHit()) {
		tm timeStruct;
		time_t timeValue = std::chrono::system_clock::to_time_t(lastNotifyAfter);
		localtime_s(&timeStruct, &timeValue);
		std::unique_ptr<char[]> buff(new char[64]);
		strftime(buff.get(), 64, "%H:%M:%S (%Y-%m-%d)", &timeStruct);
		std::wstring msg(L"Last key press was ");
		msg.append(Main::convertToWString(buff.get()));
		::StringCchCopy(iconData.szInfoTitle, 63, L"Last Key Press");
		::StringCchCopy(iconData.szInfo, 255, msg.c_str());
		::StringCchCopy(iconData.szTip, 63, msg.c_str());
		Shell_NotifyIcon(NIM_MODIFY, &iconData);
	}
	lastNotifyAfter = currentNotifyAfter;
}
