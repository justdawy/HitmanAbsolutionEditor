#pragma once
#include <string>
class Settings
{
public:
	static Settings& GetInstance();
	void LoadSettings();
	void SaveSettings();
	const bool IniFileHasKey(const std::string& section, const std::string& key);
	void GetValueFromIniFile(const std::string& section, const std::string& key, std::string& outputValue);
	void UpdateIniFile(const std::string& key, const std::string& value);
	const std::string GetRuntimeFolderPath() const;
	void SetRuntimeFolderPath(const std::string& runtimeFolderPath);
	const bool GetEnableDiscordRPC() const;
	void SetEnableDiscordRPC(const bool enableDiscordRPC);
	const std::string GetBackgroundImagePath() const;
	void SetBackgroundImagePath(const std::string& backgroundImagePath);
private:
	Settings() = default;
	Settings(const Settings& other) = delete;
	Settings& operator=(const Settings& other) = delete;
	std::string runtimeFolderPath;
	bool enableDiscordRPC = true;
	std::string backgroundImagePath = "assets/images/background.jpg";
};