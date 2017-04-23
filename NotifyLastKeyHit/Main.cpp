
#include "Main.h"
#include "KeyChecker.h"
#include "UserNotifier.h"

const std::wstring Main::INI_FILE(L"settings.ini");
const std::wstring Main::INVALID_KEY_VALUE(L"__INVALID_VALUE__");
const int Main::INI_READ_BUFF_LEN = 2048;
Main Main::instance;

void showException(const std::exception& e) {
	MessageBox(NULL, Main::convertToWString(e.what()).c_str(), L"ERROR", MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

Main::Main() {
	shutdownRequested = false;
	std::unique_ptr<wchar_t[]> ownPath(new wchar_t[2048]);
	GetCurrentDirectory(2047, ownPath.get());
	iniPath.assign(ownPath.get());
	iniPath.append(L"\\");
	iniPath.append(INI_FILE);
}

std::wstring Main::convertToWString(const char* str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}
std::string Main::convertToString(const wchar_t* str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(str);
}

std::wstring Main::readStringFromIni(const std::wstring& section, const std::wstring& key) {
	std::unique_ptr<wchar_t[]> buff(new wchar_t[INI_READ_BUFF_LEN]);
	GetPrivateProfileString(section.c_str(), key.c_str(), INVALID_KEY_VALUE.c_str(), buff.get(), INI_READ_BUFF_LEN - 1, iniPath.c_str());
	std::wstring ret(buff.get());
	if (ret.compare(INVALID_KEY_VALUE) == 0) {
		throw std::runtime_error("Cant read key " + convertToString(section.c_str()) + "." + convertToString(key.c_str()) + " from ini file "
			+ convertToString(iniPath.c_str()));
	}
	return ret;
}
int64_t Main::readIntFromIni(const std::wstring& section, const std::wstring& key) {
	std::wstring str = readStringFromIni(section, key);
	return std::stoll(str);
}
void Main::WriteStringToIni(const std::wstring& section, const std::wstring& key, const std::wstring& str) {
	WritePrivateProfileString(section.c_str(), key.c_str(), str.c_str(), iniPath.c_str());
}
void Main::WriteIntToIni(const std::wstring& section, const std::wstring& key, const int64_t value) {
	WriteStringToIni(section, key, std::to_wstring(value));
}

void Main::startCheckKeyThread() {
	try {
		KeyChecker::getInstance().run();
	}
	catch (std::exception& e) {
		showException(e);
	}
}

void Main::startNotifyThread() {
	try {
		UserNotifier().run();
	}
	catch (std::exception& e) {
		showException(e);
	}
}

int Main::main() {
	notifyAfterTimeWithoutKeyHit = std::chrono::seconds(readIntFromIni(L"Settings", L"NotifyAfter"));
	lastKeyHit = std::chrono::system_clock::from_time_t(readIntFromIni(L"Data", L"LastKeyHit"));
	hideNotificationAfter = std::chrono::seconds(readIntFromIni(L"Settings", L"HideNotificationAfter"));
	std::thread notifyThread(startNotifyThread);
	std::thread checkKeyThread(startCheckKeyThread);
	notifyThread.join();
	checkKeyThread.join();
	return 0;
}

std::chrono::system_clock::time_point Main::getLastKeyHit() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	return lastKeyHit;
}

void Main::setLastKeyHitToNow() {
	std::lock_guard<std::recursive_mutex> lock(mutex);
	lastKeyHit = std::chrono::system_clock::now();
}

void Main::writeLastKeyHitToIni() {
	WriteIntToIni(L"Data", L"LastKeyHit", std::chrono::system_clock::to_time_t(lastKeyHit));
}

void Main::showError(const wchar_t* text, const DWORD errorCode, bool doAbort) {
	std::unique_ptr<wchar_t[]> buff(new wchar_t[8192]);

	DWORD errorInError = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buff.get(), 8191, NULL);

	std::wstring out(text);
	out += L": ";
	if (errorInError == 0)
	{
		out += L"Error ";
		out += std::to_wstring(errorCode);
		out += L" occured. While formatting this error, another error";
		out += std::to_wstring(GetLastError()) + L" occured.";
	}
	else {
		out.append(buff.get());
	}
	MessageBox(NULL, out.c_str(), L"ERROR", MB_OK | MB_ICONERROR | MB_TASKMODAL);

	if (doAbort) {
		::abort();
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	try {
		return Main::getInstance().main();
	}
	catch (std::exception& e) {
		showException(e);
		return 1;
	}
}
