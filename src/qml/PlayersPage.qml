import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: playersPage
    title: i18n("Players")

    actions: [
        Kirigami.Action {
            text: i18n("Refresh")
            icon.name: "view-refresh"
            onTriggered: PlayerModel.refresh()
        }
    ]

    ListView {
        id: playerListView
        model: PlayerModel

        delegate: QQC2.ItemDelegate {
            width: playerListView.width
            highlighted: model.playerId === PlayerController.currentPlayerId

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    source: "speaker"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    color: model.available ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    QQC2.Label {
                        text: model.name
                        font.bold: model.playerId === PlayerController.currentPlayerId
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    QQC2.Label {
                        text: {
                            var parts = [model.provider]
                            if (model.playbackState && model.playbackState !== "idle") {
                                parts.push(model.playbackState)
                            }
                            return parts.join(" · ")
                        }
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                // Power button
                QQC2.ToolButton {
                    icon.name: model.powered ? "system-shutdown" : "system-shutdown-symbolic"
                    checked: model.powered
                    onClicked: {
                        // Need to select player first, then toggle power
                        PlayerController.currentPlayerId = model.playerId
                        PlayerController.setPower(!model.powered)
                    }
                    QQC2.ToolTip.text: model.powered ? i18n("Power off") : i18n("Power on")
                    QQC2.ToolTip.visible: hovered
                }

                // Volume slider
                QQC2.Slider {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                    from: 0
                    to: 100
                    value: model.volumeLevel
                    enabled: model.available
                    onMoved: {
                        PlayerController.currentPlayerId = model.playerId
                        PlayerController.setVolume(Math.round(value))
                    }
                }

                QQC2.Label {
                    text: model.volumeLevel + "%"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                }
            }

            onClicked: {
                PlayerController.currentPlayerId = model.playerId
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: playerListView.count === 0
            text: i18n("No players found")
            explanation: i18n("Make sure Music Assistant server is running and has players configured")
            icon.name: "speaker"
        }
    }

    Component.onCompleted: {
        if (MaClient.authenticated) {
            PlayerModel.refresh()
        }
    }
}
