from PyQt5.QtWidgets import QFrame, QLabel, QVBoxLayout, QHBoxLayout, QWidget, QSizePolicy, QTabWidget, QLayout, \
    QGraphicsDropShadowEffect, QStackedWidget
from PyQt5.Qt import QMovie, QPainter
from PyQt5.QtGui import QIcon, QFont, QColor
from PyQt5.QtCore import Qt, QRect

from LeftTabBarWidget import LeftTabBarWidget
from CardProductsBox import CardProductsBox
from SuggestionBox import SuggestionBox
from FakeTable import FakeTable

import sip
from utils.utils import clear
from utils.definitions import products


class ProductBox(QLabel):

    def __init__(self, parent, width, height, stacked_window=None, ref_video_thread=None):
        super().__init__()
        self.width = width
        self.height = height
        self.stacked_window = stacked_window
        self.ref_video_thread = ref_video_thread

        self.products = products

        self.top_text = QLabel("I suggest you...")
        self.personal_suggestions = None
        self.suggestion_box_layout = QHBoxLayout()
        self.suggestion = QLabel()

        self.suggestion_stack = QStackedWidget()

        self.other_drinks_text = QLabel("Other drinks you might like...")
        self.select_products_box = QLabel()
        # self.select_products_tab_widget = LeftTabBarWidget()
        self.select_products_tabs = {
            "Coffe\ndrinks": {
                "widget": QWidget(),
                "icon": "image/set_from_seco/sezione_coffee_drinks.svg",
                "tab_text": "Coffe\ndrinks",
                "index": 0,
            },
            "Tea": {
                "widget": QWidget(),
                "icon": "image/set_from_seco/sezione_tea.svg",
                "tab_text": "Tea\n",
                "index": 1,
            },
            "Chocolate\ndrinks": {
                "widget": QWidget(),
                "icon": "image/set_from_seco/sezione_chocolate_drinks.svg",
                "tab_text": "Chocolate\ndrinks",
                "index": 2,
            },
        }

        self.fake_table = FakeTable(self, self.select_products_tabs, products, self.stacked_window, self.ref_video_thread)
        self.loader = QMovie("image/loaders/spinner-loader.gif")

        self.init_ui()

    def init_ui(self):
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Minimum)
        self.setObjectName("productsBox")
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(QColor(200, 200, 200))
        shadow.setBlurRadius(50)
        shadow.setOffset(5, 9)
        self.setGraphicsEffect(shadow)

        self.top_text_ui()
        self.suggestion_ui()
        self.other_drinks_text_ui()
        self.select_products_ui()

        vbox = QVBoxLayout()
        vbox.setSizeConstraint(QLayout.SetMinimumSize)
        vbox.addWidget(self.top_text)
        vbox.addLayout(self.suggestion_box_layout)
        vbox.addWidget(self.other_drinks_text)
        vbox.addWidget(self.select_products_box)

        self.setLayout(vbox)

    def suggestion_ui(self):
        self.suggestion.setMovie(self.loader)
        self.suggestion.setAlignment(Qt.AlignCenter)
        self.suggestion.setFixedHeight(int(200 * 2.5))
        self.suggestion.setObjectName("loader")

        self.suggestion_stack.addWidget(self.suggestion)
        self.suggestion_stack.setCurrentIndex(0)

        self.startLoaderAnimation()

        self.suggestion_box_layout.setContentsMargins(11, 0, 10, 0)
        self.suggestion_box_layout.addWidget(self.suggestion_stack)

    def top_text_ui(self):
        font = QFont()
        font.setBold(True)
        font.setPixelSize(40)
        self.top_text.setFont(font)

    def select_products_ui(self):
        self.select_products_box.setMinimumHeight(int(320 * 2))
        # self.select_products_tab_widget.setTabPosition(QTabWidget.West)

        hbox = QHBoxLayout()
        # hbox.addWidget(self.select_products_tab_widget)
        hbox.addWidget(self.fake_table)
        self.select_products_box.setLayout(hbox)

    def other_drinks_text_ui(self):
        font = QFont()
        font.setBold(True)
        font.setPixelSize(40)
        self.other_drinks_text.setFont(font)

    def startLoaderAnimation(self):
        self.loader.start()

    def stopLoaderAnimation(self):
        self.loader.stop()

    def set_loader(self):
        self.suggestion_stack.setCurrentIndex(0)
        self.top_text.setText("I suggest you...")
        self.startLoaderAnimation()
        self.suggestion_stack.removeWidget(self.personal_suggestions)
        clear(self.personal_suggestions.layout())
        self.personal_suggestions.deleteLater()
        sip.delete(self.personal_suggestions)

    def set_suggestion_box(self, info_user):
        self.stopLoaderAnimation()
        self.top_text.setText("Based on your current emotion,\nI suggest you...")
        height_top_text = self.top_text.fontInfo().pixelSize()
        self.personal_suggestions = SuggestionBox(None, self.products, info_user, self.stacked_window,
                                                  self.ref_video_thread)
        self.personal_suggestions.setFixedHeight(self.suggestion.height() - height_top_text)
        self.suggestion_stack.addWidget(self.personal_suggestions)
        self.suggestion_box_layout.setContentsMargins(11, 0, 10, 0)
        self.suggestion_stack.setCurrentWidget(self.personal_suggestions)
