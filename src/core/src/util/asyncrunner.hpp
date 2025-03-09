
#include <functional>

namespace conis::core {

class AsyncRunner {

public:
    // Runs the runFunction in a separate thread and executes the waitFunction at an interval of updatesPerSecond
    static void runAndWait(std::function<void()> runFunction, std::function<void()> waitFunction);

private:
    static const int updatesPerSecond = 30;
};

} // namespace conis::core
