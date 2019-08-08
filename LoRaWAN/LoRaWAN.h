#ifndef LORAWAN_h
#define LORAWAN_h

#ifdef TARGET_DISCO_L072CZ_LRWAN1
    #include "sx1276-mbed-hal.h"
#elif TARGET_NUCLEO_L152RE
    #include "sx1276-hal.h"
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

// Error codes
#define LORA_SUCCESS                0
#define LORA_WARN_TX_TIMEOUT        1 // Tx timeout
#define LORA_WARN_RX_TIMEOUT        2 // Rx timeout
#define LORA_ERR_RX_ERROR           3 // Rx error
#define LORA_ERR_RADIO_INIT         4 // Radio initialisation error
#define LORA_ERR_RADIO_BUSSY        5 // Radio is transmitting something
#define LORA_ERR_ASYNC_DISABLED     6 // Async transmissions disabled

// Configs
#define LORA_DEFAULT_TX_TIMEOUT     2000 //ms
#define LORA_DEFAULT_RX_TIMEOUT     2000 //ms
#define LOAR_EUROPE_FREQUENCY       868000000 // 868 MHz
#define LOAR_US_FREQUENCY           915000000 // 915 MHz
#define LOAR_VIETNAM_FREQUENCY      868000000 // 868 MHz

// LoRaWAN default config
#define LORA_TX_OUTPUT_POWER        2 // dBm
#define LORA_PREAMBLE_LENGTH        8
#define LORA_SYMBOL_TIMEOUT         5
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_FHSS_ENABLED           false  
#define LORA_NB_SYMB_HOP            4     
#define LORA_IQ_INVERSION_ON        false
#define LORA_CRC_ENABLED            true
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    #define LORA_BANDWIDTH              125000 // 125 kHz, 250 kHz, 500 kHz
    #define LORA_SPREADING_FACTOR       LORA_SF12 // SF7..SF12
    #define LORA_CODINGRATE             LORA_ERROR_CODING_RATE_4_8 // 4/5, 4/6, 4/7, 4/8
#elif TARGET_NUCLEO_L152RE
    #define LORA_BANDWIDTH              0 // 0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved
    #define LORA_SPREADING_FACTOR       12 // SF7..SF12
    #define LORA_CODINGRATE             4 // 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

// LoRaWAN PinMap
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    #define LORA_SPI_MOSI               PA_7
    #define LORA_SPI_MISO               PA_6
    #define LORA_SPI_SCLK               PB_3
    #define LORA_CS                     PA_15
    #define LORA_RESET                  PC_0
    #define LORA_DIO0                   PB_4
    #define LORA_DIO1                   PB_1
    #define LORA_DIO2                   PB_0
    #define LORA_DIO3                   PB_0 // PC_13 ??
    #define LORA_DIO4                   PA_5
    #define LORA_DIO5                   PA_4
    #define LORA_ANT_RX                 PA_1
    #define LORA_ANT_TX                 PC_2
    #define LORA_ANT_BOOST              PC_1
    #define LORA_TCXO                   PA_12 // 32 MHz
#endif

class LoRaWAN {
    static void threadStarter(void const *p);
    void threadWorker();
    Thread _thread;
    
    private:
        int _rxTimeout;
        int _txTimeout;
        int _frequency;
        bool _enableAsync;
        RadioEvents_t _radioEvents;
        void (*_sendCallback)(int);
        void (*_recieveCallback)(int);
#ifdef TARGET_DISCO_L072CZ_LRWAN1
        SX1276Generic *_radio;
#elif TARGET_NUCLEO_L152RE
        SX1276MB1xAS *_radio;
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif
        
    public:
        LoRaWAN(int TxTimeout, int RxTimeout, int frequency, bool enableAsync);
        
        int init();
        int sendAsync(char *payload, int length, void (*sendCallback)(int));
        int receiveAsync(char *res, int mexLength, void (*recieveCallback)(int));
        int sendSync(char *payload, int length);
        int receiveSync(char *payload, int maxLength);
};

#endif