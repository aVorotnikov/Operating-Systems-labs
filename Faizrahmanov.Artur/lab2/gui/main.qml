import QtQuick 2.4
import QtQuick.Window 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

Window {
    id: mainWindow

    title: qsTr("Создание моделей")

    minimumWidth: 500
    minimumHeight: 700

    visible: true

    ColumnLayout {
        anchors.fill: parent
        RowLayout {
            Text {
                text: "Введите число:"
            }
            TextField {
                id: thrownNum
                Layout.fillWidth: true
                validator: RegExpValidator { regExp: /[0-9]+/ }
                onTextChanged: {
                    if (parseInt(text) > MAX_WOLF_NUMBER) {
                        text = MAX_WOLF_NUMBER.toString();
                    }
                }
                
            }
            Button {
                text: "Отправить"

                onClicked: {
                    let num = 0;
                    if (thrownNum.text !== "")
                        num = parseInt(thrownNum.text);
                    gui.sendNewWolfMessage(num);
                    thrownNum.text = "";
                }
            }
        }

        Text {
            text: "Сообщения:"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            ListView {
                anchors.fill: parent
                model: typeof (gameHistory) !== "undefined" ? gameHistory : 0

                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                clip:true

                delegate: Component {
                    Text {
                        text: message
                    }
                }
            }
        }
    }
}