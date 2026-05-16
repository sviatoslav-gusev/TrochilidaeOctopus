import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material // Подключаем Material Design
import QtQuick.Layouts
import gs.OctopusServer 1.0

ApplicationWindow {
    visible: true
    width: 850
    height: 600
    title: "Octopus Server Community Edition"

    // 1. Включаем темную тему на уровне всего приложения
    Material.theme: Material.Dark
    Material.accent: Material.Teal // Цвет акцентов (чекбоксы, фокусы)

    // Цвет самого заднего фона окна (очень темный серый)
    color: "#1e1e1e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20 // Даем немного воздуха по краям
        spacing: 15

        // --- ВЕРХНЯЯ ПАНЕЛЬ С КНОПКАМИ ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "Add Client"
                Material.background: "#2d2d30"
                onClicked: appServer.addDummyClient()
            }
            Button {
                text: "Mass Start"
                Material.background: "#2e7d32" // Насыщенный зеленый
                Material.foreground: "white"
                font.bold: true
                onClicked: appServer.massStart()
            }
            Button {
                text: "Mass Stop"
                Material.background: "#c62828" // Насыщенный красный
                Material.foreground: "white"
                font.bold: true
                onClicked: appServer.massStop()
            }

            // Пружина, которая прижмет кнопки влево
            Item { Layout.fillWidth: true }
        }

        // --- КОНТЕЙНЕР ТАБЛИЦЫ ---
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#252526" // Цвет подложки таблицы
            radius: 8        // Закругленные углы
            border.color: "#3e3e42"
            border.width: 1
            clip: true       // Обрезаем всё, что вылезает за закругления

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 2. КАСТОМНЫЙ ЗАГОЛОВОК ТАБЛИЦЫ
                Rectangle {
                    Layout.fillWidth: true
                    height: 45
                    color: "#2d2d30"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        Label { text: "ID"; Layout.preferredWidth: 50; font.bold: true; color: "#aaaaaa" }
                        Label { text: "CLIENT NAME"; Layout.preferredWidth: 200; font.bold: true; color: "#aaaaaa" }
                        Label { text: "STATUS"; Layout.preferredWidth: 120; font.bold: true; color: "#aaaaaa" }
                        Label { text: "RTT (ms)"; Layout.preferredWidth: 100; font.bold: true; color: "#aaaaaa" }
                    }
                }

                // Тонкая линия под заголовком
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3e3e42"
                }

                // 3. САМА ТАБЛИЦА (СПИСОК)
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: clientsModel
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds // Убираем пружинящий эффект macOS/iOS

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 45

                        // Эффект "Зебры": четные строки темнее, нечетные прозрачные
                        color: index % 2 === 0 ? "transparent" : "#2a2a2b"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            spacing: 0

                            // ID
                            Label {
                                text: model.client_id
                                Layout.preferredWidth: 50
                                color: "#d4d4d4"
                                font.pixelSize: 14
                            }

                            // Имя (выделяем синим, как переменные в IDE)
                            Label {
                                text: model.client_name
                                Layout.preferredWidth: 200
                                color: "#569cd6"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            // Блок Статуса (Кружок + Текст)
                            RowLayout {
                                Layout.preferredWidth: 120
                                spacing: 8

                                Rectangle {
                                    width: 12; height: 12; radius: 6
                                    color: model.conn_state === Enums.ConnectionState.Online ? "#4caf50" : "#f44336"
                                    // Легкое свечение (бордер чуть светлее самого цвета)
                                    border.color: Qt.lighter(color, 1.5)
                                    border.width: 1
                                }
                                Label {
                                    text: model.conn_state === Enums.ConnectionState.Online ? "Online" : "Offline"
                                    color: model.conn_state === Enums.ConnectionState.Online ? "#4caf50" : "#f44336"
                                    font.pixelSize: 13
                                }
                            }

                            // Пинг (Меняет цвет, если слишком высокий)
                            Label {
                                text: model.avg_roundtrip_ms.toFixed(1)
                                Layout.preferredWidth: 100
                                // Если пинг больше 100, красим в оранжевый предупреждающий
                                color: model.avg_roundtrip_ms > 100 ? "#ff9800" : "#d4d4d4"
                                font.pixelSize: 14
                            }
                        }

                        // 4. ЭФФЕКТ НАВЕДЕНИЯ МЫШКИ (Hover)
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#333333"
                            onExited: parent.color = index % 2 === 0 ? "transparent" : "#2a2a2b"
                        }
                    }
                }
            }
        }
    }
}
