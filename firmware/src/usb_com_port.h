/* 
 * File:   test.h
 * Author: Andrew
 *
 * Created on April 27, 2021, 4:29 PM
 */

#ifndef TEST_H
#define	TEST_H


#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"


//#ifdef	__cplusplus
//extern "C" {
//#endif


/* Data structure returned during read complete and 
 * write complete CDC events.
 * 
 * Note that USB_DEVICE_CDC_EVENT_DATA_WRITE_COMPLETE and 
 * USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE are defined as 
 * equivalent data structures. This allows us to use a single 
 * typedef TRANSFER_COMPLETE_DATA for both.
 */
typedef USB_DEVICE_CDC_EVENT_DATA_WRITE_COMPLETE XFR_EVENT_DATA;

typedef USB_DEVICE_CDC_TRANSFER_HANDLE XFR_HANDLE;

/* CDC read complete and write complete event callback type */
typedef void (*XFR_CALLBACK)(XFR_EVENT_DATA* pData, void* userData);
class USB_CDC {
public:
        typedef enum {
                ERROR_OK = USB_ERROR_NONE,
                ERROR_TRANSFER_SIZE_INVALID = USB_DEVICE_CDC_RESULT_ERROR_TRANSFER_SIZE_INVALID,
                ERROR_RANSFER_QUEUE_FULL = USB_DEVICE_CDC_RESULT_ERROR_TRANSFER_QUEUE_FULL,
                ERROR_INSTANCE_INVALID = USB_DEVICE_CDC_RESULT_ERROR_INSTANCE_INVALID,
                ERROR_INSTANCE_NOT_CONFIGURED = USB_DEVICE_CDC_RESULT_ERROR_INSTANCE_NOT_CONFIGURED,
                ERROR_PARAMETER_INVALID = USB_DEVICE_CDC_RESULT_ERROR_PARAMETER_INVALID,
                ERROR_ENDPOINT_HALTED = USB_DEVICE_CDC_RESULT_ERROR_ENDPOINT_HALTED,
                ERROR_TERMINATED_BY_HOST = USB_DEVICE_CDC_RESULT_ERROR_TERMINATED_BY_HOST,
                ERROR_UNKNOWN = USB_DEVICE_CDC_RESULT_ERROR
        } ENUM;

private:

        USB_DEVICE_HANDLE _deviceHandle = USB_DEVICE_HANDLE_INVALID;
        USB_CDC_LINE_CODING _lineCoding = { 0 };
        USB_CDC_CONTROL_LINE_STATE _controlLineState = { 0 };
        USB_DEVICE_CDC_INDEX _CDCIndex = USB_DEVICE_CDC_INDEX_0;

        bool _isConfigured = false;

        XFR_CALLBACK rxCallback = NULL;
        void* rxUserData = NULL;

        XFR_CALLBACK txCallback = NULL;
        void* txUserData = NULL;



public:

        USB_CDC( );

        void readCallbackRegister( XFR_CALLBACK ptr, void* userData = NULL );

        void writeCallbackRegister( XFR_CALLBACK ptr, void* userData = NULL );

        /**
         * 
         * @return 
         */
        bool openDevice( void );

        /** 
         * Initialize and preform required startup of USB CDC and device layers
         * 
         * @param timeout - max elapsed time (in CORETIMER ticks) before 
         * the startup attempt fails
         * 
         * @note MAKE SURE THE CORETIMER PERIPHERAL IS INITIALIZED AND STARTED BEFORE CALLING
         * THIS FUNCTION
         * 
         * @note Since the elapsed time is only updated after each SYS_Tasks 
         * call the timeout value should not be set close to UINT32_MAX. Otherwise 
         * there is a chance that the elapsed time value could overflow and never 
         * appear to be greater than the timeout value.
         * 
         * @param doRiverTasks - if true, call system tasks while waiting for startup
         * 
         * @note Since this function is usually used at startup the Device layer and CDC layer 
         * drivers may not be in a state that is ready for us to open and configure our 
         * USB device. If doTasks is true then we call the task routines of the USB driver
         * and device layer (and all  other polled tasks) until the USB_CDC device is 
         * configured and ready, or we timeout.
         * 
         * @return true on successful startup, otherwise false
         */
        bool startup( uint32_t timeout, bool doRiverTasks = true );

        /**
         * Schedule a data read (host -> device) request
         * 
         * @param handle - If the user doesn't plan to use the transfer handle, 
         * they can pass NULL for the handle and we will instead use a static 
         * local handle. 
         * 
         * @note This handle is valid for the lifetime 
         * of the transfer so the user should be careful to not overwrite 
         * it while it is in use
         * 
         * @param data
         * @param size
         * @return 
         */
        ENUM scheduleRead( XFR_HANDLE* handle, void* data, size_t size );

        /**
         * Schedule a data write (device -> host) request
         * 
         * @param handle - If the user doesn't plan to use the transfer handle, 
         * they can pass NULL for the handle and we will instead use a static 
         * local handle. 
         * 
         * @note This handle is valid for the lifetime 
         * of the transfer so the user should be careful to not overwrite 
         * it while it is in use
         * 
         * @param data
         * @param size
         * @return 
         */
        ENUM scheduleWrite( XFR_HANDLE* handle, void* data, size_t size );

private:

        static USB_DEVICE_CDC_EVENT_RESPONSE _CDCEventHandler(
            USB_DEVICE_CDC_INDEX index,
            USB_DEVICE_CDC_EVENT event,
            void * pData,
            uintptr_t userData );

        static void _deviceEventHandler(
            USB_DEVICE_EVENT event,
            void * pData,
            uintptr_t context );

};





//
//#ifdef	__cplusplus
//}
//#endif

#endif	/* TEST_H */

