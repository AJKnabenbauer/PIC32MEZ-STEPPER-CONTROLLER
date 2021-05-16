#include "usb_com_port.h"



USB_CDC::USB_CDC( )
{
        _lineCoding.dwDTERate = 115200;
        _lineCoding.bParityType = 0;
        _lineCoding.bDataBits = 8;
}



void USB_CDC::readCallbackRegister( XFR_CALLBACK ptr, void* userData )
{
        rxCallback = ptr;
        rxUserData = userData;
}



void USB_CDC::writeCallbackRegister( XFR_CALLBACK ptr, void* userData )
{
        txCallback = ptr;
        txUserData = userData;
}



bool USB_CDC::openDevice( void )
{
        // Check if we already have a valid device context
        if (_deviceHandle != USB_DEVICE_HANDLE_INVALID)
        {
                return true;
        }

        // Attempt to open a device
        _deviceHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE );

        if (_deviceHandle != USB_DEVICE_HANDLE_INVALID)
        {
                // If it opened okay, register a callback with device layer to get event notifications
                USB_DEVICE_EventHandlerSet( _deviceHandle, _deviceEventHandler, (uintptr_t)this );

                return true;
        }

        return false;
}



bool USB_CDC::startup( uint32_t timeout, bool doTasks )
{
        uint32_t timeStart = CORETIMER_CounterGet( );

        bool notReady = (openDevice( ) == false || _isConfigured == false) ? true : false;

        while (notReady)
        {
                if (doTasks)
                {
                      SYS_Tasks( );  
                }
                
                notReady = (openDevice( ) == false || _isConfigured == false) ? true : false;

                if ((CORETIMER_CounterGet( ) - timeStart) >= timeout) return false;
        }

        return true;
}



USB_CDC::ENUM USB_CDC::scheduleRead( XFR_HANDLE* handle, void* data, size_t size )
{
        static XFR_HANDLE fallbackHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

        XFR_HANDLE* transferHandle = (handle != NULL) ? handle : &fallbackHandle;

        ENUM result = (ENUM) USB_DEVICE_CDC_Read(
                _CDCIndex,
                transferHandle,
                data,
                size );

        return result;
}



USB_CDC::ENUM USB_CDC::scheduleWrite( XFR_HANDLE* handle, void* data, size_t size )
{
        static XFR_HANDLE fallbackHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

        XFR_HANDLE* transferHandle = (handle != NULL) ? handle : &fallbackHandle;

        ENUM result = (ENUM) USB_DEVICE_CDC_Write(
                _CDCIndex,
                transferHandle,
                data,
                size,
                USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE );

        return result;
}



void USB_CDC::_deviceEventHandler( USB_DEVICE_EVENT event, void * pData, uintptr_t context )
{
        USB_CDC* usbContext = (USB_CDC *) context;

        switch (event)
        {
                case USB_DEVICE_EVENT_POWER_DETECTED:

                        // VBUS was detected. We can attach the device 

                        USB_DEVICE_Attach( usbContext->_deviceHandle );

                        break;

                case USB_DEVICE_EVENT_POWER_REMOVED:

                        // VBUS is not available. We can detach the device

                        USB_DEVICE_Detach( usbContext->_deviceHandle );

                        usbContext->_isConfigured = false;

                        break;

                case USB_DEVICE_EVENT_RESET:

                        usbContext->_isConfigured = false;

                        break;

                case USB_DEVICE_EVENT_CONFIGURED:

                        // Device layer initialized and we should register event handlers. 
                        // Note we only support configuration 1 

                        if (((USB_DEVICE_EVENT_DATA_CONFIGURED*) pData)->configurationValue == 1)
                        {
                                // Register CDC Device handler

                                USB_DEVICE_CDC_EventHandlerSet( usbContext->_CDCIndex, _CDCEventHandler, context );

                                usbContext->_isConfigured = true;
                        }

                        break;

                case USB_DEVICE_EVENT_DECONFIGURED:
                case USB_DEVICE_EVENT_SUSPENDED:
                case USB_DEVICE_EVENT_RESUMED:
                case USB_DEVICE_EVENT_ERROR:
                case USB_DEVICE_EVENT_SOF:
                default:
                        break;
        }
}



USB_DEVICE_CDC_EVENT_RESPONSE USB_CDC::_CDCEventHandler(
        USB_DEVICE_CDC_INDEX index, USB_DEVICE_CDC_EVENT event, void * pData, uintptr_t userData )
{
        USB_CDC* usbContext = (USB_CDC *) userData;

        switch (event)
        {
                case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

                        USB_DEVICE_ControlReceive( usbContext->_deviceHandle, &usbContext->_lineCoding, sizeof (USB_CDC_LINE_CODING) );

                        break;

                case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

                        USB_DEVICE_ControlSend( usbContext->_deviceHandle, &usbContext->_lineCoding, sizeof (USB_CDC_LINE_CODING) );

                        break;

                case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

                        usbContext->_controlLineState.dtr = ((USB_CDC_CONTROL_LINE_STATE*) pData)->dtr;
                        usbContext->_controlLineState.carrier = ((USB_CDC_CONTROL_LINE_STATE*) pData)->carrier;

                        USB_DEVICE_ControlStatus( usbContext->_deviceHandle, USB_DEVICE_CONTROL_STATUS_OK );

                        break;

                case USB_DEVICE_CDC_EVENT_SEND_BREAK:

                        USB_DEVICE_ControlStatus( usbContext->_deviceHandle, USB_DEVICE_CONTROL_STATUS_OK );

                        break;

                case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

                        if (usbContext->txCallback != NULL)
                        {
                                usbContext->txCallback( (XFR_EVENT_DATA*) pData, usbContext->txUserData );
                        }

                        break;

                case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

                        if (usbContext->rxCallback != NULL)
                        {
                                usbContext->rxCallback( (XFR_EVENT_DATA*) pData, usbContext->rxUserData );
                        }

                        break;

                case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

                        USB_DEVICE_ControlStatus( usbContext->_deviceHandle, USB_DEVICE_CONTROL_STATUS_OK );

                        break;

                case USB_DEVICE_CDC_EVENT_SERIAL_STATE_NOTIFICATION_COMPLETE:
                case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:
                default:
                        break;
        }

        return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}