# serial_monitor.py

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QPlainTextEdit
from PyQt5.QtCore import (Qt,QThread,pyqtSignal,pyqtSlot)
from PyQt5.QtGui import (QFont, QColor, QIcon, QPixmap)

## -- SerialMonitor Window -------------------------------------------
class SerialMonitorWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Serial Monitor")
        self.setGeometry(100, 100, 600, 400)
        layout = QVBoxLayout()
        self.text_edit = QPlainTextEdit()
        self.text_edit.setReadOnly(True)

        self.setWindowIcon(QIcon('./images/edit.png'))
        layout.addWidget(self.text_edit)
        self.setLayout(layout)

    @pyqtSlot(str)
    def append_message(self, message):
        self.text_edit.appendPlainText(('>'+ message))