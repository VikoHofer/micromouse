## Team         : Micromouse FH-Hagenberg
## Date         : 16.07.2024     

## Description  : Help Debugging Micromouse on STM32
## Showing Serial Monitor, Configurate PID, Visualize IR-Sensors

## -- Include Section -------------------------------------------
import sys
import time
import serial

from serial import Serial
import serial.tools.list_ports

from PyQt5.QtGui import (QIcon, QPixmap)
from PyQt5.QtCore import (Qt,QThread,pyqtSignal,pyqtSlot)
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QStatusBar, QMessageBox, QWidget,
    QHBoxLayout,QVBoxLayout, QPushButton, QLabel, QComboBox)

from modules.serial_monitor import SerialMonitorWindow
from modules.ir_sensor_view import IRSensorView
from modules.pd_dashboard import Dashboard

## --------------------------------------------------------------



## -- Menu Class -------------------------------------------
class Menu(QMainWindow):
    
    # C-Tor
    def __init__(self):
        super().__init__()
        self.initUI()
        self.serial_port = None
        self.serial_reader_thread = None

    # Setup main menu
    def initUI(self):
        # LAYOUT
        main_layout = QVBoxLayout()

        # Bordering
        main_layout.setContentsMargins(10,10,10,10)
        main_layout.setSpacing(10)

        # Port Layout
        top_layout = QHBoxLayout()
        self.label_port = QLabel("SELECT PORT:")
        top_layout.addWidget(self.label_port)

        # Refresh Button
        self.btn_refresh = QPushButton("Search")
        self.btn_refresh.clicked.connect(self.search_ports)
        top_layout.addWidget(self.btn_refresh)

        # COMBOBOX => select Ports
        self.combobox_ports = QComboBox()
        top_layout.addWidget(self.combobox_ports)

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

    # Update port list
    def update_ports_combobox(self, port_list):
        self.combobox_ports.clear()
        self.combobox_ports.addItems(port_list)
        if not port_list:
            self.update_status("No Ports found", "red")
            self.btn_connect.setEnabled(True)
            self.btn_refresh.setEnabled(True)
            self.combobox_ports.setEnabled(True)
            self.enable_buttons(False)

    # Search ports
    def search_ports(self):
        ports = serial.tools.list_ports.comports()
        port_list = [port.device for port in ports]
        self.update_ports_combobox(port_list)

    # Toggle connection
    def toggle_connection(self):
        if self.serial_port and self.serial_port.is_open:
            self.disconnect_port()
        else:
            self.connect_port()


    # Connect to selected port
    def connect_port(self):
        selected_port = self.combobox_ports.currentText()
        if selected_port:
            try:
                self.serial_port = Serial(
                    selected_port,
                    baudrate=115200, 
                    stopbits=serial.STOPBITS_ONE,
                    parity=serial.PARITY_NONE,
                    timeout=0.000001 
                )
                self.serial_reader_thread = SerialReaderThread(self.serial_port)
                self.serial_reader_thread.new_message.connect(self.process_serial_message)
                self.serial_reader_thread.start()
                self.update_status(f"Connected to {selected_port}", "green")
                self.btn_connect.setText("Disconnect")
                self.btn_refresh.setEnabled(False)
                self.combobox_ports.setEnabled(False)
                self.enable_buttons(True)
            except Exception as e:
                self.update_status(f"Failed to connect: {e}", "red")

    # Disconnect from the port
    def disconnect_port(self):
        if self.serial_reader_thread:
            self.serial_reader_thread.stop()
        if self.serial_port:
            try:
                self.serial_port.close()
                self.update_status("Disconnected", "red")
                self.btn_connect.setText("Connect")
                self.btn_refresh.setEnabled(True)
                self.combobox_ports.setEnabled(True)
                self.enable_buttons(False)
            except Exception as e:
                self.update_status(f"Failed to disconnect: {e}", "red")

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
        self.serial_monitor_window.show()

    # Open IR Sensor view when button pressed
    def open_ir_sensor_view(self):
        if self.serial_port:
            self.ir_sensor_view = IRSensorView(self.serial_port)
            self.ir_sensor_view.connect_to_serial_thread(self.serial_reader_thread)  # Verbindung herstellen
            self.ir_sensor_view.show()
        else:
            QMessageBox.warning(self, "Warning", "No Serial Port Connected")


    # Open PID Dashboard
    def open_pd_dashboard(self):
        if self.serial_port:
            self.pd_dashboard = Dashboard(self.serial_port)  # Seriellen Port übergeben
            self.pd_dashboard.connect_to_serial_thread(self.serial_reader_thread)  # Verbindung herstellen
            self.pd_dashboard.show()
        else:
            QMessageBox.warning(self, "Warning", "No Serial Port Connected")

    # Use Slots to send messages to the Serial Monitor Thread
    @pyqtSlot(str)
    def process_serial_message(self, message):
        if hasattr(self, 'serial_monitor_window') and self.serial_monitor_window:
            self.serial_monitor_window.append_message(message)


## -- SerialReader Thread -------------------------------------------
## Eigener Thread zum auswerten der UART Daten!!!!
class SerialReaderThread(QThread):
    new_message = pyqtSignal(str)       # Signal für Serial Monitor
    new_sensor_data = pyqtSignal(list)  # Signal für Sensordaten
    new_motion_value = pyqtSignal(int)  # Signal für Speeddaten (Motion Plot)
    new_ctrl_value = pyqtSignal(int)    # Signal für PWM daten (Control Plot)

    def __init__(self, serial_port):
        super().__init__()
        self.serial_port = serial_port
        self._running = True
        self.buffer = b''

        # Statemachine
        self.state = "IDLE"
        self.tag = 0
        self.length = 0
        self.data = []
        self.cnt = 0
        self.ticks_for_last_byte = 0
        self.INTERBYTE_TIMEOUT_MS = 10

    def run(self):
        while self._running:
            if self.serial_port.in_waiting:
                try:
                    bytes_to_read = self.serial_port.read(self.serial_port.in_waiting)
                    self.buffer += bytes_to_read
                    while self.buffer:
                        if len(self.buffer) > 0:
                            byte = self.buffer[0]
                            self.buffer = self.buffer[1:]
                            self.PY_Protocol_Parser(byte)
                except Exception as e:
                    print(f"Buffer:{self.buffer}")
                    print(f"Error reading from serial port: {e}")

    # UART Parser
    def PY_Protocol_Parser(self, rxByte):
        """State machine to parse the incoming bytes."""
        current_time = int(time.time() * 1000)  # Millisekunden-Genauigkeit

        #print(f"STATE:: {self.state}, RXByte:: {rxByte}")

        # check timeout
        if current_time - self.ticks_for_last_byte > self.INTERBYTE_TIMEOUT_MS:
            self.state = "IDLE"
            #print("TIMEROUT DETECTED")
        self.ticks_for_last_byte = current_time

        if self.state == "IDLE":
            #print(f"tag: {rxByte}")
            # Erwartet ein Tag-Byte
            self.cnt = 0
            self.tag = rxByte
            self.state = "TAG"

        elif self.state == "TAG":
            #print(f"len: {rxByte}")
            # Erwartet ein Längenbyte
            self.length = rxByte
            if self.length <= 253:  # Begrenzung der Datenlänge, je nach Protokoll
                self.data = []

                self.cnt = 0
                self.state = "LENGTH"
            else:
                # Ungültige Länge, Zustand zurücksetzen
                self.state = "IDLE"

        elif self.state == "LENGTH":
            #print(f"Received data byte: {rxByte}")
            # Erwartet Datenbytes
            self.data.append(rxByte)
            
            self.cnt += 1
            #print(f"COUNT= {self.cnt}/{self.length}")

            if self.cnt == self.length:
                # Analyse der empfangenen Daten
                #print(f"Done parsing message: tag={self.tag} length={self.length} data={self.data}")
                self.Analyse_Data(self.tag, self.data)
                self.state = "IDLE"
        
        else:
            # Fehlerzustand, zurücksetzen
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
            # Build List
            sensor_values = self.decode_sensor_data(data)
            # Update GUI
            self.new_sensor_data.emit(sensor_values)

        elif tag == 0x06: #CMD_SEND_PWM
            pwm_value = data[0]
            self.new_ctrl_value.emit(pwm_value)
            
        elif tag == 0x07:  # CMD_SEND_SPEED
            # Build speed data [mm/s]
            motion_data = (data[0] << 8) | data[1]

            # Convert to signed 16-bit integer
            if motion_data >= 0x8000:  # Check if the sign bit is set (i.e., if it's negative)
                motion_data -= 0x10000  # Convert to negative value (two's complement)

            print(f"SPEED: {motion_data}")
            self.new_motion_value.emit(motion_data)

        else:
            # Unbekannter Befehl
            self.new_message.emit(f"Unknown command tag: {tag}")

    def decode_sensor_data(self, data):
        """Decodes the sensor data and returns it as a list of values."""
        sensor_values = []

        if len(data) >= 3:  # Überprüfen, ob mindestens 3 Bytes vorhanden sind
            # 1 Byte ID
            sensor_id = data[0]
            
            # 2-Byte-Sensorwert             
            sensor_value = (data[1] << 8) | data[2]

            # Werte zur Liste hinzufügen
            sensor_values.append(sensor_id)
            sensor_values.append(sensor_value)
        else:
            print(f"Received incomplete sensor data: {data}")

        return sensor_values

    def stop(self):
        self._running = False
        self.wait()  # Warten auf das Beenden des Threads
        self.serial_port.close()  # Serialport schließen
## ------------------------------------------------------------------

## -- Main -------------------------------------------
if __name__ == '__main__':
    app = QApplication(sys.argv)

    window = Menu()
    window.show()
    app.exec()

    
## ---------------------------------------------------