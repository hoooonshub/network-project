import QtQuick 2.15
import QtQuick.Controls 2.15

// 원카드 게임 UI
Page {
    id: gameRoot
    width: 800
    height: 600

     Rectangle {
        anchors.fill: parent
        color: "#8FD984"

    }

    property var handCards: []  // 본인 손에 있는 카드들
    property int myIndex: -1    // 서버로부터 받을 나의 인덱스 (화면 상의 나의 위치를 결정)
    property int player0Count: 7 // 초기 7장 카드 개수로 세팅
    property int player1Count: 7
    property int player2Count: 7
    property int player3Count: 7
    property int currentTurnIndex: -1
    property string cardBackImage: "assets/back.png"
    property string drawDeckImage: "assets/drawdeck.png"
    property string topCardImage: "assets/back.png"
    property int cardWidth: 60
    property int cardHeight: 90

    property string gameResult: ""

    // 카드를 제출하면 그 카드를 제거한 핸드를 만들기 위한 함수
    function removeCardFromHand(card) {
        var idx = handCards.indexOf(card);
        if (idx !== -1) {
            var newHand = handCards.slice();
            newHand.splice(idx, 1);
            handCards = newHand;
        }
    }

    // 카드를 제출하면 남은 카드의 개수를 업데이트하기 위한 함수
    function updateDiscardCount(index) {
        if (index === player0Area.playerIndex) {
            player0Count -= 1;
        } else if (index === player1Area.playerIndex) {
            player1Count -= 1;
        } else if (index === player2Area.playerIndex) {
            player2Count -= 1;
        } else if (index === player3Area.playerIndex) {
            player3Count -= 1;
        }
    }

    // 카드를 드로우하면 남은 카드의 개수를 증가하는 업데이트를 위한 함수
    function updateDrawCount(index) {
        if (index === -1) { // 시작 시 초기 세팅
            return;
        }
        console.log("값 누른다 -> " + player0Count);
        if (index === player0Area.playerIndex) {
            player0Count += 1;
            console.log("값 더해짐" + player0Count);
        } else if (index === player1Area.playerIndex) {
            player1Count += 1;
        } else if (index === player2Area.playerIndex) {
            player2Count += 1;
        } else if (index === player3Area.playerIndex) {
            player3Count += 1;
        }
    }

    // 플레이어 0 영역 - 좌측 하단
    Column {
        id: player0Area
        property int playerIndex: 0       
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10                        
        spacing: 5                                 

        Grid {
            id: player0CardRow
            columns: 10
            spacing: 5                 
            flow: Grid.LeftToRight        
            layoutDirection: Qt.LeftToRight    
            
            // "본인"이라면 자신의 손패 카드
            Repeater {
                id: player0Repeater
                model: handCards                   // 본인 손패 카드 리스트 (문자열 배열)

                delegate: Item {
                    width: cardWidth; height: cardHeight   
                    visible: myIndex === player0Area.playerIndex
                    enabled: currentTurnIndex === player0Area.playerIndex
                    
                    y: index >= 10 ? cardHeight - 10 : 0       // 핸드 카드 11개부터는 2행으로 배치
                    Image {
                        anchors.fill: parent
                        source: "assets/" + modelData + ".png"   // 예: "assets/H3.png"
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor      
                        onClicked: {
                            // 서버로 "PLAY [카드코드]" 메시지 전송 (예: "PLAY H3")
                            gameClient.sendMsg("PLAY " + modelData)
                        }
                    }
                }
            }

            // "상대"라면 카드 뒷면 
            Image {
                width: cardWidth; height: cardHeight
                source: cardBackImage
                fillMode: Image.PreserveAspectFit
                visible: myIndex !== player0Area.playerIndex
            }
            // "상대"라면 몇 개 남았는지
            Text {
                text: player0Count + " Left"
                color: "white"
                font.pixelSize: 17
                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: cardBackImage.verticalCenter
                visible: myIndex !== player0Area.playerIndex
            }
        }

        // 플레이어 0 프로필 및 턴 상태 표시 (프로필 + "MYTURN"/"WAITING")
        Row {
            id: player0InfoRow
            spacing: 5
            // 플레이어 프로필
            Rectangle {
                id: player0Profile
                width: 50; height: 50
                color: "#cccccc"                          // 연한 회색으로 표시 (프로필 이미지 자리)

                Text {
                    anchors.centerIn: parent
                    text: player0Area.playerIndex === myIndex ? "ME" : "OPP" // 본인과 상대방 위치 구분을 위한 마커
                }
            }
            // 턴 상태 텍스트: 본인 차례이면 "MYTURN", 아니면 "WAITING"
            Text {
                text: player0Area.playerIndex === currentTurnIndex ? "MYTURN" : "WAITING"
                color: player0Area.playerIndex === currentTurnIndex ? "#2d22c9" : "white"
                font.pixelSize: player0Area.playerIndex == currentTurnIndex ? 20 : 16
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    // [플레이어 0의 컴포넌트와 동일; 위치만 변경]
    // 플레이어 1 영역 - 화면 우하단
    Column {
        id: player1Area
        property int playerIndex: 1
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        // 플레이어 1의 카드
        Grid {
            id: player1CardRow
            spacing: 5
            columns: 10
            flow: Grid.RightToLeft
            layoutDirection: Qt.RightToLeft

            // "본인"이라면 자신의 손패 카드
            Repeater {
                id: player1Repeater
                model: handCards                   // 본인 손패 카드 리스트 (문자열 배열)

                delegate: Item {
                    width: cardWidth; height: cardHeight   
                    visible: myIndex === player1Area.playerIndex
                    enabled: currentTurnIndex === player1Area.playerIndex
                    
                    y: index >= 10 ? cardHeight - 10 : 0
                    Image {
                        anchors.fill: parent
                        source: "assets/" + modelData + ".png"   // 예: "assets/H3.png"
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor      
                        onClicked: {
                            // 서버로 "PLAY [카드코드]" 메시지 전송 (예: "PLAY H3")
                            gameClient.sendMsg("PLAY " + modelData)
                        }
                    }
                }
            }

            // "상대라면" 카드 뒷면 
            Image {
                
                width: cardWidth; height: cardHeight
                source: cardBackImage
                fillMode: Image.PreserveAspectFit
                visible: myIndex !== player1Area.playerIndex
            }
            // 몇 개 남았는지
            Text {
               
                text: player1Count + " Left"
                color: "white"
                font.pixelSize: 17
                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: cardBackImage.verticalCenter
                visible: myIndex !== player1Area.playerIndex
            }
        }

        // 플레이어 1 프로필 및 턴 상태 ("MYTURN"/"WAITING")
        Row {
            id: player1InfoRow
            anchors.right: parent.right               // 우측 정렬 (프로필을 코너 쪽에 배치)
            spacing: 5
            // 프로필 턴 상태 텍스트를 먼저 배치하여 프로필이 오른쪽에 오도록 설정
            Text {
                text: player1Area.playerIndex === currentTurnIndex ? "MYTURN" : "WAITING"
                color: player1Area.playerIndex === currentTurnIndex ? "#2d22c9" : "white"
                font.pixelSize: player1Area.playerIndex == currentTurnIndex ? 20 : 16
                verticalAlignment: Text.AlignVCenter
            }
            Rectangle {
                id: player1Profile
                width: 50; height: 50
                color: "#cccccc"

                Text {
                    anchors.centerIn: parent
                    text: player1Area.playerIndex === myIndex ? "ME" : "OPP"
                }
            }
        }
    }

    // [플레이어 0의 컴포넌트와 동일; 위치만 변경]
    // 플레이어 2 영역 - 화면 우상단
    Column {
        id: player2Area
        property int playerIndex: 2
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        spacing: 5

        // 플레이어 2 프로필 및 턴 상태
        Row {
            id: player2InfoRow
            anchors.right: parent.right               // 우측 정렬
            spacing: 5
            Text {
                text: player2Area.playerIndex === currentTurnIndex ? "MYTURN" : "WAITING"
                color: player2Area.playerIndex === currentTurnIndex ? "#2d22c9" : "white"
                font.pixelSize: player2Area.playerIndex == currentTurnIndex ? 20 : 16
                verticalAlignment: Text.AlignVCenter
            }
            Rectangle {
                id: player2Profile
                width: 50; height: 50
                color: "#cccccc"

                Text {
                    anchors.centerIn: parent
                    text: player2Area.playerIndex == myIndex ? "ME" : "OPP"
                }
            }
        }
        
        Grid {
            id: player2CardsRow
            anchors.right: parent.right               // 우측 정렬
            spacing: 5
            columns: 10
            flow: Grid.RightToLeft
            layoutDirection: Qt.RightToLeft

            // "본인"이라면 자신의 손패 카드
            Repeater {
                id: player2Repeater
                model: handCards                   

                delegate: Item {
                    width: cardWidth; height: cardHeight   
                    visible: myIndex === player2Area.playerIndex
                    enabled: currentTurnIndex === player2Area.playerIndex
                    
                    y: index >= 10 ? (cardHeight + player2InfoRow.height + 10) : 0
                    Image {
                        anchors.fill: parent
                        source: "assets/" + modelData + ".png"   // 예: "assets/H3.png"
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor      
                        onClicked: {
                            // 서버로 "PLAY [카드코드]" 메시지 전송 (예: "PLAY H3")
                            gameClient.sendMsg("PLAY " + modelData)
                        }
                    }
                }
            }

            // "상대라면" 카드 뒷면 
            Image {
                
                width: cardWidth; height: cardHeight
                source: cardBackImage
                fillMode: Image.PreserveAspectFit
                visible: myIndex !== player2Area.playerIndex
            }
            // 몇 개 남았는지
            Text {
                
                text: player2Count + " Left"
                color: "white"
                font.pixelSize: 17
                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: cardBackImage.verticalCenter
                visible: myIndex !== player2Area.playerIndex
            }
        }
    }

    // [플레이어 0의 컴포넌트와 동일; 위치만 변경]
    // 플레이어 3 영역 - 화면 좌상단
    Column {
        id: player3Area
        property int playerIndex: 3
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        spacing: 5

        // 플레이어 3 프로필 및 턴 상태
        Row {
            id: player3InfoRow
            spacing: 5

            Rectangle {
                id: player3Profile
                width: 50; height: 50
                color: "#cccccc"

                Text {
                    anchors.centerIn: parent
                    text: player3Area.playerIndex === myIndex ? "ME" : "OPP"
                }
            }

            Text {
                text: player3Area.playerIndex === currentTurnIndex ? "MYTURN" : "WAITING"
                color: player3Area.playerIndex === currentTurnIndex ? "#2d22c9" : "white"
                font.pixelSize: player3Area.playerIndex == currentTurnIndex ? 20 : 16
                verticalAlignment: Text.AlignVCenter
            }
        }

        // 플레이어 3 카드들
        Grid {
            id: player3CardsRow
            spacing: 5
            columns: 10
            flow: Grid.LeftToRight
            layoutDirection: Qt.LeftToRight

            // "본인"이라면 자신의 손패 카드
            Repeater {
                id: player3Repeater
                model: handCards                   // 본인 손패 카드 리스트 (문자열 배열)
                
                delegate: Item {
                    width: cardWidth; height: cardHeight   
                    visible: myIndex === player3Area.playerIndex
                    enabled: currentTurnIndex === player3Area.playerIndex
                    
                    y: index >= 10 ? (cardHeight + play3InfoRow.height + 10) : 0
                    Image {
                        anchors.fill: parent
                        source: "assets/" + modelData + ".png"   // 예: "assets/H3.png"
                        fillMode: Image.PreserveAspectFit
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor      
                        onClicked: {
                            // 서버로 "PLAY [카드코드]" 메시지 전송 (예: "PLAY H3")
                            gameClient.sendMsg("PLAY " + modelData)
                        }
                    }
                }
            }

            // "상대라면" 카드 뒷면 
            Image {
                
                width: cardWidth; height: cardHeight
                source: cardBackImage
                fillMode: Image.PreserveAspectFit
                visible: myIndex !== player3Area.playerIndex
            }
            // 몇 개 남았는지
            Text {
                
                text: player3Count + " Left"
                color: "white"
                font.pixelSize: 17
                verticalAlignment: Text.AlignVCenter
                anchors.verticalCenter: cardBackImage.verticalCenter
                visible: myIndex !== player3Area.playerIndex
            }
        }
    }

    // --- 중앙 영역: 최근 제출된 카드와 더미 덱 ---
    // 최근 제출된 카드 (앞면으로 한 장 보여줌)
    Image {
        id: topCardImageId
        width: cardWidth; height: cardHeight
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: topCardImage
        fillMode: Image.PreserveAspectFit
    }

    // 드로우 더미 덱 (카드 뭉치) - 최근 카드 왼쪽에 뒷면으로 표시, 클릭 가능
    MouseArea {
        id: drawDeckArea
        width: cardWidth; height: cardHeight
        anchors.verticalCenter: topCardImageId.verticalCenter
        anchors.right: topCardImageId.left
        anchors.rightMargin: 40                    
        cursorShape: Qt.PointingHandCursor

        enabled: currentTurnIndex === myIndex   // 본인 차례일 때만 클릭 활성화

        // 사용자 클릭 시 "DRAW" 명령을 서버로 전송
        onClicked: {
            gameClient.sendMsg("DRAW")
        }

        // 더미 덱 이미지 
        Image {
            anchors.fill: parent
            source: drawDeckImage
            fillMode: Image.PreserveAspectFit
        }
    }

    // 게임 종료 시 띄울 팝업창
    Popup {
        id: gameOverPopup
        x: (root.width - width) / 2
        y: (root.height - height) / 2
        width: 300
        height: 180
        modal: true
        focus: true
        visible: showGameOverPopup

        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#666666"
            border.width: 1
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Text {
                id: resultText
                text: gameResult === "WIN" ? "You Win!" : "You Lose"
                font.pixelSize: 20
                color: "#000000"
                
            }

            // 확인 클릭 시 다시 홈 화면으로 이동
            Button {
                text: "확인"
                width: parent.width * 0.5
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    gameOverPopup.close()
                    gameLoader.active = false
                    homeContent.visible = true
                }
            }
        }
    }

    // --- 서버 메시지 처리 ---
    // gameClient 객체의 messageReceived 시그널을 받아 파싱하여 UI 갱신
    Connections {
        target: gameClient          // C++에서 등록된 gameClient(Qt 객체) - messageReceived(QString) 시그널 제공
        function onMessageReceived(msg) {
            var parts = msg.split(" ");

            var command = parts[0];

            switch (command) {
                // 'INDEX 1' 과 같은 형태로 파싱
                // 게임 초기 세팅 시 본인의 위치 파악 용 인덱스 정보 
                case "INDEX":
                    myIndex = parts[1];
                    break;

                // 'HAND H9,SK,CA,HT' 와 같은 형태로 파싱
                // 카드를 드로우할 때 본인 핸드를 계속 업데이트하기 위한 command
                case "HAND":
                    updateDrawCount(currentTurnIndex);

                    if (currentTurnIndex !== -1 && currentTurnIndex !== myIndex) { // 드로우한 본인만 업데이트하기 위해 본인 아니면 break;
                        break;
                    }

                    var cardsList = parts[1].trim().split(",")    // 콤마로 구분된 카드코드 토큰이 담긴 배열
                    handCards = cardsList.filter(function(item) {
                        return item.length > 0;
                    });
                    break;

                // 'DISCARD H2' 와 같은 형태로 파싱
                // 카드를 냈을 때 더미 상단 카드와 플레이어에게 남은 카드, 손에 있는 카드를 업데이트하기 위한 command
                case "DISCARD":
                    topCardImage = "assets/" + parts[1] + ".png"
                    removeCardFromHand(parts[1])
                    updateDiscardCount(currentTurnIndex);
                    break;

                // 'TURN 2' 와 같은 형태로 파싱
                // 턴이 넘어갈 때 누구의 턴인지 알기 위해 인덱스 번호와 함께 정보를 수신
                // 
                case "TURN":
                    console.log("TURN " + parts[1])
                    var turn = parts[1];
                    currentTurnIndex = turn;
                    break;

                // 'GAME_OVER' 로 수신
                // 게임 종료를 알리기 위한 문자열 값
                case "GAME_OVER":
                    gameResult = parts[1];
                    gameOverPopup.open();
                    break;

                default:
                    break;
            }
        }
    }
}
