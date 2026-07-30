namespace BLEDebugDisplay;

using System.Globalization;
using System.ComponentModel.Design;
using System.Data;
using System.Diagnostics;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading.Tasks;

class Program
{
	enum StepReceiveState { Idle, ReceivingTime, ReceivingVelocity }
	static readonly string BLEDebuggerUUID = "9e150000-2424-3535-4711-0000decafbad";
	static readonly string BLEDebuggerNotifyUUID = "9e150004-2424-3535-4711-0000decafbad";
	static readonly string BLEDebuggerResetUUID = "9e150003-2424-3535-4711-0000decafbad";
	static readonly string BLEDebuggerCommandUUID = "9e150002-2424-3535-4711-0000decafbad";
	static readonly int MaxConnectRetries = 3;

//	static async Task SendCommand(IBLEDevice BLEDebugger, byte tag, byte[] data)
//	{
//		byte[] payload = new byte[1 + data.Length];  // ← was 2 + data.Length
//		payload[0] = tag;
//		payload[1] = (byte)data.Length;
//		data.CopyTo(payload, 1);                     // ← was CopyTo(payload, 2)
//
//		bool success = await BLEDebugger.WriteCharacteristic(BLEDebuggerCommandUUID, payload);
//		if (!success)
//		{
//			Console.WriteLine("Error: Could not send payload to mouse");
//		}
//	}
static async Task SendCommand(IBLEDevice BLEDebugger, byte tag, byte[] data)
{
    // Fix: Allocate 2 bytes (Tag + Length) + the data length
    byte[] payload = new byte[2 + data.Length];  
    payload[0] = tag;
    payload[1] = (byte)data.Length;
    
    // Fix: Copy data starting AFTER the tag and length bytes (index 2)
    data.CopyTo(payload, 2);                     

    bool success = await BLEDebugger.WriteCharacteristic(BLEDebuggerCommandUUID, payload);
    if (!success)
    {
        Console.WriteLine("Error: Could not send payload to mouse");
    }
}

	static private void PrintCommands()
	{
		Console.WriteLine("Verfügbare Befehle: reset, move + value, step, encoder + n_fields + top_speed + acceleration + final speed");
	}

	struct commandType
	{
		public const byte cmd_reset         =  0x4;
		public const byte cmd_printd        =    0;
		public const byte cmd_SetWall       = 0x81;
		public const byte cmd_Move          = 0x82;
		public const byte cmd_step          = 0x83;
		public const byte cmd_MoveEncoder   = 0x84;
	}
	
	static void SaveStepResponseCSV(List<(float Time, float Velocity)> Samples)
	{
		string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss-fff");
		string exeFolder = AppContext.BaseDirectory;
		string filePath = Path.Combine(exeFolder, $"step_response_{timestamp}.csv");

		var sb = new StringBuilder();
		sb.AppendLine("TIME,VELOCITY");

		foreach (var (Time, Velocity) in Samples)
		{
			//sb.AppendLine($"{Time},{Velocity}");
			sb.AppendLine($"{Time.ToString(CultureInfo.InvariantCulture)},{Velocity.ToString(CultureInfo.InvariantCulture)}");
		}

		File.WriteAllText(filePath, sb.ToString());
		Console.WriteLine($"Saved {Samples.Count} samples to {filePath}");
	}

	static void SaveEncoderProfileCSV(List<(float Time, float Actual, float Target)> Samples)
	{
		string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss-fff");
		string exeFolder = AppContext.BaseDirectory;
		string filePath = Path.Combine(exeFolder, $"encoder_profile_{timestamp}.csv");

		var sb = new StringBuilder();
		sb.AppendLine("TIME,ACTUAL_VELOCITY,TARGET_VELOCITY");

		foreach (var (Time, Actual, Target) in Samples)
			sb.AppendLine($"{Time},{Actual},{Target}");

		File.WriteAllText(filePath, sb.ToString());
		Process.Start(new ProcessStartInfo(filePath) { UseShellExecute = true });
		Console.WriteLine($"Saved {Samples.Count} encoder samples to {filePath}");
	}

	static async Task<int> Main(string[] args)
	{

		if (args.Length == 1 && args[0] == "MMSAPI") //enable API for simulator
		{
			MMSAPI.DEACTIVATEAPI = false;
		}

		IBLEDevice BLEDebugger = new BLEDevice(BLEDebuggerUUID);

		PrintCommands();

		int ConnectRetries = 0;
		bool ConnectionSuccessful = false;
		Console.WriteLine("Connecting to Mausi...");
		Console.WriteLine(" _  _");
		Console.WriteLine("(o)(o)--.");
		Console.WriteLine(" \\../ (  )");
		Console.WriteLine(" m\\/m--m'`--.");
		while (!(ConnectionSuccessful = await BLEDebugger.DiscoverAndConnect()) && ConnectRetries < MaxConnectRetries)
		{
			Console.WriteLine("Failed to connect to mouse. Retrying...");
			await Task.Delay(1000); 
            ConnectRetries++;
		}

		if (!ConnectionSuccessful)
		{
			Console.WriteLine("Could not connect to device.");
			await Task.Delay(1000);
			return -1;
		}
		else
		{
			Console.WriteLine($"Connected to device {BLEDebugger.GetDeviceName()} ({BLEDebugger.GetDeviceAddress()})!");
		}


		var stepSamples = new List<(float Time, float Velocity)>();
		var timeValues = new List<float>();
		var stepState = StepReceiveState.Idle;
		// New encoder profile state
		var encoderSamples = new List<(float Time, float Actual, float Target)>();
		var encoderTimes = new List<float>();
		var encoderActual = new List<float>();
		var encoderState = StepReceiveState.Idle; // reuse same enum

		Func<byte[], Task> NotifyCallback = (byte[] Message) =>
		{

			if (Message.Length <= 0 || Message[0] == 0xF8)
			{
                return Task.CompletedTask; //magic value 0xF8 due to UART error?? -> ignore
            }
			

			byte MessageType = Message[0];
			switch(MessageType)
			{
				case 0:
				//	if(MMSAPI.DEACTIVATEAPI)
				//	{
						Console.WriteLine(Encoding.UTF8.GetString(Message[1..Message.Length]));
					//}
					break;
				case 1:
					if(Message.Length >= 2)
					{
						SetWall(Message[1]);
					}
					break;
				case 2:
					if(Message.Length >= 2)
					{
						Move(Message[1]);
					}
					break;
				case 3:
					string header = Encoding.UTF8.GetString(Message[1..]).TrimEnd('\0');

					if (header.StartsWith("TIME"))
					{
						stepState = StepReceiveState.ReceivingTime;
						timeValues.Clear();

						// THE FIX: You MUST clear the old velocity samples when a new run begins!
						// Otherwise, a dropped packet corrupts all future runs.
						stepSamples.Clear(); 
					}
				//case 3:
				//	string header = Encoding.UTF8.GetString(Message[1..]).TrimEnd('\0');

				//	if (header.StartsWith("TIME"))
				//	{
				//		stepState = StepReceiveState.ReceivingTime;
				//		timeValues.Clear();
				//	}
				//	else if (header.StartsWith("VELOCITY"))
				//	{
				//		stepState = StepReceiveState.ReceivingVelocity;
				//	}
				//	else if (Message.Length >= 5) // it's a float, not a header
				//	{
				//		float value = BitConverter.ToSingle(Message, 1);

				//		if (stepState == StepReceiveState.ReceivingTime)
				//		{
				//			timeValues.Add(value);
				//		}
				//		else if (stepState == StepReceiveState.ReceivingVelocity)
				//		{
				//			int index = stepSamples.Count;
				//			if (index < timeValues.Count)
				//				stepSamples.Add((timeValues[index], value));

				//			// All velocity samples received
				//			if (stepSamples.Count == timeValues.Count && timeValues.Count > 0)
				//			{
				//				SaveStepResponseCSV(stepSamples);
				//				stepSamples.Clear();
				//				stepState = StepReceiveState.Idle;
				//			}
				//		}
				//	}
					break;
				case 4:
					string encHeader = Encoding.UTF8.GetString(Message[1..]).TrimEnd('\0');

					if (encHeader.StartsWith("TIME"))
					{
						encoderState = StepReceiveState.ReceivingTime;
						encoderTimes.Clear();
						encoderActual.Clear();
						encoderSamples.Clear();
					}
					else if (encHeader.StartsWith("ACTUAL"))
					{
						encoderState = StepReceiveState.ReceivingVelocity;
					}
					else if (encHeader.StartsWith("TARGET"))
					{
						encoderState = StepReceiveState.Idle; // reuse Idle as "ReceivingTarget"
					}
					else if (Message.Length >= 5)
					{
						float value = BitConverter.ToSingle(Message, 1);

						if (encoderState == StepReceiveState.ReceivingTime)
						{
							encoderTimes.Add(value);
						}
						else if (encoderState == StepReceiveState.ReceivingVelocity)
						{
							encoderActual.Add(value);
						}
						else if (encoderState == StepReceiveState.Idle && encoderActual.Count > 0)
						{
							int index = encoderSamples.Count;
							if (index < encoderTimes.Count && index < encoderActual.Count)
								encoderSamples.Add((encoderTimes[index], encoderActual[index], value));

							if (encoderSamples.Count == encoderTimes.Count && encoderTimes.Count > 0)
							{
								SaveEncoderProfileCSV(encoderSamples);
								encoderSamples.Clear();
							}
						}
					}
					break;

				default:
					Console.WriteLine($"Something went wrong! Messagetype: {MessageType}");
					Console.WriteLine($"Full message: 0x{BitConverter.ToString(Message).Replace("-","")}");
					break;
			}

			return Task.CompletedTask;
		};



		if (!await BLEDebugger.SubscribeNotify(BLEDebuggerNotifyUUID, NotifyCallback))
		{
			Console.WriteLine("Failed to subscribe to notify characteristic!");
            await Task.Delay(1000);
            return -2;
		}

		while (true)
		{
			string? Message = Console.ReadLine();
			if (Message == null) continue;

			string[] Parts = Message.Split(' ');
			string Command = Parts[0];
			byte Value = Parts.Length > 1 && byte.TryParse(Parts[1], out byte parsed) ? parsed : (byte)0;

			if (Command.Equals("reset", StringComparison.CurrentCultureIgnoreCase)
					|| Command.Equals("r", StringComparison.CurrentCultureIgnoreCase))
			{
				bool ResetSuccessful = await BLEDebugger.WriteCharacteristic(BLEDebuggerResetUUID, [1]);
				if (!ResetSuccessful) Console.WriteLine("Failed to reset target device!");
			}
			else if (Command.Equals("move", StringComparison.CurrentCultureIgnoreCase))
			{
				await SendCommand(BLEDebugger, commandType.cmd_Move, [Value]);
			}
			else if (Command.Equals("step", StringComparison.CurrentCultureIgnoreCase))
			{
				await SendCommand(BLEDebugger, commandType.cmd_step, []);
			}
			else if (Command.Equals("encoder", StringComparison.CurrentCultureIgnoreCase))
{
    // Parse up to 4 float values, defaulting to 0f if missing
    // Using InvariantCulture ensures "." is always parsed as a decimal separator
    float val1 = Parts.Length > 1 && float.TryParse(Parts[1], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float f1) ? f1 : 0f;
    float val2 = Parts.Length > 2 && float.TryParse(Parts[2], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float f2) ? f2 : 0f;
    float val3 = Parts.Length > 3 && float.TryParse(Parts[3], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float f3) ? f3 : 0f;
    float val4 = Parts.Length > 4 && float.TryParse(Parts[4], System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out float f4) ? f4 : 0f;

    // Convert each float to a 4-byte array
    byte[] b1 = BitConverter.GetBytes(val1);
    byte[] b2 = BitConverter.GetBytes(val2);
    byte[] b3 = BitConverter.GetBytes(val3);
    byte[] b4 = BitConverter.GetBytes(val4);

    // Combine all 16 bytes into a single payload
    byte[] payload = new byte[16];
    b1.CopyTo(payload, 0);
    b2.CopyTo(payload, 4);
    b3.CopyTo(payload, 8);
    b4.CopyTo(payload, 12);

    // Send the 16-byte payload
    await SendCommand(BLEDebugger, commandType.cmd_MoveEncoder, payload);
}
			else
			{
				Console.WriteLine($"Unbekannter Befehl: {Command}. Verfügbare Befehle: reset, move + value, step, encoder + n_fields + top_speed + acceleration + final speed ");
			}
		}
        
        return 0;
	}

	static void SetWall(byte Direction)
	{
		string Output = $"SetWall: {Direction} ";
		switch(Direction) //fixed directions for mouse in simulator!!
		{
			case 0:
				Output += "(north/top)";
				MMSAPI.SetWallAsync('w');
				break;
			case 1:
				Output += "(west/left)";
				MMSAPI.SetWallAsync('s');
				break;
			case 2:
				Output += "(east/right)";
				MMSAPI.SetWallAsync('n');
				break;
			case 3:
				Output += "(south/bottom)";
				MMSAPI.SetWallAsync('e');
				break;
		}
		Console.WriteLine(Output);
	}

	static void Move(byte Direction)
	{
		string Output = $"MoveDirection: {Direction} ";
		switch(Direction)
		{
			case 0:
				Output += "(forward)";
				MMSAPI.MoveForwardAsync();
				break;
			case 1:
				Output += "(turn left)";
				MMSAPI.TurnLeftAsync();
				break;
			case 2:
				Output += "(turn right)";
				MMSAPI.TurnRightAsync();
				break;
			default:
				break;
		}
		Console.WriteLine(Output);
	}
}
