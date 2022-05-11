from PyQt5.QtWidgets import QLabel, QGridLayout
from CardProduct import CardProduct
from ProductWindow import ProductWindow


class CardProductsBox(QLabel):

    def __init__(self, parent, products, max_num_col=3, max_num_row=2, stacked_window=None, ref_video_thread=None):
        super().__init__()
        self.stacked_window = stacked_window
        self.video_thread = ref_video_thread

        self.products = [
            CardProduct(self, product['image_path'], product['text'], product['ingredients'], product['price']) for
            product in products.values()
        ]
        self.new_page_product = {}

        self.max_num_col = max_num_col
        self.max_num_row = max_num_row
        self.grid = QGridLayout()
        self.init_ui()

    def init_ui(self):
        self.setObjectName("productsBox")
        card_width = 240
        card_height = card_width

        for p in self.products:
            p.setFixedSize(card_width, card_height)
            self.new_page_product[p.text] = ProductWindow(None, p.get_products_info(), self.stacked_window, self.video_thread)
            p.clicked.connect(self.push_button)

        self.grid.setVerticalSpacing(0)
        self.setLayout(self.grid)
        self.create_card()

    def create_card(self):
        r = 0
        for i, product in enumerate(self.products):
            c = i % self.max_num_col
            # TODO: trovare formula matematica per r
            if (i % self.max_num_col == 0) and i:
                r += 1
            self.grid.addWidget(product, r, c)

    def push_button(self):
        self.stacked_window.addWidget(self.new_page_product[self.sender().text])
        self.new_page_product[self.sender().text].reset()
        self.stacked_window.setCurrentWidget(self.new_page_product[self.sender().text])
        self.video_thread.pause_loop()
