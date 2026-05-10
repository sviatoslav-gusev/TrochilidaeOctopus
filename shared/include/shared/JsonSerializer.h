#pragma once

#include <QJsonObject>
#include <QMetaProperty>

#include <optional>

namespace gs::helpers {

class JsonSerializer
{

/**
 * Recursive gadget serialization helper
 * meta_obj - msg description
 * gadget_ptr - ptr to protocol message class-gadget
 */
static QJsonObject SerializeGadget(const QMetaObject* meta_obj,
                                   const void* const gadget_ptr);

/**
 * Recursive gadget deserialization helper
 */
static bool DeserializeGadget(const QMetaObject* const meta_obj,
                              void* const gadget_ptr,
                              const QJsonObject& json);

public:

/**
 * Convert protocol message class-gadget to QJsonObject
 */
template <typename T>
static QJsonObject ToJson(const T& obj) {
    // T::staticMetaObject gens MOC for every Q_GADGET
    return SerializeGadget(&T::staticMetaObject, &obj);
}

/**
 * Extract message class-gadget from QJsonObject.
 */
template <typename T>
static std::optional<T> FromJson(const QJsonObject& json) {
    T obj;

    if (DeserializeGadget(&T::staticMetaObject, &obj, json)) {
        return obj;
    }
    return std::nullopt;
}

};

} // namespace gs::helpers
