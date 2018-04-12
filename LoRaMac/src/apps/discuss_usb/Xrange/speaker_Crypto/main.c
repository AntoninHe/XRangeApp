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

#define printf(x) PRINTF(x)


#if defined( USE_BAND_868 )

#define RF_FREQUENCY                                867100000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             5        // dBm

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
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Length of the payload in LoRaMacBuffer
 */
static uint8_t LoRaMacTxPayloadLen = 0;

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

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here

uint8_t i_test=0;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

uint16_t UpLinkCounter;

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

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

//void PrepareFrameTx()
void SendFrameTx()
{
	uint8_t pktHeaderLen = 0;
	uint32_t mic = 0;
	//const void* payload = fBuffer;
	void* payload ;
	uint8_t framePort = 1; // fPort;

	//--------------------------------------------------------------//     
	uint16_t MyBuffer[2]={0x1234,0xABCD};

	payload = MyBuffer;
        LoRaMacTxPayloadLen=4; // buffer length
	//--------------------------------------------------------------//     

	LoRaMacBuffer[pktHeaderLen++] = 0x40;//macHdr->Value;

	LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
	LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
	LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
	LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

	LoRaMacBuffer[pktHeaderLen++] = 0x00;// fCtrl->Value

	LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
	LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

	LoRaMacBuffer[pktHeaderLen++] = framePort;

	LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );

	LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

	LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

	LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
	LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
	LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
	LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

	LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

	UpLinkCounter++;

	//  MHDR -----------------------------------------------------------------
        
            // MType : 
                // 010 Unconfirmed data up

            // RFU
                // 000

            // Major
		// 00 LoRaWan R1
                 
	// =0x40
	// [0]= 0x40;


	// FHDR ------------------------------------------------------------------

            // DevAddr :
                // device adress
                // 4 bytes
                // =0xXXXXXXXX

            // Fctrl :
                // ADR 
                    // 0 no data adaptation

                // ADRACK
                    // 0 no acknowledgement request

                // RFU in uplink
                    // 0

                // no acknowledgement
                    // 0 

                // FOptsLen
                    // 0x00 Size of MAC command
            // [4] = 0x00;

            // Fcnt :
            // frame counter
                // =0xII = i++

            // FOpts :
                // Frame option
                // void
                 
        // XXXX XXXX 0000 0000 // only DevAdress
        // 0xXXXXXXXXII00

	// Fport ----------------------------------------------------------------
            // 0x01 no mac
            // [5] = 0x01;

	// Payload ----------------------------------------------------------------
	// MIC ------------------------------------------------------------------
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
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

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
    printf("Start Speaker\n\r");															
    State = RX;
    while( 1 )
    {
        switch( State )
        {
        case RX:
                if( BufferSize > 0 )
		{
                    DelayMs( 500 );
                    printf("Sent SMILE \n\r");
                    SendFrameTx();
                    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );
		}
            break;
        case TX:
                printf("Finish Send \n");
                Radio.Rx( RX_TIMEOUT_VALUE );
            break;
        case RX_TIMEOUT:
                printf("RX TIMEOUT \n");
        case RX_ERROR:
                Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
                printf("RX ERROR \n");
            break;
        case TX_TIMEOUT:
                Radio.Rx( RX_TIMEOUT_VALUE );
                State = LOWPOWER;
                printf("TX TIMEOUT \n");
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }
        TimerLowPowerHandler( );
	State = RX;
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
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
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
