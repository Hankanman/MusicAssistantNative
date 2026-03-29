import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: settingsPage
    title: i18n("Settings")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Kirigami.Heading {
                text: i18n("Server Connection")
                level: 2
                Kirigami.FormData.isSection: true
            }

            QQC2.TextField {
                id: serverUrlField
                Kirigami.FormData.label: i18n("Server URL:")
                placeholderText: "http://192.168.1.100:8095"
                text: MaClient.serverUrl || ""
                Layout.fillWidth: true
            }

            QQC2.TextField {
                id: tokenField
                Kirigami.FormData.label: i18n("Token:")
                placeholderText: i18n("JWT token or leave empty for login")
                echoMode: TextInput.Password
                Layout.fillWidth: true
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18n("Or login with credentials")
            }

            QQC2.TextField {
                id: usernameField
                Kirigami.FormData.label: i18n("Username:")
                placeholderText: i18n("admin")
                Layout.fillWidth: true
            }

            QQC2.TextField {
                id: passwordField
                Kirigami.FormData.label: i18n("Password:")
                echoMode: TextInput.Password
                Layout.fillWidth: true
            }

            RowLayout {
                Kirigami.FormData.label: " "
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: MaClient.connected ? i18n("Disconnect") : i18n("Connect")
                    icon.name: MaClient.connected ? "network-disconnect" : "network-connect"
                    onClicked: {
                        if (MaClient.connected) {
                            MaClient.disconnect()
                        } else {
                            MaClient.connectToServer(serverUrlField.text)

                            if (tokenField.text.length > 0) {
                                // Wait for connection, then auth with token
                                connectAndAuth.token = tokenField.text
                                connectAndAuth.useCredentials = false
                            } else if (usernameField.text.length > 0) {
                                connectAndAuth.username = usernameField.text
                                connectAndAuth.password = passwordField.text
                                connectAndAuth.useCredentials = true
                            }
                        }
                    }
                }

                QQC2.BusyIndicator {
                    running: (MaClient.connected || MaClient.serverReady) && !MaClient.authenticated
                    visible: running
                    implicitWidth: Kirigami.Units.gridUnit * 2
                    implicitHeight: Kirigami.Units.gridUnit * 2
                }
                QQC2.Label {
                    visible: MaClient.connected && !MaClient.serverReady
                    text: i18n("Waiting for server...")
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }

        // Status section
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("Connected to %1 (v%2)", MaClient.serverName, MaClient.serverVersion)
            visible: MaClient.authenticated
        }

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: false
        }

        Item { Layout.fillHeight: true }
    }

    QtObject {
        id: connectAndAuth
        property string token: ""
        property string username: ""
        property string password: ""
        property bool useCredentials: false
    }

    Connections {
        target: MaClient
        function onServerReadyChanged() {
            // Only send auth AFTER the server has sent its info message
            if (MaClient.serverReady) {
                if (connectAndAuth.useCredentials) {
                    MaClient.loginWithCredentials(connectAndAuth.username, connectAndAuth.password)
                } else if (connectAndAuth.token.length > 0) {
                    MaClient.authenticate(connectAndAuth.token)
                }
            }
        }
        function onConnectionError(error) {
            errorMessage.text = error
            errorMessage.visible = true
        }
        function onAuthenticatedChanged() {
            if (MaClient.authenticated) {
                errorMessage.visible = false
                // Save credentials on successful auth
                MaClient.saveSettings()
            }
        }
    }

    Component.onCompleted: {
        // Load saved settings into the form
        MaClient.loadSettings()
        serverUrlField.text = MaClient.serverUrl || ""

        // Auto-connect if we have saved credentials
        if (MaClient.hasSavedSettings()) {
            MaClient.connectWithSavedSettings()
        }
    }
}
