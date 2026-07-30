# ir_sensor_view.py

from PyQt5.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QCheckBox, QSpinBox, QFormLayout
from PyQt5.QtCore import QTimer, pyqtSlot
from PyQt5.QtGui import (QFont, QColor, QIcon, QPixmap)
import serial

class IRSensorView(QWidget):
    def __init__(self, serial_port):
        super().__init__()
        self.serial_port = serial_port
        self.initUI()
        self.sensor_states = [False] * 6  # Start with all sensors disabled
        self.current_values = [0] * 6
        self.min_values = [float('inf')] * 6
        self.max_values = [float('-inf')] * 6

    def initUI(self):
        self.setWindowTitle("IR Sensor View")
        self.setGeometry(100, 100, 600, 400)

        self.setWindowIcon(QIcon('./images/ruler.png'))        

        layout = QVBoxLayout()

        # Sensor rows layout
        self.sensor_rows = []

        for i in range(6):
            row_layout = QHBoxLayout()

            # Enable checkbox
            checkbox = QCheckBox(f"Sensor {i+1} Enable:")
            checkbox.setChecked(False)  # Deaktiviert zu Beginn
            checkbox.toggled.connect(self.make_checkbox_toggled(i))

            # Current value
            current_value_label = QLabel("Current: 0")

            # Min value
            min_value_label = QLabel("Min: 0")

            # Max value
            max_value_label = QLabel("Max: 0")

            # Add to layout
            row_layout.addWidget(checkbox)
            row_layout.addWidget(current_value_label)
            row_layout.addWidget(min_value_label)
            row_layout.addWidget(max_value_label)

            layout.addLayout(row_layout)

            # Save references for updating later
            self.sensor_rows.append({
                'checkbox': checkbox,
                'current': current_value_label,
                'min': min_value_label,
                'max': max_value_label
            })

        self.setLayout(layout)

    def make_checkbox_toggled(self, index):
        """Returns a function that properly passes the index to the checkbox toggled handler."""
        def toggled(state):
            self.set_sensor_state(index, state)
        return toggled

    def connect_to_serial_thread(self, serial_thread):
        """Verbindet das Sensor-Daten-Signal mit dieser Klasse."""
        serial_thread.new_sensor_data.connect(self.update_sensor_values)

    @pyqtSlot(list)
    def update_sensor_values(self, sensor_values):
        """Aktualisiert die Sensoranzeige basierend auf den empfangenen Werten."""
        ID = sensor_values[0]
        value = sensor_values[1]
        # Update current value
        self.sensor_rows[ID]['current'].setText(f"Current: {value}")
        
        # Update min and max values
        self.min_values[ID] = min(value, self.min_values[ID])
        self.max_values[ID] = max(value, self.max_values[ID])
        
        self.sensor_rows[ID]['min'].setText(f"Min: {self.min_values[ID]}")
        self.sensor_rows[ID]['max'].setText(f"Max: {self.max_values[ID]}")

    def set_sensor_state(self, index, state):
        # Bestimme den Tag basierend auf dem Zustand des Sensors
        tag = 0x02 if state else 0x03
        
        # Länge des Datenbereichs ist immer 1 Byte (Sensor-ID)
        length = 0x01
        
        # Daten: Sensor-ID
        data = bytes([index])
        
        # Befehl zusammenstellen
        command = bytes([tag]) + bytes([length]) + data
        #print("Command to send:", [hex(b) for b in command])  # Ausgabe als Hex-Werte
        
        # Befehl senden
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(command)