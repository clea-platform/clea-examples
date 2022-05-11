from PyQt5.QtWidgets import QLabel, QStackedWidget, QPushButton, QVBoxLayout, QHBoxLayout, QGraphicsDropShadowEffect
from PyQt5.QtGui import QFont, QColor, QIcon
from PyQt5.QtCore import Qt, QSize
from PyQt5.QtSvg import QSvgWidget

from CardProductsBox import CardProductsBox


class FakeTable(QLabel):
    def __init__(self, parent, tabs, products, stacked_window=None, ref_video_thread=None):
        super(FakeTable, self).__init__(parent)
        self.tabs = tabs
        self.products = products
        self.stacked_window = stacked_window
        self.ref_video_thread = ref_video_thread

        self.button_tabs = []
        self.panels = []

        self.stacked_panels = QStackedWidget()

        self.shadow = QGraphicsDropShadowEffect()

        self.init_ui()

    def init_ui(self):
        self.shadow.setColor(QColor(200, 200, 200))
        self.shadow.setBlurRadius(20)
        self.shadow.setOffset(2, 0)
        self.setObjectName("fakeTable")
        self.setStyleSheet("QLabel#fakeTable{background: transparent}")
        buttons = self.buttons_ui()
        buttons.setContentsMargins(15, 0, 0, 298)
        panels = self.panels_ui()
        hbox = QHBoxLayout()
        hbox.setAlignment(Qt.AlignLeft)
        hbox.setSpacing(0)
        hbox.setContentsMargins(0, 0, 0, 0)
        hbox.addLayout(buttons)
        hbox.addLayout(panels)
        hbox.addStretch(1)
        self.setLayout(hbox)

    def buttons_ui(self):
        for i, [tab_name, info_widget] in enumerate(self.tabs.items()):
            button = QPushButton(tab_name)
            button.setObjectName("buttonTab")
            button.setFixedSize(70, 100)
            shadow = QGraphicsDropShadowEffect()
            shadow.setColor(QColor(200, 200, 200))
            shadow.setBlurRadius(20)
            shadow.setOffset(-5, 0)
            button.setGraphicsEffect(shadow)
            if i:
                button.setStyleSheet(
                    "QPushButton#buttonTab{"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #f0efef;"
                    "color: #f0efef;"
                    "}"
                    "QPushButton#buttonTab:pressed {"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #ffffff;"
                    "color: #ffffff;"
                    "}"
                )
            else:
                button.setStyleSheet(
                    "QPushButton#buttonTab{"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #ffffff;"
                    "color: #ffffff;"
                    "}"
                )
            icon = QSvgWidget(info_widget['icon'])
            icon.setFixedSize(QSize(68, 68))
            icon.setStyleSheet("background-color: transparent")
            # icon.setStyleSheet("border: 1px solid blue")
            text_layout = QHBoxLayout()
            text_layout.setContentsMargins(0,0,0,0)
            text = QLabel(info_widget['tab_text'])
            text.setStyleSheet("background-color: transparent")
            text.setAlignment(Qt.AlignCenter)
            text_layout.addWidget(text)
            vbox = QVBoxLayout()
            vbox.setAlignment(Qt.AlignCenter)
            vbox.setContentsMargins(0, 0, 0, 0)
            vbox.addWidget(icon)
            vbox.addLayout(text_layout)
            button.setLayout(vbox)
            self.button_tabs.append(button)
        vbox = QVBoxLayout()
        vbox.setAlignment(Qt.AlignLeft)
        vbox.setSpacing(1)

        for btn in self.button_tabs:
            btn.clicked.connect(self.change_stack)
            vbox.addWidget(btn)
        return vbox

    def panels_ui(self):
        self.stacked_panels.setFixedSize(900, 600)
        self.stacked_panels.setStyleSheet(""
                                            "border-top-right-radius: 10px;"
                                            "border-bottom-left-radius: 10px;"
                                            "border-bottom-right-radius: 10px;"
                                          ""
                                          )
        self.stacked_panels.setGraphicsEffect(self.shadow)
        for tab_name, info_widget in self.tabs.items():
            tab_products = {key: p for key, p in self.products.items() if p["underTab"] == tab_name}
            card_product_box = CardProductsBox(self, tab_products, stacked_window=self.stacked_window,
                                               ref_video_thread=self.ref_video_thread)
            vbox = QVBoxLayout()
            vbox.addWidget(card_product_box)
            self.stacked_panels.addWidget(card_product_box)

        vbox = QVBoxLayout()
        vbox.setAlignment(Qt.AlignCenter)
        vbox.setContentsMargins(0, 0, 0, 0)

        self.stacked_panels.setCurrentIndex(0)
        vbox.addWidget(self.stacked_panels)
        return vbox

    def change_stack(self):
        sender = self.sender()
        stack_index = self.tabs[sender.text()]['index']
        for i, button in enumerate(self.button_tabs):
            if i != stack_index:
                button.setStyleSheet(
                    "QPushButton#buttonTab{"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #f0efef;"
                    "color: #f0efef;"
                    "}"
                    "QPushButton#buttonTab:pressed {"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #ffffff;"
                    "color: #ffffff;"
                    "}"
                )
            else:
                button.setStyleSheet(
                    "QPushButton#buttonTab{"
                    "border-top-left-radius: 10px;"
                    "border-bottom-left-radius: 10px;"
                    "border-top-right-radius: 0px;"
                    "border-bottom-right-radius: 0px;"
                    "background-color: #ffffff;"
                    "color: #ffffff;"
                    "}"
                )
        self.stacked_panels.setCurrentIndex(stack_index)