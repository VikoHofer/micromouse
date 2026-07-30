# TODO for Microumouse Debugger

- Step oder Encoder funktionieren momentan getrennt, aufgrund der Callback Funktion welche in der Maus Registriert ist.
    Step Function: Timer_init in State_SolveMaze.c
    Encoder Function: Timer_init in Motion_control.c in der Move Funktion State_SolveMaze_Process
- Testen auf Windows
- Refactor!
- Bugfix für BLE auf Windows,  Programm stürzt ab bzw. beendet frühzeitig
- Automatisierter Graphen in Excel
- Erweiterungen der Commands für weitere Systemparamter wie Winkelgeschwindigkeit, Einstellen von Geschwindigkeit, etc.
- Automatic .csv-file generation auf Linux
