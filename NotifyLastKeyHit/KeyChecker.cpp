#include "KeyChecker.h"
#include <cstdlib>

KeyChecker KeyChecker::instance;

KeyChecker::KeyChecker() {
	keyboardHook = mouseHook = NULL;
}

KeyChecker::~KeyChecker() {
	if (keyboardHook != NULL) {
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = NULL;
	}
	if (mouseHook != NULL) {
		UnhookWindowsHookEx(mouseHook);
		mouseHook = NULL;
	}
}

LRESULT CALLBACK KeyChecker::keyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code == HC_ACTION && (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)) {
		KBDLLHOOKSTRUCT* keyValues = (KBDLLHOOKSTRUCT*)lParam;
		switch (keyValues->vkCode) {
		case VK_RETURN:
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
			Main::getInstance().setLastKeyHitToNow();
		}
	}
	return CallNextHookEx(getInstance().keyboardHook, code, wParam, lParam);
}

LRESULT CALLBACK KeyChecker::mouseProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code == HC_ACTION && (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP)) {
		Main::getInstance().setLastKeyHitToNow();
	}
	return CallNextHookEx(getInstance().mouseHook, code, wParam, lParam);
}

void KeyChecker::run() {
	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, NULL, NULL);
	if (keyboardHook == NULL) {
		Main::getInstance().showError(L"Hook Keyboard failed", GetLastError());
	}
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, NULL, NULL);
	if (mouseHook == NULL) {
		Main::getInstance().showError(L"Hook Mouse failed", GetLastError());
	}

	MSG msg = { 0 };
	while (!Main::getInstance().isShutdownRequested() && GetMessage(&msg, NULL, 0, 0) != 0) {
		// Hook works by inserting a message into our message-queue. So we have to use GetMessage and cant sleep for too long
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		std::this_thread::yield();
	}

	UnhookWindowsHookEx(keyboardHook);
	UnhookWindowsHookEx(mouseHook);
	keyboardHook = mouseHook = NULL;
}
