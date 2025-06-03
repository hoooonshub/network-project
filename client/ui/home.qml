import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

import "."

// 기본 로비(홈) 화면 UI 
Window {
    id: root
    visible: true
    width: 800
    height: 600
    title: qsTr("One Card")

    Rectangle {
        id: homeContent
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1A3350" }
            GradientStop { position: 1.0; color: "#0F2137" }
        }
        visible: !gameLoader.active

        Column {
            anchors.centerIn: parent
            spacing: 40
            width: parent.width

            Item {
                width: parent.width
                height: 120

                Text {
                    id: logoShadow
                    text: qsTr("One Card")
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 64
                    font.bold: true
                    color: "#00000080"
                    x: logoText.x + 2
                    y: logoText.y + 2
                }

                Text {
                    id: logoText
                    text: qsTr("One Card")
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 64
                    font.bold: true
                    color: "#FFD54F"
                }
            }

            // 게임 대기열 진입을 위한 버튼
            Button {
                id: quickGameBtn
                text: qsTr("Quick Game")
                anchors.horizontalCenter: parent.horizontalCenter
                width: 240
                height: 60

                contentItem: Text {
                    text: qsTr("Quick Game")
                    anchors.centerIn: parent
                    font.pixelSize: 20
                    font.bold: true
                    color: "#1A3350"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                // 버튼 클릭 시 게임 매칭 대기열 진입을 위해 서버에 데이터 요청
                onClicked: {
                    console.log("Quick Game 버튼 클릭됨")
                    gameClient.sendMsg("QUEUE_IN")
                }
            }
        }
    }

    // 게임 UI를 띄우기 위한 세팅
    Loader {
        id: gameLoader
        anchors.fill: parent
        active: false
        source: "GameWindow.qml"
    }

    GameQueuePopup {
        id: matchPopup
        anchors.centerIn: root
    }

    Connections {
        target: gameClient
        function onMessageReceived(message) {
            console.log("Received from server:", message)
            if (message.trim() === "SUCCESS_IN") {
                console.log("queue popup Open")
                matchPopup.open()
            }

            // 서버에서 게임 시작 응답 시 홈 화면 UI를 안 보이게 만들고
            // 게임 화면 UI를 보이게 만듦
            if (message.trim() === "GAME_START") {
                homeContent.visible = false
                gameLoader.active = true
            }
        }
    }
}