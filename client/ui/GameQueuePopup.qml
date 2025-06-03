import QtQuick 2.15
import QtQuick.Controls 2.15

// 게임 매칭 대기열 진입 시 뜨는 팝업 창 UI
Popup {
    id: gameQueuePopup
    width: parent ? parent.width * 0.4 : 320
    height: parent ? parent.height * 0.3 : 200

    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose

    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0

    background: Rectangle {
        color: "#000000"
        radius: 12
    }

    // 팝업 배경을 눌러도 아무 동작 없이 클릭만 흡수하도록 설정
    MouseArea {
        anchors.fill: parent
        onClicked: {}
    }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            id: matchingText
            text: qsTr("게임 매칭 중...")
            font.pixelSize: 24
            color: "#FFFFFF"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Button {
            id: cancelButton
            text: qsTr("매칭 취소")
            width: parent.width * 0.6
            height: 40
            anchors.horizontalCenter: parent.horizontalCenter

            font.pixelSize: 18
            
            contentItem: Text {
                text: qsTr("매칭 취소")
                anchors.centerIn: parent
                font.pixelSize: 18
                color: "#000000"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                // 서버로 큐 취소 메시지 전송 ("QUEUE_OUT")
                console.log("매칭 취소 버튼 클릭 – 서버에 QUEUE_OUT 전송")
                gameClient.sendMsg("QUEUE_OUT")
            }
        }
    }

    Connections {
        target: gameClient
        function onMessageReceived(message) {
            if (message.trim() === "SUCCESS_OUT") {
                console.log("match popup Close")
                gameQueuePopup.close()
            } else if (message.trim() === "GAME_START") {
                console.log("Game_Start")
                gameQueuePopup.close()
            }
        }
    }
}