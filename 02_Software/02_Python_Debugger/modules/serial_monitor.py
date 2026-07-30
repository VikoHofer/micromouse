# serial_monitor.py

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QPlainTextEdit, QPushButton, QHBoxLayout, QFileDialog
from PyQt5.QtCore import pyqtSlot
from PyQt5.QtGui import QIcon
import csv
import re
import os

## -- SerialMonitor Window -------------------------------------------
class SerialMonitorWindow(QWidget):
    def __init__(self, autosave_on_close=False):
        super().__init__()
        self.autosave_on_close = autosave_on_close
        self.messages = []            # raw messages
        self.parsed_values = []       # numeric values extracted
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Serial Monitor")
        self.setGeometry(100, 100, 1000, 800)

        main_layout = QVBoxLayout()
        top_layout = QHBoxLayout()

        # save button
        self.btn_save = QPushButton("Save CSV")
        self.btn_save.clicked.connect(self.on_save_clicked)
        top_layout.addWidget(self.btn_save)

        # quick save without dialog (useful for tests)
        self.btn_save_quick = QPushButton("Quick Save")
        self.btn_save_quick.clicked.connect(self.on_quick_save_clicked)
        top_layout.addWidget(self.btn_save_quick)

        # optional: clear buffer button
        self.btn_clear = QPushButton("Clear")
        self.btn_clear.clicked.connect(self.on_clear_clicked)
        top_layout.addWidget(self.btn_clear)

        main_layout.addLayout(top_layout)

        self.text_edit = QPlainTextEdit()
        self.text_edit.setReadOnly(True)
        self.setWindowIcon(QIcon('./images/edit.png'))
        main_layout.addWidget(self.text_edit)
        self.setLayout(main_layout)

    @pyqtSlot(str)
    def append_message(self, message):
        """Append incoming message and extract numeric values."""
        # normalize and store raw line
        text = message.rstrip('\r\n')
        self.messages.append(text)
        # show in UI
        self.text_edit.appendPlainText(text)

        # extract ints from the message line (one or more per line)
        # accepts negative numbers too
        nums = re.findall(r'-?\d+', text)
        for n in nums:
            try:
                val = int(n)
                self.parsed_values.append(val)
            except:
                pass

    def on_save_clicked(self):
        """Ask user for file path and save CSV."""
        path, _ = QFileDialog.getSaveFileName(self, "Save CSV", "serial_log.csv", "CSV Files (*.csv);;All Files (*)")
        if path:
            try:
                self.save_csv(path)
                self.text_edit.appendPlainText(f"Saved {len(self.parsed_values)} values to: {path}")
            except Exception as e:
                self.text_edit.appendPlainText(f"Error saving CSV: {e}")

    def on_quick_save_clicked(self):
        """Save to working directory with fixed name for quick tests."""
        default = os.path.join(os.getcwd(), "serial_log_quick.csv")
        try:
            self.save_csv(default)
            self.text_edit.appendPlainText(f"Quick-saved {len(self.parsed_values)} values to: {default}")
        except Exception as e:
            self.text_edit.appendPlainText(f"Error quick-saving CSV: {e}")

    def on_clear_clicked(self):
        """Clear UI and buffers."""
        self.messages.clear()
        self.parsed_values.clear()
        self.text_edit.clear()

    def save_csv(self, path):
        """Write extracted numeric values (one per row) to CSV file."""
        # write header "value" and then one number per row
        with open(path, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow(['value'])
            for v in self.parsed_values:
                writer.writerow([v])

    def closeEvent(self, event):
        """Auto-save on close if requested."""
        if self.autosave_on_close and self.parsed_values:
            # default quick filename
            default = os.path.join(os.getcwd(), "serial_log_autosave.csv")
            try:
                self.save_csv(default)
                self.text_edit.appendPlainText(f"Autosaved {len(self.parsed_values)} values to: {default}")
            except Exception as e:
                self.text_edit.appendPlainText(f"Error autosaving CSV: {e}")
        super().closeEvent(event)
