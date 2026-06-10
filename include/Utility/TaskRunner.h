#pragma once

#include <thread>
#include <functional>
#include <exception>

#include "Logger.h"

class TaskRunner
{
public:
    static void RunAsync(std::function<void()> task)
    {
        std::thread([task]() {
            try
            {
                task();
            }
            catch (const std::exception& e)
            {
                Logger::GetInstance().Log(Logger::Level::Error, "Unhandled exception in background thread: {}", e.what());
            }
            catch (...)
            {
                Logger::GetInstance().Log(Logger::Level::Error, "Unknown unhandled exception in background thread.");
            }
        }).detach();
    }
};
