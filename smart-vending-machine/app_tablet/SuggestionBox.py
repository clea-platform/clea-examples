from PyQt5.QtWidgets import QLabel, QHBoxLayout, QSizePolicy
from PyQt5.QtCore import Qt

from CardProduct import CardProduct
from ProductWindow import ProductWindow

from utils import suggestion_strategy


class SuggestionBox(QLabel):

    def __init__(self, parent, products=None, info_user=None, stacked_window=None, video_thread=None):
        super().__init__(parent=parent)

        self.stacked_window = stacked_window
        self.video_thread = video_thread

        self.products = products
        self.info_user = info_user

        self.new_page_product = {}

        if info_user:
            self.suggested_products = suggestion_strategy.suggest_product(**info_user)
        else:
            self.suggested_products = [key for key in self.products.keys()]

        self.init_ui()

    def init_ui(self):
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Minimum)
        self.setWordWrap(True)
        self.setAlignment(Qt.AlignCenter)
        self.setObjectName("suggestionBox")

        cards = self.cards_ui()

        self.setLayout(cards)

    def cards_ui(self):
        card_width = int(int((self.width() / 4) - 35) * 2.3)
        card_height = card_width

        hbox = QHBoxLayout()
        hbox.setAlignment(Qt.AlignLeft)
        hbox.setSpacing(50)
        for key, product in self.products.items():
            if key in self.suggested_products:
                p = CardProduct(self, product['image_path'], product['text'], product['ingredients'], product['price'],
                                icon_size=(162, 162))
                self.new_page_product[key] = ProductWindow(None, p.get_products_info(), self.stacked_window, self.video_thread)
                p.setMaximumSize(card_width, card_height)
                p.setMinimumSize(card_width, card_height)
                p.clicked.connect(self.push_button)
                hbox.addWidget(p)
        return hbox

    def push_button(self):
        self.stacked_window.addWidget(self.new_page_product[self.sender().text])
        self.stacked_window.setCurrentWidget(self.new_page_product[self.sender().text])
        self.video_thread.pause_loop()



