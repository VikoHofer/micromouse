using System.Text;
using System;
using Microsoft.Win32.SafeHandles;

#if LINUX
using Linux.Bluetooth;
using Linux.Bluetooth.Extensions;
#endif
#if WINDOWS
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;
using Windows.Devices.Enumeration;
#endif

namespace BLEDebugDisplay;


public interface IBLEDevice
{
    /// <summary>
    /// Starts device discovery and connects the target device once found.
    /// </summary>
    /// <param name="TimeoutMS">Timeout in milliseconds. Stops discovery if device was not found within the set timeframe.</param>
    /// <returns>True when target device was found and successfully connected. False otherwise.</returns>
    Task<bool> DiscoverAndConnect(int TimeoutMS = 15000);

    /// <summary>
    /// Reads a value from a characteristic.
    /// </summary>
    /// <param name="CharacteristicUUID">UUID of the target characteristic. Must be part of the initially provided service.</param>
    /// <returns>Value of type T on success, null otherwise.</returns>
    Task<byte[]?> ReadCharacteristic(string CharacteristicUUID);

    /// <summary>
    /// Writes a value to a characteristic.
    /// </summary>
    /// <param name="CharacteristicUUID">UUID of the target characteristic. Must be part of the initially provided service.</param>
    /// <param name="Data">Data to write to the characteristic.</param>
    /// <returns>True on success, false otherwise.</returns>
    Task<bool> WriteCharacteristic(string CharacteristicUUID, byte[] Data);

    /// <summary>
    /// Subscribes to a notify characteristic.
    /// </summary>
    /// <param name="CharacteristicUUID">UUID of the target characteristic. Must be part of the initially provided service.</param>
    /// <param name="Callback">Callback function/Action when a notify event is received.</param>
    /// <returns>True on success, false otherwise.</returns>
    Task<bool> SubscribeNotify(string CharacteristicUUID, Func<byte[], Task> Callback);

    /// <summary>
    /// Gets the address of the connected device.
    /// </summary>
    /// <returns>Address of the connected device, null if no device is connected.</returns>
    string? GetDeviceAddress();

    /// <summary>
    /// Gets the name of the connected device.
    /// </summary>
    /// <returns>Name of the connected device, null if no device is connected.</returns>
    string? GetDeviceName();
}

#if LINUX
public class BLEDevice : IBLEDevice
{
    string ServiceUUID;
    Device? Device = null;
    IGattService1? GattService = null;

    Adapter? PhysicalBluetoothAdapter = null;
    TaskCompletionSource PhysicalAdapterPoweredOnTask = new TaskCompletionSource();
    TaskCompletionSource? DeviceConnectedTask;
    TaskCompletionSource? DeviceFoundTask;
    TaskCompletionSource? ServicesResolvedTask;

    string? DeviceAddress = null;
    string? DeviceName = null;

    // --- constructor ---
    /// <summary>
    /// Constructs a new BLEDevice. For Linux based devices only.
    /// </summary>
    /// <param name="DeviceServiceUUID">The UUID of the service the target device must have.</param>
    /// <exception cref="NotSupportedException">Throws a NotSupportedException if any other operating system than Linux is used.</exception>
    public BLEDevice(string DeviceServiceUUID)
    {
        if (!OperatingSystem.IsLinux()) throw new NotSupportedException("BLEDeviceLinux is only supported on Linux based systems!");

        ServiceUUID = BlueZManager.NormalizeUUID(DeviceServiceUUID);
    }

    // --- interface functions ---

    public async Task<bool> DiscoverAndConnect(int TimeoutMS)
    {
        var PhysicalAdapters = await BlueZManager.GetAdaptersAsync();
        PhysicalBluetoothAdapter = PhysicalAdapters.FirstOrDefault();

        if (PhysicalBluetoothAdapter == null) throw new NotSupportedException("Your device does not have a physical Bluetooth interface!");

        //wait until adapter is powered on
        PhysicalBluetoothAdapter.PoweredOn += BLAdapter_PoweredOn;
        await PhysicalAdapterPoweredOnTask.Task;

        //check if device is already connected
        if (await IsDeviceAlreadyConnected()) return true;

        //device not connected -> discover new devices
        DeviceConnectedTask = new TaskCompletionSource();
        DeviceFoundTask = new TaskCompletionSource();
        ServicesResolvedTask = new TaskCompletionSource();

        PhysicalBluetoothAdapter.DeviceFound += BLAdapter_DeviceFound;
        await PhysicalBluetoothAdapter.StartDiscoveryAsync();

        await DeviceFoundTask.Task;

        try
        {
            await PhysicalBluetoothAdapter.StopDiscoveryAsync();
        }
        catch { }

        if(Device == null) return false;

        Console.WriteLine("Device found!");

        Device.Connected += Device_Connected;
        Device.Disconnected += Device_Disconnected;
        Device.ServicesResolved += Device_ServicesResolved;

        if(!await Device.GetTrustedAsync()) await Device.SetTrustedAsync(true);

        await Device.ConnectAsync();
        await DeviceConnectedTask.Task;
        Console.WriteLine("Device connected!");

        //request properties and services
        Device1Properties DeviceProperties = await Device.GetAllAsync();

        DeviceAddress = DeviceProperties.Address;
        DeviceName = DeviceProperties.Name;

        await ServicesResolvedTask.Task;

        //get correct service
        var Services = await Device.GetServicesAsync();

        foreach (IGattService1 Service in Services)
        {
            GattService1Properties GattProperties = await Service.GetAllAsync();
            if (GattProperties.UUID == ServiceUUID)
            {
                GattService = Service;
                break;
            }
        }

        Console.WriteLine($"Got {Services.Count} for device {DeviceName}!");

        return GattService != null;
    }

    public async Task<byte[]?> ReadCharacteristic(string CharacteristicUUID)
    {
        if (GattService == null) return null;

        try
        {
            GattCharacteristic Characteristic = await GattService.GetCharacteristicAsync(BlueZManager.NormalizeUUID(CharacteristicUUID));
            return await Characteristic.GetValueAsync();
        }
        catch
        {
            return null;
        }
    }

    public async Task<bool> WriteCharacteristic(string CharacteristicUUID, byte[] Data)
    {
        if (GattService == null) return false;

        try
        {
            GattCharacteristic Characteristic = await GattService.GetCharacteristicAsync(BlueZManager.NormalizeUUID(CharacteristicUUID));
            await Characteristic.WriteValueAsync(Data, new Dictionary<string, object>() { { "type", "command" } });

            return true;
        }
        catch
        {
            return false;
        }
    }
public async Task<bool> SubscribeNotify(string CharacteristicUUID, Func<byte[], Task> Callback)
    {
        if (GattService == null) return false;

        GattCharacteristic Characteristic = await GattService.GetCharacteristicAsync(BlueZManager.NormalizeUUID(CharacteristicUUID));

        // Hook directly into the native BlueZ event system (Zero Latency)
        Characteristic.Value += async (sender, args) =>
        {
            await Callback(args.Value);
        };

        // Tell the Bluetooth module to start sending data
        await Characteristic.StartNotifyAsync();

        return true;
    }
   // public async Task<bool> SubscribeNotify(string CharacteristicUUID, Func<byte[], Task> Callback)
   // {
   //     if (GattService == null) return false;

   //     CancellationTokenSource CTSource = new CancellationTokenSource();
   //     GattCharacteristic Characteristic = await GattService.GetCharacteristicAsync(BlueZManager.NormalizeUUID(CharacteristicUUID));

   //     Task.Run(() => DoNotifyStreaming(Characteristic, Callback, CTSource.Token));

   //     return true;
   // }

    public string? GetDeviceAddress()
    {
        return DeviceAddress;
    }

    public string? GetDeviceName()
    {
        return DeviceName;
    }

    // --- helper functions ---
    private async Task<bool> IsDeviceAlreadyConnected()
    {
        if (PhysicalBluetoothAdapter == null) return false;

        var ConnectedDevices = await PhysicalBluetoothAdapter.GetDevicesAsync();
        foreach (var ConnectedDevice in ConnectedDevices)
        {
            if (await ConnectedDevice.GetConnectedAsync())
            {
                DeviceProperties DeviceProperties = await ConnectedDevice.GetPropertiesAsync();
                var Services = await ConnectedDevice.GetServicesAsync();
                foreach (var Service in Services)
                {
                    if (await Service.GetUUIDAsync() == ServiceUUID)
                    {
                        Console.WriteLine("Found connected device!");
                        GattService = Service;
                        DeviceAddress = DeviceProperties.Address;
                        DeviceName = DeviceProperties.Name;
                        return true;
                    }
                }
            }
        }

        return false;
    }

//private async Task DoNotifyStreaming(GattCharacteristic Characteristic, Func<byte[], Task> Callback, CancellationToken CT)
//    {
//        var (dbusHandle, mtu) = await Characteristic.AcquireNotifyAsync(new Dictionary<string, object>());
//        IntPtr fd = dbusHandle.DangerousGetHandle();
//
//        using (var safeHandle = new SafeFileHandle(fd, ownsHandle: true))
//        {
//            using var stream = new FileStream(safeHandle, FileAccess.Read, 1, true);
//            
//            // FIX: Create a 1-byte buffer. This forces the stream to return immediately 
//            // the millisecond a single byte arrives over Bluetooth.
//            byte[] buffer = new byte[1]; 
//
//            try 
//            {
//                while (!CT.IsCancellationRequested)
//                {
//                    // Request exactly 1 byte.
//                    int bytesRead = await stream.ReadAsync(buffer, 0, 1, CT);
//                    
//                    if (bytesRead == 0) break; 
//
//                    await Callback(buffer[..bytesRead]);
//                }
//            }
//            catch (Exception ex)
//            {
//                Console.WriteLine(ex.Message);
//            }
//            finally 
//            {
//                await Characteristic.StopNotifyAsync();
//            }
//        }
//    }
    // --- internal startup / connection callbacks ---
    private async Task BLAdapter_PoweredOn(Adapter adapter, BlueZEventArgs e)
    {
        Console.WriteLine("BLE Adapter powered on!");
        //await adapter.StartDiscoveryAsync();
        PhysicalAdapterPoweredOnTask.TrySetResult();
    }

    private async Task BLAdapter_DeviceFound(Adapter adapter, DeviceFoundEventArgs e)
    {
        Device device = e.Device;
        Device1Properties DeviceProperties = await device.GetAllAsync(); //must request device properties beforehand to ensure other stuff works
        string[]? ServicesUUIDs = DeviceProperties.UUIDs;

        if (ServicesUUIDs != null && ServicesUUIDs.Contains(ServiceUUID) && DeviceFoundTask != null && !DeviceFoundTask.Task.IsCompleted) //-> found target device
        {
            Device = device;
            DeviceFoundTask?.SetResult(); //raise device found event
        }
    }
    private async Task Device_Connected(Device device, BlueZEventArgs e)
    {
        DeviceConnectedTask?.SetResult(); //raise device connected event
    }

    private static async Task Device_Disconnected(Device device, BlueZEventArgs e)
    {
        await device.ConnectAsync();
    }

    private async Task Device_ServicesResolved(Device sender, BlueZEventArgs eventArgs)
    {
        ServicesResolvedTask?.TrySetResult();
    }
}
#endif

#if WINDOWS
public class BLEDevice : IBLEDevice
{
    Guid ServiceUUID;

    TaskCompletionSource? DeviceFoundFlag;
    BluetoothLEDevice? TargetDevice = null;
    GattDeviceService? TargetService = null;

    public BLEDevice(string DeviceServiceUUID)
    {
        ServiceUUID = new Guid(DeviceServiceUUID);
    }

    public async Task<bool> DiscoverAndConnect(int TimeoutMS = 15000)
    {
        if (TargetDevice != null && TargetService != null) return true;


        var Watcher = new BluetoothLEAdvertisementWatcher();
        Watcher.ScanningMode = BluetoothLEScanningMode.Active;

        Watcher.AdvertisementFilter.Advertisement = new BluetoothLEAdvertisement();
        Watcher.AdvertisementFilter.Advertisement.ServiceUuids.Add(ServiceUUID);

        ulong DiscoveredDeviceAddress = 0;
        DeviceFoundFlag = new TaskCompletionSource();
        Watcher.Received += (watcher, args) =>
        {
            DiscoveredDeviceAddress = args.BluetoothAddress;
            DeviceFoundFlag.TrySetResult();
        };
        Watcher.Start();

        await DeviceFoundFlag.Task;
        Watcher.Stop();
        await Task.Delay(300);

        TargetDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(DiscoveredDeviceAddress);
        var ServiceResult = await TargetDevice.GetGattServicesAsync();

        if(ServiceResult.Status != GattCommunicationStatus.Success)
        {
            Console.WriteLine("Error: " + ServiceResult.Status);
        }

        foreach(var Service in ServiceResult.Services)
        {
            if(Service.Uuid == ServiceUUID)
            {
                TargetService = Service;
                //Console.WriteLine($"Found device {TargetDevice.Name} with service {TargetService.Uuid}!");
                break;
            }
        }


/*
        // Query for extra properties you want returned
        string[] requestedProperties = { "System.Devices.Aep.DeviceAddress", "System.Devices.Aep.IsConnected" };


        DeviceFoundFlag = new TaskCompletionSource();
        DeviceWatcher deviceWatcher =
                    DeviceInformation.CreateWatcher(
                            BluetoothLEDevice.GetDeviceSelectorFromPairingState(false),
                            requestedProperties,
                            DeviceInformationKind.AssociationEndpoint);
        deviceWatcher.Added += async (Watcher, DeviceInfo) =>
        {
            if(!DeviceInfo.Name.Contains("Mouse")) return;

            Console.WriteLine("Added " + DeviceInfo.Name + ": " + String.Join(", ", DeviceInfo.Properties.Keys));
            BluetoothLEDevice bluetoothLeDevice = await BluetoothLEDevice.FromIdAsync(DeviceInfo.Id);
            Console.WriteLine("Status: " + bluetoothLeDevice.ConnectionStatus);
            DeviceAccessStatus AccessStatus = await bluetoothLeDevice.RequestAccessAsync();
            Console.WriteLine("Status: " + AccessStatus);
            var Result = await bluetoothLeDevice.GetGattServicesAsync();
            Console.WriteLine("Gatt status: " + Result.Status);
            Console.WriteLine("Status: " + bluetoothLeDevice.ConnectionStatus);

        };
        deviceWatcher.Updated += (Watcher, DeviceInfo) =>
        {
           Console.WriteLine("Updated " + DeviceInfo.Id + ": " + (DeviceInfo.Properties.ContainsKey("System.Devices.Aep.IsConnected") ? ((bool)DeviceInfo.Properties["System.Devices.Aep.IsConnected"] ? "Connected" : "Not connected") : "No connection property"));
        };

        deviceWatcher.Start();
        DeviceFoundFlag.Task.Wait();*/
        return true;
    }

    public string? GetDeviceAddress()
    {
        if (TargetDevice == null) return null;

        return TargetDevice.BluetoothAddress.ToString();
    }

    public string? GetDeviceName()
    {
        if (TargetDevice == null) return null;

        return TargetDevice.Name;
    }

    public async Task<byte[]?> ReadCharacteristic(string CharacteristicUUID)
    {
        if (TargetService == null) return null;

        GattCharacteristic? Characteristic = (await TargetService.GetCharacteristicsForUuidAsync(new Guid(CharacteristicUUID))).Characteristics.FirstOrDefault();
        if (Characteristic == null) return null;

        if (!Characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Read)) return null;

        GattReadResult ReadResult = await Characteristic.ReadValueAsync();
        if (ReadResult.Status != GattCommunicationStatus.Success) return null;

        var Reader = DataReader.FromBuffer(ReadResult.Value);
        byte[] Output = new byte[Reader.UnconsumedBufferLength];
        Reader.ReadBytes(Output);

        return Output;
    }

    public async Task<bool> SubscribeNotify(string CharacteristicUUID, Func<byte[], Task> Callback)
    {
        if (TargetService == null) return false;

        GattCharacteristic? Characteristic = (await TargetService.GetCharacteristicsForUuidAsync(new Guid(CharacteristicUUID))).Characteristics.FirstOrDefault();
        if (Characteristic == null) return false;

        bool HasNotify = Characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Notify);
        bool HasIndicate = Characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Indicate);
        if (!(HasNotify | HasIndicate)) return false;

        Characteristic.ValueChanged += (Sender, Args) =>
        {
            var Reader = DataReader.FromBuffer(Args.CharacteristicValue);
            byte[] Output = new byte[Reader.UnconsumedBufferLength];
            Reader.ReadBytes(Output);

            Callback(Output);
        };

        GattCommunicationStatus CommunicationStatus = await Characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(HasNotify ? GattClientCharacteristicConfigurationDescriptorValue.Notify : GattClientCharacteristicConfigurationDescriptorValue.Indicate);
        
        return CommunicationStatus == GattCommunicationStatus.Success;
    }

    public async Task<bool> WriteCharacteristic(string CharacteristicUUID, byte[] Data)
    {        
        if (TargetService == null) return false;

        GattCharacteristic? Characteristic = (await TargetService.GetCharacteristicsForUuidAsync(new Guid(CharacteristicUUID))).Characteristics.FirstOrDefault();
        if (Characteristic == null) return false;

        if (!Characteristic.CharacteristicProperties.HasFlag(GattCharacteristicProperties.Write)) return false;

        var Writer = new DataWriter();
        Writer.WriteBytes(Data);

        GattCommunicationStatus CommunicationResults = await Characteristic.WriteValueAsync(Writer.DetachBuffer());
        return CommunicationResults == GattCommunicationStatus.Success;
    }
      private async void WatcherReceived(BluetoothLEAdvertisementWatcher Watcher, BluetoothLEAdvertisementReceivedEventArgs Args)
    {
        BluetoothLEDevice Device = await BluetoothLEDevice.FromBluetoothAddressAsync(Args.BluetoothAddress);
        
        if(!Device.DeviceInformation.Pairing.CanPair)
        {
            Console.WriteLine($"Cannot pair with device {Device.Name}!");
            return;
        }

        var PairingResult = await Device.DeviceInformation.Pairing.PairAsync();
        if(PairingResult.Status != Windows.Devices.Enumeration.DevicePairingResultStatus.Paired && PairingResult.Status != Windows.Devices.Enumeration.DevicePairingResultStatus.AlreadyPaired)
        {
            Console.WriteLine($"Pairing with device {Device.Name} failed! {PairingResult.Status}");
        }
        
        if (Device != null)
        {
            var result = await Device.GetGattServicesAsync();
            if (result.Status == GattCommunicationStatus.Success)
            {
                foreach (var Service in result.Services)
                {
                    if (Service.Uuid == ServiceUUID)
                    {
                        Console.WriteLine($"Found matching device: {Device.Name}");
                        TargetDevice = Device;
                        TargetService = Service;
                        DeviceFoundFlag?.TrySetResult();
                        Watcher.Stop();
                    }
                }
            }
            else
            {
                Console.WriteLine("Failed to connect to device: " + result.Status);
            }
        }
    }
}
#endif
