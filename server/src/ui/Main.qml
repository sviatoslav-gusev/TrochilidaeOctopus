import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import gs.OctopusServer 1.0

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: 1000
    width: 1000
    height: 650
    title: "Octopus Server Community Edition"

    Material.theme: Material.Dark
    Material.accent: Material.Teal
    color: "#1e1e1e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // --- Header panel with buttons ---
        RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Button {
                        text: "Add Client"
                        Layout.preferredHeight: 36
                        contentItem: Label { text: parent.text; color: "white";
                            font.bold: true; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle {
                            color: parent.down ? "#222224" : (parent.hovered ? "#3e3e42" : "#2d2d30")
                            border.color: "#434346"
                            radius: 0 // СТРОГИЙ НУЛЕВОЙ РАДИУС
                        }
                        onClicked: editorDialog.openEditor({})
                    }
                    Button {
                        text: "Mass Start"
                        Layout.preferredHeight: 36
                        contentItem: Label { text: parent.text; color: "white";
                            font.bold: true; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle {
                            color: parent.down ? "#1b5e20" : (parent.hovered ? "#2e7d32" : "#1b5e20")
                            radius: 0
                        }
                        onClicked: appServer.massStart()
                    }
                    Button {
                        text: "Mass Stop"
                        Layout.preferredHeight: 36
                        contentItem: Label { text: parent.text; color: "white";
                            font.bold: true; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle {
                            color: parent.down ? "#b71c1c" : (parent.hovered ? "#c62828" : "#b71c1c")
                            radius: 0
                        }
                        onClicked: appServer.massStop()
                    }
                    Item { Layout.fillWidth: true }
                }

        // --- Tab header ---
        Rectangle {
            Layout.fillWidth: true
            height: 35
            color: "#252526"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 10

                Label { text: "ID"; font.bold: true; Layout.preferredWidth: 50 }
                Label { text: "Client name"; font.bold: true; Layout.preferredWidth: 150 }
                Label { text: "Ping target"; font.bold: true; Layout.preferredWidth: 130 }
                Label { text: "Exec mode"; font.bold: true; Layout.preferredWidth: 110 }
                Label { text: "Run state"; font.bold: true; Layout.preferredWidth: 110 }
                Label { text: "Connection"; font.bold: true; Layout.preferredWidth: 110 }
                Label { text: "Current RTT"; font.bold: true; Layout.fillWidth: true }
            }
        }

        // --- CLIENTS TABLE ---
        ListView {
            id: clientsListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: clientsModel
            spacing: 6
            clip: true

            delegate: Rectangle {
                id: rowContainer
                width: clientsListView.width

                // Dynamic expansion height
                height: isExpanded ? 220 : 45
                color: index % 2 === 0 ? "#1a1a1a" : "#222224"
                radius: 4
                border.color: isExpanded ? Material.accentColor : "transparent"
                border.width: isExpanded ? 1 : 0

                // Inner line state
                property bool isExpanded: false
                property var historyData: null

                // Refreshing history
                function refreshHistory() {
                    if (isExpanded) {
                        rowContainer.historyData = appServer.getClientSummaryMetrics(model.client_id)
                    }
                }

                // Smooth line expansion
                Behavior on height {
                    NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
                }

                // REALTIME UPDATES (on opened panel)
                Timer {
                    interval: 5000 // request updates each 5 sec
                    running: rowContainer.isExpanded
                    repeat: true
                    onTriggered: rowContainer.refreshHistory()
                }

                // Open/close expanded panel by mouse click
                MouseArea {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 45
                    cursorShape: Qt.PointingHandCursor

                    onClicked: {
                        rowContainer.isExpanded = !rowContainer.isExpanded
                        rowContainer.refreshHistory()
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    // --- CLIENT MAIN LINE ---
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 25
                        spacing: 10

                        Label { text: "#" + model.client_id; color: "#858585"; Layout.preferredWidth: 50 }
                        Label { text: model.client_name; font.bold: true; Layout.preferredWidth: 150 }
                        Label { text: model.target; Layout.preferredWidth: 130 }

                        Label {
                            text: model.exec_mode === 0 ? "Demo" : "ICMP"
                            Layout.preferredWidth: 110
                            color: "#00bcd4"
                        }

                        Label {
                            text: model.running_state === 0 ? "Stopped" : "Running"
                            Layout.preferredWidth: 110
                            color: "#00bcd4"
                        }

                        // Connection state (Online/Offline)
                        RowLayout {
                            Layout.preferredWidth: 110
                            spacing: 6
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                color: model.conn_state === 1 ? "#4caf50" : "#f44336"
                            }
                            Label {
                                text: model.conn_state === 1 ? "Online" : "Offline"
                                color: model.conn_state === 1 ? "#4caf50" : "#f44336"
                            }
                        }

                        // Live ping from model
                        Label {
                            text: model.avg_roundtrip_ms.toFixed(1) + " ms"
                            color: model.avg_roundtrip_ms > 100 ? "#ff9800" : "#d4d4d4"
                            font.pixelSize: 14
                            Layout.fillWidth: true
                        }

                        // Line edit button
                        Button {
                            text: "Edit"
                            Layout.preferredWidth: 60
                            Layout.preferredHeight: 28

                            contentItem: Label {
                                text: parent.text;
                                color: "#e0e0e0";
                                font.pixelSize: 12;
                                font.bold: true;
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Rectangle {
                                color: parent.down ? "#222224" : (parent.hovered ? "#434346" : "#333337")
                                border.color: "#555557"
                                radius: 0
                            }

                            onClicked: {
                                editorDialog.openEditor({
                                    "client_id": model.client_id,
                                    "client_name": model.client_name,
                                    "target": model.target,
                                    "token": model.token,
                                    "exec_mode": model.exec_mode,
                                    "running_state": model.running_state,
                                    "ping_timeout_ms": model.ping_timeout_ms,
                                    "metrics_report_interval_s": model.metrics_report_interval_s
                                })
                            }
                        }

                        // Open/close arrow
                        Label {
                            text: rowContainer.isExpanded ? "▲" : "▼"
                            color: "#858585"
                            font.pixelSize: 12
                            Layout.rightMargin: 10
                        }
                    }

                    // --- EXPANDED HISTPRY (GRID) ---
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#111112"
                        radius: 4
                        visible: rowContainer.height > 60 // Hide elems on closure
                        opacity: rowContainer.isExpanded ? 1.0 : 0.0

                        Behavior on opacity { NumberAnimation { duration: 150 } }

                        GridLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            columns: 5
                            rowSpacing: 6
                            columnSpacing: 20

                            // Title
                            Label { text: "Period"; font.bold: true; color: Material.accentColor }
                            Label { text: "Avg RTT"; font.bold: true; color: Material.accentColor }
                            Label { text: "Jitter"; font.bold: true; color: Material.accentColor }
                            Label { text: "Losses"; font.bold: true; color: Material.accentColor }
                            Label { text: "Packets sent"; font.bold: true; color: Material.accentColor }

                            Label { text: "Raw client probe"; font.bold: true }
                            Label { text: model.avg_roundtrip_ms.toFixed(1) + " ms" }
                            Label { text: model.avg_jitter_ms.toFixed(1) + " ms" }
                            Label { text: model.loss_percentage.toFixed(1) + "%" }
                            Label { text: model.sent_count }

                            Label { text: "Last 10 mins" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["10mins"]
                                          ? rowContainer.historyData["10mins"]["rtt"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["10mins"]
                                          ? rowContainer.historyData["10mins"]["jitter"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["10mins"]
                                          ? rowContainer.historyData["10mins"]["loss"].toFixed(1) + "%"
                                          : "0" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["10mins"]
                                          ? rowContainer.historyData["10mins"]["sent"]
                                          : "0" }

                            Label { text: "Last hour" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["hour"]
                                          ? rowContainer.historyData["hour"]["rtt"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["hour"]
                                          ? rowContainer.historyData["hour"]["jitter"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["hour"]
                                          ? rowContainer.historyData["hour"]["loss"].toFixed(1) + "%"
                                          : "0" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["hour"]
                                          ? rowContainer.historyData["hour"]["sent"]
                                          : "0" }

                            Label { text: "Last day" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["day"]
                                          ? rowContainer.historyData["day"]["rtt"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["day"]
                                          ? rowContainer.historyData["day"]["jitter"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["day"]
                                          ? rowContainer.historyData["day"]["loss"].toFixed(1) + "%"
                                          : "0" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["day"]
                                          ? rowContainer.historyData["day"]["sent"]
                                          : "0" }

                            Label { text: "Last week" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["week"]
                                          ? rowContainer.historyData["week"]["rtt"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["week"]
                                          ? rowContainer.historyData["week"]["jitter"].toFixed(1) + " ms"
                                          : "—" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["week"]
                                          ? rowContainer.historyData["week"]["loss"].toFixed(1) + "%"
                                          : "0" }
                            Label { text: rowContainer.historyData && rowContainer.historyData["week"]
                                          ? rowContainer.historyData["week"]["sent"]
                                          : "0" }
                        }
                    }
                }
            }
        }
    }

    ClientEditorDialog {
        id: editorDialog
    }
}
