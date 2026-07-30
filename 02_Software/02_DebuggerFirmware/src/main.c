#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>
#include <stdlib.h>
#include "Services.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/devicetree.h>

LOG_MODULE_REGISTER(Main, LOG_LEVEL_DBG);

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define ExternalResetTime K_MSEC(250)

static struct FirmwareVersion_t FirmwareVersion = {
	.Major = 1,
	.Minor = 1,
	.Patch = 0
};

static const struct gpio_dt_spec status_led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec external_reset = GPIO_DT_SPEC_GET(DT_PATH(external_resets, ctrl_0), gpios);

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart20));

// --- advertising settings ---
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	800, /* Min Advertising Interval 500ms (800*0.625ms) */
	801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
	NULL /* Set to NULL for undirected advertising */
);

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_Service_VAL),
};

// --- BLE service callbacks ---
static void SendCommandCB(char* Buffer, uint32_t Length)
{
	LOG_INF("Got command %s", Buffer);

	int ret = uart_tx(uart, Buffer, Length, SYS_FOREVER_US);
	if (ret) {
		LOG_ERR("Failed to send command via uart!");
	}
}

static void DoResetCB()
{
	LOG_INF("Reset Command received!");

	gpio_pin_set_dt(&external_reset, 1);
	k_sleep(ExternalResetTime);
	gpio_pin_set_dt(&external_reset, 0);
}

static struct Service_Callbacks app_callbacks = {
	.set_command_cb = SendCommandCB,
	.do_reset_cb = DoResetCB
};

static void adv_work_handler(struct k_work *work)
{
	int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Failed to re-enable advertising (err %d)", err);
		return;
	}

	LOG_INF("Advertising successfully started");
}

static struct k_work adv_work;

static void advertising_start(void)
{
	k_work_submit(&adv_work);
}

bool DeviceConnected = false;

// --- connect / disconnect handlers ---
static void on_connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}

	LOG_INF("BLE device connected!");

	//stop advertising after a device is connected
	bt_le_adv_stop(); 

	DeviceConnected = true;
	gpio_pin_set_dt(&status_led, 1);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("BLE device disconnected (reason %u)", reason);

	gpio_pin_set_dt(&status_led, 0);
	DeviceConnected = false;

	//re-enable advertising after the device has disconnected
	advertising_start();
}

static void on_recycled(void)
{
	LOG_INF("Connection object available from previous conn. Disconnect is complete!\n");
	advertising_start();
}

#ifdef CONFIG_BT_SMP
static void on_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		LOG_INF("Security changed: %s level %u", addr, level);
	} else {
		LOG_INF("Security failed: %s level %u err %d", addr, level, err);
	}
}
#endif

struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
	.recycled = on_recycled,
#ifdef CONFIG_BT_SMP
	.security_changed = on_security_changed,
#endif
};

// --- main ---
int main(void)
{
	int err;
	
	LOG_INF("Starting Micromouse BLE Debugger");

	// initialize gpios
	while (!gpio_is_ready_dt(&status_led)) 
	{
		k_sleep(K_MSEC(100));
	}

	err = gpio_pin_configure_dt(&status_led, GPIO_OUTPUT_INACTIVE);
	if (err)
	{
		LOG_ERR("Failed to initialize status led!");
		return -1;
	}

	while (!gpio_is_ready_dt(&external_reset)) 
	{
		k_sleep(K_MSEC(100));
	}

	err = gpio_pin_configure_dt(&external_reset, GPIO_OUTPUT_INACTIVE);
	if (err)
	{
		LOG_ERR("Failed to initialize external reset!");
		return -1;
	}

	// initialize uart / wait until device is ready
	while(!device_is_ready(uart))
	{
		k_sleep(K_MSEC(100));
	}

	//create static address
	bt_addr_le_t addr;
    err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
    if (err) {
        LOG_ERR("Invalid BT address (err %d)\n", err);
    }

    err = bt_id_create(&addr, NULL);
    if (err < 0) {
        LOG_ERR("Creating new ID failed (err %d)\n", err);
    }

	// enable / initialize bluetooth
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return -1;
	}
	bt_conn_cb_register(&connection_callbacks);

	// initialize BLE services
	err = Services_init(&app_callbacks, &FirmwareVersion);
	if (err) {
		LOG_ERR("Failed to init bluetooth service (err:%d)", err);
		return -1;
	}

	k_work_init(&adv_work, adv_work_handler);

	LOG_INF("Bluetooth initialized");
    
	// start bluetooth advertising
	advertising_start();

	// --- UART RX (polling) ---

	uint64_t BlinkTS = k_uptime_get();
	uint64_t LastPacketTS = 0;
	#define TimeOutMS 2
	bool isInTimeout = true;

	#define UART_BUF_RX 250
	volatile uint8_t UartBuffer[UART_BUF_RX];
	volatile uint16_t BufferIndex = 0;
	
	while (1) {
        volatile uint8_t byte;

        //try to read one byte
        int ret = uart_poll_in(uart, &byte);
		uint64_t CurrentTS = k_uptime_get();

        if (ret == 0 && BufferIndex < UART_BUF_RX) 
		{
			//-> got a new byte
			isInTimeout = false;
			UartBuffer[BufferIndex++] = byte;
			LastPacketTS = CurrentTS;
        }
		else
		{
			//-> no new byte, or buffer full
			if(!isInTimeout && (CurrentTS - LastPacketTS >= TimeOutMS || BufferIndex >= UART_BUF_RX))
			{
				//send buffer via BLE
				Services_SendDebugNotify(UartBuffer, BufferIndex);
				
				isInTimeout = true; //indicate that buffer was already transmitted via BLE
				BufferIndex = 0;
				
			}
		}

		if(!DeviceConnected)
		{
			//LED blinks when no device is connected -> indicate active BLE advertising
			if(CurrentTS - BlinkTS >= 100)
			{
				gpio_pin_toggle_dt(&status_led);
				BlinkTS = CurrentTS;
			}
		}
    }
}