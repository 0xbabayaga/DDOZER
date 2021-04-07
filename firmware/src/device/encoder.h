#ifndef _ENCODER_H
#define _ENCODER_H

#define ENC_QUEUE_SIZE          8
#define ENC_MULTI_STEP2_LEVEL   10
#define ENC_MULTI_STEP10_LEVEL  20

typedef enum _EncKeys
{
    eKeyNone = 0,
    eKeyFwd = 1,
    eKeyFwd2 = 2,
    eKeyFwd10 = 10,
    eKeyBwd = -1,
    eKeyBwd2 = -2,
    eKeyBwd10 = -10,
    eKeyPress = 30,
    eKeyLongPress = 40
}   eEncKey;

void    ENC_Init(void);
eEncKey ENC_getPressedKey(void);

#endif