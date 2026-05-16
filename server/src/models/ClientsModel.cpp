#include "server/models/ClientsModel.h"

namespace gs::server::models {

gs::protocol::ClientSettings ClientItem::ToClientSettings() const
{
    gs::protocol::ClientSettings res;
    res.exec_mode = exec_mode;
    res.target = target;
    res.running_state = running_state;
    res.ping_timeout_ms = ping_timeout_ms;
    res.metrics_report_interval_s = metrics_report_interval_s;
    return res;
}

void ClientItem::UpdateClientSettings(const gs::protocol::ClientSettings & settings) {
    exec_mode = settings.exec_mode;
    target = settings.target;
    running_state = settings.running_state;
    ping_timeout_ms = settings.ping_timeout_ms;
    metrics_report_interval_s = settings.metrics_report_interval_s;
}

ClientsModel::ClientsModel(QObject *parent)
    : QAbstractListModel(parent)
{}

int ClientsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_clients.size();
}

// QML calls this for each cell to get value
QVariant ClientsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_clients.size()) {
        return QVariant();
    }

    const ClientItem &client = m_clients.at(index.row());

    switch (role) {
    case IdRole:        return client.client_id;
    case NameRole:      return client.client_name;
    case TargetRole:    return client.target;
    case TokenRole:     return client.token;

    case ExecModeRole:  return static_cast<int>(client.exec_mode);
    case ConnRole:      return static_cast<int>(client.conn_state);
    case RunRole:       return static_cast<int>(client.running_state);

    case TimeoutRole:   return static_cast<int>(client.ping_timeout_ms);
    case MetricsIntervalRole:
                        return static_cast<int>(client.metrics_report_interval_s);

    case RoundtripRole: return client.avg_roundtrip_ms;
    case JitterRole:    return client.avg_jitter_ms;
    case LossPercRole:  return client.loss_percentage;
    default:            return QVariant();
    }
}

// Bind Enum with text naming for QML
QHash<int, QByteArray> ClientsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole]       = "client_id";
    roles[NameRole]     = "client_name";
    roles[TargetRole]   = "target";
    roles[TokenRole]    = "token";

    roles[ExecModeRole] = "exec_mode";
    roles[ConnRole]     = "conn_state";
    roles[RunRole]      = "running_state";

    roles[TimeoutRole]  = "ping_timeout_ms";
    roles[MetricsIntervalRole]
                        = "metrics_report_interval_s";

    roles[RoundtripRole]= "avg_roundtrip_ms";
    roles[JitterRole]   = "avg_jitter_ms";
    roles[LossPercRole] = "loss_percentage";
    return roles;
}

std::optional<ClientItem> ClientsModel::GetClient(uint32_t client_id) const
{
    const int idx = FindClientIndex(client_id);
    if (idx >= 0) {
        return m_clients.at(idx);
    }
    return std::nullopt;
}

void ClientsModel::AddOrUpdateClient(const ClientItem& client)
{
    const int idx = FindClientIndex(client.client_id);
    if (idx >= 0) {
        m_clients[idx] = client;
        emit dataChanged(index(idx), index(idx)); // Ask QML to redraw line
    } else {
        const int new_client_idx = m_clients.size();

        beginInsertRows(QModelIndex(), m_clients.size(), m_clients.size());
        m_clients.append(client);
        m_client_id_to_index.emplace(client.client_id, new_client_idx);
        endInsertRows();
    }
}

void ClientsModel::SetConnectionState(uint32_t client_id,
                                      gs::enums::ConnectionState state)
{
    const int idx = FindClientIndex(client_id);
    if (idx >= 0) {
        m_clients[idx].conn_state = state;
        emit dataChanged(index(idx), index(idx), {ConnRole}); // Redraw ConnRole cell
    }
}

void ClientsModel::UpdateMetrics(uint32_t client_id, double roundtrip_ms,
                                 double jitter_ms, double loss_percentage)
{
    const int idx = FindClientIndex(client_id);
    if (idx >= 0) {
        m_clients[idx].avg_roundtrip_ms = roundtrip_ms;
        m_clients[idx].avg_jitter_ms = jitter_ms;
        m_clients[idx].loss_percentage = loss_percentage;          // Redraw 3 cells
        emit dataChanged(index(idx), index(idx), {RoundtripRole, JitterRole, LossPercRole});
    }
}

void ClientsModel::RemoveClient(uint32_t client_id)
{
    const int idx = FindClientIndex(client_id);
    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_clients.removeAt(idx);
        m_client_id_to_index.remove(client_id);

        // Shift right half of indices
        for (qsizetype i = idx; i < m_clients.size(); ++i) {
            m_client_id_to_index[m_clients[i].client_id] = i;
        }
        endRemoveRows();
    }
}

int ClientsModel::FindClientIndex(uint32_t client_id) const
{
    return m_client_id_to_index.value(client_id, -1);
}

} // namespace gs::server::models
