#pragma once

#include "Main.h"

class UserNotifier
{
private:
	static HWND createWindow();
	void checkForNotification();
	std::chrono::system_clock::time_point lastNotifyAfter;
	NOTIFYICONDATA iconData;
public:
	UserNotifier();
	virtual ~UserNotifier();
	void run();
};
