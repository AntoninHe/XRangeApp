/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"
#include "usb-printf.h"
#include "LoRaMacCrypto.h"
#include "LoRaMac.h"
#include "Key.h"
#include "delay.h"
#include "uart-usb-board.h"
#include "mbedtls/base64.h"
#include "timer.h"
#include "radio.h"

#define VCOM_BUFF_SIZE 256
#define MSG_YES '!'
#define MSG_NO '?'
#define MSG_END '*'

extern Uart_t UartUsb;

#define RX_TIMEOUT_VALUE                            0
#define BUFFER_SIZE                                 64 // Define the payload size here
#define SERIAL_TIMEOUT_VALUE                        10000000

//#define printf(x) PRINTF(x)


#if defined( USE_BAND_868 )

#define RF_FREQUENCY                                867100000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             14        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif


/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255


/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaBridgeToGatewayBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaBridgeToGatewayBuffer
 */
static uint16_t LoRaBridgeToGatewayBufferPktLen = 0;

/*!
 * Length of the payload in LoRaBridgeToGatewayBuffer
 */
//static uint8_t LoRaMacTxPayloadLen = 0;

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t LoRaMacNwkSKey[] =
{
    LORAMACNWKSKEY
};
/*
 * static uint8_t LoRaMacNwkSKey[] =
 * {
 *      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 */

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t LoRaMacAppSKey[] =
{
    LORAMACAPPSKEY
};
/*
 * static uint8_t LoRaMacAppSKey[] =
 * {
 *      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 */

/*!
 * End-device address
 */
static uint32_t LoRaMacDevAddr = LORAMACDEVADDR;

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

uint16_t BufferSize = BUFFER_SIZE;

uint16_t UpLinkCounter;

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

static bool new_gateway_node_to_gateway_pc = false;
static bool node_bridge_to_node_gateway = false;

static size_t sizeDataFromBridgeNodeForPcGateway = 0;

static uint8_t dataFromBridgeNodeForPcGateway[ VCOM_BUFF_SIZE ]={0};
static uint8_t dataFromPcForNodeGateway[ VCOM_BUFF_SIZE ]={0};

//static uint8_t device_data[ VCOM_BUFF_SIZE/2 ]={'m','y','_','d','a','t','A'}; //size 7

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

// void debug_print_state(){
//     if (UartUsbIsUsbCableConnected()){
//         switch( State )
//         {
//             case RX:
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 UartUsbPutChar( &UartUsb, 'X' ); 
//                 break;

//             case TX:
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 UartUsbPutChar( &UartUsb, 'X' ); 
//                 break;

//             case TX_TIMEOUT:
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 UartUsbPutChar( &UartUsb, 'X' ); 
//                 UartUsbPutChar( &UartUsb, '_' ); 
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 UartUsbPutChar( &UartUsb, 'I' ); 
//                 UartUsbPutChar( &UartUsb, 'M' ); 
//                 UartUsbPutChar( &UartUsb, 'E' ); 
//                 UartUsbPutChar( &UartUsb, 'O' ); 
//                 UartUsbPutChar( &UartUsb, 'U' ); 
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 break;

//             case RX_TIMEOUT:
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 UartUsbPutChar( &UartUsb, 'X' ); 
//                 UartUsbPutChar( &UartUsb, '_' ); 
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 UartUsbPutChar( &UartUsb, 'I' ); 
//                 UartUsbPutChar( &UartUsb, 'M' ); 
//                 UartUsbPutChar( &UartUsb, 'E' ); 
//                 UartUsbPutChar( &UartUsb, 'O' ); 
//                 UartUsbPutChar( &UartUsb, 'U' ); 
//                 UartUsbPutChar( &UartUsb, 'T' ); 
//                 break;

//             case RX_ERROR:
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 UartUsbPutChar( &UartUsb, 'X' ); 
//                 UartUsbPutChar( &UartUsb, '_' );
//                 UartUsbPutChar( &UartUsb, 'E' ); 
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 UartUsbPutChar( &UartUsb, 'O' ); 
//                 UartUsbPutChar( &UartUsb, 'R' ); 
//                 break;

//             case LOWPOWER:
//                 UartUsbPutChar( &UartUsb, 'L' ); 
//                 break;

//             default:
//                 UartUsbPutChar( &UartUsb, 'D' ); 
//                 break;
//         }
//         UartUsbPutChar( &UartUsb, '\n' ); 
//         UartUsbPutChar( &UartUsb, '\r' ); 
//     }
// }

void PrepareFrameTx(uint8_t *MyBuffer, uint8_t LoRaMacTxPayloadLen)
{
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    uint16_t payload_device[VCOM_BUFF_SIZE]={0}; 
    uint8_t framePort = 1; // fPort;

    memset( LoRaBridgeToGatewayBuffer, 0 , LORAMAC_PHY_MAXPAYLOAD ); // clear the buffer
    memcpy( payload_device, MyBuffer, LoRaMacTxPayloadLen );

    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = 0x40;//macHdr->Value;

    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = 0x00;// fCtrl->Value

    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

    LoRaBridgeToGatewayBuffer[pktHeaderLen++] = framePort;

    LoRaMacPayloadEncrypt( (uint8_t* ) payload_device, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaBridgeToGatewayBuffer[pktHeaderLen] );

    LoRaBridgeToGatewayBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

    LoRaMacComputeMic( LoRaBridgeToGatewayBuffer, LoRaBridgeToGatewayBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

    LoRaBridgeToGatewayBuffer[LoRaBridgeToGatewayBufferPktLen + 0] = mic & 0xFF;
    LoRaBridgeToGatewayBuffer[LoRaBridgeToGatewayBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
    LoRaBridgeToGatewayBuffer[LoRaBridgeToGatewayBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
    LoRaBridgeToGatewayBuffer[LoRaBridgeToGatewayBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

    LoRaBridgeToGatewayBufferPktLen += LORAMAC_MFR_LEN;

    UpLinkCounter++; 

    //------------------------ DEBUG -----------------------------------//
    // memset( LoRaBridgeToGatewayBuffer, 0 , LORAMAC_PHY_MAXPAYLOAD ); // clear the buffer
    // LoRaBridgeToGatewayBufferPktLen = LoRaMacTxPayloadLen;
    // memcpy( LoRaBridgeToGatewayBuffer, MyBuffer, LoRaMacTxPayloadLen );
    // UartUsbPutBuffer( &UartUsb , (uint8_t*)LoRaBridgeToGatewayBuffer , LoRaMacTxPayloadLen );
    //------------------------ DEBUG -----------------------------------//
}

int serial(uint8_t *vcomBufferForPC, uint8_t len_buffer_device){;

    uint8_t vcomBufferPcFromPc[VCOM_BUFF_SIZE ]={0};
    uint32_t iTimeOut;
    uint8_t pcCpmt=0;
    uint8_t readVar[5];
    uint8_t test_get=0;
    size_t olen = 0;

    if (UartUsbIsUsbCableConnected()){
        if( new_gateway_node_to_gateway_pc == true ){ 
            UartUsbPutChar( &UartUsb, MSG_YES );
        }
        else{ 
            UartUsbPutChar( &UartUsb, MSG_NO ); 
        }

        iTimeOut=1;
        test_get = 2;
        while( test_get != 0 ) 
        {
            test_get = UartUsbGetChar( &UartUsb, readVar );
            
            if( test_get == 0 && ( readVar[0] == MSG_NO || readVar[0] == MSG_YES )){ // Pc responded

                if( new_gateway_node_to_gateway_pc == true ){  
                    UartUsbPutBuffer( &UartUsb , (uint8_t*)vcomBufferForPC , len_buffer_device );
                    new_gateway_node_to_gateway_pc = false;
                }

                if ( readVar[0] == MSG_YES ){ // Pc have device_data to transmit
                    pcCpmt = 0;
                    while( readVar[0]!= MSG_END ){
                        while(UartUsbGetChar( &UartUsb, readVar ) != 0);
                        vcomBufferPcFromPc[pcCpmt++] = readVar[0];
                    }
                    pcCpmt = pcCpmt - 1;
                    mbedtls_base64_decode(dataFromPcForNodeGateway,sizeof(dataFromPcForNodeGateway), &olen , vcomBufferPcFromPc , pcCpmt);
                    PrepareFrameTx(dataFromPcForNodeGateway,olen);
                    node_bridge_to_node_gateway = true;
                }
                break; // done 
            }
            iTimeOut++;
            if( iTimeOut % SERIAL_TIMEOUT_VALUE == 0 )
                break; // try again to contact PC
        }
    }
    return 0;
}

void discussSerial(){
    size_t olen = 0;
    uint8_t vcomBufferForPC[ VCOM_BUFF_SIZE ]={0};

    if(new_gateway_node_to_gateway_pc == true){
        mbedtls_base64_encode(vcomBufferForPC, sizeof(vcomBufferForPC), &olen , dataFromBridgeNodeForPcGateway, sizeDataFromBridgeNodeForPcGateway);
        vcomBufferForPC[olen++] = MSG_END;      
    }
    serial( vcomBufferForPC, olen);
}

/**
 * Main application entry point.
 */
int main( void )
{
    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    RadioEvents.TxDone    = OnTxDone;
    RadioEvents.RxDone    = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError   = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );

#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

    Radio.Rx( RX_TIMEOUT_VALUE );
    DelayMs( 500 );

    while( 1 )
    {
        //debug_print_state();
        switch( State )
        {
        case RX:
                if( BufferSize > 0 )
        {
                    DelayMs( 500 );// debug
                    new_gateway_node_to_gateway_pc = true;
                    discussSerial();        
                    Radio.Rx( RX_TIMEOUT_VALUE );
                    State = LOWPOWER;
        }
            break;
        case TX:
                Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
                Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
            break;
        case TX_TIMEOUT:
                node_bridge_to_node_gateway = true;
                Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            discussSerial();        
            if(node_bridge_to_node_gateway == true)
            {
                node_bridge_to_node_gateway = false;
                Radio.Send( LoRaBridgeToGatewayBuffer, LoRaBridgeToGatewayBufferPktLen );
            }
            // Set low power
            break;
        }
        TimerLowPowerHandler( );
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );

    sizeDataFromBridgeNodeForPcGateway = size;
    memcpy( dataFromBridgeNodeForPcGateway, payload, sizeDataFromBridgeNodeForPcGateway );

    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}
