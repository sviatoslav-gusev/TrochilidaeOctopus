import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Dialog {
    id: editorDialog
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: 450
    modal: true
    focus: true
    title: currentId === 0 ? "Create New Client" : "Edit Client #" + currentId

    // Inner window condition
    property int currentId: 0

    // If to pass empty {}, there will be creation mode
    function openEditor(clientData) {
        currentId = clientData.client_id || 0
        nameField.text = clientData.client_name || ""
        targetField.text = clientData.target || "127.0.0.1"
        tokenField.text = clientData.token || appServer.generateToken()
        execModeCombo.currentIndex = clientData.exec_mode !== undefined
                ? clientData.exec_mode
                : 0
        runStateCombo.currentIndex = clientData.running_state !== undefined
                ? clientData.running_state
                : 1
        timeoutSpin.value = clientData.ping_timeout_ms || 500
        intervalSpin.value = clientData.metrics_report_interval_s || 10
        open()
    }

    footer: DialogButtonBox {
        id: buttonBox

        standardButtons: Dialog.Save | Dialog.Cancel

        Button {
            text: "Delete Client"
            // Show only during edition, not new one
            visible: editorDialog.currentId !== 0

            Material.accent: Material.Red
            highlighted: true

            anchors.verticalCenter: parent.verticalCenter

            onClicked: {
                appServer.deleteClient(editorDialog.currentId)
                // Closing window
                editorDialog.reject()
            }
        }
    }

    // Collect data and send to C++ via "Save"
    onAccepted: {
        var data = {
            "client_id": currentId,
            "client_name": nameField.text,
            "target": targetField.text,
            "token": tokenField.text,
            "exec_mode": execModeCombo.currentIndex,
            "run_state": runStateCombo.currentIndex,
            "ping_timeout_ms": timeoutSpin.value,
            "metrics_report_interval_s": intervalSpin.value
        }
        appServer.saveClient(data)
    }

    // --- Main grid ---
    GridLayout {
        columns: 2
        anchors.fill: parent
        rowSpacing: 10
        columnSpacing: 15

        Label { text: "Client Name:" }
        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: "e.g. Branch Office Router"
        }

        Label { text: "Target IP/Host:" }
        TextField {
            id: targetField
            Layout.fillWidth: true
        }

        Label { text: "Auth Token:" }
        RowLayout {
            Layout.fillWidth: true
            TextField {
                id: tokenField
                Layout.fillWidth: true
                readOnly: true
            }
            Button {
                text: "Generate"
                onClicked: tokenField.text = appServer.generateToken()
            }
        }

        Label { text: "Exec Mode:" }
        ComboBox {
            id: execModeCombo
            Layout.fillWidth: true
            model: ["Demo (Simulation)", "ICMP Engine"]
        }

        Label { text: "Running State:" }
        ComboBox {
            id: runStateCombo
            Layout.fillWidth: true
            model: ["Stopped", "Running"]
        }

        Label { text: "Ping Timeout (ms):" }
        SpinBox {
            id: timeoutSpin
            Layout.fillWidth: true
            from: 10; to: 5000; stepSize: 10
            editable: true
        }

        Label { text: "Metrics Interval (s):" }
        SpinBox {
            id: intervalSpin
            Layout.fillWidth: true
            from: 1; to: 3600; stepSize: 1
            editable: true
        }
    }
}
