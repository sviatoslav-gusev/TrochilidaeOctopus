#pragma once

#include <qtypes.h>

namespace gs::enums {

enum class MetricsResolution{
    Raw = 0,
    Minute,
    TenMins,
    Hour,
    Day,
    Week,
    Month
};

qint64 ToIntervalSec(const MetricsResolution val);

} // namespace gs::enums
