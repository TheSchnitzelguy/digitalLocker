/*4x4 Matrix Keypad connected to Arduino
  This code prints the key pressed on the keypad to the serial port*/

#include <Keypad.h>
#include <Password.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define pswCorrect A1
#define pswFalse A2

int lockLatch = 12;
int ledPin = 11;
const byte numRows = 4; //number of rows on the keypad
const byte numCols = 4; //number of columns on the keypad

long displayTimeout = 0;
long passwordTimeout = 0;

Password psw = Password("0000"); //unlock passcode, this will be your unlock code


//keymap defines the key pressed according to the row and columns just as appears on the keypad
char keymap[numRows][numCols] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Code that shows the the keypad connections to the arduino terminals
byte rowPins[numRows] = {9, 8, 7, 6}; //Rows 0 to 3
byte colPins[numCols] = {5, 4, 3, 2}; //Columns 0 to 3

//initializes an instance of the Keypad class
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

//Initialise an instance and Set the LCD I2C address NOTE: for MH boards use 0x3f as i2c address
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup()
{
  //init pins
  pinMode(pswCorrect, OUTPUT);
  pinMode(pswFalse, OUTPUT);
  pinMode(lockLatch, OUTPUT);
  pinMode(ledPin, OUTPUT);

  //initialise devices
  Serial.begin(9600);
  lcd.begin(20, 4);
  myKeypad.addEventListener(keypadEvent);

  //OLED: Write characters on the display
  // NOTE: Cursor Position: Lines and Characters start at 0
  //Start at character 4 on line 0
  lcd.setCursor(0, 0);
  lcd.print("Voer pincode in:");

  delay(1000);
}

//If key is pressed, this key is stored in 'keypressed' variable
//If key is not equal to 'NO_KEY', then this key is printed out
//if count=17, then count is reset back to 0 (this means no key is pressed during the whole keypad scan process

void loop()
{

  char keypressed = myKeypad.getKey();
  digitalWrite(ledPin, LOW);

  //keypad handling
  if (keypressed != NO_KEY)
  {
    Serial.print(keypressed);
    digitalWrite(ledPin, HIGH);   // sets the LED on
    displayTimeout = millis();
    delay(200);
  }

  if ((millis() - displayTimeout) < 10000)
  {
    
    lcd.setBacklight(HIGH);
  }
  else
  {
    //displayTimeout  = millis();
    lcd.setBacklight(LOW);
  }

  if  ((millis() - passwordTimeout) > 30000)
  {
    passwordTimeout = millis();
    passwordReset();
  }

  //lcd handling
  if (Serial.available())
  {
    delay(100);
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0)
    {
      // display each character to the LCD
      lcd.write(Serial.read());
    }
  }
}

int pincodeLength = 0;

void keypadEvent(KeypadEvent specialKeys)
{
  switch (myKeypad.getState())
  {
    case PRESSED:
      Serial.print("Enter:");
      Serial.println(specialKeys);
      delay(10);
      Serial.write(254);

      switch (specialKeys)
      {
        case '*': {
            checkPassword();
            delay(1);
            break;
          }

        case '#': {
            psw.reset();
            delay(1);
            lcd.clear();
            delay(1000);
            lcd.print("Voer pincode in:");
            pincodeLength = 0;
            break;
          }

        default:
          {
            psw.append(specialKeys);
            delay(1);
            lcd.setCursor(pincodeLength, 1);
            lcd.print("*");
            pincodeLength++;
            break;
          }
      }
  }
}

void checkPassword()
{

  if (psw.evaluate())  //if password is right open

  {
    Serial.println("Accepted");
    Serial.write(254); delay(10);
    //Add code to run if it works
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pincode correct");
    delay(1000);
    lcd.setCursor(2, 1);
    lcd.print("Status: open");
    delay(1000);
    digitalWrite(lockLatch, HIGH);
    digitalWrite(pswCorrect, HIGH);
    delay(3000);
    digitalWrite(lockLatch, LOW);
    digitalWrite(pswCorrect, LOW);

  }



  else
  {
    Serial.println("Denied"); //if passwords wrong keep locked
    //add code to run if it did not work

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Pincode fout");
    delay(1000);
    lcd.setCursor(1, 1);
    lcd.print("Status: closed");
    delay(1000);
    digitalWrite(pswFalse, HIGH);
    digitalWrite(lockLatch, LOW);
    delay(5000);
    digitalWrite(pswFalse, LOW);

  }

passwordReset();

}

void passwordReset()

{
  psw.reset();
  delay(1);
  lcd.clear();
  delay(1000);
  lcd.print("Voer pincode in:");
  pincodeLength = 0;
}

