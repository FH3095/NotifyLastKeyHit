#pragma once

#include "Main.h"

class KeyChecker
{
private:
	static LRESULT CALLBACK keyboardProc(int code, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK mouseProc(int code, WPARAM wParam, LPARAM lParam);
	static KeyChecker instance;
	KeyChecker();
	HHOOK keyboardHook;
	HHOOK mouseHook;
public:
	static inline KeyChecker& getInstance() {
		return instance;
	}
	virtual ~KeyChecker();
	void run();
};
