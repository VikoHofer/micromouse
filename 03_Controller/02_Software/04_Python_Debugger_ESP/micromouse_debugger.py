## Team         : Micromouse FH-Hagenberg
## Date         : 16.07.2024     

## Description  : Help Debugging Micromouse on STM32
## Showing Serial Monitor, Configurate PID, Visualize IR-Sensors

## -- Include Section -------------------------------------------
import os
import sys
import time
import asyncio
from bleak import BleakClient, BleakScanner, BleakError

from PyQt5.QtGui import (QFont, QColor, QIcon, QPixmap)
from PyQt5.QtCore import (Qt,QThread,pyqtSignal,pyqtSlot, QTimer)
from PyQt5.QtWidgets import (
    QApplication, QMainWindow,
    QStatusBar,QAction,
    QDesktopWidget,
    QSpacerItem, QMessageBox, QSizePolicy,
    QWidget, QGroupBox, QFrame,
    QSpinBox, QDoubleSpinBox,
    QGridLayout,QHBoxLayout,QVBoxLayout,
    QPlainTextEdit, QPushButton,
    QRadioButton, QLabel, QCheckBox, QComboBox)

from modules.serial_monitor import SerialMonitorWindow
from modules.ir_sensor_view import IRSensorView
from modules.pd_dashboard import Dashboard

import pyqtgraph as pg

sys.coinit_flags = 0  # Set to 0 for MTA (wichtig für Bleak unter Windows)

# Feste BLE-Geräteadresse (ersetzen!)
FIXED_DEVICE_ADDRESS = "a0:b7:65:15:6f:22"
BLE_CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
## --------------------------------------------------------------


## -- Menu Class -------------------------------------------
class Menu(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.ble_reader_thread = None # Hier initialisieren!

        self.device_address = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

    # Setup main menu
    def initUI(self):
        # LAYOUT
        main_layout = QVBoxLayout()

        # Bordering
        main_layout.setContentsMargins(10,10,10,10)
        main_layout.setSpacing(10)

        # Port Layout
        top_layout = QHBoxLayout()

        # Connect Button
        self.btn_connect = QPushButton("Connect")
        self.btn_connect.clicked.connect(self.toggle_connection)
        top_layout.addWidget(self.btn_connect)

        # Logo and Button Layout
        bottom_layout = QVBoxLayout()

        # Logo
        self.label = QLabel()
        pixmap = QPixmap('./images/Micromouse_Logo.png')
        pixmap_scaled = pixmap.scaledToWidth(250) 
        self.label.setPixmap(pixmap_scaled)

        # Buttons
        buttons_layout = QVBoxLayout()
        
        self.label_port = QLabel("Choose function")
        buttons_layout.addWidget(self.label_port, alignment=Qt.AlignCenter)

        self.button1 = QPushButton("Serial Monitor")
        self.button2 = QPushButton("PID Control")
        self.button3 = QPushButton("IR-Sensor View")
        self.enable_buttons(False)
        
        buttons_layout.addWidget(self.button1)
        self.button1.clicked.connect(self.open_serial_monitor)

        buttons_layout.addWidget(self.button2)
        self.button2.clicked.connect(self.open_pd_dashboard)

        buttons_layout.addWidget(self.button3)
        self.button3.clicked.connect(self.open_ir_sensor_view)

        bottom_layout.addWidget(self.label, alignment=Qt.AlignCenter)
        bottom_layout.addLayout(buttons_layout)

        main_layout.addLayout(top_layout)
        main_layout.addLayout(bottom_layout)

        # Main Window Config
        main_widget = QWidget()
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)

        # StatusBar
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)

        self.status_label = QLabel()
        self.status_bar.addWidget(self.status_label)
        self.update_status("Not connected", "red")

        self.setWindowIcon(QIcon('./images/car--plus.png'))
        self.setWindowTitle("Micromouse Debugger Menu")
        self.resize(450, 300)

## ---------------------------------------------------------
    def toggle_connection(self):
            if self.ble_reader_thread:
                self.disconnect_device()
            else:
                self.connect_device()

    def connect_device(self):
        self.ble_reader_thread = BLEReaderThread(FIXED_DEVICE_ADDRESS, BLE_CHARACTERISTIC_UUID)
        self.ble_reader_thread.new_message.connect(self.process_serial_message)
        self.ble_reader_thread.start()

        try:
            async def wait_for_connection():
                await asyncio.wait_for(self.ble_reader_thread.connected_event.wait(), timeout=5.0)
            asyncio.run(wait_for_connection())
            self.update_status(f"Verbunden mit {FIXED_DEVICE_ADDRESS}", "green")
            self.btn_connect.setText("Disconnect")
            self.enable_buttons(True)
        except asyncio.TimeoutError:
            self.update_status(f"Verbindung mit {FIXED_DEVICE_ADDRESS} fehlgeschlagen (Timeout).", "red")
            self.disconnect_device()
        except BleakError as e:
            self.update_status(f"Verbindungsfehler: {e}", "red")
            self.disconnect_device()

    def disconnect_device(self):
        if self.ble_reader_thread:
            self.ble_reader_thread.stop()
            self.ble_reader_thread.wait()
            self.ble_reader_thread = None
            self.update_status("Getrennt", "red")
            self.btn_connect.setText("Connect")
            self.enable_buttons(False)

    @pyqtSlot(str)
    def process_serial_message(self, message):
        if hasattr(self, 'serial_monitor_window') and self.serial_monitor_window:
            self.serial_monitor_window.append_message(message)

    # enable buttons when connected
    def enable_buttons(self, enable):
        self.button1.setEnabled(enable)
        self.button2.setEnabled(enable)
        self.button3.setEnabled(enable)
    
    # update connected state
    def update_status(self, message, color):
        self.status_label.setText(message)
        self.status_label.setStyleSheet(f"color: {color}")


    # Open Serial Monitor when button pressed
    def open_serial_monitor(self):
        self.serial_monitor_window = SerialMonitorWindow()
        if self.ble_reader_thread:  # Überprüfen, ob ein Thread existiert
            self.ble_reader_thread.new_message.connect(self.serial_monitor_window.append_message)
        self.serial_monitor_window.show()

    # Open IR Sensor view when button pressed
    def open_ir_sensor_view(self):
        if self.ble_reader_thread:
            self.ir_sensor_view = IRSensorView(self.ble_reader_thread.client) 
            self.ble_reader_thread.new_sensor_data.connect(self.ir_sensor_view.update_sensor_values) # Signal verbinden
            self.ir_sensor_view.show()
        else:
            QMessageBox.warning(self, "Warnung", "Keine BLE-Verbindung hergestellt")


    # Open PID Dashboard
    def open_pd_dashboard(self):
        if self.ble_reader_thread:
            self.pd_dashboard = Dashboard(self.ble_reader_thread.client) 
            self.ble_reader_thread.new_motion_value.connect(self.pd_dashboard.update_motion_plot)
            self.ble_reader_thread.new_ctrl_value.connect(self.pd_dashboard.update_ctrl_plot)
            self.pd_dashboard.show()
        else:
            QMessageBox.warning(self, "Warnung", "Keine BLE-Verbindung hergestellt")

    # Use Slots to send messages to the Serial Monitor Thread
    @pyqtSlot(str)
    def process_serial_message(self, message):
        if hasattr(self, 'serial_monitor_window') and self.serial_monitor_window:
            self.serial_monitor_window.append_message(message)

## -----------------------------------------------------------------------------------------------


## -- BLEReader Thread -------------------------------------------
class BLEReaderThread(QThread):
    new_message = pyqtSignal(str)       # Signal für BLE-Monitor
    new_sensor_data = pyqtSignal(list)   # Signal für Sensordaten
    new_motion_value = pyqtSignal(int)   # Signal für Speeddaten (Motion Plot)
    new_ctrl_value = pyqtSignal(int)     # Signal für PWM-Daten (Control Plot)

    def __init__(self, device_address, characteristic_uuid):
        super().__init__()
        self.device_address = device_address
        self.characteristic_uuid = characteristic_uuid
        self.client = BleakClient(self.device_address)
        self.connected_event = asyncio.Event()
        self._running = True
        self.buffer = b''

        # State-Machine
        self.state = "IDLE"
        self.tag = 0
        self.length = 0
        self.data = []
        self.cnt = 0
        self.ticks_for_last_byte = 0
        self.INTERBYTE_TIMEOUT_MS = 10

    async def connect_and_listen(self):
        try:
            await self.client.connect()
            print(f"Verbunden mit BLE-Gerät: {self.device_address}")
            self.connected_event.set()
            await self.client.start_notify(self.characteristic_uuid, self.handle_notification)
            while self._running and self.client.is_connected:
                await asyncio.sleep(0.1)
            await self.client.stop_notify(self.characteristic_uuid)
        except BleakError as e:  # Fange Bleak-spezifische Fehler
            print(f"BLE-Fehler: {e}")
        except Exception as e:
            print(f"Fehler in der BLE-Kommunikation: {e}")
        finally:
            if self.client.is_connected:
                try:
                    await self.client.disconnect()
                except Exception as e:
                    print(f"Fehler beim Trennen: {e}")
            self.connected_event.clear()

    def run(self):
        asyncio.run(self.connect_and_listen())

    def stop(self):
        self._running = False
        if self.client.is_connected: #disconnect only if connected
            asyncio.run(self.client.disconnect())

    def handle_notification(self, _, data):
        self.buffer += data
        while self.buffer:
            byte = self.buffer[0]
            self.buffer = self.buffer[1:]
            self.PY_Protocol_Parser(byte)


    # BLE Parser (gleich wie UART Parser)
    def PY_Protocol_Parser(self, rxByte):
        print(f"RXByte Parser: {rxByte}")
        """State machine to parse the incoming bytes."""
        current_time = int(time.time() * 1000)  # Millisekunden-Genauigkeit

        # Check timeout
        if current_time - self.ticks_for_last_byte > self.INTERBYTE_TIMEOUT_MS:
            self.state = "IDLE"

        self.ticks_for_last_byte = current_time

        if self.state == "IDLE":
            self.cnt = 0
            self.tag = rxByte
            self.state = "TAG"

        elif self.state == "TAG":
            self.length = rxByte
            if self.length <= 253:
                self.data = []
                self.cnt = 0
                self.state = "LENGTH"
            else:
                self.state = "IDLE"

        elif self.state == "LENGTH":
            self.data.append(rxByte)
            self.cnt += 1
            if self.cnt == self.length:
                self.Analyse_Data(self.tag, self.data)
                self.state = "IDLE"
        else:
            self.state = "IDLE"

    def Analyse_Data(self, tag, data):
        """Analysiert die empfangenen Daten basierend auf dem Tag."""
        if tag == 0x01:  # CMD_STRING
            try:
                message = bytes(data).decode('utf-8')
                self.new_message.emit(message)
            except UnicodeDecodeError:
                self.new_message.emit("Error decoding string data")

        elif tag == 0x04:  # CMD_SENSOR_VALUE
            sensor_values = self.decode_sensor_data(data)
            self.new_sensor_data.emit(sensor_values)

        elif tag == 0x06:  # CMD_SEND_PWM
            pwm_value = data[0]
            self.new_ctrl_value.emit(pwm_value)

        elif tag == 0x07:  # CMD_SEND_SPEED
            motion_data = (data[0] << 8) | data[1]
            if motion_data >= 0x8000:
                motion_data -= 0x10000
            print(f"SPEED: {motion_data}")
            self.new_motion_value.emit(motion_data)

        else:
            self.new_message.emit(f"Unknown command tag: {tag}")

    def decode_sensor_data(self, data):
        """Decodes the sensor data and returns it as a list of values."""
        sensor_values = []
        if len(data) >= 3:
            sensor_id = data[0]
            sensor_value = (data[1] << 8) | data[2]
            sensor_values.append(sensor_id)
            sensor_values.append(sensor_value)
        else:
            print(f"Received incomplete sensor data: {data}")
        return sensor_values

## ------------------------------------------------------------------

    def stop(self):
        self._running = False
        self.wait()  # Warten auf das Beenden des Threads
## ------------------------------------------------------------------

## -- Main -------------------------------------------
if __name__ == '__main__':
    app = QApplication(sys.argv)

    window = Menu()
    window.show()
    app.exec()

    
## ---------------------------------------------------