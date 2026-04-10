import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: settingsPage
    title: i18n("Settings")

    FormCard.FormHeader {
        title: i18n("Server Connection")
    }

    FormCard.FormCard {
        FormCard.FormTextFieldDelegate {
            id: serverUrlField
            label: i18n("Server URL")
            text: MaClient.serverUrl || ""
            placeholderText: "http://192.168.1.100:8095"
        }


        FormCard.FormTextFieldDelegate {
            id: tokenField
            label: i18n("Access Token")
            placeholderText: i18n("Long-lived access token")
            echoMode: TextInput.Password
        }


        FormCard.FormButtonDelegate {
            text: MaClient.connected ? i18n("Disconnect") : i18n("Connect")
            icon.name: MaClient.connected ? "network-disconnect" : "network-connect"
            onClicked: {
                if (MaClient.connected) {
                    MaClient.disconnect()
                } else {
                    errorMessage.visible = false
                    MaClient.connectToServer(serverUrlField.text)
                    if (tokenField.text.length > 0) {
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
    }

    FormCard.FormHeader {
        title: i18n("Login with Credentials")
        visible: !MaClient.authenticated
    }

    FormCard.FormCard {
        visible: !MaClient.authenticated

        FormCard.FormTextFieldDelegate {
            id: usernameField
            label: i18n("Username")
            placeholderText: i18n("admin")
        }


        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18n("Password")
            echoMode: TextInput.Password
        }
    }

    FormCard.FormHeader {
        title: i18n("Status")
    }

    FormCard.FormCard {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("Connected to %1 (v%2)", MaClient.serverName || i18n("server"), MaClient.serverVersion)
            visible: MaClient.authenticated
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: MaClient.connected && !MaClient.authenticated
            type: Kirigami.MessageType.Information
            text: i18n("Authenticating...")
        }

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: false
        }

        FormCard.FormTextDelegate {
            visible: SendspinClient.registered
            text: i18n("Local Player")
            description: i18n("Registered as %1", SendspinClient.playerName)
        }

        FormCard.FormTextDelegate {
            visible: MaClient.authenticated && !SendspinClient.registered
            text: i18n("Local Player")
            description: i18n("Connecting...")
        }
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
                MaClient.saveSettings()
            }
        }
    }

    Component.onCompleted: {
        MaClient.loadSettings()
        serverUrlField.text = MaClient.serverUrl || ""
        // Pre-populate token so manual reconnect works without re-entering it
        if (MaClient.token.length > 0) {
            connectAndAuth.token = MaClient.token
        }
    }
}
