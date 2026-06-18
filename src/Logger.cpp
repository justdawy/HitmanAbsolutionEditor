#include <iomanip>
#include <sstream>
#include <time.h>
#include <chrono>
#include "Logger.h"
Logger::Logger()
{
	InitializeSRWLock(&srwLock);
}
Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}
const char* Logger::LevelToString(const Level level)
{
    switch (level)
    {
    case Level::Info:
        return "INFO: ";
    case Level::Warning:
        return "WARNING: ";
    case Level::Error:
        return "ERROR: ";
    default:
        return "";
    }
}
const char* Logger::TypeToString(const Type type)
{
    switch (type)
    {
    case Type::Editor:
        return "Editor";
    case Type::SDK:
        return "SDK";
    case Type::Pipe:
        return "Pipe";
    default:
        return "";
    }
}
std::vector<Logger::Message>& Logger::GetMessages()
{
    return messages;
}
void Logger::ClearMessage(const int id)
{
    for (auto it = messages.begin(); it != messages.end(); ++it)
    {
        if (it->id == id)
        {
            messages.erase(it);
            break;
        }
    }
}
void Logger::ClearAllMessages()
{
    messages.clear();
}
std::string Logger::GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    std::stringstream stringstream;
    localtime_s(&localTime, &time);
    stringstream << std::put_time(&localTime, "%H:%M:%S");
    return stringstream.str();
}
std::string Logger::GetLastError()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return std::string();
    }
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}