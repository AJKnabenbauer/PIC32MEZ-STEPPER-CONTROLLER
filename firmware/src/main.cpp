

/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#include <limits.h>                     // MIN and MAX type values ie. 

#include <string.h>
#include <stdarg.h>                     // For va lists
#include <stdio.h>

//#include <stdbool.h>

#include "string_helper.h"


#include <string>

#include "usb_com_port.h"



/***************************************************************************
 * Helper MACROs and functions to more easily control the individual OCMPs
 * and TMR that are used to control the RGB LEDs PWM.
 ***************************************************************************/

// Remap OMCP8 to Red portion of RGB LED
#define LED_RGB_RED_ENABLE() OCMP8_Enable()
#define LED_RGB_RED_DISABLE() OCMP8_Disable()
#define LED_RGB_RED_VAL_GET() OCMP8_CompareSecondaryValueGet()
#define LED_RGB_RED_VAL_SET(val) OCMP8_CompareSecondaryValueSet(val)

// Remap OMCP5 to Green portion of RGB LED
#define LED_RGB_GREEN_ENABLE() OCMP5_Enable()
#define LED_RGB_GREEN_DISABLE() OCMP5_Disable()
#define LED_RGB_GREEN_VAL_GET() OCMP5_CompareSecondaryValueGet()
#define LED_RGB_GREEN_VAL_SET(val) OCMP5_CompareSecondaryValueSet(val)

// Remap OMCP3 to Blue portion of RGB LED
#define LED_RGB_BLUE_ENABLE() OCMP3_Enable()
#define LED_RGB_BLUE_DISABLE() OCMP3_Disable()
#define LED_RGB_BLUE_VAL_GET() OCMP3_CompareSecondaryValueGet()
#define LED_RGB_BLUE_VAL_SET(val) OCMP3_CompareSecondaryValueSet(val)

// Set the RGB LEDs color
void LED_RGB_setColor( uint16_t red, uint16_t green, uint16_t blue );

// Enable the OCMPs and TMR 
void LED_RGB_enable( void );

// Disable the OCMPs and TMR 
void LED_RGB_disable( void );


// Red LED is active low
#define LED_RED_On() LED_RED_Clear()
#define LED_RED_Off() LED_RED_Set()


// Yellow LED is active low
#define LED_YELLOW_On() LED_YELLOW_Clear()
#define LED_YELLOW_Off() LED_YELLOW_Set()


// Green LED is active low
#define LED_GREEN_On() LED_GREEN_Clear()
#define LED_GREEN_Off() LED_GREEN_Set()



// UART6 
uint8_t rxByte = 0;

UART_ERROR UART6_Error = UART_ERROR_NONE;

void UART6_RxCallback( uintptr_t context );

void UART6_TxCallback( uintptr_t context );

bool uart_tx_string( const std::string& str );

bool uart_tx_string_v( const char * format, ... );



// USB

USB_CDC USB1;

uint8_t USB_ALIGNED readBuffer[1024] = { 0 };
uint32_t readBufferBytes = 0;
bool readComplete = true;

uint8_t USB_ALIGNED writeBuffer[1024] = { 0 };
uint32_t writeBufferBytes = 0;
bool writeComplete = true;

bool usbError = false;

void USB_RxCallback( XFR_EVENT_DATA* pData, void* userData );
void USB_TxCallback( XFR_EVENT_DATA* pData, void* userData );

//std::string testString = "this is a test";

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************



int main( void )
{
        /* Initialize all modules */
        SYS_Initialize( NULL );

        CORETIMER_Start( );

        TMR2_PeriodSet( 10000 );
        LED_RGB_setColor( 500, 500, 500 );
        LED_RGB_enable( );

        LED_GREEN_Off( );
        LED_RED_Off( );
        LED_YELLOW_Off( );

        bool SW1_StateOld = SW1_Get( ), SW2_StateOld = SW2_Get( );
        bool SW3_StateOld = SW3_Get( ), SW4_StateOld = GPIO_RC15_Get( );

        UART6_ReadCallbackRegister( UART6_RxCallback, (uintptr_t) NULL );
        UART6_WriteCallbackRegister( UART6_TxCallback, (uintptr_t) NULL );

        UART6_Read( &rxByte, 1 );

        USB1.readCallbackRegister( USB_RxCallback, NULL );
        USB1.writeCallbackRegister( USB_TxCallback, NULL );
        usbError = !(USB1.startup( UINT32_MAX - 10000 ));


        XFR_HANDLE handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

        USB_CDC::ENUM result;

        // Start reading on USB
        if (!usbError)
        {
                readComplete = false;
                result = USB1.scheduleRead( NULL, readBuffer, sizeof (readBuffer) );
                usbError = (result != USB_CDC::ERROR_OK) ? true : false;
        }

        while (true)
        {
                if (usbError)
                {
                        LED_RED_On( );

                        uart_tx_string(
                                "Last Error: " + std::string( USB_CDC::enum_c_string( USB1.getLastError_enum( ) ) ) + "\r\n"
                                "Last Error String: " + std::string( USB1.getLastError_c_string( ) ) + "\r\n"
                                "\r\n"
                                );
                }

                if (!usbError)
                {
                        if (readComplete && writeComplete)
                        {
                                // copy the read data so we can echo back
                                memcpy( writeBuffer, readBuffer, readBufferBytes );

                                // initiate a write of the data
                                writeBufferBytes = readBufferBytes;
                                writeComplete = false;
                                result = USB1.scheduleWrite( NULL, writeBuffer, readBufferBytes );
                                usbError = (result != USB_CDC::ERROR_OK) ? true : false;

                                // if the write was successful then we start another read
                                if (!usbError)
                                {
                                        readBufferBytes = 0;
                                        readComplete = false;
                                        result = USB1.scheduleRead( NULL, readBuffer, sizeof (readBuffer) );
                                        usbError = (result != USB_CDC::ERROR_OK) ? true : false;
                                }
                        }
                }


                if (SW1_Get( ) != SW1_StateOld || SW2_Get( ) != SW2_StateOld || 
                        SW3_Get( ) != SW3_StateOld || GPIO_RC15_Get( ) != SW4_StateOld )
                {
                        SW1_StateOld = SW1_Get( );
                        SW2_StateOld = SW2_Get( );
                        SW3_StateOld = SW3_Get( );
                        SW4_StateOld = GPIO_RC15_Get( );

                        uart_tx_string(
                                "Last Error: " + USB_CDC::enum_string( USB1.getLastError_enum( ) )  + "\r\n"
                                "Last Error String: " + USB1.getLastError_string( )  + "\r\n"
                                "\r\n" );

                        int irp_status = -5;

                        if (handle != USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
                        {
                                USB_DEVICE_IRP* irp = (USB_DEVICE_IRP*) handle;
                                irp_status = irp->status;

                        }

                }

                readComplete ? LED_YELLOW_Off( ) : LED_YELLOW_On( );
                writeComplete ? LED_GREEN_Off( ) : LED_GREEN_On( );
                usbError ? LED_RED_On( ) : LED_RED_Off( );

                /* Maintain state machines of all polled MPLAB Harmony modules. */

                SYS_Tasks( );

        }

        /* Execution should not come here during normal operation */

        return ( EXIT_FAILURE);
}



void LED_RGB_setColor( uint16_t red, uint16_t green, uint16_t blue )
{
        // TMR2 is the timer that drives our PWM period
        uint16_t period = TMR2_PeriodGet( );

        // We have to subtract our color value from the max
        // value since the LED is active low.
        LED_RGB_RED_VAL_SET( period - red );
        LED_RGB_GREEN_VAL_SET( period - green );
        LED_RGB_BLUE_VAL_SET( period - blue );
}



void LED_RGB_enable( void )
{
        TMR2_Start( );
        LED_RGB_RED_ENABLE( );
        LED_RGB_GREEN_ENABLE( );
        LED_RGB_BLUE_ENABLE( );
}



void LED_RGB_disable( void )
{
        TMR2_Stop( );
        LED_RGB_RED_DISABLE( );
        LED_RGB_GREEN_DISABLE( );
        LED_RGB_BLUE_DISABLE( );
}



void UART6_RxCallback( uintptr_t context )
{
        UART6_Error = UART6_ErrorGet( );

        if (UART6_Error == UART_ERROR_NONE)
        {
                //LED_RED_On();
        } else
        {
                UART6_Read( &rxByte, 1 );
        }
}



void UART6_TxCallback( uintptr_t context )
{
        UART6_Error = UART6_ErrorGet( );

        if (UART6_Error != UART_ERROR_NONE)
        {
                //LED_RED_On();
        }

        LED_YELLOW_Off( );
}



bool uart_tx_string( const std::string& str )
{
        UART6_Write( (void*) str.c_str( ), str.length( ) );
        return true;
}



bool uart_tx_string_v( const char * format, ... )
{
        static char buffer[512] = { 0 };
        int numChars = 0;
        int sizeData = 0;

        if (UART6_WriteIsBusy( ) || UART6_Error != UART_ERROR_NONE)
        {
                return false;
        }

        va_list args;
        va_start( args, format );
        numChars = vsnprintf( buffer, sizeof (buffer), format, args );
        va_end( args );

        if (numChars <= 0)
        {
                return false;
        }

        // check if string size is within our buffer or not and change 
        // the count of bytes to be sent
        sizeData = numChars < sizeof (buffer) ? numChars : sizeof (buffer) - 1;

        UART6_Write( buffer, sizeData );
        LED_YELLOW_On( );

        return true;
}



void USB_RxCallback( XFR_EVENT_DATA* pData, void* userData )
{

        readComplete = true;
        readBufferBytes = pData->length;
}



void USB_TxCallback( XFR_EVENT_DATA* pData, void* userData )
{

        writeComplete = true;
        writeBufferBytes = 0;
}



/*******************************************************************************
 End of File
 */

