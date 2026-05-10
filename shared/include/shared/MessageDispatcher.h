#pragma once

#include "shared/JsonSerializer.h"
#include "shared/Enums.h"
#include "shared/protocol/FunctionalLayer.h"
#include "shared/protocol/SessionLayer.h"

#include <QJsonObject>
#include <QJsonValue>
#include <variant>
#include <QDebug>

namespace gs::helpers {

// 1. Full list of all messages
using AllMessages = std::variant<
    gs::protocol::Heartbeat,
    gs::protocol::Log,

    gs::protocol::LoginRequest,
    gs::protocol::ChallengeRequest,
    gs::protocol::ChallengeResponse,
    gs::protocol::LoginResponse,

    gs::protocol::NetworkMetricsReport,

    gs::protocol::StatusGetRequest,
    gs::protocol::StatusGetResponse,
    gs::protocol::StatusSetRequest,
    gs::protocol::StatusSetResponse

    // Add all new message types here too!
    >;

class MessageDispatcher {
public:
    // Dispatch: Json -> visitor(msg_type_instance);
    template <typename Visitor>
    static void Dispatch(const QJsonObject& json, Visitor&& visitor) {

        // 1. Have we header?
        const QJsonValue header_val = json.value("header");
        if (!header_val.isObject()){
            qWarning() << "Dispatcher Drop: 'header' object is missing.";
            return;
        }

        // 2. Extract MsgType as numeric ID
        const int type_id_int = header_val.toObject().value("type").toInt(-1);
        if (type_id_int == -1 || type_id_int == static_cast<int>(enums::MsgType::Unknown)) {
            qWarning() << "Dispatcher Drop: Unknown or missing MsgType.";
            return;
        }

        // 3. Cast numeric to enum
        const enums::MsgType target_type = static_cast<enums::MsgType>(type_id_int);

        // 4. Compile-time iterating of AllMessages
        if (!IterateAndDispatch<AllMessages>(target_type, json, visitor)) {
            qWarning() << "Dispatcher Drop: Unhandled message type ID:" << type_id_int;
        }
    }

private:
    // MsgType iter recursion
    template <typename Variant, size_t I = 0, typename Visitor>
    static bool IterateAndDispatch(enums::MsgType target_type, const QJsonObject& json, Visitor& visitor)
    {
        if constexpr (I < std::variant_size_v<Variant>)
        {
            // Obtain exact MsgType from template/std::variant
            using MsgType = std::variant_alternative_t<I, Variant>;

            // Is MsgType-enum from JSON equal to compile-time MsgType-enum of specific struct?
            if (target_type == MsgType::TYPE_ID) {
                // Found? Parse!
                const std::optional<MsgType> msg_opt = JsonSerializer::FromJson<MsgType>(json);
                if (msg_opt.has_value()) {
                    // Call handling lambda
                    visitor(msg_opt.value());
                } else {
                    qWarning() << "Dispatcher Error: Failed to parse valid structure for type ID:"
                               << static_cast<int>(target_type);
                }
                return true;
            }

            // No? Check next combination (тоже внутри блока if constexpr!)
            return IterateAndDispatch<Variant, I + 1>(target_type, json, visitor);
        }

        // Came to the end of dispatching
        return false;
    }
};

} // namespace gs::helpers
