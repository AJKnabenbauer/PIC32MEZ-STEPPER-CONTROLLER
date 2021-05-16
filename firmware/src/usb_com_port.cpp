#include "usb_com_port.h"



USB_CDC::USB_CDC( )
{
        _lineCoding.dwDTERate = 115200;
        _lineCoding.bParityType = 0;
        _lineCoding.bDataBits = 8;
}



void USB_CDC::readCallbackRegister( XFR_CALLBACK ptr, void* userData )
{
        _rxCallback = ptr;
        _rxUserData = userData;
}



void USB_CDC::writeCallbackRegister( XFR_CALLBACK ptr, void* userData )
{
        _txCallback = ptr;
        _txUserData = userData;
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

        setLastError( ERROR_UNKNOWN, "Unable to open USB device" );

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

                if ((CORETIMER_CounterGet( ) - timeStart) >= timeout)
                {
                        setLastError( ERROR_UNKNOWN, "Timeout reached during USB_CDC startup" );
                        return false;
                }
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

        return setLastError( result );
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

        return setLastError( result );
}



USB_CDC::ENUM USB_CDC::setLastError( ENUM errorEnum, const char* errorString )
{
        _lastErrorEnum = errorEnum;
        _lastErrorString = errorString;
        return errorEnum;
}



USB_CDC::ENUM USB_CDC::getLastError_enum( )
{
        return _lastErrorEnum;
}



const char* USB_CDC::getLastError_c_string( )
{
        return _lastErrorString;
}



std::string USB_CDC::getLastError_string( )
{
        return std::string( _lastErrorString );
}



const char* USB_CDC::enum_c_string( ENUM enumerator )
{
        /*
         * Note that we are intentionally using a switch statement instead of something 
         * with better performance such as an array or map. This is done because some
         * of the values are based off of error enums from the other USB drivers so the
         * values are not sequential and may not be consistent if things change. The other 
         * reason is because this function is only intended to be used rarely when you 
         * need to log an error.
         */
        switch (enumerator)
        {
                case ERROR_OK:
                        return "ERROR_OK";
                case ERROR_TRANSFER_SIZE_INVALID:
                        return "ERROR_TRANSFER_SIZE_INVALID";
                case ERROR_RANSFER_QUEUE_FULL:
                        return "ERROR_RANSFER_QUEUE_FULL";
                case ERROR_INSTANCE_INVALID:
                        return "ERROR_INSTANCE_INVALID";
                case ERROR_INSTANCE_NOT_CONFIGURED:
                        return "ERROR_INSTANCE_NOT_CONFIGURED";
                case ERROR_PARAMETER_INVALID:
                        return "ERROR_PARAMETER_INVALID";
                case ERROR_ENDPOINT_HALTED:
                        return "ERROR_ENDPOINT_HALTED";
                case ERROR_TERMINATED_BY_HOST:
                        return "ERROR_TERMINATED_BY_HOST";
                case ERROR_UNKNOWN:
                        return "ERROR_UNKNOWN";
                default:
                        return "UNKNOW ENUM";
        }
}



std::string USB_CDC::enum_string( ENUM enumerator )
{
        return std::string( enum_c_string( enumerator ) );
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

                        if (usbContext->_txCallback != NULL)
                        {
                                usbContext->_txCallback( (XFR_EVENT_DATA*) pData, usbContext->_txUserData );
                        }

                        break;

                case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

                        if (usbContext->_rxCallback != NULL)
                        {
                                usbContext->_rxCallback( (XFR_EVENT_DATA*) pData, usbContext->_rxUserData );
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