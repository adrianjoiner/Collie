#include <Keyboard.h>
#include <Keypad.h>

/*

Version 1.0
=========
- VIC20 / CBM64 keyboard simulation

TODO:
- Add support for hot keyboard switching
- Find a use for the RESTORE key

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

const int OSX = 1;
const int WINDOWS = 2;
int KeyboardEmulationMode = WINDOWS;

#define INPUT_PIN0 PIN_B1 // Teensy pin 0
#define INPUT_PIN1 PIN_B0 // pin 1
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
#define SHIT_TO_UPPER_CASE 0x20
#define SINGLE_QUOTE 0x27

#define DEFAULT_KEY_DELAY 0x5A

// CBM keyboard based on an 8 x 8 matrix
const byte ROWS = 8;
const byte COLS = 8;

// Control keys
const char _DELETE = 0x05;
const char _RIGHT_ARROW = 0x08;
const char _UP_ARROW = 0x07;
const char _DOWN_ARROW = 0x06;
const char _LEFT_ARROW = 0x0A;
const char _RETURN = 0x0B;
const char _INSERT_DELETE = 0x0C; // Mapped to Delete key
const char _CONTROL = 0x0D;
const char _LEFT_RIGHT_CURSOR = 0xE;
const char _RUN_STOP = 0x0F; // runstop = ALT_LEFT for modifier purpurposes
const char _RIGHT_ALT = 0x0F;
const char _LEFT_SHIFT = 0x10;
const char _UP_DOWN_CURSOR = 0x11;
const char _RIGHT_SHIFT = 0x12;
const char _F1 = 0x13;
const char _F2 = 0x14;
const char _F3 = 0x15;
const char _F4 = 0x1B;
const char _F5 = 0x17;
const char _F6 = 0x1C;
const char _F7 = 0x19;
const char _F8 = 0x1D;
const char _CBM = 0x14;
const char _HOME = 0x18;
const char _POUND = 0x09;

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

bool shiftEnabled = false;
bool altEnabled = false;
bool ctrlEnabled = false;
bool cbmEnabled = false;
bool controlSent = false;

// Arduino method - run before the controller loop starts
void setup()
{
    Keyboard.begin();
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
            // Possible states are : IDLE, PRESSED, HOLD, or RELEASED
            if (kpd.key[i].stateChanged) // Only find keys that have changed state.
            {
                if (IsModifierKey(kpd.key[i].kchar))
                {
                    setModifierStatus(kpd.key[i].kchar, kpd.key[i].kstate);
                }

                if ((kpd.key[i].kstate == PRESSED) && (IsAlphanumericKey(kpd.key[i].kchar)) && ctrlEnabled && !controlSent)
                {
                    controlSent = SendControlKeyCombinations(kpd.key[i].kchar);
                    if (controlSent)
                    {
                        WaitForKeyPress();
                        controlSent = false;
                    }
                }
                else if (kpd.key[i].kstate == RELEASED)
                {
                    if (IsAlphanumericKey(kpd.key[i].kchar))
                    {
                        Keyboard.print(trueKey(kpd.key[i].kchar));
                    }
                    else if (kpd.key[i].kchar == _RETURN)
                    {
                        Keyboard.println("");
                    }
                    else if (IsCursorKey(kpd.key[i].kchar))
                    {
                        SendCursorKey(kpd.key[i].kchar);
                    }
                    else if (kpd.key[i].kchar == _INSERT_DELETE)
                    {
                        SendDelete(kpd.key[i].kchar);
                    }
                    else if (kpd.key[i].kchar == _POUND)
                    {
                        SendPoundSign();
                    }
                    else if (IsFunctionKey(kpd.key[i].kchar))
                    {
                        SendFunctionKey(kpd.key[i].kchar);
                    }
                }
            }
        }
    }


/*
    Helper functions
*/
bool IsModifierKey(char scannedKey)
{
    return (scannedKey == _RIGHT_SHIFT || scannedKey == _LEFT_SHIFT || scannedKey == _RUN_STOP || scannedKey == _CONTROL || scannedKey == _CBM) || scannedKey == _HOME;
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

bool IsNonStandard(char scannedKey)
{
    return (scannedKey == _F1 || scannedKey == _F3 || scannedKey == _F5 || scannedKey == _F7 || scannedKey == _POUND);
}

bool IsFunctionKey(char scannedKey)
{
    return (scannedKey == _F1 || scannedKey == _F3 || scannedKey == _F5 || scannedKey == _F7);
}

void WaitForKeyPress()
{
    bool _keyPressed = false;
    while (!_keyPressed)
    {
        kpd.getKeys();
        for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
        {
            if (kpd.key[i].stateChanged)
            {
                if (kpd.key[i].kstate == PRESSED)
                {
                    _keyPressed = true;
                }
            }
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
            modifiedKey = scannedKey - SHIFT_TO_UPPER_CASE;
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
            modifiedKey = SINGLE_QUOTE;
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
        case _INSERT_DELETE:
            modifiedKey = _DELETE;
            break;
        case '/':
            modifiedKey = '?';
            break;
        case '-':
            modifiedKey = '_';
        }
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
        case '-':
            modifiedKey = '~';
            break;
        }
    }
    return modifiedKey;
}

void setModifierStatus(char scannedKey, KeyState state)
{
    shiftEnabled = (((scannedKey == _LEFT_SHIFT) || (scannedKey == _RIGHT_SHIFT)) && ((state == PRESSED) || (state == HOLD)));

    // We use Alt has a shift hold key for cursor movement
    if (scannedKey == _RUN_STOP)
    {
        if (state == PRESSED)
        {
            altEnabled = true;
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(DEFAULT_KEY_DELAY);
        }
        else if (state == RELEASED)
        {
            Keyboard.release(KEY_LEFT_SHIFT);
            altEnabled = false;
        }
    }

    if(scannedKey == _CBM){
        if (state == PRESSED)
        {
            cbmEnabled = true;
            Keyboard.press(KEY_LEFT_GUI);
            delay(DEFAULT_KEY_DELAY);

        }
        else if (state == RELEASED)
        {
            cbmEnabled = false;
            Keyboard.release(KEY_LEFT_GUI);
            delay(DEFAULT_KEY_DELAY);
        }
    }

    if(scannedKey == _CONTROL){
        if (state == PRESSED)
        {
            ctrlEnabled = true;
            Keyboard.press(KEY_RIGHT_CTRL);
            delay(DEFAULT_KEY_DELAY);
        }
        else if (state == RELEASED)
        {
            Keyboard.release(KEY_RIGHT_CTRL);
            delay(DEFAULT_KEY_DELAY);
            ctrlEnabled = false;
        }
    }
}

/*
    Custom key send functions
*/
void SendCursorKey(char _cursorKey)
{
    // nb Keyboard.print() doesn't work with cursor keys. need to press and release
    if (shiftEnabled)
    {
        if (_cursorKey == _LEFT_RIGHT_CURSOR)
        {
            Keyboard.press(KEY_RIGHT_ARROW);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_RIGHT_ARROW);
        }
        else
        {
            Keyboard.press(KEY_DOWN_ARROW);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_DOWN_ARROW);
        }
    }
    else
    {
        if (_cursorKey == _LEFT_RIGHT_CURSOR)
        {
            Keyboard.press(KEY_LEFT_ARROW);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_LEFT_ARROW);
        }
        else if (_cursorKey == _UP_DOWN_CURSOR)
        {
            Keyboard.press(KEY_UP_ARROW);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_UP_ARROW);
        }
    }
}

void SendFunctionKey(char funcKey)
{
    if (shiftEnabled)
    {
        switch (funcKey)
        {
        case _F1:
            Keyboard.press(KEY_F2);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F2);
            break;
        case _F3:
            Keyboard.press(KEY_F4);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F4);
            break;
        case _F5:
            Keyboard.press(KEY_F6);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F6);
            break;
        case _F7:
            Keyboard.press(KEY_F8);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F8);
            break;
        }
    }
    else
    {
        switch (funcKey)
        {
        case _F1:
            Keyboard.press(KEY_F1);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F1);
            break;
        case _F3:
            Keyboard.press(KEY_F3);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F3);
            break;
        case _F5:
            Keyboard.press(KEY_F5);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F5);
            break;
        case _F7:
            Keyboard.press(KEY_F7);
            delay(DEFAULT_KEY_DELAY);
            Keyboard.release(KEY_F7);
            break;
        }
    }
}
void SendPoundSign()
{
    switch (KeyboardEmulationMode)
    {
    case WINDOWS:
        Keyboard.press(KEY_RIGHT_ALT);
        Keyboard.press('0');
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release('0');
        Keyboard.press('1');
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release('1');
        Keyboard.press('6');
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release('6');
        Keyboard.press('3');
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release('3');
        Keyboard.release(KEY_RIGHT_ALT);
        break;

    case OSX:
        Keyboard.press(KEY_RIGHT_ALT);
        Keyboard.press('3');
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release('3');
        Keyboard.release(KEY_RIGHT_ALT);
        break;
    }
}

void SendDelete(char scannedKey)
{
    if (shiftEnabled)
    {
        Keyboard.press(KEY_DELETE);
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release(KEY_DELETE);
    }
    else
    {
        Keyboard.press(KEY_BACKSPACE);
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release(KEY_BACKSPACE);
    }
}

bool SendControlKeyCombinations(char scannedKey)
{
    if((scannedKey >= 'a') && (scannedKey <= 'z')){
        SendAsControl(scannedKey);
    }
    return controlSent;
}

void SendAsControl(char _key)
{

    // not using compiler options to switch as will add as a macro later
    Keyboard.releaseAll();
    if (KeyboardEmulationMode == OSX)
    {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(_key);
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release(_key);
        Keyboard.release(KEY_LEFT_GUI);
    }
    else
    {
        Keyboard.press(KEY_RIGHT_CTRL);
        Keyboard.press(_key);
        delay(DEFAULT_KEY_DELAY);
        Keyboard.release(_key);
        Keyboard.release(KEY_RIGHT_CTRL);
    }

    controlSent = true;
    ctrlEnabled = false;
}