#include "shared/JsonSerializer.h"

#include <QJsonValue>
#include <QMetaType>
#include <QString>
#include <QVariant>

namespace gs::helpers {

QJsonObject JsonSerializer::SerializeGadget(const QMetaObject* meta_obj,
                                            const void* const gadget_ptr)
{
    QJsonObject json;

    for (int i = 0; i < meta_obj->propertyCount(); ++i)
    {
        const QMetaProperty prop = meta_obj->property(i);
        const QLatin1String key(prop.name()); // All varnames are latin, so QLatin1String.

        const QVariant value = prop.readOnGadget(gadget_ptr);

        const QMetaObject* const child_meta = value.metaType().metaObject();
        if (child_meta) {
            // Is value also gadget? Yes, call recursion
            json.insert(key, SerializeGadget(child_meta, value.constData()));
        }
        else {
            // No, it is plain value
            json.insert(key, QJsonValue::fromVariant(value));
        }
    }
    return json;
}

bool JsonSerializer::DeserializeGadget(const QMetaObject* const meta_obj,
                                       void* const gadget_ptr,
                                       const QJsonObject& json)
{
    for (int i = 0; i < meta_obj->propertyCount(); ++i)
    {
        const QMetaProperty prop = meta_obj->property(i);
        const QLatin1String key(prop.name());

        if (!json.contains(key)) {
            qWarning() << "Error: Lack of" << key << "field in" << meta_obj->className();
            return false;
        }

        const QJsonValue json_value = json.value(key);
        const QMetaObject* const child_meta = prop.metaType().metaObject();


        if (child_meta && json_value.isObject()) {
            // Is json_value recursive? Yes, call recursion

            // 1. Get gadget copy
            QVariant sub_gadget_variant = prop.readOnGadget(gadget_ptr);

            // 2. Fill data from json
            if (!DeserializeGadget(child_meta, sub_gadget_variant.data(), json_value.toObject())) {
                return false;
            }

            // 3. Write updated to the parent
            prop.writeOnGadget(gadget_ptr, sub_gadget_variant);
        }
        else {
            // No, it is plain value
            prop.writeOnGadget(gadget_ptr, json_value.toVariant());
        }
    }
    return true;
}

} // namespace gs::helpers
