import QtQuick 1.1
import QOSGDeclarative 1.0

Rectangle
{
    width: 800
    height: 480
    color: "black";

    Row
    {
        width: parent.width;
        height: parent.height;

        Rectangle
        {
            height:parent.height;
            width:parent.width/4;
            color: "#252525";
        }

        Rectangle
        {
            height: parent.height;
            width: parent.width*3/4;
            color: "#444444";

            QOSGViewport
            {
                height: parent.height*2/3;
                width: parent.width;
                anchors.verticalCenter: parent.verticalCenter;
            }
        }
    }
}
