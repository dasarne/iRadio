/**
 * @file lcd_esp.h
 * @author Michael Hoffmann (michael.hoffmann@fh-dortmund.de)
 * @brief
 * @version 0.1
 * @date 2023-02-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <cstdbool>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <regio.h>
#include <Arduino.h>

class HD44780 : public Print {
public:
    HD44780(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin);
    virtual ~HD44780() { ; }
    /**
     * @brief Present column count of LCD display (width)
     *
     */
    typedef enum { C20 = 20, C16 = 16, C10 = 10 } col_t;

    /**
     * @brief Present row count of LCD display (height)
     *
     */
    typedef enum { R4 = 4, R2 = 2, R1 = 1 } row_t;

    /**
     * @brief Overwritten method from Arduino library Print class
     */
    virtual size_t write(uint8_t value) override;

    /**
     * @brief Overwritten method from Arduino library Print class
     */
    virtual size_t write(const uint8_t *buffer, size_t size) override;

    void clear();
    void cls() { clear(); }
    void setPos(uint8_t x, uint8_t y);
    void setXY(uint8_t x, uint8_t y) { setPos(x, y); }
    void locate(uint8_t row, uint8_t col) { setPos(col, row); }
    void create(uint8_t location, const uint8_t charmap[8]);
    void on(bool en = true);
    void off() { on(false); }
    void characterBlink(bool en);
    void homePos() { _pdto->ir_write(_instr0_return_home); }
    void cursorOn(bool en = true);
    void cursorOff() { cursorOn(false); }
    void shiftCursor(bool en = true);
    void shiftDisplay() { shiftCursor(false); }
    void leftRight(bool en = true);
    void rightLeft() { leftRight(false); }

protected:
    void init();
    void reset();
    void set_cgram_address(uint8_t address);
    void set_ddram_address(uint8_t address);
    static constexpr uint8_t SET_CGRAM = 0x40; // CGRAM instruction code
    static constexpr uint8_t SET_DDRAM = 0x80; // DDRAM instruction code
    // Data transfer object
    std::unique_ptr<RegIO> _pdto{};
    // LCD characteristic
    uint8_t _rows = 0;
    uint8_t _cols = 0;
    // Copy of internal registers of display controller (only level 0)
    uint8_t _instr0_clear_display;
    uint8_t _instr0_return_home;
    uint8_t _instr0_entry_mode_set;
    uint8_t _instr0_display_onoff_control;
    uint8_t _instr0_cursor_display_shift;
    uint8_t _instr0_function_set;
    // Corresponding bits of Instruction Level 0 / First controller generation
    static constexpr uint8_t INSTR0_EMS_ID = 0x02; // cursor increment/decrement
    static constexpr uint8_t INSTR0_EMS_S  = 0x01; // display shift
    static constexpr uint8_t INSTR0_DOC_D  = 0x04; // display on/off
    static constexpr uint8_t INSTR0_DOC_C  = 0x02; // cursor on/off
    static constexpr uint8_t INSTR0_DOC_B  = 0x01; // cursor blink on/off
    static constexpr uint8_t INSTR0_CDS_SC = 0x08; // display shift, cursor move
    static constexpr uint8_t INSTR0_CDS_RL = 0x04; // shift right, shift left
    static constexpr uint8_t INSTR0_FS_DL  = 0x10; // data length 8 bits, 4 bits
    static constexpr uint8_t INSTR0_FS_N   = 0x08; // number of rows 2, 1
    static constexpr uint8_t INSTR0_FS_F   = 0x04; // font 5x10, 5x8 dots

private:
    HD44780(const HD44780 &); // Disable default constructor
    void operator=(const HD44780 &); // Disable copy constructor
    const uint8_t RESET_PIN = 0;
};

class RW1073 : public HD44780 {
public:
    RW1073(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin = 0);
    virtual ~RW1073() { ; }
    void begin(row_t rows = R4, col_t cols = C20);
    void setBlinking(uint8_t loc, uint8_t blink, uint8_t pattern);
    void iconBlink(bool en);
    void testingShowIcon();
    void powerDown(bool en);

protected:
    void init(row_t rows, col_t cols);
    void set_segram_address(uint8_t address);
    static constexpr uint8_t SET_SEGRAM = 0x40; // SEGRAM instruction code
    // Copy of internal registers of display controller (extended to level 1)
    bool _re_set; // state of RE flag (extension register)
    uint8_t _instr1_power_down_mode;
    uint8_t _instr1_entry_mode_set;
    uint8_t _instr1_extended_function_set;
    uint8_t _instr1_shift_enable;
    uint8_t _instr1_function_set;
    // Corresponding bits of Instruction Level 1 (and extension of 0) / RE bit added
    static constexpr uint8_t INSTR1_PDM_PD  = 0x01; // power down set/clear
    static constexpr uint8_t INSTR1_EMS_BID = 0x01; // bidirection Seg60->1/Seg1->60
    static constexpr uint8_t INSTR1_EFS_FW  = 0x04; // font width 6-dot, 5-dot
    static constexpr uint8_t INSTR1_EFS_BW  = 0x02; // black/white inverted cursor
    static constexpr uint8_t INSTR1_EFS_NW  = 0x01; // 4-line display, 1/2-line display
    static constexpr uint8_t INSTR1_SE_DS4  = 0x08; // enable 4th line display shift
    static constexpr uint8_t INSTR1_SE_DS3  = 0x04; // enable 3th line display shift
    static constexpr uint8_t INSTR1_SE_DS2  = 0x02; // enable 2nd line display shift
    static constexpr uint8_t INSTR1_SE_DS1  = 0x01; // enable 1st line display shift
    static constexpr uint8_t FS_RE          = 0x04; // set/clear RE bit (extension reg)
    static constexpr uint8_t INSTR0_FS_DH   = 0x02; // display shift: per line/smooth dot scroll
    static constexpr uint8_t INSTR0_FS_REV  = 0x01; // reverse display
    static constexpr uint8_t INSTR1_FS_BE   = 0x02; // CGRAM/SEGRAM blink enable

private:
    RW1073(const RW1073 &); // Disable default constructor
    void operator=(const RW1073 &); // Disable copy constructor
};

class SSD1803A : public RW1073 {
public:
    SSD1803A(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin = 0);
    virtual ~SSD1803A() { ; }
    void begin(row_t rows = R4, col_t cols = C20);

protected:
    void init(row_t rows, col_t cols);
    // Copy of internal registers of display controller (extended to level 2)
    bool _is_set; // state of IS flag (special register)
    uint8_t _instr2_dhight_bias_ddshift;
    uint8_t _instr2_internal_osc_freq;
    uint8_t _instr2_scroll_enable;
    uint8_t _instr2_function_set;
    uint8_t _instr2_power_icon_control_contrast;
    uint8_t _instr2_follower_control;
    uint8_t _instr2_constrast_set;
    uint8_t _instr_scroll_quantity;

private:
    SSD1803A(const SSD1803A &); // Disable default constructor
    void operator=(const SSD1803A &); // Disable copy constructor
};
