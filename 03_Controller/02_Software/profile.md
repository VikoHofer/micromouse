Erklärung der Funktionen
Profile_Reset(Profile* profile)
Setzt das Profil auf seinen Anfangszustand zurück, inklusive Geschwindigkeit, Position und Zustand.

Nutzung:
Wird verwendet, um ein Profil zu initialisieren oder nach einer Bewegung zurückzusetzen.

Profile_Start(Profile* profile, float distance, float top_speed, float final_speed, float acceleration)
Startet das Profil für eine bestimmte Strecke (distance), mit einer maximalen Geschwindigkeit (top_speed), einer Endgeschwindigkeit (final_speed) und einer Beschleunigung (acceleration).

Nutzung:
Initialisiert eine Bewegung, indem es die Startparameter festlegt.

Profile_Stop(Profile* profile)
Stoppt das Profil sofort und setzt die Zielgeschwindigkeit auf 0.

Nutzung:
Beendet eine Bewegung frühzeitig, z. B. bei einem Hindernis.

Profile_Finish(Profile* profile)
Markiert das Profil als abgeschlossen und setzt die Geschwindigkeit auf die Zielgeschwindigkeit.

Nutzung:
Wird automatisch oder manuell aufgerufen, wenn die Bewegung beendet ist.

Profile_Update(Profile* profile)
Aktualisiert die Geschwindigkeit und Position des Profils basierend auf der aktuellen Geschwindigkeit, Beschleunigung und verbleibender Distanz.

Nutzung:
Muss regelmäßig (z. B. im Timer-Interrupt) aufgerufen werden, um das Profil aktuell zu halten.

Profile_GetBrakingDistance(Profile* profile)
Berechnet die Distanz, die benötigt wird, um mit der aktuellen Geschwindigkeit auf die Endgeschwindigkeit abzubremsen.

Nutzung:
Kann genutzt werden, um rechtzeitig von Beschleunigung auf Abbremsen umzuschalten.

Profile_GetPosition(Profile* profile)
Gibt die aktuelle Position des Profils zurück.

Nutzung:
Wird verwendet, um die Fortschritte zu überwachen.

Profile_GetSpeed(Profile* profile)
Gibt die aktuelle Geschwindigkeit des Profils zurück.

Nutzung:
Wird verwendet, um die Sollgeschwindigkeit für den Geschwindigkeitsregler zu ermitteln.

Profile_GetIncrement(Profile* profile)
Gibt die Veränderung der Position pro Intervall zurück.

Nutzung:
Kann zur Berechnung von Positionserhöhungen genutzt werden.

Profile_GetAcceleration(Profile* profile)
Gibt die aktuelle Beschleunigung des Profils zurück.

Nutzung:
Kann genutzt werden, um die Dynamik des Profils anzupassen.

Profile_SetSpeed(Profile* profile, float speed)
Setzt die aktuelle Geschwindigkeit manuell.

Nutzung:
Wird selten benötigt, außer bei speziellen Anpassungen.

Profile_SetTargetSpeed(Profile* profile, float speed)
Setzt die Zielgeschwindigkeit des Profils.

Nutzung:
Kann zur Laufzeit verwendet werden, um die Zielgeschwindigkeit dynamisch zu ändern.

Profile_AdjustPosition(Profile* profile, float adjustment)
Passt die aktuelle Position des Profils um einen bestimmten Wert an.

Nutzung:
Wird z. B. für Fehlerkorrekturen eingesetzt.

Profile_SetPosition(Profile* profile, float position)
Setzt die Position auf einen bestimmten Wert.

Nutzung:
Kann genutzt werden, um die Position direkt zu initialisieren.

Beispiel: move(n)
Das folgende Beispiel zeigt, wie du die Funktionen für eine move(float distance)-Funktion nutzen könntest. Diese Funktion verwendet einen Geschwindigkeitsregler velocity_ctrl und setzt das Fahrzeug in Bewegung.

Code für move(n)
c
Code kopieren
```
#include "profile.h"

// Globale Variable für das Profil
Profile forward_profile;

// Dummy-Geschwindigkeitsregler
void velocity_ctrl(float target_speed) {
    // Zielgeschwindigkeit mit aktueller Encoder-Geschwindigkeit vergleichen
    // und die PWM entsprechend anpassen.
    printf("Target Speed: %.2f mm/s\n", target_speed);
}

void move(float distance) {
    float top_speed = 500.0f;       // Maximale Geschwindigkeit (mm/s)
    float final_speed = 0.0f;       // Geschwindigkeit am Ziel
    float acceleration = 1000.0f;  // Beschleunigung (mm/s²)

    // Profil initialisieren und starten
    Profile_Reset(&forward_profile);
    Profile_Start(&forward_profile, distance, top_speed, final_speed, acceleration);

    // Bewegungsschleife
    while (Profile_GetPosition(&forward_profile) < forward_profile.distance) {
        // Profil aktualisieren
        Profile_Update(&forward_profile);

        // Sollgeschwindigkeit holen
        float target_speed = Profile_GetSpeed(&forward_profile);

        // Geschwindigkeitsregler mit Zielgeschwindigkeit aufrufen
        velocity_ctrl(target_speed);

        // Simulierter Delay für Update-Intervall (z. B. 1 ms)
        HAL_Delay(1);
    }

    // Bewegung abschließen
    Profile_Stop(&forward_profile);
}

```
Erläuterung
Das Profil wird mit den gewünschten Bewegungsparametern (distance, top_speed, final_speed, acceleration) gestartet.
In einer Schleife wird regelmäßig Profile_Update aufgerufen, um die Sollgeschwindigkeit und Position zu aktualisieren.
Der Geschwindigkeitsregler velocity_ctrl wird mit der Sollgeschwindigkeit aus dem Profil angesteuert.
Die Schleife läuft, bis die Bewegung abgeschlossen ist (z. B. Bremsweg erreicht).
Am Ende wird die Bewegung gestoppt.
Dieses Beispiel dient als Grundlage, die du auf dein Projekt anpassen kannst. Soll ich den velocity_ctrl noch genauer implementieren oder weitere Details hinzufügen?


# Anwendung
```
// initialization process
State_t const* State_Initialize_Process(void)
{
	Profile forward_profile;
	float top_speed = 300.0f;       // Maximale Geschwindigkeit (mm/s)
    float final_speed = 0.0f;       // Geschwindigkeit am Ziel
    float acceleration = 1000.0f;
	
	Profile_Reset(&forward_profile);
	Profile_Init(&forward_profile);
    Profile_Start(&forward_profile, 800, top_speed, final_speed, acceleration);
	
	// PD Controller for speed
    static PIDController testPID_R = {0, 0, 0, 0, 0, 0};
    static PIDController testPID_L = {0, 0, 0, 0, 0, 0};
	PID_Init(&testPID_L, 0.126231, 0.929430, 0, 200);
	PID_Init(&testPID_R, 0.126231, 0.929430, 0, 200);
	
	while(1){
		//create_step_respose();
		//PY_Process();
		if(BUTTON1_Pressed()){
			while (!Profile_IsFinished(&forward_profile)) {
				// Profil aktualisieren
				Profile_Update(&forward_profile);

				// Sollgeschwindigkeit holen
				float target_speed = Profile_GetSpeed(&forward_profile);
				PY_Printf("Target Speed: %f", target_speed);
				
				// Geschwindigkeitsregler mit Zielgeschwindigkeit aufrufen
				testPID_L.setpoint = target_speed;
				testPID_R.setpoint = target_speed;
				Speed_Ctrl(&testPID_L, &testPID_R);

				// Simulierter Delay für Update-Intervall (z. B. 1 ms)
				Delay_ms(1);
			}
				
		}
		Motor_Left_SetPWM(0);
		Motor_Right_SetPWM(0);
		Profile_Reset(&forward_profile);
		Profile_Start(&forward_profile, 800, top_speed, final_speed, acceleration);

		}
		//PY_Process();
		

	/*
	* In loop until Btn1 is pressed
	while(1){
		* Choose direction with  btn2
		* LEDs will show which direction is set (Startposition)
	}
	*/

	
	// switch to SolzeMaze state
	return &State_SolveMaze;
	
}

```