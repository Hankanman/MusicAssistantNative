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

    header: ColumnLayout {
        spacing: 0
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: PlayerController.currentPlayerId !== ""
            type: Kirigami.MessageType.Information
            text: i18n("Active player: %1", PlayerController.playerName || PlayerController.currentPlayerId)
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: SendspinClient.registered
            type: Kirigami.MessageType.Positive
            text: i18n("This device registered as: %1 (select it above to play through PC speakers)", SendspinClient.playerName)
        }
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: MaClient.authenticated && !SendspinClient.registered
            type: Kirigami.MessageType.Warning
            text: i18n("Registering this device as a player...")
        }
    }

    ListView {
        id: playerListView
        model: PlayerModel

        delegate: QQC2.ItemDelegate {
            width: playerListView.width
            highlighted: model.playerId === PlayerController.currentPlayerId

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    source: model.playerId === PlayerController.currentPlayerId ? "media-playback-start" : "speaker"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    color: model.available
                        ? (model.playerId === PlayerController.currentPlayerId ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor)
                        : Kirigami.Theme.disabledTextColor
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        QQC2.Label {
                            text: model.name
                            font.bold: model.playerId === PlayerController.currentPlayerId
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Kirigami.Chip {
                            text: i18n("Active")
                            visible: model.playerId === PlayerController.currentPlayerId
                            closable: false
                            checkable: false
                        }
                    }
                    QQC2.Label {
                        text: {
                            var parts = [model.provider]
                            if (model.playbackState && model.playbackState !== "idle") {
                                parts.push(model.playbackState)
                            }
                            if (!model.available) parts.push(i18n("unavailable"))
                            return parts.join(" · ")
                        }
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }

                QQC2.ToolButton {
                    icon.name: "system-shutdown"
                    checked: model.powered
                    onClicked: {
                        PlayerController.currentPlayerId = model.playerId
                        PlayerController.setPower(!model.powered)
                    }
                    QQC2.ToolTip.text: model.powered ? i18n("Power off") : i18n("Power on")
                    QQC2.ToolTip.visible: hovered
                }

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
                console.log("Selected player:", model.playerId, model.name)
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: playerListView.count === 0
            text: i18n("No players found")
            explanation: i18n("Make sure Music Assistant server has players configured and they are powered on")
            icon.name: "speaker"
        }
    }

    onVisibleChanged: {
        if (visible && MaClient.authenticated) {
            PlayerModel.refresh()
        }
    }
}
