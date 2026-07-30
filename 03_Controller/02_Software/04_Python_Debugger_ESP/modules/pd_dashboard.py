# pd_dashboard.py

## -- Include Section -------------------------------------------
import os
import sys
import time
import serial
import numpy as np
import csv

from serial import Serial
import serial.tools.list_ports

from PyQt5.QtGui import QFont, QColor, QIntValidator, QIcon  
from PyQt5.QtCore import Qt, QThread, pyqtSignal, pyqtSlot
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QLabel, QSizePolicy, QMessageBox, QSpacerItem, QComboBox, QLineEdit
)

import pyqtgraph as pg

## --------------------------------------------------------------

BLE_CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"


# High DPI scaling for better visuals
if hasattr(Qt, 'AA_EnableHighDpiScaling'):
    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
if hasattr(Qt, 'AA_UseHighDpiPixmaps'):
    QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)

# Color palette
palette = ("#101418", "#c00000", "#c000c0", "#c06000", "#00c000", "#0072c3", "#6fdc8c", "#d2a106")

# DataChannel Class
class DataChannel:
    def __init__(self, points):
        self.points = points
        self.data_ = np.zeros(shape=self.points, dtype=float)

    def add_new_value(self, value):
        self.data_[:-1] = self.data_[1:]
        self.data_[-1] = value

    def data(self):
        return self.data_

hline_style = 'border: 2px solid gray'

# Dashboard Class
class Dashboard(QWidget):

    def __init__(self, ble_client):
        super().__init__()

        self.ble_client = ble_client
        self.device = None
        self.data = []
        self.parameters = {}
        self.nChannels = 8
        self.nPoints = 400
        self.telemetry = [DataChannel(self.nPoints) for _ in range(self.nChannels)]
        self.plots = [pg.PlotItem for _ in range(self.nChannels)]
        self.initUI()

    def initUI(self):
        # Configure plot settings
        pg.setConfigOption('background', pg.mkColor(25, 50, 75))
        pg.setConfigOption('background', palette[0])
        pg.setConfigOption('foreground', 'y')
        pg.setConfigOptions(antialias=True)
        styles = {'color': 'cyan', 'font-size': '13px'}

        # Output plot configuration
        self.output_plot = pg.PlotWidget()
        self.output_plot.setYRange(-1, 5)
        self.output_plot.setXRange(0, 1000)
        self.output_plot.disableAutoRange()
        self.output_plot.setTitle("<span style=\"color:cyan;\">Controller outputs</span>")
        self.output_plot.setLabel('left', 'Volts', **styles)
        self.output_plot.setLabel('bottom', 'time (ms)', **styles)
        self.output_plot.addLegend(offset=(-5, 20))
        self.output_plot.showGrid(x=True, y=True)
        self.output_plot.getPlotItem().getAxis('left').setWidth(60)

        # Motion plot configuration
        self.motion_plot = pg.PlotWidget()
        self.motion_plot.setYRange(-5000, 5000)
        self.motion_plot.setXRange(0, 2000)
        self.motion_plot.disableAutoRange()
        self.motion_plot.setTitle("<span style=\"color:cyan;\">Motion</span>")
        self.motion_plot.setLabel('left', 'Speed (mm/s)', **styles)
        self.motion_plot.setLabel('bottom', 'time (ms)', **styles)
        self.motion_plot.showGrid(x=True, y=True)
        self.motion_plot.getPlotItem().getAxis('left').setWidth(60)
        self.motion_plot.addLegend(offset=(-5, 20))

        # Buttons
        self.btn_id = QPushButton('ID', self)
        self.btn_id.clicked.connect(self.get_id)

        # Voltage Selection
        self.voltage_selector = QComboBox(self)
        self.voltage_selector.addItems([f"{i/10:.1f}V" for i in range(0, 51)])  # 0V - 5V in 0.1V steps

        # Input Field for numbers between 0-5000
        self.number_input = QLineEdit(self)
        self.number_input.setPlaceholderText("Enter 0-5000 [mm/s]")
        self.number_input.setValidator(QIntValidator(0, 5000))  # Only allow numbers between 0-5000

        self.btn_move = QPushButton('PD-MOVE', self)
        self.btn_move.clicked.connect(self.send_move)

        self.btn_step = QPushButton('PD-STEP', self)
        self.btn_step.clicked.connect(self.send_step)

        self.btn_clear = QPushButton('CLR', self)
        self.btn_clear.clicked.connect(self.clear_plot)

        # Create layouts for arranging buttons and inputs
        id_layout = QHBoxLayout()  # ID button and Voltage selector
        id_layout.addWidget(self.btn_id)
        id_layout.addWidget(self.voltage_selector)

        step_layout = QHBoxLayout()  # Step button and number input
        step_layout.addWidget(self.btn_step)
        step_layout.addWidget(self.number_input)

        move_clear_layout = QHBoxLayout()  # MOVE and CLR buttons
        move_clear_layout.addWidget(self.btn_move)
        move_clear_layout.addWidget(self.btn_clear)

        # Combine all button layouts in a vertical layout
        button_layout = QVBoxLayout()
        button_layout.addLayout(id_layout)
        button_layout.addLayout(step_layout)
        button_layout.addLayout(move_clear_layout)

        # Sidebar
        self.side_bar = QWidget()
        side_layout = QVBoxLayout()
        side_layout.addLayout(button_layout)
        self.side_bar.setLayout(side_layout)
        self.side_bar.setFixedWidth(300)

        # Plot Layout
        plot_layout = QVBoxLayout()
        plot_layout.addWidget(self.output_plot)
        plot_layout.addWidget(self.motion_plot)

        center_layout = QHBoxLayout()
        center_layout.addLayout(plot_layout)
        center_layout.addWidget(self.side_bar)

        # Main layout
        main_vbox = QVBoxLayout()
        main_vbox.addLayout(center_layout)

        self.setLayout(main_vbox)  # Set layout for the Dashboard widget

        # Window configuration
        self.setWindowTitle("PD Dashboard")
        self.setMinimumSize(960, 720)
        self.resize(960, 720)

        self.setWindowIcon(QIcon('./images/dashboard.png'))

        self.show()


    # make connection to serial port class
    def connect_to_serial_thread(self, serial_thread):
        """Connect SerialReader Thread Slots."""
        serial_thread.new_motion_value.connect(self.update_motion_plot)
        serial_thread.new_ctrl_value.connect(self.update_ctrl_plot)

    # Motion plot
    @pyqtSlot(int)
    def update_motion_plot(self, motion_value):
        """Update plots with new sensor data."""
        speed = motion_value
        self.time_data_motion.append(self.X_cnt_motion)
        self.speed_data.append(speed)
        self.X_cnt_motion += 1  # Increment by 10 ms for each update
        #print(f"cnt_motion: {self.X_cnt_motion}")

        # Ensure both time_data and speed_data have the same length
        if len(self.time_data_motion) != len(self.speed_data):
            min_length = min(len(self.time_data_motion), len(self.speed_data))
            self.time_data_motion = self.time_data_motion[-min_length:]
            self.speed_data = self.speed_data[-min_length:]

        # Limit number of data points to avoid memory overflow
        max_data_points = 1000
        if len(self.time_data_motion) > max_data_points:
            self.time_data_motion = self.time_data_motion[-max_data_points:]
            self.speed_data = self.speed_data[-max_data_points:]

        # Update the plot
        self.motion_plot.plot(self.time_data_motion, self.speed_data, clear=False, pen=pg.mkPen(color='cyan', width=2))

        '''
        # Save the data as a CSV file
        with open('motion_data.csv', mode='w', newline='') as file:
            writer = csv.writer(file, delimiter=';')  # Use comma as delimiter for columns
            writer.writerow(['Time (ms)', 'Speed (mm/s)'])  # Write header
            for time, speed in zip(self.time_data_motion, self.speed_data):
                writer.writerow([time, speed])  # Write each time-speed pair as separate columns

                '''

    # convert PWM value into Voltage
    def map_value(self, x, in_min=0, in_max=255, out_min=0, out_max=5):
        return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min)

    # Control plot
    @pyqtSlot(int)
    def update_ctrl_plot(self, ctrl_value):
        """Update plots with new control data."""
        print(f"ctrl_value: {ctrl_value}")
        voltage = self.map_value(ctrl_value)
        self.time_data_ctrl.append(self.X_cnt_ctrl)
        self.voltage_data.append(voltage)
        self.X_cnt_ctrl += 1  # Increment by 10 ms for each update
        #print(f"cnt_crtl: {self.X_cnt_motion}")

        # Ensure both time_data and voltage_data have the same length
        if len(self.time_data_ctrl) != len(self.voltage_data):
            min_length = min(len(self.time_data_ctrl), len(self.voltage_data))
            self.time_data_ctrl = self.time_data_ctrl[-min_length:]
            self.voltage_data = self.voltage_data[-min_length:]

        # Limit number of data points to avoid memory overflow
        max_data_points = 1000
        if len(self.time_data_ctrl) > max_data_points:
            self.time_data_ctrl = self.time_data_ctrl[-max_data_points:]
            self.voltage_data = self.voltage_data[-max_data_points:]

        # Update the plot
        self.output_plot.plot(self.time_data_ctrl, self.voltage_data, clear=True, pen=pg.mkPen(color='red', width=2))


    # Button Handlers 
    async def get_id(self): # Send CMD
        voltage_text = self.voltage_selector.currentText()  # Beispiel: "5.0V"
        voltage = float(voltage_text.replace("V", ""))  # Entfernt das "V" und wandelt den Rest in eine Zahl um
        voltage = int(voltage * 51)  # Konvertiere zu einem Integer-Wert im Bereich von 0-255
        print(f"Voltage: {voltage}")

        # CMD_GET_ID
        tag = 0x05
        length = 0x01
        data = voltage-1

        # Plot Data
        self.X_cnt_motion = 0
        self.time_data_motion = []  # Initialize time_data
        self.speed_data = []  # Initialize speed_data

        # Befehl zusammenstellen
        command = bytes([tag]) + bytes([length]) + bytes([data])
        print(f"{command}")

        # Befehl senden
        # Befehl senden
        await self.send_command(command)

    async def send_move(self):
        # CMD_MOVE_PD
        tag = 0x09
        length = 0x01
        data = 0x00

        # Plot Data
        self.X_cnt_motion = 0
        self.X_cnt_ctrl = 0
        self.time_data_motion = []  # Initialize time_data
        self.time_data_ctrl = []  # Initialize time_data
        self.speed_data = []  # Initialize speed_data
        self.voltage_data = [] # Initialize PWM data
        self.soll_speed_data = []  # Initialize soll speed data

        # Parameters for trapezoidal velocity profile
        ramp_up_end = 750  # End of acceleration phase (1 ms steps)
        constant_end = 1250  # End of constant speed phase (1 ms steps)
        max_speed = 3000  # Maximum speed (mm/s)
        total_points = 2000  # Total points (2 seconds in 1 ms steps)
        time_interval = 1  # 1 ms per point

        # Generate trapezoidal setpoint (soll) data
        for i in range(total_points):
            if i <= ramp_up_end:
                # Acceleration phase (ramp up)
                setpoint = (i / ramp_up_end) * max_speed
            elif i <= constant_end:
                # Constant speed phase
                setpoint = max_speed
            else:
                # Deceleration phase (ramp down)
                setpoint = max_speed * (1.0 - (i - constant_end) / (total_points - constant_end))

            self.soll_speed_data.append(setpoint)
            self.time_data_motion.append(i * time_interval)

        # Plot Sollgeschwindigkeit as dashed line
        self.motion_plot.plot(
            self.time_data_motion, 
            self.soll_speed_data, 
            pen=pg.mkPen(color='yellow', width=2, style=Qt.DashLine), 
            name="set speed"
        )

        # Befehl zusammenstellen
        command = bytes([tag]) + bytes([length]) + bytes([data])
        #print(f"{command}")

        # Befehl senden
        await self.send_command(command)


    async def send_step(self):
        # CMD_STEP_PD
        tag = 0x08
        length = 0x02

        # get speed 
        set_speed_str = self.number_input.text()
        set_speed_int = int(set_speed_str)

        print(f"set speed: {set_speed_int}")
        
        MSB = (set_speed_int >> 8) & 0xFF  
        LSB = set_speed_int & 0xFF         

        # Plot Data
        self.X_cnt_motion = 0
        self.X_cnt_ctrl = 0
        self.time_data_motion = []  # Initialize time_data
        self.time_data_ctrl = []  # Initialize time_data
        self.speed_data = []  # Initialize speed_data
        self.voltage_data = [] # Initialize PWM data

        # set point line
        ideal_speed_line = pg.InfiniteLine(pos=set_speed_int, angle=0, pen=pg.mkPen(color='green', width=2, style=Qt.DashLine))
        self.motion_plot.addItem(ideal_speed_line)

        # Befehl zusammenstellen
        command = bytes([tag]) + bytes([length]) + bytes([MSB]) + bytes([LSB])
        #print(f"{command}")

        # Befehl senden
        await self.send_command(command)

    def clear_plot(self):
        self.output_plot.clear()
        self.motion_plot.clear()
    
    async def send_command(self, command):
        await self.ble_client.write_gatt_char(BLE_CHARACTERISTIC_UUID, command)