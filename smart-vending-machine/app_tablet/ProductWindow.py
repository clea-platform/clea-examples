from PyQt5.QtCore import Qt, QSize, QThread, pyqtSignal
from PyQt5.QtWidgets import QLabel, QVBoxLayout, QHBoxLayout, QWidget, QPushButton, QGraphicsDropShadowEffect, QProgressBar
from PyQt5.QtGui import QPixmap, QFont, QColor
from PyQt5.QtSvg import QSvgWidget

import time
from utils.suggestion_strategy import suggest_product
from astarte.as_conn import Astarte, send_data


class ProductWindow(QWidget):
    def __init__(self, parent=None, info_product=None, stacked_ref=None, video_thread=None):
        super(ProductWindow, self).__init__(parent=parent)
        self.icon = info_product['icon']
        self.text_name = info_product['text_name']
        self.text_ingredients = info_product['text_ingredients']
        self.price = info_product['price']
        self.stacked_ref = stacked_ref
        self.video_thread = video_thread

        self.size_icon = QSize(170, 170)

        self.sugar_rect = []
        self.max_sugar = 5
        self.sugar_amount = 2
        self.card = QLabel()
        self.go_back_button = QPushButton("Go back")
        self.confirm_button = QPushButton("CONFIRM")

        self.p_bar_container = QHBoxLayout()
        self.p_text_top = QLabel()
        self.p_bar_card = QLabel()
        self.p_text_below = QLabel()
        self.p_bar = QProgressBar()
        self.calc = FakeCountProgress()
        self.timeout_event = TimeoutEvent()

        self.ready_container = QHBoxLayout()
        self.ready_card = QLabel()
        self.ready_image_label = QLabel()
        self.ready_image = QSvgWidget("image/set_from_seco/messaggio_di_output.svg")

        self.init_ui()

    def init_ui(self):

        self.card_ui()
        self.progress_bar_ui()
        self.ready_ui()
        footer = self.footer()

        self.card.setMinimumHeight(550)
        self.p_bar_card.setMinimumHeight(450)
        self.ready_card.setMinimumHeight(500)

        vbox = QVBoxLayout()
        vbox.setAlignment(Qt.AlignTop)
        vbox.addWidget(self.card)
        vbox.addSpacing(20)
        vbox.addLayout(self.p_bar_container)
        vbox.addSpacing(20)
        vbox.addLayout(self.ready_container)
        vbox.addStretch(1)
        vbox.addLayout(footer)
        vbox.setContentsMargins(20, 100, 20, 20)

        self.setLayout(vbox)

    def card_ui(self):
        self.card.setObjectName("secondPageCard")
        self.card.setStyleSheet("QLabel#secondPageCard{border: 2px solid #eaeaea; border-radius: 4px; background-color: white}")
        # shadow = QGraphicsDropShadowEffect()
        # shadow.setColor(QColor(200, 200, 200))
        # shadow.setBlurRadius(50)
        # shadow.setOffset(7, 10)
        # self.card.setGraphicsEffect(shadow)

        top_text = self.top_text()
        card_product = self.card_product_and_sugar()
        buttons = self.push_buttons()
        vbox = QVBoxLayout()
        vbox.addLayout(top_text)
        vbox.addStretch(1)
        vbox.addWidget(card_product)
        vbox.addStretch(1)

        vbox.addLayout(buttons)
        vbox.setContentsMargins(20, 30, 20, 20)
        self.card.setLayout(vbox)

    def top_text(self):
        font = QFont()
        font.setBold(True)
        font.setPixelSize(48)
        text = QLabel("Your selection...")
        text.setFont(font)
        hbox = QHBoxLayout()
        hbox.setAlignment(Qt.AlignLeft)
        hbox.setContentsMargins(0, 25, 0, 0)
        hbox.addWidget(text)
        hbox.addStretch(1)
        return hbox

    def card_product_and_sugar(self):
        card_product = QLabel()
        card_product.setObjectName("secondPageCardProduct")
        card_product.setStyleSheet("QLabel#secondPageCardProduct{border: 3px solid #eaeaea; border-radius: 8px; background-color: white}")
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(QColor(234, 234, 234))
        shadow.setBlurRadius(40)
        shadow.setOffset(3, 3)
        card_product.setGraphicsEffect(shadow)
        card_product.setMinimumHeight(230)

        hbox = QHBoxLayout()

        # Icon
        image_label = QSvgWidget(self.icon)
        image_label.setFixedSize(self.size_icon)
        # text
        vbox_text = self.text_product_ui()
        # Sugar
        sugar_slider = self.sugar_ui()

        hbox.addWidget(image_label)
        hbox.addStretch(1)
        hbox.addLayout(vbox_text)
        hbox.addStretch(1)
        hbox.addLayout(sugar_slider)
        card_product.setLayout(hbox)
        return card_product

    def text_product_ui(self):
        vbox = QVBoxLayout()
        vbox.setAlignment(Qt.AlignCenter)
        vbox.setSpacing(3)
        vbox.setContentsMargins(0, 0, 100, 0)
        text_name_label = QLabel(self.text_name)
        font_text_name_label = QFont()
        font_text_name_label.setBold(True)
        font_text_name_label.setPixelSize(40)
        text_name_label.setFont(font_text_name_label)
        layout_text_name_label = QHBoxLayout()
        layout_text_name_label.setContentsMargins(0, 0, 0, 0)
        layout_text_name_label.addWidget(text_name_label)
        text_ingredients_label = QLabel(self.text_ingredients)
        font_text_ingredients_label = QFont()
        font_text_ingredients_label.setBold(False)
        font_text_ingredients_label.setPixelSize(20)
        text_ingredients_label.setFont(font_text_ingredients_label)
        text_ingredients_label.setStyleSheet("color: grey")
        layout_text_ingredients_label = QHBoxLayout()
        # layout_text_ingredients_label.setContentsMargins(0, 0, 0, 30)
        layout_text_ingredients_label.addWidget(text_ingredients_label)
        price_label = QLabel(self.price)
        font_price_label = QFont()
        font_price_label.setBold(True)
        font_price_label.setPixelSize(23)
        price_label.setFont(font_price_label)
        vbox.addLayout(layout_text_name_label)
        vbox.addLayout(layout_text_ingredients_label)
        vbox.addWidget(price_label)

        return vbox

    def sugar_ui(self):
        hbox = QHBoxLayout()

        sugar_text = QLabel("Sugar")
        font = QFont()
        font.setPixelSize(28)
        sugar_text.setFont(font)
        size_sugar_rect = QSize(13, 50)
        size_sugar_button = QSize(32, 32)
        style_plus_minus = "QPushButton#plusMinusButton{border-radius: 12px;background-color: #11b0ef;color: white;}" \
                           "QPushButton#plusMinusButton:pressed {background-color: #7ecff0;}" \
                           "QPushButton#plusMinusButton:disabled {background-color: #7ecff0;}"
        sugar_button_minus = QPushButton("-")
        sugar_button_minus.setObjectName("plusMinusButton")
        sugar_button_minus.setStyleSheet(style_plus_minus)
        sugar_button_minus.setFont(font)
        sugar_button_minus.setFixedSize(size_sugar_button)
        sugar_button_minus.clicked.connect(self.minus_sugar)
        sugar_button_plus = QPushButton("+")
        sugar_button_plus.setObjectName("plusMinusButton")
        sugar_button_plus.setStyleSheet(style_plus_minus)
        sugar_button_plus.setFont(font)
        sugar_button_plus.setFixedSize(size_sugar_button)
        sugar_button_plus.clicked.connect(self.plus_sugar)
        for i in range(0, self.max_sugar):
            self.sugar_rect.append(QLabel())
            self.sugar_rect[i].setFixedSize(size_sugar_rect)
            if i < self.sugar_amount:
                self.sugar_rect[i].setObjectName("sugarYes")
                self.sugar_rect[i].setStyleSheet("QLabel#sugarYes{border-radius: 5px; background-color: #11b0ef;}")
            else:
                self.sugar_rect[i].setObjectName("sugarNo")
                self.sugar_rect[i].setStyleSheet("QLabel#sugarNo{border-radius: 5px; background-color: #d8d8d8;}")

        hbox.addWidget(sugar_text)
        hbox.addWidget(sugar_button_minus)
        for rect in self.sugar_rect:
            hbox.addWidget(rect)
        hbox.addWidget(sugar_button_plus)

        return hbox

    def push_buttons(self):
        font = QFont()
        font.setBold(True)
        font.setPixelSize(30)
        self.confirm_button.setFont(font)
        self.confirm_button.setObjectName("secondPageConfirmButton")
        self.confirm_button.setStyleSheet(
            "QPushButton#secondPageConfirmButton{"
            "border-radius: 12px;"
            "background-color: #11b0ef;"
            "color: white;"
            "}"
            "QPushButton#secondPageConfirmButton:pressed {"
            "background-color: #7ecff0;"
            "}"
            "QPushButton#secondPageConfirmButton:disabled {"
            "background-color: #7ecff0;"
            "}"
        )
        self.confirm_button.setFixedSize(300, 70)
        self.confirm_button.clicked.connect(self.confirm)
        self.go_back_button.setFont(font)
        self.go_back_button.setObjectName("secondPageGoBackButton")
        self.go_back_button.setStyleSheet(
            "QPushButton#secondPageGoBackButton{"
            "border-radius: 12px;"
            "background-color: #b9ac1e;"
            "color: white;"
            "}"
            "QPushButton#secondPageGoBackButton:pressed {"
            "background-color: #b7b273;"
            "}"
            "QPushButton#secondPageGoBackButton:disabled {"
            "background-color: #b7b273;"
            "}"
        )
        self.go_back_button.setFixedSize(200, 70)
        self.go_back_button.clicked.connect(self.go_back)
        hbox = QHBoxLayout()
        hbox.addWidget(self.go_back_button)
        hbox.addStretch(1)
        hbox.addWidget(self.confirm_button)
        return hbox

    def progress_bar_ui(self):
        self.p_bar.setEnabled(False)
        self.p_bar.setObjectName("progressDispensing")
        self.p_bar_card.setObjectName("secondPageCardProgressBar")
        self.p_bar_card.setStyleSheet("QLabel#secondPageCardProgressBar{"
                                      "border: 3px solid #eaeaea;"
                                      "border-radius: 3px;"
                                      "background-color: white;"
                                      "}"
                                      )
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(QColor(200, 200, 200))
        shadow.setBlurRadius(50)
        shadow.setOffset(7, 10)
        self.p_bar_card.setGraphicsEffect(shadow)
        layout_text_top = QHBoxLayout()
        layout_text_top.setContentsMargins(0, 0, 0, 75)
        font_text_top = QFont()
        font_text_top.setBold(True)
        font_text_top.setPixelSize(60)
        self.p_text_top.setText("Dispensing")
        self.p_text_top.setFont(font_text_top)
        layout_text_top.addWidget(self.p_text_top)
        layout_text_bottom = QHBoxLayout()
        layout_text_bottom.setContentsMargins(10, 0, 0, 120)
        font_text_bottom = QFont()
        font_text_bottom.setBold(False)
        font_text_bottom.setPixelSize(20)
        self.p_text_below.setText("I'm heating the water...")
        self.p_text_below.setFont(font_text_bottom)
        layout_text_bottom.addWidget(self.p_text_below)
        vbox = QVBoxLayout()
        vbox.setContentsMargins(40, 0, 40, 0)
        vbox.addLayout(layout_text_top)
        vbox.addWidget(self.p_bar)
        vbox.addLayout(layout_text_bottom)
        vbox.setAlignment(Qt.AlignLeft)
        self.p_bar_card.setLayout(vbox)
        self.p_bar_card.hide()
        self.p_bar_container.addWidget(self.p_bar_card)

    def ready_ui(self):
        self.ready_card.setObjectName("readyCard")
        self.ready_card.setStyleSheet("QLabel#readyCard{border: 3px solid #eaeaea; border-radius: 3px; background-color: white;}")
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(QColor(200, 200, 200))
        shadow.setBlurRadius(50)
        shadow.setOffset(7, 10)

        self.ready_card.setGraphicsEffect(shadow)
        size_label = QSize(400, 400)
        self.ready_image.setFixedSize(size_label)
        self.ready_image.setStyleSheet("background-color: transparent")

        read_text = QLabel("Your drink\nis\nREADY!")
        read_text.setAlignment(Qt.AlignCenter)
        font = QFont()
        font.setBold(True)
        font.setPixelSize(50)
        read_text.setFont(font)
        hbox = QHBoxLayout()
        hbox.setAlignment(Qt.AlignCenter)
        hbox.addWidget(self.ready_image)
        hbox.addSpacing(200)
        hbox.addWidget(read_text)
        self.ready_card.setLayout(hbox)
        self.ready_card.hide()
        self.ready_container.addWidget(self.ready_card)

    def footer(self):
        seco_label = QLabel()
        seco_image = QPixmap("image/seco-logo-1.png").scaled(self.size_icon, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        seco_label.setPixmap(seco_image)
        clea_label = QLabel()
        clea_image = QPixmap("image/clea_logo.png").scaled(self.size_icon, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        clea_label.setPixmap(clea_image)
        hbox = QHBoxLayout()
        hbox.addWidget(seco_label)
        hbox.addStretch(1)
        hbox.addWidget(clea_label)
        return hbox

    def go_back(self):
        self.stacked_ref.setCurrentIndex(0)
        self.stacked_ref.removeWidget(self)
        self.video_thread.unpause()

    def confirm(self):
        self.confirm_button.setEnabled(False)
        self.go_back_button.setEnabled(False)
        self.p_bar_card.show()
        self.p_bar.setStyleSheet("QProgressBar#progressDispensing::chunk {"
                                 "background-color: #11b0ef;"
                                 "border-radius: 10px;"
                                 "}")

        self.calc.countChanged.connect(self.onCountChanged)
        self.calc.start()

    def onCountChanged(self, value):
        self.p_bar.setValue(value)
        if value == 100:

            self.to_astarte()

            self.p_text_top.setText("Dispensed")
            self.p_text_below.setText("Finish...")
            self.p_bar.setStyleSheet("QProgressBar#progressDispensing::chunk {"
            "background-color: #41c126;"
            "border-radius: 10px;"
            "}")
            self.timeout_event.timeoutSignal.connect(self.go_back)
            self.timeout_event.start()
            self.ready_card.show()

    def minus_sugar(self):
        if self.sugar_amount > 0:
            self.sugar_amount -= 1
            self.sugar_rect[self.sugar_amount].setObjectName("sugarNo")
            self.sugar_rect[self.sugar_amount].setStyleSheet("QLabel#sugarNo{border-radius: 5px; background-color: #d8d8d8;}")

    def plus_sugar(self):
        if self.sugar_amount < self.max_sugar:
            self.sugar_amount += 1
            self.sugar_rect[self.sugar_amount-1].setObjectName("sugarYes")
            self.sugar_rect[self.sugar_amount-1].setStyleSheet("QLabel#sugarYes{border-radius: 5px; background-color: #11b0ef;}")

    def reset(self):
        # Sugar
        self.sugar_amount = 2
        for i in range(0, self.max_sugar):
            if i < self.sugar_amount:
                self.sugar_rect[i].setObjectName("sugarYes")
                self.sugar_rect[i].setStyleSheet("QLabel#sugarYes{border-radius: 5px; background-color: #11b0ef;}")
            else:
                self.sugar_rect[i].setObjectName("sugarNo")
                self.sugar_rect[i].setStyleSheet("QLabel#sugarNo{border-radius: 5px; background-color: #d8d8d8;}")

        # Confirm button
        self.confirm_button.setEnabled(True)
        self.go_back_button.setEnabled(True)
        # Progress Bar
        self.p_bar_card.hide()
        self.p_bar.setValue(0)
        # Ready
        self.ready_card.hide()

    def to_astarte(self):
        user_info = self.video_thread.get_info_user()
        suggestions = suggest_product(**user_info)
        if self.text_name in suggestions:
            suggestion = self.text_name
        else:
            suggestion = suggestions[0] if len(suggestions) else "None"

        data_to_astarte = {
            "age":  int(user_info["age"]) if "age" in user_info else 0,
            "gender": user_info["gender"] if "gender" in user_info else "None",
            "emotion": user_info["emotion"] if "emotion" in user_info else "None",
            "suggestion": suggestion,
            "choice": self.text_name,
            "price": float(self.price[:-1]),
        }

        astarte = Astarte()
        try:
            send_data(astarte.device, data_to_astarte)
        except Exception as e:
            print(f"Errore inivio ad Astarte: {e}")


class TimeoutEvent(QThread):
    timeoutSignal = pyqtSignal()

    def run(self):
        time.sleep(3)
        self.timeoutSignal.emit()


class FakeCountProgress(QThread):
    """
    Runs a counter thread.
    """
    countChanged = pyqtSignal(int)

    def run(self):
        count = 0
        while count < 100:
            count += 1
            time.sleep(0.05)
            self.countChanged.emit(count)
