import QtQuick 2.0;
import org.ukui.kwin 2.0;

ScreenEdgeItem {
    edge: ScreenEdgeItem.LeftEdge
    mode: ScreenEdgeItem.Touch
    onActivated: {
        workspace.slotToggleShowDesktop();
    }
}
