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

    ColumnLayout {
        spacing: 0

        // Local player card — always shown at top
        QQC2.ItemDelegate {
            Layout.fillWidth: true
            highlighted: PlayerController.currentPlayerId === "__local__"

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Icon {
                    source: "computer"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    color: Kirigami.Theme.positiveTextColor
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        QQC2.Label {
                            text: LocalPlayer.playerName
                            font.bold: PlayerController.currentPlayerId === "__local__"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Kirigami.Chip {
                            text: i18n("This device")
                            closable: false
                            checkable: false
                        }
                    }
                    QQC2.Label {
                        text: LocalPlayer.playing ? i18n("Local playback · playing") : i18n("Local playback · idle")
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                    }
                }

                QQC2.Slider {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                    from: 0
                    to: 100
                    value: LocalPlayer.volume
                    onMoved: LocalPlayer.setVolume(Math.round(value))
                }

                QQC2.Label {
                    text: LocalPlayer.volume + "%"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                }
            }

            onClicked: {
                PlayerController.currentPlayerId = "__local__"
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Section header for remote players
        Kirigami.Heading {
            text: i18n("Remote Players")
            level: 4
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }

        // Remote players list
        Repeater {
            model: PlayerModel

            QQC2.ItemDelegate {
                Layout.fillWidth: true
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
                }
            }
        }

        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: PlayerModel.count === 0
            text: i18n("No remote players found")
            explanation: i18n("Make sure Music Assistant server has players configured")
            icon.name: "speaker"
        }
    }

    onVisibleChanged: {
        if (visible && MaClient.authenticated) {
            PlayerModel.refresh()
        }
    }
}
