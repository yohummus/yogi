#include "time.hh"


namespace helpers {

std::chrono::milliseconds float_to_timeout(float val)
{
    if (val <= 0.0) {
        return std::chrono::milliseconds::max();
    }
    else {
        return std::chrono::milliseconds(static_cast<int>(val * 1000));
    }
}

} // namespace helpers
