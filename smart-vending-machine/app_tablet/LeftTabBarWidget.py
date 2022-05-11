import copy

from PyQt5.QtWidgets import QStyle, QStylePainter, QStyleOptionTab, QTabBar, QTabWidget, QSizePolicy, QFrame, QGraphicsDropShadowEffect
from PyQt5.QtGui import QFont, QColor
from PyQt5.QtCore import Qt, QSize


class HorizontalTabBar(QTabBar):
    size_icon = QSize(80, 80)
    padding_y_tab = 30
    padding_x_tab = 50

    def __init__(self, parent=None):
        super(HorizontalTabBar, self).__init__(parent=parent)
        font = QFont("Times new roman")
        font.setBold(False)
        font.setPixelSize(17)
        self.setFont(font)

    def paintEvent(self, event):

        painter = QStylePainter(self)
        option = QStyleOptionTab()
        for index in range(self.count()):
            self.initStyleOption(option, index)

            size = self.tabSizeHint(index)
            step_from_the_top = 8
            step_from_the_left = (size.width() - self.size_icon.width()) / 2
            rect_image = copy.deepcopy(self.tabRect(index))
            rect_image.setY(rect_image.y() + step_from_the_top)
            rect_image.setX(rect_image.x() + step_from_the_left)
            rect_text = copy.deepcopy(self.tabRect(index))
            rect_text.setY(rect_text.y() + self.size_icon.height() - 20)

            painter.drawControl(QStyle.CE_TabBarTabShape, option)
            painter.drawItemPixmap(
                rect_image,
                Qt.AlignLeft,
                self.tabIcon(index).pixmap(self.tabIcon(index).actualSize(self.size_icon))
            )

            painter.drawText(
                rect_text,
                Qt.AlignCenter,
                self.tabText(index)
            )

    def tabSizeHint(self, index):
        size = QTabBar.tabSizeHint(self, index)
        # if size.width() < size.height():
        #     size.transpose()
        #     size.setHeight(size.height() + self.padding_y_tab)
        size.setHeight(size.height() + self.padding_y_tab)
        size.setWidth(size.width() + self.padding_x_tab)
        return size


class LeftTabBarWidget(QTabWidget):
    def __init__(self, parent=None):
        super().__init__(parent=parent)
        self.setTabBar(HorizontalTabBar())
        self.init_ui()

    def init_ui(self):
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Minimum)
        self.setObjectName("LeftTabBar")
        # self.setStyleSheet("QTabWidget#LeftTabBar::pane { border: 20; background-color: white}")

