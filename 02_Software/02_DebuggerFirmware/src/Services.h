#ifndef BT_Services_H_
#define BT_Services_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

// --- UUIDs ---
//Service UUID
#define BT_UUID_Service_VAL BT_UUID_128_ENCODE(0x9E150000, 0x2424, 0x3535, 0x4711, 0x0000decafbad)
#define BT_UUID_Service BT_UUID_DECLARE_128(BT_UUID_Service_VAL)

//Firmware version UUID
#define BT_UUID_Service_FirmwareVersion_VAL BT_UUID_128_ENCODE(0x9E150001, 0x2424, 0x3535, 0x4711, 0x0000decafbad)
#define BT_UUID_Service_FirmwareVersion BT_UUID_DECLARE_128(BT_UUID_Service_FirmwareVersion_VAL)

//Send Command UUID
#define BT_UUID_Service_SendCommand_VAL BT_UUID_128_ENCODE(0x9E150002, 0x2424, 0x3535, 0x4711, 0x0000decafbad)
#define BT_UUID_Service_SendCommand BT_UUID_DECLARE_128(BT_UUID_Service_SendCommand_VAL)

//Reset UUID
#define BT_UUID_Service_Reset_VAL BT_UUID_128_ENCODE(0x9E150003, 0x2424, 0x3535, 0x4711, 0x0000decafbad)
#define BT_UUID_Service_Reset BT_UUID_DECLARE_128(BT_UUID_Service_Reset_VAL)

//Debug Notify UUID
#define BT_UUID_Service_DebugNotify_VAL BT_UUID_128_ENCODE(0x9E150004, 0x2424, 0x3535, 0x4711, 0x0000decafbad)
#define BT_UUID_Service_DebugNotify BT_UUID_DECLARE_128(BT_UUID_Service_DebugNotify_VAL)

// --- Callbacks ---

//device name
typedef void (*set_command_cb_t)(char* Buffer, uint32_t Length);
typedef void (*do_reset_cb_t)();

//callback struct used by the service
struct Service_Callbacks {
	set_command_cb_t set_command_cb;
	do_reset_cb_t do_reset_cb;
};

struct FirmwareVersion_t
{
	uint8_t Major;
	uint8_t Minor;
	uint8_t Patch;
};

/// @brief Initializes the service
/// @param callbacks Struct containing callbacks for functionality.
/// @param FirmwareVersion Structu containing the firmware version.
/// @return 0 if operation was successful, an errorcode otherwise.
int Services_init(struct Service_Callbacks *callbacks, struct FirmwareVersion_t *FirmwareVersion);

/// @brief Notifies the subscriber about new debug messages.
/// @param DebugMessage Pointer to debug message.
/// @param size_bytes Size of data to send.
/// @return 0 if operation was successful, an errorcode otherwise.
int Services_SendDebugNotify(char* DebugMessage, uint16_t size_bytes);

/// @brief Check if a client enabled the notify characteristic.
/// @return True if enabled, false otherwise.
bool Services_IsSensorNotifyEnabled();

#ifdef __cplusplus
}
#endif

#endif
