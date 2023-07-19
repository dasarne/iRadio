/**
 * @file iRadioEncoder.hpp
 * @author Dieter Zumkehr, Arne v.Irmer (Dieter.Zumkehr @ fh-Dortmund.de, Arne.vonIrmer @ tu-dortmund.de)
 * @brief Kapselt die Ansteuerung des Encoders
 * @version 0.1
 * @date 2023-04-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef IRADIOENCODER_HPP_
#define IRADIOENCODER_HPP_

#include <Arduino.h>

/**
 * @brief Mit welchem Encoder-Status hat der Nutzer das Display 
 */
enum EncoderState
{
    nothing,
    longPress,
    shortPress,
    rotation
};

class RadioEncoder
{
private:
    ESP32Encoder encoder;
public:
    int encoderPosition;
    
    /**
     * @brief Initialisieren der Encoder Hardware
     */
    void init();

    /**
     * @brief Ließt die aktuelle Position des Encoders aus
     * 
     * @return EncoderState Hat sich etwas am Encoder getan?
     */
    EncoderState refreshPosition();

    /**
     * @brief Setzt die aktuelle Position des Encoders
     * 
     * @param pos 
     */
    void setEncoder(u_int8_t pos);
};

#endif // IRADIOENCODER_HPP_