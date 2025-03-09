
#include "asyncrunner.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

namespace conis::core {

void AsyncRunner::runAndWait(std::function<void()> runFunction, std::function<void()> waitFunction) {
    std::future<void> result = std::async(std::launch::async, runFunction);
    auto targetFrameTime = std::chrono::milliseconds(1000 / AsyncRunner::updatesPerSecond);

    while (result.wait_for(targetFrameTime) != std::future_status::ready) {
        auto start = std::chrono::high_resolution_clock::now();
        waitFunction();
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate time taken for wait function
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto elapsedTime = duration.count();

        // Calculate the remaining time to maintain consistent FPS
        auto remainingTime = targetFrameTime - std::chrono::milliseconds(elapsedTime);
        if (remainingTime > std::chrono::milliseconds(0)) {
            std::this_thread::sleep_for(remainingTime);
        }
    }
    waitFunction(); // Ensure wait is also called at the end to propegate any last updates
}

} // namespace conis::core
