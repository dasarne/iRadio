/**
 * @file regio.cpp
 * @author Michael Hoffmann (michael.hoffmann@fh-dortmund.de)
 * @brief Implements the SSD1803 I2C and SPI bus protocols
 * @version 0.1
 * @date 2023-01-25
 *
 * @copyright Copyright (c) 2023
 * 
 * (Citations refer to enclosed [SSD1803 specs file](../ssd1803a_2_0.pdf).)
 */
#include <regio.h>
#include <cstring>

/**
 * @brief Loops until BF is clear
 * 
 * @return true BF is clear
 * @return false Time-out (>25ms) occurred
 */
bool RegIO::loop_until_ready() const
{
    const unsigned long time_now = millis();
    bool is_ready = false;
    while (millis() - time_now <= TIMEOUT_MS)
    {
        if (has_bf_clear())
        {
            is_ready = true;
            break;
        }
    }
    return is_ready;
}

SPIOps::SPIOps(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck)
    : vspi(new SPIClass(VSPI))
{
    vspi->begin(my_sck, my_miso, my_mosi, my_ss);
    pinMode(vspi->pinSS(), OUTPUT);
    digitalWrite(vspi->pinSS(), HIGH);
    delay(1); // MISO line needs some time to settle
}

bool SPIOps::ir_write(const void *comd, uint8_t size) const
{
    // Checking for value errors
    if ((comd == nullptr) || (size == 0) || (size >= 80))
        return false;
    // Q.v. p. 27 f.--will be reversed by hardware
    constexpr uint8_t INSTR_WRT = 0b00011111; // 0 RS RW ... (1Fh)
    // Sending commands on bus
    constexpr size_t BUFSIZE = 3;
    uint8_t spi_buf[BUFSIZE];
    for (uint_fast8_t i = 0; i < size; i++)
    {
        // Preparing SPI command word
        const uint8_t *cmd = static_cast<const uint8_t *>(comd);
        spi_buf[0] = INSTR_WRT;
        spi_buf[1] = (cmd[i] & 0x0f);
        spi_buf[2] = (cmd[i] & 0xf0) >> 4;
        // Drop command on bus
        if (loop_until_ready() == false)
            return false;
        vspi->beginTransaction(SPISettings(VSPI_CLOCK, VSPI_BIT_ORDER, VSPI_DATA_MODE));
        digitalWrite(vspi->pinSS(), LOW);
        vspi->transfer(spi_buf, 3);
        digitalWrite(vspi->pinSS(), HIGH);
        vspi->endTransaction();
    }
    return true;
}

uint8_t SPIOps::bfa_read() const
{
    // Q.v. p. 27 f.--will be reversed by hardware
    constexpr uint8_t BUSY_RD = 0b00111111; // 0 RS RW ... (3Fh)
    const uint16_t TX = BUSY_RD;
    // Reading busy register from bus
    vspi->beginTransaction(SPISettings(VSPI_CLOCK, VSPI_BIT_ORDER, VSPI_DATA_MODE));
    digitalWrite(vspi->pinSS(), LOW);
    uint16_t rx = vspi->transfer16(TX);
    digitalWrite(vspi->pinSS(), HIGH);
    vspi->endTransaction();
    return rx >> 8;
}

bool SPIOps::dr_write(const void *src, uint8_t size) const
{
    // Checking for value errors
    if ((src == nullptr) || (size == 0) || (size >= 80))
        return false;
    // Q.v. p. 27 f.--will be reversed by hardware
    constexpr uint8_t DATA_WRT = 0b01011111; // 0 RS RW ... (5Fh)
    // Sending data on bus
    constexpr size_t BUFSIZE = 3;
    uint8_t spi_buf[BUFSIZE];
    for (uint_fast8_t i = 0; i < size; i++)
    {
        // Preparing SPI command word
        const uint8_t *data = static_cast<const uint8_t *>(src);
        spi_buf[0] = DATA_WRT;
        spi_buf[1] = (data[i] & 0x0f);
        spi_buf[2] = (data[i] & 0xf0) >> 4;
        // Drop command on bus
        if (loop_until_ready() == false)
            return false;
        vspi->beginTransaction(SPISettings(VSPI_CLOCK, VSPI_BIT_ORDER, VSPI_DATA_MODE));
        digitalWrite(vspi->pinSS(), LOW);
        vspi->transfer(spi_buf, 3);
        digitalWrite(vspi->pinSS(), HIGH);
        vspi->endTransaction();
    }
    return true;
}

// TODO: Test me.
bool SPIOps::dr_read(void *dst, uint8_t size) const
{
    // Checking for value errors
    if ((dst == nullptr) || (size == 0) || (size >= 80))
        return false;
    // Q.v. p. 27 f.--will be reversed by hardware
    constexpr uint8_t DATA_RD = 0b01111111; // 0 RS RW ...
    // Reading data from bus
    uint8_t *data = static_cast<uint8_t *>(dst);
    vspi->beginTransaction(SPISettings(VSPI_CLOCK, VSPI_BIT_ORDER, VSPI_DATA_MODE));
    digitalWrite(vspi->pinSS(), LOW);
    vspi->transfer(DATA_RD);
    do {
        uint_fast8_t i = 0;
        while (true) {
            delayMicroseconds(EXECUTION_DELAY);
            data[i++] = vspi->transfer(0x00);
            if (i >= size)
                break;
        }
    } while (false);
    digitalWrite(vspi->pinSS(), HIGH);
    vspi->endTransaction();
    return true;
}
