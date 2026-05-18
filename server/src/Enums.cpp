#include "server/Enums.h"

namespace gs::enums {

qint64 ToIntervalSec(const MetricsResolution val) {
    using enum MetricsResolution;

    switch (val) {
    case Raw: return 1;
    case Minute: return 60;
    case TenMins: return 600;
    case Hour: return 3600;
    case Day: return 24*3600;
    case Week: return 7*24*3600;
    case Month: return 30*24*3600;
    }
    return 1;
}

} // namespace gs::enums
