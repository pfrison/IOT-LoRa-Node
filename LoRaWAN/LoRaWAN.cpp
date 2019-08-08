#include "LoRaWAN.h"
#include "mbed.h"

// ----- CONSTRUCTORS -----

/**
 * LoRaWAN initialisation.
 */
LoRaWAN::LoRaWAN(int TxTimeout, int RxTimeout, int frequency, bool enableAsync)
        : _thread(&LoRaWAN::threadStarter, this) /*<-- deprecated because it doesn't handle errors*/ {
    _txTimeout = TxTimeout;
    _rxTimeout = RxTimeout;
    _frequency = frequency;
    _enableAsync = enableAsync;
}

// ----- PRIVATE METHODS -----

/**
 * Thread for async transmission
 */
bool _transmissionInProgress = false;
int _transmissionCode;
char *_transmissionPayload;
int _transmissionPayloadMaxLength;
void LoRaWAN::threadStarter(void const *p){
    LoRaWAN *instance = (LoRaWAN *) p;
    if(!instance->_enableAsync)
        return;
    instance->threadWorker();
}
void LoRaWAN::threadWorker(){
    while(1){
        // wait for a transmisson to start
        while(!_transmissionInProgress)
            sleep();
        // wait for the transmission to complete
        while(_transmissionInProgress)
            sleep();
        
        // radio sleep
        _radio->Sleep();
        
        // send results
        if(_sendCallback != NULL)
            _sendCallback(_transmissionCode);
        else if(_recieveCallback != NULL)
            _recieveCallback(_transmissionCode);
    }
}

#ifdef TARGET_DISCO_L072CZ_LRWAN1
    void OnTxDone(void *radio, void *userThisPtr, void *userData){
        _transmissionCode = LORA_SUCCESS;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnTxTimeout(void *radio, void *userThisPtr, void *userData){
        _transmissionCode = LORA_WARN_TX_TIMEOUT;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnRxDone(void *radio, void *userThisPtr, void *userData, uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr){
        _transmissionCode = LORA_SUCCESS;
        for(int i=0; i<_transmissionPayloadMaxLength; i++)
            _transmissionPayload[i] = (char) payload[i];
        _transmissionPayload[_transmissionPayloadMaxLength - 1] = '\0';
        _transmissionInProgress = false;
    }
    void OnRxTimeout(void *radio, void *userThisPtr, void *userData){
        _transmissionCode = LORA_WARN_RX_TIMEOUT;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnRxError(void *radio, void *userThisPtr, void *userData){
        _transmissionCode = LORA_ERR_RX_ERROR;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
#elif TARGET_NUCLEO_L152RE
    void OnTxDone(void){
        _transmissionCode = LORA_SUCCESS;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnTxTimeout(void){
        _transmissionCode = LORA_WARN_TX_TIMEOUT;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr){
        _transmissionCode = LORA_SUCCESS;
        for(int i=0; i<_transmissionPayloadMaxLength; i++)
            _transmissionPayload[i] = (char) payload[i];
        _transmissionPayload[_transmissionPayloadMaxLength - 1] = '\0';
        _transmissionInProgress = false;
    }
    void OnRxTimeout(void){
        _transmissionCode = LORA_WARN_RX_TIMEOUT;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
    void OnRxError(void){
        _transmissionCode = LORA_ERR_RX_ERROR;
        _transmissionPayload = NULL;
        _transmissionInProgress = false;
    }
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

// ----- PUBLIC METHODS -----

/**
 * Initialisation : configure SX1276Generic radio and events
 */
int LoRaWAN::init(){
    // radio init
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    _radio = new SX1276Generic(NULL, RFM95_SX1276,
            LORA_SPI_MOSI, LORA_SPI_MISO, LORA_SPI_SCLK, LORA_CS, LORA_RESET,
            LORA_DIO0, LORA_DIO1, LORA_DIO2, LORA_DIO3, LORA_DIO4, LORA_DIO5);
#elif TARGET_NUCLEO_L152RE
    _radio = new SX1276MB1xAS(NULL);
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif
    
    _radioEvents.TxDone = OnTxDone;
    _radioEvents.RxDone = OnRxDone;
    _radioEvents.RxError = OnRxError;
    _radioEvents.TxTimeout = OnTxTimeout;
    _radioEvents.RxTimeout = OnRxTimeout;
    
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    bool init_ok = _radio->Init(&_radioEvents);
    if (!init_ok)
        return LORA_ERR_RADIO_INIT;
#elif TARGET_NUCLEO_L152RE
    _radio->Init(&_radioEvents);
    if(_radio->Read(REG_VERSION) == 0x00)
        return LORA_ERR_RADIO_INIT;
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

    // set frequency
    _radio->SetChannel(_frequency);
    
    // RxTx config
    _radio->SetTxConfig(MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
            LORA_SPREADING_FACTOR, LORA_CODINGRATE,
            LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
            LORA_IQ_INVERSION_ON, _txTimeout);
    _radio->SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
            LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
            LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0,
            LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
            LORA_IQ_INVERSION_ON, true);
    
    return LORA_SUCCESS;
}

/**
 * Send data with LoRaWAN. This function is not blocking
 */
int LoRaWAN::sendAsync(char *payload, int length, void (*sendCallback)(int)){
    if(_transmissionInProgress)
        return LORA_ERR_RADIO_BUSSY;
    if(!_enableAsync)
        return LORA_ERR_ASYNC_DISABLED;
        
    _sendCallback = sendCallback;
    _recieveCallback = NULL;
    _transmissionInProgress = true;
    
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    _radio->Send(payload, length);
#elif TARGET_NUCLEO_L152RE // char * to uint8_t * convertion
    uint8_t intpayload[length];
    for(int i=0; i<length; i++)
        intpayload[i] = (uint8_t) payload[i];
    _radio->Send(intpayload, length);
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

    return LORA_SUCCESS;
}

/**
 * Send data with LoRaWAN. This function is not blocking
 */
int LoRaWAN::receiveAsync(char *res, int maxLength, void (*recieveCallback)(int)){
    if(_transmissionInProgress)
        return LORA_ERR_RADIO_BUSSY;
    if(!_enableAsync)
        return LORA_ERR_ASYNC_DISABLED;
    
    _sendCallback = NULL;
    _recieveCallback = recieveCallback;
    _transmissionPayload = res;
    _transmissionPayloadMaxLength = maxLength;
    _transmissionInProgress = true;
    
    _radio->Rx(_rxTimeout);
    
    return LORA_SUCCESS;
}

/**
 * Send data with LoRaWAN. This function is blocking
 */
int LoRaWAN::sendSync(char *payload, int length){
    if(_transmissionInProgress)
        return LORA_ERR_RADIO_BUSSY;
    _transmissionInProgress = true;
    
#ifdef TARGET_DISCO_L072CZ_LRWAN1
    _radio->Send(payload, length);
#elif TARGET_NUCLEO_L152RE // char * to uint8_t * convertion
    uint8_t intpayload[length];
    for(int i=0; i<length; i++)
        intpayload[i] = (uint8_t) payload[i];
    _radio->Send(intpayload, length);
#else
    #error Cette librairie est uniquement compatible avec la DISCO_L072CZ_LRWAN1 ou la NUCLEO_L152RE
#endif

    while(_transmissionInProgress)
        sleep();
        
    _radio->Sleep();

    return _transmissionCode;
}

/**
 * Send data with LoRaWAN. This function is blocking
 */
int LoRaWAN::receiveSync(char *payload, int maxLength){
    if(_transmissionInProgress)
        return LORA_ERR_RADIO_BUSSY;
    _transmissionInProgress = true;
    
    _transmissionPayload = payload;
    _transmissionPayloadMaxLength = maxLength;
    
    _radio->Rx(_rxTimeout);
    
    while(_transmissionInProgress)
        sleep();
    
    _radio->Sleep();
    
    return _transmissionCode;
}