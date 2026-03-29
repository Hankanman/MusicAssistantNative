import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: queuePage
    title: i18n("Queue")

    actions: [
        Kirigami.Action {
            text: i18n("Clear Queue")
            icon.name: "edit-clear-all"
            onTriggered: QueueController.clearQueue()
        }
    ]

    header: RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.margins: Kirigami.Units.smallSpacing

        QQC2.Label {
            text: i18n("%1 items", QueueController.itemCount)
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
        }

        QQC2.ToolButton {
            icon.name: "media-playlist-shuffle"
            checked: QueueController.shuffleEnabled
            onClicked: QueueController.setShuffle(!QueueController.shuffleEnabled)
            QQC2.ToolTip.text: i18n("Shuffle")
            QQC2.ToolTip.visible: hovered
        }
        QQC2.ToolButton {
            icon.name: {
                switch (QueueController.repeatMode) {
                case "one": return "media-repeat-single"
                case "all": return "media-repeat-all"
                default: return "media-repeat-none"
                }
            }
            checked: QueueController.repeatMode !== "off"
            onClicked: {
                switch (QueueController.repeatMode) {
                case "off": QueueController.setRepeat("all"); break
                case "all": QueueController.setRepeat("one"); break
                case "one": QueueController.setRepeat("off"); break
                }
            }
            QQC2.ToolTip.text: i18n("Repeat: %1", QueueController.repeatMode)
            QQC2.ToolTip.visible: hovered
        }
    }

    ListView {
        id: queueListView
        model: QueueController.itemModel

        delegate: QQC2.ItemDelegate {
            id: queueDelegate
            width: queueListView.width
            highlighted: model.itemIndex === QueueController.currentIndex

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: (model.itemIndex + 1) + "."
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    horizontalAlignment: Text.AlignRight
                }

                Image {
                    source: model.imageUrl ? "image://ma/" + model.imageUrl : ""
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2.5
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2.5
                    fillMode: Image.PreserveAspectCrop
                    visible: source !== ""
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    QQC2.Label {
                        text: model.name || ""
                        font.bold: model.itemIndex === QueueController.currentIndex
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    QQC2.Label {
                        text: {
                            var parts = []
                            if (model.artistName) parts.push(model.artistName)
                            if (model.albumName) parts.push(model.albumName)
                            return parts.join(" - ")
                        }
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        color: Kirigami.Theme.disabledTextColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        visible: text !== ""
                    }
                }

                QQC2.Label {
                    text: model.duration > 0 ? formatTime(model.duration) : ""
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                }

                QQC2.ToolButton {
                    icon.name: "edit-delete-remove"
                    onClicked: QueueController.removeItem(model.queueItemId)
                    QQC2.ToolTip.text: i18n("Remove from queue")
                    QQC2.ToolTip.visible: hovered
                }
            }

            onClicked: QueueController.playIndex(model.itemIndex)
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: queueListView.count === 0
            text: i18n("Queue is empty")
            explanation: i18n("Play something from the library to fill the queue")
            icon.name: "amarok_playlist"
        }
    }

    function formatTime(seconds) {
        if (seconds <= 0) return ""
        var mins = Math.floor(seconds / 60)
        var secs = Math.floor(seconds % 60)
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
