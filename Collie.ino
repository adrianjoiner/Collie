#include <Keyboard.h>
#include <Keypad.h>

/*

Version 0.1
=========
- VIC20 / CBM64 keyboard simulation
- Sends all shifted / non shifted alphanumeric keys
- No control keys are sent to the USB port (except RETURN)
- Keys are only sent to usb when they are released

TODO:
- Send SHIFT & HOLD cursor keys
- Send control keys (GUI Key, Function keys, delete etc)
- Refactor


Acknowledements / References
============================
https://www.arduino.cc/en/Reference/MouseKeyboard
https://github.com/Nullkraft/Keypad
https://github.com/pimatic/homeduino/tree/master/libraries/Keypad/examples/MultiKey

NON STANDARD KEY MAPPINGS
=========================
Run/Stop = LEFT_ALT
Left arrow (top right) = ESCAPE
Up arrow = | (pipe)
Inst / DEL = Backspace

NON STANDARD SHIFTS
====================
Shift + Colon = [ Left square bracker
Shift + Semicolon = ] Right square bracket
Shift + up/down cursor = cursor up
Shift + left/right cursor = cursor right
Shift + insert/delete = toggle insert / delete mode
Shift + Up Arrow (pi) = ^
Shift + Minus = ~ tilde

ALT KEY MAPPINGS
================
ALT + / = \ Backslash
ALT + ' = ` Back single quote
AlT + ; = {
ALT + : = }

NOTES
=====
The teensy keyboard library tries to be too clever with character converstions
Whenever you print a char it translates it to the keyboard you have selected
in the options whever you want it to or not.
Only way to avoid actually getting the char you send to the port is to select the US Keyboad
*/

#define INPUT_PIN0 PIN_B0 // Teensy pin 0
#define INPUT_PIN1 PIN_B1 // pin 1
#define INPUT_PIN2 PIN_B2 // pin 1
#define INPUT_PIN3 PIN_B3 // pin 1
#define INPUT_PIN4 PIN_B7 // pin 1
#define INPUT_PIN5 PIN_D0 // pin 1
#define INPUT_PIN6 PIN_D1 // pin 1
#define INPUT_PIN7 PIN_D2 // pin 1

#define OUTPUT_PIN0 PIN_F1 // Pin 20
#define OUTPUT_PIN1 PIN_F4 // Pin 19
#define OUTPUT_PIN2 PIN_F5 // Pin 18
#define OUTPUT_PIN3 PIN_F6 // Pin 17
#define OUTPUT_PIN4 PIN_F7 // Pin 16
#define OUTPUT_PIN5 PIN_B6 // Pin 15
#define OUTPUT_PIN6 PIN_B5 // Pin 14
#define OUTPUT_PIN7 PIN_B4 // Pin 13

// CBM keyboard based on an 8 x 8 matrix
const byte ROWS = 8;
const byte COLS = 8;

// Control keys
const char _DELETE = 5;
const char _RIGHT_ARROW = 8;
const char _UP_ARROW = 7;
const char _DOWN_ARROW = 6;
const char _LEFT_ARROW = 10;
const char _RETURN = 11;
const char _INSERT_DELETE = 12; // Mapped to Delete key
const char _CONTROL = 13;
const char _LEFT_RIGHT_CURSOR = 14;
const char _RUN_STOP = 15; // runstop = ALT_LEFT for modifier purpurposes
const char _RIGHT_ALT = 15;
const char _LEFT_SHIFT = 16;
const char _UP_DOWN_CURSOR = 17;
const char _RIGHT_SHIFT = 18;
const char _F1 = 19;
const char _F2 = 26;
const char _F3 = 21;
const char _F4 = 27;
const char _F5 = 23;
const char _F6 = 28;
const char _F7 = 25;
const char _F8 = 29;
const char _CBM = 20;
const char _HOME = 24;
const char _POUND = 9;

// Physical keyboard mapping
char keys[ROWS][COLS] = {
    _LEFT_ARROW, 'w', 'r', 'y', 'i', 'p', '*', _RETURN,               // COLA
    '1', '3', '5', '7', '9', '+', _POUND, _INSERT_DELETE,             // COLB
    _CONTROL, 'a', 'd', 'g', 'j', 'l', ';', _LEFT_RIGHT_CURSOR,       // COLC
    _RUN_STOP, _LEFT_SHIFT, 'x', 'v', 'n', ',', '/', _UP_DOWN_CURSOR, // COLD
    ' ', 'z', 'c', 'b', 'm', '.', _RIGHT_SHIFT, _F1,                  // COLE
    _CBM, 's', 'f', 'h', 'k', ':', '=', _F3,                          // COLF
    'q', 'e', 't', 'u', 'o', '@', '|', _F5,                           // COLG
    '2', '4', '6', '8', '0', '-', _HOME, _F7                          // COLH
};

byte rowPins[ROWS] = {INPUT_PIN0, INPUT_PIN1, INPUT_PIN2, INPUT_PIN3, INPUT_PIN4, INPUT_PIN5, INPUT_PIN6, INPUT_PIN7};         //connect to the row pinouts of the kpd
byte colPins[COLS] = {OUTPUT_PIN0, OUTPUT_PIN1, OUTPUT_PIN2, OUTPUT_PIN3, OUTPUT_PIN4, OUTPUT_PIN5, OUTPUT_PIN6, OUTPUT_PIN7}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, COLS, ROWS);

String msg;
bool shiftEnabled = false;
bool altEnabled = false;

// Arduino method - run before the controller loop starts
void setup()
{
    Keyboard.begin();
}

void setModifierStatus(char scannedKey, KeyState state)
{
    shiftEnabled = (((scannedKey == _LEFT_SHIFT) || (scannedKey == _RIGHT_SHIFT)) && ((state == PRESSED) || (state == HOLD)));
    altEnabled = ((scannedKey == _RUN_STOP) && ((state == PRESSED) || (state == HOLD)));
}

bool IsModifierKey(char scannedKey)
{
    return (scannedKey == _RIGHT_SHIFT || scannedKey == _LEFT_SHIFT || scannedKey == _RUN_STOP);
}

bool IsAlphanumericKey(char scannedKey)
{
    return (((scannedKey >= ' ') && (scannedKey <= '~'))); // space to tilde
}

bool IsCursorKey(char scannedKey)
{
    return (scannedKey == _LEFT_RIGHT_CURSOR || scannedKey == _UP_DOWN_CURSOR);
}

bool IsControlKey(char scannedKey)
{
    return (scannedKey == _RETURN || scannedKey == _INSERT_DELETE || scannedKey == _CONTROL || scannedKey == _LEFT_RIGHT_CURSOR || scannedKey == _RUN_STOP || scannedKey == _LEFT_SHIFT || scannedKey == _UP_DOWN_CURSOR || scannedKey == _RIGHT_SHIFT || scannedKey == _CBM || scannedKey == _HOME);
}

void SendCursoKey(char _cursorKey)
{
    // nb Keyboard.print() doesn't work with cursor keys. need to press and release
    if (shiftEnabled)
    {
        if (_cursorKey == _LEFT_RIGHT_CURSOR)
        {
            Keyboard.press(KEY_RIGHT_ARROW);
            delay(90);
            Keyboard.release(KEY_RIGHT_ARROW);
        }
        else
        {
            Keyboard.press(KEY_DOWN_ARROW);
            delay(90);
            Keyboard.release(KEY_DOWN_ARROW);
        }
    }
    else
    {
        if (_cursorKey == _LEFT_RIGHT_CURSOR)
        {
            Keyboard.press(KEY_LEFT_ARROW);
            delay(90);
            Keyboard.release(KEY_LEFT_ARROW);
        }
        else if (_cursorKey == _UP_DOWN_CURSOR)
        {
            Keyboard.press(KEY_UP_ARROW);
            delay(90);
            Keyboard.release(KEY_UP_ARROW);
        }
    }
}

char trueKey(char scannedKey)
{
    char modifiedKey = scannedKey;

    if (shiftEnabled)
    {
        // If key alpha, convert to upper case
        if ((scannedKey >= 'a') && (scannedKey <= 'z'))
        {
            modifiedKey = scannedKey - 32;
        }

        // Convert non standard / non alpha to shifted version
        switch (scannedKey)
        {
        case ':':
            modifiedKey = '[';
            break;
        case ';':
            modifiedKey = ']';
            break;
        case '|':
            modifiedKey = '^';
            break;
        case '-':
            modifiedKey = 126;
            break;
        case '1':
            modifiedKey = '!';
            break;
        case '2':
            modifiedKey = '"';
            break;
        case '3':
            modifiedKey = '#';
            break;
        case '4':
            modifiedKey = '$';
            break;
        case '5':
            modifiedKey = '%';
            break;
        case '6':
            modifiedKey = '&';
            break;
        case '7':
            modifiedKey = 39;
            break;
        case '8':
            modifiedKey = '(';
            break;
        case '9':
            modifiedKey = ')';
            break;
        case ',':
            modifiedKey = '<';
            break;
        case '.':
            modifiedKey = '>';
            break;
        case _F1:
            modifiedKey = _F2;
            break;
        case _F3:
            modifiedKey = _F4;
            break;
        case _F5:
            modifiedKey = _F6;
            break;
        case _F7:
            modifiedKey = _F8;
            break;
        case _INSERT_DELETE:
            modifiedKey = _DELETE;
            break;
        case '/':
            modifiedKey = '?';
        }
    }

    // Non standard keys
    switch (scannedKey)
    {
    case _POUND:
        modifiedKey = '£';
        break;
    }

    // alt shifted keys (where no matching key on keyboard)
    if (altEnabled)
    {
        switch (scannedKey)
        {
        case '\'':
            modifiedKey = '`';
            break;
        case '/':
            modifiedKey = '\\';
            break;
        case ':':
            modifiedKey = '{';
            break;
        case ';':
            modifiedKey = '}';
            break;
        }
    }
    return modifiedKey;
}

// Main Arduino controller loop
void loop()
{
    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
        for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
        {
            if (kpd.key[i].stateChanged) // Only find keys that have changed state.
            {
                char keyToSend = ' ';

                if (IsModifierKey(kpd.key[i].kchar))
                {
                    setModifierStatus(kpd.key[i].kchar, kpd.key[i].kstate);
                }

                if (kpd.key[i].kstate == RELEASED)
                {
                    // Possible states are : IDLE, PRESSED, HOLD, or RELEASED
                    if (IsAlphanumericKey(kpd.key[i].kchar) || kpd.key[i].kchar == _POUND)
                    {
                        Keyboard.print(trueKey(kpd.key[i].kchar));
                    }
                    else if (kpd.key[i].kchar == _RETURN)
                    {
                        Keyboard.println("");
                    }
                    else if (IsCursorKey(kpd.key[i].kchar))
                    {
                        SendCursoKey(kpd.key[i].kchar);
                    }
                }
            }
        }
    }
}
