#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <string>
#include <locale>
#include <codecvt>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include <atomic>

class Main
{
private:
	static const std::wstring INI_FILE;
	static const std::wstring INVALID_KEY_VALUE;
	static const int INI_READ_BUFF_LEN;
	static Main instance;
	std::wstring iniPath;
	std::recursive_mutex mutex;
	std::chrono::system_clock::duration notifyAfterTimeWithoutKeyHit;
	std::chrono::system_clock::duration hideNotificationAfter;
	std::chrono::system_clock::time_point lastKeyHit;
	std::atomic_bool shutdownRequested;
	Main();
public:
	inline static Main& getInstance() {
		return instance;
	}
	static std::wstring convertToWString(const char* str);
	static std::string convertToString(const wchar_t* str);
	std::wstring readStringFromIni(const std::wstring& section, const std::wstring& key);
	int64_t readIntFromIni(const std::wstring& section, const std::wstring& key);
	void WriteStringToIni(const std::wstring& section, const std::wstring& key, const std::wstring& str);
	void WriteIntToIni(const std::wstring& section, const std::wstring& key, const int64_t value);

	static void startCheckKeyThread();
	static void startNotifyThread();
	int main();

	std::chrono::system_clock::time_point getLastKeyHit();
	void setLastKeyHitToNow();
	void writeLastKeyHitToIni();

	void showError(const wchar_t* text, const DWORD errorCode, bool doAbort = true);

	inline bool isShutdownRequested() {
		return shutdownRequested.load();
	}
	inline void setShutdownRequested() {
		shutdownRequested = true;
	}

	inline std::chrono::system_clock::duration getNotifyAfterTimeWithoutKeyHit() {
		return notifyAfterTimeWithoutKeyHit;
	}

	inline std::chrono::system_clock::duration getHideNotificationAfter() {
		return hideNotificationAfter;
	}
};
