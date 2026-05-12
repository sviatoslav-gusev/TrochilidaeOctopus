#pragma once
#include <QString>
#include <QCryptographicHash>

namespace gs::helpers {

class CryptoHelper {
public:
    // "Token + Nonce" -> SHA-256 hash
    static QString SolveChallenge(const QString& token, const QString& nonce) {
        const QByteArray data = (token + nonce).toUtf8();
        const QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
        return QString(hash.toHex());
    }
};

} // namespace gs::helpers
