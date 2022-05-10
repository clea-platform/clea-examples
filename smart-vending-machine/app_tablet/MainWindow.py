from PyQt5.QtCore import Qt, QSize, pyqtSignal
from PyQt5.QtWidgets import QLabel, QVBoxLayout, QHBoxLayout, QWidget, QInputDialog

from PyQt5.QtGui import QImage, qRgb, QPixmap, QMouseEvent

from VideoThread import VideoThread
# from VideoThread_openVino import VideoThread
from ProductBox import ProductBox


def toQImage(im):
    """
    Utility function to convert a numpy array to a QImage object.
    Args:
        im          numpy array to be converted. It can be a 2D (BW) image or a color image (3 channels + alpha)
    Returns:
        QImage      The image created converting the numpy array
    """
    gray_color_table = [qRgb(i, i, i) for i in range(256)]
    if im is None:
        return QImage()
    if len(im.shape) == 2:  # 1 channel image
        qim = QImage(im.data, im.shape[1], im.shape[0], im.strides[0], QImage.Format_Indexed8)
        qim.setColorTable(gray_color_table)
        return qim
    elif len(im.shape) == 3:
        if im.shape[2] == 3:
            qim = QImage(im.data, im.shape[1], im.shape[0], im.strides[0], QImage.Format_RGB888)
            return qim
        elif im.shape[2] == 4:
            qim = QImage(im.data, im.shape[1], im.shape[0], im.strides[0], QImage.Format_ARGB32)
            return qim


class QLabel_clickable(QLabel):
    clicked = pyqtSignal()

    def mouseDoubleClickEvent(self, a0: QMouseEvent) -> None:
        self.clicked.emit()


class MainWindow(QWidget):

    def __init__(self, width, height, stacked_window=None):
        super().__init__()

        self.width = width
        self.height = height
        self.stacked_window = stacked_window

        self.size_icon = QSize(170, 170)
        self.font_size = int(0.016*self.width)
        self.imgs_width = int(0.15*self.width)

        self.video_thread = VideoThread()
        self.video_thread.updated.connect(self.new_image_slot, type=Qt.QueuedConnection)
        self.current_user = {}

        self.init_ui()
        self.video_thread.start()
        self.pause = False

    def init_ui(self):
        """Method to initialize the UI: layouts and components"""
        self.setWindowTitle("Vending Machine")

        ratio = self.width / self.height
        width = self.width / 2.5

        self.adapted_w = int(width)
        self.adapted_h = int(width / ratio)

        # widget video
        self.viewer = QLabel()
        self.viewer.setObjectName("cameraWidget")
        self.viewer.setStyleSheet("QLabel#cameraWidget {background-color: black;}")
        self.viewer.setAlignment(Qt.AlignCenter)
        self.viewer_layout = QHBoxLayout()
        self.viewer_layout.setContentsMargins(0, 15, 0, 0)
        self.viewer_layout.addWidget(self.viewer)

        # widget products
        self.products_box = ProductBox(self, self.width, self.height - 5, self.stacked_window, self.video_thread)
        self.products_box.setStyleSheet("background-color: #ffffff")
        self.products_box.setMinimumHeight(25)
        self.products_box_layout = QHBoxLayout()
        self.products_box_layout.setContentsMargins(0, 0, 0, 20)
        self.products_box_layout.addWidget(self.products_box)

        # Footer
        self.footer = self.footer_ui()

        vbox = QVBoxLayout()
        vbox.addLayout(self.viewer_layout)
        vbox.addLayout(self.products_box_layout)
        vbox.addStretch(1)
        vbox.addLayout(self.footer)
        vbox.setContentsMargins(20, 0, 20, 20)
        self.setLayout(vbox)

    def footer_ui(self):
        seco_label = QLabel_clickable()
        seco_image = QPixmap("image/seco-logo-1.png").scaled(self.size_icon, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        seco_label.setPixmap(seco_image)
        seco_label.clicked.connect(self.show_secret_frame_thr)
        clea_label = QLabel_clickable()
        clea_image = QPixmap("image/clea_logo.png").scaled(self.size_icon, Qt.KeepAspectRatio, Qt.SmoothTransformation)
        clea_label.setPixmap(clea_image)
        clea_label.clicked.connect(self.show_secret_time_thr)
        hbox = QHBoxLayout()
        hbox.addWidget(seco_label)
        hbox.addStretch(1)
        hbox.addWidget(clea_label)
        return hbox

    def new_image_slot(self):
        """Qt Slot for updated signal of the FaceRecogniser. Called every time a new frame is elaborated"""
        if not self.pause:
            image = self.video_thread.get_current_frame()
            self.updateView(image)
            if str(self.current_user) != str(self.video_thread.get_info_user()):
                self.current_user = self.video_thread.get_info_user()
                if not self.current_user:
                    self.products_box.set_loader()
                    self.viewer.setStyleSheet("QLabel#cameraWidget {background-color: black;}")
                else:
                    self.products_box.set_suggestion_box(self.current_user)
                    self.viewer.setStyleSheet("QLabel#cameraWidget {background-color: transparent;}")

    def updateView(self, mat=None):
        """Update the view converting the current state (np.ndarray) to an image (QPixmap) and showing it on screen"""
        if mat is None:
            mat = self.last_mat
        else:
            self.last_mat = mat            

        qim = toQImage(mat)  # first convert to QImage
        qpix = QPixmap.fromImage(qim)  # then convert to QPixmap
        self.viewer.setPixmap(qpix.scaled(QSize(self.adapted_w, self.adapted_h), Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def show_secret_frame_thr(self):
        self.video_thread.pause_loop()
        num, pressed = QInputDialog.getInt(self, "Options Frame", "Frame threshold: ", self.video_thread.frame_threshold)
        if pressed:
            self.video_thread.set_frame_threshold(num)
        self.video_thread.unpause()

    def show_secret_time_thr(self):
        self.video_thread.pause_loop()
        num, pressed = QInputDialog.getInt(self, "Options Time", "Time pause threshold: ", self.video_thread.time_threshold)
        if pressed:
            self.video_thread.set_time_threshold(num)
        self.video_thread.unpause()

    def closeEvent(self, event):
        print("CLOSED")
        self.video_thread.deactivate()
        event.accept()
