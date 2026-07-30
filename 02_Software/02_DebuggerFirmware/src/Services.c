#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "Services.h"

LOG_MODULE_REGISTER(BLEServices, LOG_LEVEL_DBG);

static volatile bool notify_mysensor_enabled = false;
static struct Service_Callbacks svc_callbacks;
static struct FirmwareVersion_t internal_firmware_version;

// --- forward declarations ---
static ssize_t read_firmware_version(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

static ssize_t write_command(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

static ssize_t reset_command(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

static void config_changed_DebugNotify(const struct bt_gatt_attr *attr, uint16_t value);


// --- service declaration ---

//only use encrypted versions of security management layer is enabled
#ifdef CONFIG_BT_SMP 
	#define CUSTOM_PERM_WRITE BT_GATT_PERM_WRITE_ENCRYPT
	#define CUSTOM_PERM_READ BT_GATT_PERM_READ_ENCRYPT
#else
	#define CUSTOM_PERM_WRITE BT_GATT_PERM_WRITE
	#define CUSTOM_PERM_READ BT_GATT_PERM_READ
#endif

BT_GATT_SERVICE_DEFINE(
	service_definition, BT_GATT_PRIMARY_SERVICE(BT_UUID_Service),

	//Debug notification service (NOTE: index of this entry is used by Services_send_sensor_notify)
	BT_GATT_CHARACTERISTIC(BT_UUID_Service_DebugNotify, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
	//DebugNotify configuration service
	BT_GATT_CCC(config_changed_DebugNotify, CUSTOM_PERM_READ | CUSTOM_PERM_WRITE),

	//Firmware version service
	BT_GATT_CHARACTERISTIC(BT_UUID_Service_FirmwareVersion, BT_GATT_CHRC_READ, CUSTOM_PERM_READ,
					read_firmware_version, NULL, &internal_firmware_version),

	//Device name service
	BT_GATT_CHARACTERISTIC(BT_UUID_Service_SendCommand, BT_GATT_CHRC_WRITE, CUSTOM_PERM_WRITE,
					NULL, write_command, NULL),

	//Reset service
	BT_GATT_CHARACTERISTIC(BT_UUID_Service_Reset, BT_GATT_CHRC_WRITE, CUSTOM_PERM_WRITE,
					NULL, reset_command, NULL),
);

int Services_init(struct Service_Callbacks *callbacks, struct FirmwareVersion_t *FirmwareVersion)
{
	if(callbacks == 0 || FirmwareVersion == 0) return -EFAULT;

	svc_callbacks.set_command_cb = callbacks->set_command_cb;
	svc_callbacks.do_reset_cb = callbacks->do_reset_cb;

	internal_firmware_version.Major = FirmwareVersion->Major;
	internal_firmware_version.Minor = FirmwareVersion->Minor;
	internal_firmware_version.Patch = FirmwareVersion->Patch;

	return 0;
}

int Services_SendDebugNotify(char* DebugMessage, uint16_t size_bytes)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &service_definition.attrs[1], DebugMessage, size_bytes);
}

bool Services_IsSensorNotifyEnabled()
{
    return notify_mysensor_enabled;
}

// --- callbacks ---
//gets called when the firmware version was requested
static ssize_t read_firmware_version(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	if(conn == 0 || attr == 0 || buf == 0) return BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE);

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(struct FirmwareVersion_t));
}



//gets called when the device name was set
static ssize_t write_command(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	if(conn == 0 || attr == 0 || buf == 0) return BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE);

	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if(svc_callbacks.set_command_cb != 0) svc_callbacks.set_command_cb(buf, len);

	return len;
}

static ssize_t reset_command(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	if(conn == 0 || attr == 0 || buf == 0) return BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE);

	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if(svc_callbacks.do_reset_cb != 0) svc_callbacks.do_reset_cb();

	return len;
}

//gets called when the sensor notification configuration was changed (enabled/disabled)
static void config_changed_DebugNotify(const struct bt_gatt_attr *attr, uint16_t value)
{
	notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}