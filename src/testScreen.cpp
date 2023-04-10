#include <iRadioDisplay.hpp>
#include <log.h>

// Logging-Tag für Easy-Logger
static const char *TAG = "STREAM-T";

void TestScreen::showScreen()
{
    String head = " 4567890123456789";

    String chars[16];
    for (char c = 0; c < 255; c++)
    {
        chars[c / 20] += (char)c;
    }
    
    for(int i=0;i<16;i++)
    {
        chars[i]+=String("                    ").substring(0,20-chars[i].length());
    }

    u_int8_t pos = 0;

    iEncoder.setEncoder(0);
    while (true)
    {

        pos = iEncoder.encoderPosition;
        char buf[] = "000";
        sprintf(buf, "%03d", pos * 3 * 20);
        lcd.setPos(0, 0);
        lcd.print(String(buf) + head);
        lcd.setPos(0, 1);
        lcd.print(chars[pos * 3]);
        lcd.setPos(0, 2);
        lcd.print(chars[pos * 3 + 1]);
        lcd.setPos(0, 3);
        lcd.print(chars[pos * 3 + 2]);

        EncoderState state = iEncoder.refreshPosition();
        switch (state)
        {
        case rotation:

            break;
        case shortPress:
        case longPress:
            return;
            break;
        case nothing:
        default:
            break;
        }
    }
}