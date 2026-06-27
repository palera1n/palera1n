#ifdef WITH_GUI

#include "event.hpp"

#include "wx/AppFrame.hpp"
#include "state.hpp"
#include "sequence.hpp"
#include "utils.h"

#include <wx/wx.h>
#include <thread>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

static inline DeviceState make_fresh_state()
{
    return DeviceState{};
}

void normal_device_event_cb(
    const idevice_event_t* event,
    void* user_data)
{
    #ifdef WITH_GUI
    auto* frame = static_cast<MainFrame*>(user_data);
    if (!frame || !event)
        return;
    #endif

    DeviceState state = make_fresh_state();

    if (event->event == IDEVICE_DEVICE_REMOVE)
    {
        #ifdef WITH_GUI
        send_device_state(frame, state);
        #endif
        return;
    }

    if (event->event != IDEVICE_DEVICE_ADD)
        return;

    idevice_t device = nullptr;
    if (idevice_new(&device, event->udid) != IDEVICE_E_SUCCESS)
        return;

    state.connected = true;
    state.mode = DeviceMode::Normal;
    state.udid = event->udid ? event->udid : "";

    lockdownd_client_t client = nullptr;

    if (lockdownd_client_new_with_handshake(device, &client, "palera1n") == LOCKDOWN_E_SUCCESS)
    {
        plist_t product = nullptr;
        plist_t version = nullptr;
        plist_t ecid = nullptr;

        char *p = nullptr, *v = nullptr;
        uint64_t ecid_val = 0;

        lockdownd_get_value(client, nullptr, "ProductType", &product);
        lockdownd_get_value(client, nullptr, "ProductVersion", &version);
        lockdownd_get_value(client, nullptr, "UniqueChipID", &ecid);

        if (product) plist_get_string_val(product, &p);
        if (version) plist_get_string_val(version, &v);
        if (ecid) plist_get_uint_val(ecid, &ecid_val);

        state.productType = p ? p : "";
        state.productVersion = v ? v : "";
        state.ecid = ecid_val;

        free(p);
        free(v);

        if (product) plist_free(product);
        if (version) plist_free(version);
        if (ecid) plist_free(ecid);

        lockdownd_client_free(client);
    }

    idevice_free(device);

    state.isSupported = SequenceIsSupported(state.productType);

    #ifdef WITH_GUI
    send_device_state(frame, state);
    #endif
}

void recovery_device_event_cb(
    const irecv_device_event_t* event,
    void* user_data)
{
    #ifdef WITH_GUI
    auto* frame = static_cast<MainFrame*>(user_data);
    if (!frame || !event)
        return;
    #endif

    DeviceState state = make_fresh_state();

    if (event->type == IRECV_DEVICE_REMOVE)
    {
        #ifdef WITH_GUI
        send_device_state(frame, state);
        #endif
        return;
    }

    if (event->type != IRECV_DEVICE_ADD)
        return;

    state.connected = true;

    if (event->mode == IRECV_K_DFU_MODE)
    {
        state.mode = DeviceMode::DFU;
        #ifdef WITH_GUI
        send_device_state(frame, state);
        #endif
        return;
    }

    if (!(event->mode == IRECV_K_RECOVERY_MODE_1 ||
        event->mode == IRECV_K_RECOVERY_MODE_2 ||
        event->mode == IRECV_K_RECOVERY_MODE_3 ||
        event->mode == IRECV_K_RECOVERY_MODE_4))
    {
        return;
    }

    state.mode = DeviceMode::Recovery;

    uint64_t ecid_val = 0;
    if (event->device_info && event->device_info->ecid)
        ecid_val = event->device_info->ecid;
    state.ecid = ecid_val;

    irecv_client_t client = nullptr;

    int attempts = 0;
    const int max_attempts = 10;

    while (attempts < max_attempts)
    {
        if (irecv_open_with_ecid(&client, state.ecid) == IRECV_E_SUCCESS)
            break;

        attempts++;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    if (client)
    {
        VERBOSE("Opened client after %d attempts", attempts);

        irecv_device_t device = nullptr;
        if (irecv_devices_get_device_by_client(client, &device) == IRECV_E_SUCCESS && device)
        {
            state.productType = device->product_type ? device->product_type : "";
            state.displayName = device->display_name ? device->display_name : "";
        }

        irecv_close(client);
    }
    else
    {
        ERROR("Failed to open irecv client after retries");
    }

    state.isSupported = SequenceIsSupported(state.productType);

    #ifdef WITH_GUI
    send_device_state(frame, state);
    #endif
}

#endif // WITH_GUI
