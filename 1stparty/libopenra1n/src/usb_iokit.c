#ifdef __APPLE__

#include <usb_iokit.h>

#include <stdbool.h>

#include <CommonCrypto/CommonCrypto.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#include <utils.h>

static void cf_dictionary_set_int16(CFMutableDictionaryRef dict, const void *key, uint16_t val) {
    CFNumberRef cf_val = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &val);

    if(cf_val != NULL) {
        CFDictionarySetValue(dict, key, cf_val);
        CFRelease(cf_val);
    }
}

bool query_usb_interface(io_service_t serv, CFUUIDRef plugin_type, CFUUIDRef interface_type, LPVOID *interface) {
    IOCFPlugInInterface **plugin_interface;
    SInt32 score;
    bool ret = false;

    if(IOCreatePlugInInterfaceForService(serv, plugin_type, kIOCFPlugInInterfaceID, &plugin_interface, &score) == kIOReturnSuccess) {
        ret = (*plugin_interface)->QueryInterface(plugin_interface, CFUUIDGetUUIDBytes(interface_type), interface) == kIOReturnSuccess;
        IODestroyPlugInInterface(plugin_interface);
    }

    IOObjectRelease(serv);
    return ret;
}

static bool open_usb_interface(usb_handle_t *handle) {
    IOUSBFindInterfaceRequest req;
    io_iterator_t iter;
    io_service_t intfServ;
    IOReturn kr;

    req.bInterfaceClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
    req.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
    req.bAlternateSetting = kIOUSBFindInterfaceDontCare;

    kr = (*handle->device)->CreateInterfaceIterator(handle->device, &req, &iter);
    if(kr != kIOReturnSuccess) return false;

    while((intfServ = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
        IOCFPlugInInterface **plugin = NULL;
        IOUSBInterfaceInterface245 **intf = NULL;
        SInt32 score = 0;
        HRESULT res;

        kr = IOCreatePlugInInterfaceForService(intfServ, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin, &score);

        IOObjectRelease(intfServ);

        if(kr != kIOReturnSuccess || !plugin) continue;

        res = (*plugin)->QueryInterface(plugin, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID245), (LPVOID *)&intf);

        (*plugin)->Release(plugin);

        if(res != S_OK || !intf) continue;

        kr = (*intf)->USBInterfaceOpen(intf);
        if(kr != kIOReturnSuccess) {
            (*intf)->Release(intf);
            continue;
        }

        handle->interface = intf;
        IOObjectRelease(iter);
        return true;
    }

    IOObjectRelease(iter);
    return false;
}

void close_usb_handle(usb_handle_t *handle) {
    if(handle->async_event_source) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), handle->async_event_source, kCFRunLoopDefaultMode);
        CFRelease(handle->async_event_source);
        handle->async_event_source = NULL;
    }

    if(handle->interface) {
        (*handle->interface)->USBInterfaceClose(handle->interface);
        (*handle->interface)->Release(handle->interface);
        handle->interface = NULL;
    }

    if(handle->device) {
        (*handle->device)->USBDeviceClose(handle->device);
        (*handle->device)->Release(handle->device);
        handle->device = NULL;
    }
}

bool open_usb_device(io_service_t serv, usb_handle_t *handle)
{
    if (!query_usb_interface(serv, kIOUSBDeviceUserClientTypeID, kIOUSBDeviceInterfaceID320, (LPVOID *)&handle->device)) return false;

    IOReturn kr;

    for (int i = 0; i < 10; i++) {
        kr = (*handle->device)->USBDeviceOpenSeize(handle->device);

        if (kr == kIOReturnSuccess) break;
        if (kr != kIOReturnExclusiveAccess) goto fail;

        usleep(100000);
    }

    if (kr != kIOReturnSuccess) goto fail;

    kr = (*handle->device)->SetConfiguration(handle->device, 1);

    if (kr != kIOReturnSuccess && kr != kIOReturnExclusiveAccess) goto fail;

    kr = (*handle->device)->CreateDeviceAsyncEventSource(
        handle->device,
        &handle->async_event_source
    );

    if (kr != kIOReturnSuccess) goto fail;

    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        handle->async_event_source,
        kCFRunLoopDefaultMode
    );

    if (!open_usb_interface(handle)) goto fail;

    return true;
fail:
    close_usb_handle(handle);
    return false;
}

bool wait_usb_handle(usb_handle_t *handle) {
    CFMutableDictionaryRef matching_dict;
    io_iterator_t iter;
    io_service_t serv;
    bool ret = false;

    while((matching_dict = IOServiceMatching(kUSBDeviceClassName)) != NULL) {
        cf_dictionary_set_int16(matching_dict, CFSTR(kUSBVendorID), handle->vid);
        cf_dictionary_set_int16(matching_dict, CFSTR(kUSBProductID), handle->pid);

        if(IOServiceGetMatchingServices(0, matching_dict, &iter) == kIOReturnSuccess) {
            while((serv = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
                if(open_usb_device(serv, handle)) {
                    ret = true;
                    break;
                }
            }

            IOObjectRelease(iter);
            if(ret) break;
            sleep_ms(USB_TIMEOUT);
        }
    }

    return ret;
}

void reset_usb_handle(usb_handle_t *handle) {
    if(handle->device) {
        (*handle->device)->ResetDevice(handle->device);
        (*handle->device)->USBDeviceReEnumerate(handle->device, 0);
    }
}

transfer_ret_t send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len) {
    transfer_ret_t result;

    if(!handle || !handle->interface) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    IOUSBDevRequestTO req = {
        .wLenDone = 0,
        .pData = p_data,
        .bRequest = b_request,
        .bmRequestType = bm_request_type,
        .wLength = OSSwapLittleToHostInt16(w_len),
        .wValue = OSSwapLittleToHostInt16(w_value),
        .wIndex = OSSwapLittleToHostInt16(w_index),
        .completionTimeout = USB_TIMEOUT,
        .noDataTimeout = USB_TIMEOUT,
    };

    IOReturn ret = (*handle->device)->DeviceRequestTO(handle->device, &req);

    if(ret == kIOReturnSuccess) {
        result.sz = req.wLenDone;
        result.ret = USB_TRANSFER_OK;
    } else if(ret == kUSBPipeStalled) {
        result.ret = USB_TRANSFER_STALL;
    } else {
        result.ret = USB_TRANSFER_ERROR;
    }

    return result;
}

static void usb_async_cb(void *refcon, IOReturn ret, void *arg) {
    transfer_ret_t *transfer_ret = refcon;

    if(transfer_ret != NULL) {
        memcpy(&transfer_ret->sz, &arg, sizeof(transfer_ret->sz));

        if(ret == kIOReturnSuccess) {
            transfer_ret->ret = USB_TRANSFER_OK;
        } else if(ret == kUSBPipeStalled) {
            transfer_ret->ret = USB_TRANSFER_STALL;
        } else {
            transfer_ret->ret = USB_TRANSFER_ERROR;
        }
    }

    CFRunLoopStop(CFRunLoopGetCurrent());
}

transfer_ret_t send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout) {
    transfer_ret_t result;

    if(!handle || !handle->interface) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    IOUSBDevRequestTO req = {
        .wLenDone = 0,
        .pData = p_data,
        .bRequest = b_request,
        .bmRequestType = bm_request_type,
        .wLength = OSSwapLittleToHostInt16(w_len),
        .wValue = OSSwapLittleToHostInt16(w_value),
        .wIndex = OSSwapLittleToHostInt16(w_index),
        .completionTimeout = USB_TIMEOUT,
        .noDataTimeout = USB_TIMEOUT,
    };

    if((*handle->device)->DeviceRequestAsyncTO(handle->device, &req, usb_async_cb, &result) == kIOReturnSuccess) {
        sleep_ms(usb_abort_timeout);

        if((*handle->device)->USBDeviceAbortPipeZero(handle->device) == kIOReturnSuccess) {
            CFRunLoopRun();
            return result;
        }
    }

    return result;
}

transfer_ret_t send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len) {
    transfer_ret_t result;

    if(!handle || !handle->interface) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    IOUSBDevRequest req = {
        .bmRequestType = bm_request_type,
        .bRequest = b_request,
        .wValue = w_value,
        .wIndex = w_index,
        .wLength = (UInt16)w_len,
        .pData = p_data,
    };

    IOReturn ret = (*handle->interface)->ControlRequest(handle->interface, 0, &req);

    result.ret = (ret == kIOReturnSuccess) ? USB_TRANSFER_OK : USB_TRANSFER_ERROR;
    result.sz = req.wLenDone;

    return result;
}

transfer_ret_t send_interface_bulk_transfer(const usb_handle_t *handle, void *data, uint32_t len) {
    transfer_ret_t result;

    if(!handle || !handle->interface) {
        result.ret = USB_TRANSFER_ERROR;
        result.sz = 0;
        return result;
    }

    IOReturn ret = (*handle->interface)->WritePipe(handle->interface, 2, data, len);

    result.ret = (ret == kIOReturnSuccess) ? USB_TRANSFER_OK : USB_TRANSFER_ERROR;
    result.sz = len;

    return result;
}

void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid)
{
    handle->vid = vid;
    handle->pid = pid;
    handle->serv = IO_OBJECT_NULL;
    handle->device = NULL;
    handle->interface = NULL;
    handle->async_event_source = NULL;
}

#endif
