from PyQt5.QtWidgets import QLabel, QVBoxLayout, QGraphicsDropShadowEffect, QFrame, QHBoxLayout, QPushButton, QStylePainter, QBoxLayout
from PyQt5.QtGui import QPixmap, QPaintEvent, QFont, QFontDatabase, QPainter, QColor
from PyQt5.QtCore import Qt, QSize, QMargins
from PyQt5.QtSvg import QSvgWidget, QSvgRenderer

import copy


class CardProduct(QPushButton):
    def __init__(self, parent, icon, text_name, text_ingredients, price, icon_size=(130, 130)):
        super().__init__(parent=parent)
        self.icon = icon
        self.size_icon = QSize(*icon_size)
        self.text = text_name
        self.text_ingredients = text_ingredients
        self.price = "{:.2f}€".format(price)
        self.card_ui()

    def card_ui(self):
        self.setObjectName("cardProductButton")
        self.setStyleSheet("QPushButton#cardProductButton{"
                           "border: 2px solid #eaeaea;"
                           "border-radius: 9px;"
                           "background-color: #ffffff;"
                           "}"
                           "QPushButton#cardProductButton:pressed{"
                           "border-radius: 9px;"
                           "border: 2px solid #eaeaea;"
                           "background-color: #888888;"
                           "}"
                           )
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(QColor(200, 200, 200))
        shadow.setBlurRadius(40)
        shadow.setOffset(5, 8)
        self.setGraphicsEffect(shadow)

        self.draw_inside()

    def draw_inside(self):
        size = self.sizeHint()
        size_factor = 1 if self.size_icon.height() == 130 else 1.25
        # -------- rect ---------
        # Icon
        rect_image = copy.deepcopy(self.rect())
        rect_image.setY(rect_image.y() - 52)
        # Text name
        rect_text_name = copy.deepcopy(rect_image)
        rect_text_name.setY(int(rect_text_name.y() + self.size_icon.height() + size.height()/2 - 20))
        # Text ingredients
        rect_text_ingredients = copy.deepcopy(rect_text_name)
        rect_text_ingredients.setY(rect_text_ingredients.y() + 10)
        # Text price
        rect_text_price = copy.deepcopy(rect_text_ingredients)
        rect_text_price.setY(rect_text_price.y() + 32)
        # .........................

        # Icon
        image_label = QSvgWidget(self.icon)
        image_label.setGeometry(rect_image)
        image_label.setFixedSize(self.size_icon)
        image_label.setStyleSheet("background-color: transparent")
        layout_image = QHBoxLayout()
        layout_image.setContentsMargins(0, 0, 0, 0)
        layout_image.setAlignment(Qt.AlignCenter)
        layout_image.addWidget(image_label)

        # Text name
        text_name_label = QLabel()
        text_name_label.setGeometry(rect_text_name)
        text_name_label.setAlignment(Qt.AlignCenter)
        text_name_label.setStyleSheet("background-color: transparent")
        font_text_name = QFont()
        font_text_name.setBold(True)
        font_text_name.setPixelSize(int(27 * size_factor))
        layout_text_name = QHBoxLayout()
        layout_text_name.setContentsMargins(0, int(5*size_factor), 0, 0)
        layout_text_name.addWidget(text_name_label)
        text_name_label.setFont(font_text_name)
        text_name_label.setText(self.text)
        # Text ingredients
        text_ingredients_label = QLabel()
        text_ingredients_label.setGeometry(rect_text_price)
        font_text_ingredients = QFont()
        font_text_ingredients.setPixelSize(int(14*size_factor))
        layout_text_ingredients = QHBoxLayout()
        layout_text_ingredients.setContentsMargins(0, 0, 0, int(20*size_factor))
        layout_text_ingredients.addWidget(text_ingredients_label)
        text_ingredients_label.setFont(font_text_ingredients)
        text_ingredients_label.setStyleSheet("color: grey; background-color: transparent;")
        text_ingredients_label.setAlignment(Qt.AlignCenter)
        text_ingredients_label.setText(self.text_ingredients)
        # Text price
        price_label = QLabel()
        price_label.setGeometry(rect_text_price)
        price_label.setAlignment(Qt.AlignCenter)
        price_label.setStyleSheet("background-color: transparent")
        font_price = QFont()
        font_price.setBold(True)
        font_price.setPixelSize(int(15*size_factor))
        price_label.setFont(font_price)
        price_label.setText(self.price)
        layout_price = QHBoxLayout()
        layout_price.setContentsMargins(0, 0, 0, int(30*size_factor))
        layout_price.setAlignment(Qt.AlignCenter)
        layout_price.addWidget(price_label)

        vbox = QVBoxLayout()
        vbox.addLayout(layout_image)
        vbox.addLayout(layout_text_name)
        vbox.addLayout(layout_text_ingredients)
        vbox.addWidget(price_label)
        vbox.setSpacing(0)
        self.setLayout(vbox)

    def get_products_info(self):
        info_product = {
            "icon": self.icon,
            "text_name": self.text,
            "text_ingredients": self.text_ingredients,
            "price": self.price
        }
        return info_product