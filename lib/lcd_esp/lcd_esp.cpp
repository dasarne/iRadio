#include "lcd_esp.h"

// Constructor for SPI data transfer object
HD44780::HD44780(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin)
    : _pdto(new SPIOps(my_ss, my_miso, my_mosi, my_sck)), RESET_PIN(my_reset_pin)
{
    // Init reset pin if defined
    if (RESET_PIN != 0)
        pinMode(RESET_PIN, OUTPUT);

    // First two have no parameter bits, use remaining to set standard config.
    _instr0_clear_display = 0x01;
    _instr0_return_home = 0x02;
    // ID_1: moving increment, S_0: display shift disabled
    _instr0_entry_mode_set = 0x04 | INSTR0_EMS_ID;
    // D_1: display on, C_1: cursor on, B_0: blink off
    _instr0_display_onoff_control = 0x08 | INSTR0_DOC_D | INSTR0_DOC_C;
    // SC_0: cursor shift, RL: shift to right
    _instr0_cursor_display_shift = 0x10 | INSTR0_CDS_RL;
    // DL_1: 8 bit interface, N_1: 2-lines, F_0: 5 x 8 dots
    _instr0_function_set = 0x20 | INSTR0_FS_DL | INSTR0_FS_N;
}

void HD44780::clear()
{
    _pdto->ir_write(_instr0_clear_display);
}

void HD44780::setPos(uint8_t x, uint8_t y)
{
    switch (_rows)
    {
    case R4:
        y = (y < R4) ? y : y % R4;
        x = (x < (80 / R4)) ? x : x % (80 / R4);
        set_ddram_address(y * 0x20 + x);
        break;
    case R2:
        y = (y < R2) ? y : y % R2;
        x = (x < (80 / R2)) ? x : x % (80 / R2);
        set_ddram_address(y * 0x40 + x);
        break;
    case R1:
        x = (x < 80) ? x : x % 80;
        set_ddram_address(x);
        break;
    }
}

void HD44780::create(uint8_t location, const uint8_t charmap[8])
{
    const uint8_t LAST_ADDRESS = _pdto->address_read();
	location &= 0x7;
    set_cgram_address(location << 3);
	_pdto->dr_write(charmap, 8);
    set_ddram_address(LAST_ADDRESS);
}

void HD44780::on(bool en)
{
    if (en)
        _instr0_display_onoff_control |= INSTR0_DOC_D;
    else
        _instr0_display_onoff_control &= static_cast<uint8_t>(~INSTR0_DOC_D) & 0x0f;
    _pdto->ir_write(_instr0_display_onoff_control);
}

void HD44780::characterBlink(bool en)
{
    if (en)
        _instr0_display_onoff_control |= INSTR0_DOC_B;
    else
        _instr0_display_onoff_control &= ~INSTR0_DOC_B;
    _pdto->ir_write(_instr0_display_onoff_control);
}

void HD44780::cursorOn(bool en)
{
    if (en)
        _instr0_display_onoff_control |= INSTR0_DOC_C;
    else
        _instr0_display_onoff_control &= ~INSTR0_DOC_C;
    _pdto->ir_write(_instr0_display_onoff_control);
}

void HD44780::shiftCursor(bool en)
{
    if (en)
        _instr0_cursor_display_shift &= ~INSTR0_CDS_SC;
    else
        _instr0_cursor_display_shift |= INSTR0_CDS_SC;
    _pdto->ir_write(_instr0_cursor_display_shift);
}

void HD44780::leftRight(bool en)
{
    if (en)
        _instr0_cursor_display_shift |= INSTR0_CDS_RL;
    else
        _instr0_cursor_display_shift &= ~INSTR0_CDS_RL;
    _pdto->ir_write(_instr0_cursor_display_shift);
}

void HD44780::init()
{
    delay(42); // >40ms when Vcc=2.7 Volt, >15ms when Vcc=4.5 Volt
    _pdto->ir_write(_instr0_function_set);
    delay(5);
    _pdto->ir_write(_instr0_function_set);
    delay(1);
    _pdto->ir_write(_instr0_function_set);
    //
    _pdto->ir_write(_instr0_display_onoff_control);
    _pdto->ir_write(_instr0_clear_display);
    _pdto->ir_write(_instr0_entry_mode_set);
}

// Initializing by Hardware RESET input pin if available
void HD44780::reset()
{
    if (RESET_PIN == 0)
        return;
    delay(50);
    digitalWrite(RESET_PIN, LOW);
    delay(4);
    digitalWrite(RESET_PIN, HIGH);
    delay(20);
}

void HD44780::set_cgram_address(uint8_t address)
{
    address &= 0x3f;
    _pdto->ir_write(SET_CGRAM | address);
}

void HD44780::set_ddram_address(uint8_t address)
{
    address &= 0x7f;
    _pdto->ir_write(SET_DDRAM | address);
}

size_t HD44780::write(uint8_t value)
{
	return _pdto->dr_write(value) ? 1 : 0;
}

size_t HD44780::write(const uint8_t *buffer, size_t size)
{
    return _pdto->dr_write(buffer, size) ? size : 0;
}

// Constructor for RW1073/SSD1803 display driver using SPI serial bus
RW1073::RW1073(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin)
    : HD44780(my_ss, my_miso, my_mosi, my_sck, my_reset_pin)
{
    // RE is clear after POR
    _re_set = false;
    // PD_0: power on
    _instr1_power_down_mode = 0x02;
    // TODO: What's the matter with that?
    _instr1_entry_mode_set = 0x06;
    // FW_0: 5-dot font, BW_0: black white inverting off, NW_1: 4-line display
    _instr1_extended_function_set = 0x08 | INSTR1_EFS_NW;
    _instr1_shift_enable = 0x10;
    // Beware: Settings of Function Set differ from HD44780.
    // DL_1: 8 bit interface, N_1: 2-lines, DH_1: display enable, REV_0: normal
    _instr0_function_set = 0x20 | INSTR0_FS_DL | INSTR0_FS_N | INSTR0_FS_DH;
    // DL_1: 8 bit interface, N_1: 2-lines, RE_1: set RE, BE_0: blink disable
    _instr1_function_set = 0x20 | INSTR0_FS_DL | INSTR0_FS_N | FS_RE;
}

void RW1073::begin(row_t rows, col_t cols)
{
    reset();
    init(rows, cols);
}

void RW1073::setBlinking(uint8_t loc, uint8_t blink, uint8_t pattern)
{
    loc &= 0x0f;
    blink &= 0x03;
    pattern &= 0x1f; // Always 5-dot font. TODO: character width dependend.
    set_segram_address(loc);
    // _pdto->dr_write((blink << 6) | pattern);
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
    //
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
    _pdto->dr_write(0b10011111);
}

void RW1073::iconBlink(bool en)
{
    if (en)
        _instr1_function_set |= INSTR1_FS_BE;
    else
        _instr1_function_set &= ~INSTR1_FS_BE;
    _pdto->ir_write(_instr1_function_set); _re_set = true;
    _pdto->ir_write(_instr0_function_set); _re_set = false;
}

void RW1073::testingShowIcon()
{
    uint8_t last_address = _pdto->address_read();
    _pdto->ir_write(_instr1_function_set | INSTR1_FS_BE); _re_set = true;
    _pdto->ir_write(SET_SEGRAM | 2);
    _pdto->dr_write(0b10010000);
    _pdto->ir_write(_instr0_function_set); _re_set = false;
    _pdto->ir_write(SET_DDRAM | last_address);
}

void RW1073::powerDown(bool en)
{
    if (en)
        _instr1_power_down_mode |= INSTR1_PDM_PD;
    else
        _instr1_power_down_mode &= ~INSTR1_PDM_PD;
    _pdto->ir_write(_instr1_function_set); _re_set = true;
    _pdto->ir_write(_instr1_power_down_mode);
    _pdto->ir_write(_instr0_function_set); _re_set = false;
}

void RW1073::init(row_t rows, col_t cols)
{
    // Tune the prepared row setting for hardware
    switch (rows)
    {
    case R4:
        ; // The constructor has done 4-line setup by default
        break;
    case R2:
        // NW_0: Use default N_1 bit from "Function Set" for 2-line
        _instr1_extended_function_set &= ~INSTR1_EFS_NW;
        break;
    case R1:
        _instr1_extended_function_set &= ~INSTR1_EFS_NW;
        // N_0: Make this a 1-line setting
        _instr0_function_set &= ~INSTR0_FS_N;
        _instr1_function_set &= ~INSTR0_FS_N;
        break;
    }
    _rows = rows;
    _cols = cols;
    // Dump setting to hardware
    delay(64);
    _pdto->ir_write(_instr0_function_set); _re_set = false;
    _pdto->ir_write(_instr0_entry_mode_set);
    _pdto->ir_write(_instr1_function_set); _re_set = true;
    _pdto->ir_write(_instr1_extended_function_set);
    _pdto->ir_write(SET_SEGRAM | 0x00);
    for (uint8_t i=0; i<16; i++)
        _pdto->dr_write(0x00);
    _pdto->ir_write(_instr0_function_set); _re_set = false;
    _pdto->ir_write(_instr0_display_onoff_control);
    _pdto->ir_write(_instr0_clear_display);
    while (_pdto->has_bf_set())
        ;
}

void RW1073::set_segram_address(uint8_t address)
{
    address &= 0x0f;
    _pdto->ir_write(_instr1_function_set); _re_set = true;
    _pdto->ir_write(SET_SEGRAM | address);
    _pdto->ir_write(_instr0_function_set); _re_set = false;
}

// Constructor for SSD1803A display driver using SPI serial bus
SSD1803A::SSD1803A(uint8_t my_ss, uint8_t my_miso, uint8_t my_mosi, uint8_t my_sck, uint8_t my_reset_pin)
    : RW1073(my_ss, my_miso, my_mosi, my_sck, my_reset_pin)
{
    ;
}

void SSD1803A::begin(row_t rows, col_t cols)
{
    reset();
    init(rows, cols);
}

void SSD1803A::init(row_t rows, col_t cols)
{
    _pdto->ir_write(0x3a);
    _pdto->ir_write(0x09);
    _pdto->ir_write(0x06);
    _pdto->ir_write(0x1e);
    _pdto->ir_write(0x39);
    _pdto->ir_write(0x1b);
    _pdto->ir_write(0x6c);
    _pdto->ir_write(0x56);
    _pdto->ir_write(0x7a);
    _pdto->ir_write(0x38);
    _pdto->ir_write(0x0f);
}
