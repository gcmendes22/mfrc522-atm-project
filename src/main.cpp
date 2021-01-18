/**************************************************************************/
/*!
  @file   main.cpp
  @author Guilherme Mendes up201909929

  Simple ATM with RFID-RC522 
  
  All the main messages are displayed on LCD16x4
  
  Bugs:
    -> We only have 1.5 seconds to choose the option.
    -> Everything we write on the keyboard is not written on the monitor
  
  Note: The card must be close to the reader during the entire operation
*/
/**************************************************************************/

#include <SPI.h>
#include <Arduino.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS 10
#define RST 5
#define BUFSIZE 10
#define TIMEOUT 1500
#define BAUDRATE 9600
#define MONEYBLOCK 1
#define PASSWORDBLOCK 5

MFRC522 nfc(SS, RST);
LiquidCrystal lcd(8,9,2,3,4,6);

byte keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
byte keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };

/**************************************************************************/
/*!
  @brief Clean a row on the LCD.
  @param row number of the row.
  @returns void
*/
/**************************************************************************/

void cleanRowLCD(int row);

/**************************************************************************/
/*!
  @brief Read block from tag's memory.
  @param i number of the block
  @param data data that is necessary to read the tag
  @param serial serial of tag
  @returns void
*/
/**************************************************************************/

boolean readBlock(int i, byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief Write on block of tag's memory.
  @param i number of the block
  @param data data that is necessary to read the tag
  @param serial serial of tag
  @returns void
*/
/**************************************************************************/

boolean writeBlock(int i, byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief Show the bank statement.
  @param data buffer to read the memory
  @param serial serial uid of the tag 
  @returns void
*/
/**************************************************************************/

void seeBankStatement(byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief Deposit an amount in the account.
  @param amount amount to deposit. 
  @param data buffer to read the memory
  @param serial serial uid of the tag
  @returns void
*/
/**************************************************************************/

boolean depositMoney(int amount, byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief Withdraw an amount of the account.
  @param amount amount to withdraw.
  @param data buffer to read the memory
  @param serial serial uid of the tag
  @returns boolean
*/
/**************************************************************************/

boolean withdrawMoney(int amount, byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief Leave the program.
  @returns boolean
*/
/**************************************************************************/

void leaveProgram();

/**************************************************************************/
/*!
  @brief Check if the RFID Reader exist or not. 
  @returns void
*/
/**************************************************************************/

void verifyRfidReader();

/**************************************************************************/
/*!
  @brief Check if the tag is present on the reader.
  @param data data on the tag's memory that allows to verify it.
  @returns true if is present, false if is not.
*/
/**************************************************************************/

boolean verifyTag(byte *data);

/**************************************************************************/
/*!
  @brief Print on the screen the menu.
  @returns void
*/
/**************************************************************************/

void printMenu();

/**************************************************************************/
/*!
  @brief Find the word after the first white-space
  @param s string to analize
  @returns string (first word after the first space)
*/
/**************************************************************************/

String findWordAfterSpace(String s);

/**************************************************************************/
/*!
  @brief  Function to select the option of the menu.
    if A -> Show us the account statement.
    if B -> Deposit an amount
    if C -> Withdraw an amount
    if D -> Change password
    if E -> Leave program.
  @returns void
*/
/**************************************************************************/

void selectOption(byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief  Function to print an array of bytes
  @param array array of bytes
  @param length length of array
  @param label name of the array
  @returns void
*/
/**************************************************************************/

void printByteArray(byte array[], int length, String label);

/**************************************************************************/
/*!
  @brief  Function to convert an array of bytes into an int16
  @param array array of bytes
  @returns the first element of the array, wich is the integer value
*/
/**************************************************************************/

uint16_t convertBytesToInt16(byte array[]);

/**************************************************************************/
/*!
  @brief  Function to ask the password to the user
  @param data buffer to read the memory
  @param serial serial uid of the tag
  @returns void 
*/
/**************************************************************************/

boolean askCredentials(byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief  Function to update the password
  @param data buffer to read the memory
  @param serial serial uid of the tag
  @returns void 
*/
/**************************************************************************/

boolean updatePassword(byte data[MAX_LEN], byte serial[5]);

/**************************************************************************/
/*!
  @brief  Function to animate the searching operation
  @returns void 
*/
/**************************************************************************/

void lcdLoadingAnimation();

/**************************************************************************/
/*!
  @brief  Function to display all the errors on screen
  @param error number of the error
  @returns void 
*/
/**************************************************************************/

void displayError(int error);

/**************************************************************************/
/*!
  @brief  Function to display all the successes on screen
  @param success number of the success
  @returns void 
*/
/**************************************************************************/

void displaySuccess(int success);

/**************************************************************************/
/**************************************************************************/
void setup() {

  // Initialization of SPI protocol, Serial communtications, LCD communications and Rfid reader.
  SPI.begin();
  Serial.begin(BAUDRATE);
  Serial.setTimeout(TIMEOUT);
  Serial.println("Looking for MFRC522...");
  nfc.begin();
  lcd.begin(16,4);

  lcd.setCursor(0,0);
  lcd.print("Searching...");
  lcdLoadingAnimation();

  verifyRfidReader();
}

boolean flag = true;

void loop() {
  byte status;
  byte data[MAX_LEN];
  byte serial[5];
  int i;

  if(verifyTag(data)) {

    // Get tag UID
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    // Print tag UID
    Serial.print("Tag UID: ");
    lcd.setCursor(0,0);
    lcd.print("UID: ");
    for (i = 0; i <= 3; i++) {
      Serial.print(serial[i], HEX);
      Serial.print("");
      lcd.print(serial[i], HEX);
    }
    Serial.println(""); 
    
    // Select the tag to talk
    nfc.selectTag(serial);

    // Ask the password and verify if it is correct
    if(askCredentials(data, serial)) {
      printMenu();
      selectOption(data, serial);
    }else {
      displayError(8);
    }  

    nfc.haltTag();
  }
}

/**************************************************************************/
/**************************************************************************/

boolean askCredentials(byte data[MAX_LEN], byte serial[5]) {
  Serial.println("Please, introduce your password: ");
  String attempt = Serial.readStringUntil('\n');
  readBlock(PASSWORDBLOCK, data, serial);
  String password = String();
  for(int i = 0; i < 16; i++)
    password.concat((char)data[i]);

  return (password.compareTo(attempt) == 0) ? true : false;
}

boolean updatePassword(byte data[MAX_LEN], byte serial[5]) {
  Serial.println("Please, introduce the new password: ");
  String newPassword = Serial.readStringUntil('\n');
  if(newPassword.length() == 0 || newPassword.length() > 5) {
    displayError(10);
    return false;
  }
  newPassword.concat("a");
  byte buff[32];
  newPassword.getBytes(buff, newPassword.length());
  boolean status = writeBlock(PASSWORDBLOCK, buff, serial);
  if(status) {
    displaySuccess(7);
    Serial.println("Swipe the card through the reader again.");
    return true;
  } else {
    displayError(9);
    return false;
  }
}

void lcdLoadingAnimation() {
  lcd.setCursor(0,1);
  for(int i = 0; i < 16; i++) {
    lcd.print("*");
    delay(500);
  }
}

void cleanRowLCD(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void seeBankStatement(byte data[MAX_LEN], byte serial[5]) {
  lcd.setCursor(0,1);
  lcd.print("->SEE STATEMENT");
  Serial.println("");
  Serial.println("Option A - See bank statement");
  readBlock(MONEYBLOCK, data, serial);
  String amount = String("");
  for(int i = 0; i < 16; i++) {
    if(data[i] >= 48 && data[i] <= 57)
        amount.concat((char)data[i]);
  }
  Serial.print("Total amount: ");
  if(amount == "") Serial.print("0");
  else Serial.print(amount);
  Serial.println(" $");
  Serial.println("Swipe the card through the reader again.");
}

boolean depositMoney(int amount, byte data[MAX_LEN], byte serial[5]) {
  cleanRowLCD(1);
  cleanRowLCD(2);
  cleanRowLCD(3);
  lcd.setCursor(0,1);
  lcd.print("->DEPOSIT MONEY");
  Serial.println("Option B - Deposit money");

  if(amount < 0) {
    displayError(1);
    return false;
  } else {
    String amountInString = String(amount*10);
    byte buff[32];
    amountInString.getBytes(buff, amountInString.length());
    String savedMoney = String("");

    readBlock(MONEYBLOCK, data, serial);
    for(int i = 0; i < 16; i++) {
      if(data[i] >= 48 && data[i] <= 57)
        savedMoney.concat((char)data[i]);
    }
    
    int saved = savedMoney.toInt();
    int totalAmount = saved + amount;

    String totalAmountInString = String(totalAmount*10);
    byte buff2[32];
    totalAmountInString.getBytes(buff2, totalAmountInString.length());
    writeBlock(MONEYBLOCK, buff2, serial);
    displaySuccess(2);
    Serial.println("Swipe the card through the reader again.");
    return true;
  } 
}

boolean withdrawMoney(int amount, byte data[MAX_LEN], byte serial[5]) {
  cleanRowLCD(1);
  cleanRowLCD(2);
  cleanRowLCD(3);
  lcd.setCursor(0,1);
  lcd.print("->WITHDRAW MONEY");
  Serial.println("Option C - Withdraw money");

  if(amount < 0) {
    displayError(2);
    return false;
  } else {
    String amountInString = String(amount*10);
    byte buff[32];
    amountInString.getBytes(buff, amountInString.length());
    String savedMoney = String("");

    readBlock(MONEYBLOCK, data, serial);
    for(int i = 0; i < 16; i++) {
      if(data[i] >= 48 && data[i] <= 57)
        savedMoney.concat((char)data[i]);
    }
    
    int saved = savedMoney.toInt();
    int totalAmount = saved - amount;
    if(amount > saved) {
      displayError(3);
      return false;
    }

    String totalAmountInString = String(totalAmount*10);
    byte buff2[32];
    totalAmountInString.getBytes(buff2, totalAmountInString.length());
    writeBlock(MONEYBLOCK, buff2, serial);
    displaySuccess(3);
    Serial.println("Swipe the card through the reader again.");
    return true;
  } 
}

void leaveProgram() {
  displaySuccess(1);
  cleanRowLCD(0);
  cleanRowLCD(1);
  cleanRowLCD(2);
  cleanRowLCD(3);
  lcd.setCursor(0,0);
  lcd.print("Leaving...");
  delay(1000);
  lcd.noDisplay();
  exit(0);
}

void verifyRfidReader() {
    byte version = nfc.getFirmwareVersion();
  if (!version) {
    displayError(5);
    while(1);
  }

  displaySuccess(6);
  Serial.print("Firmware ver. 0x");
  Serial.print(version, HEX);
  Serial.println(".");
  Serial.println("Swipe the tag to start the application.");
}

void printMenu() {
  Serial.println("===============MENU===============");
  Serial.println("A -> See bank statement");
  Serial.println("B -> Deposit money");
  Serial.println("C -> Withdraw money");
  Serial.println("D -> Update password");
  Serial.println("E -> Exit");
  Serial.println("==================================");
  Serial.println("Choose an option and then press <enter>");
  Serial.print("(if the option is B or C, specify the amount after the option separated by <white space>): ");
}

String findWordAfterSpace(String s) {
  for(int i = 0; i < s.length()-1; i++){
    if(isSpace(s[i])) {
      String word = s.substring(i);
      return word;
    }
  }
  return "";
}

boolean verifyTag(byte *data) {
  return (nfc.requestTag(MF1_REQIDL, data) == MI_OK) ? true : false;
}

void printByteArray(byte array[], int length, String label) {
  Serial.print(label);
  Serial.print(": ");
  for(int i = 0; i < length; i++) {
    Serial.print(array[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");
}

uint16_t convertBytesToInt16(byte array[]) {
  uint16_t sixteen[16/2];
  for (int i = 0; i < 16; i += 2)
    sixteen[i/2] = array[i] | (uint16_t)array[i+1] << 8;
  
  return array[0];
}

boolean readBlock(int i, byte data[MAX_LEN], byte serial[5]) {
  byte status = nfc.authenticate(MF1_AUTHENT1A, i, keyA, serial);
  if(status == MI_OK) {
    status = nfc.readFromTag(i, data);
    if(status == MI_OK) {
      displaySuccess(5);
      return true;
    } else {
      displayError(6);
      return false;
    }
  } else if((status = nfc.authenticate(MF1_AUTHENT1B, i, keyB, serial)) == MI_OK){
    status = nfc.readFromTag(i, data);
    
    if(status == MI_OK) {
      displaySuccess(5);
      return true;
    } else {
      displayError(6);
      return false;
    }
  } else {
    displayError(7);
    return false;
  }
}

boolean writeBlock(int i, byte data[MAX_LEN], byte serial[5]) {
  byte status = nfc.authenticate(MF1_AUTHENT1A, i, keyA, serial);
  if(status == MI_OK) {
    status = nfc.writeToTag(i, data);
    if(status == MI_OK) {
      displaySuccess(6);
      return true;
    } else {
      displayError(7);
      return false;
    }
  } else {
    byte status = nfc.authenticate(MF1_AUTHENT1B, i, keyB, serial);
    if(status == MI_OK) {
      status = nfc.writeToTag(i, data);
      if(status == MI_OK) {
        displaySuccess(6);
        return true;
      } else {
        displayError(7);
        return false;
      }
    }
  }
}

void displayError(int error) {
  cleanRowLCD(1);
  cleanRowLCD(2);
  cleanRowLCD(3);
  lcd.setCursor(0,1);
  lcd.print("ERROR:");
  lcd.setCursor(0,2);
  switch(error) {
    case 1: 
      Serial.println("Error: The amount to be deposited must be greater than 0.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("INVALID AMOUNT");
      break;
    case 2:
      Serial.println("Error: The amount to be withdrawn must be greater than 0.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("INVALID AMOUNT");
      break;
    case 3:
      Serial.println("Error: The amount to be withdrawn must be less than the amount you have.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("INVALID AMOUNT");
      break;
    case 4: 
      Serial.println("Error: Invalid option.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("INVALID OPTION");
      break;
    case 5:
      Serial.print("Error: Didn't find MFRC522 board.");
      break;
    case 6:
      Serial.println("Error: Failed to read from memory.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("READING FAILED");
      break;
    case 7:
      Serial.println("Error: Failed to authenticate.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("AUTH FAILED");
      break;
    case 8:
      Serial.println("The password is incorrect.");
      Serial.println("Swipe the card through the reader and try again.");
      lcd.print("WRONG PASSWORD");
      break;
    case 9:
      Serial.println("An error occurred while updating your password. Try again.");
      Serial.println("Swipe the card through the reader again.");
      lcd.print("UPDATING ERROR");
     break;
    case 10:
      Serial.println("The password cannot be null and must be less or equal than 5 characters.");
      lcd.print("INVALID PASSWORD");
      break;
    default:
      Serial.println("Error: An unexpected error happened.");
      lcd.print("UNEXPECTED");
      Serial.println("Swipe the card through the reader and try again.");
      break;
  }
}

void displaySuccess(int success) {
  switch(success) {
    case 1: 
      Serial.println("Leaving the program...");
      break;
    case 2:
      Serial.println("The indicated amount has been successfully deposited.");
      break;
    case 3:
      Serial.println("The indicated amount has been successfully withdrawn from your account.");
      break;
    case 4: 
      Serial.println("Found chip MFRC522 ");
      break;
    case 5: 
      Serial.println("Reading memory...");
      break;
    case 6: 
      Serial.println("Writing on memory...");
      break;
    case 7:
      Serial.println("Your password was updated successfully.");
      break;
    default:
      Serial.println("");
      break;
  }
}

void selectOption(byte data[MAX_LEN], byte serial[5]) {
  String s = Serial.readStringUntil('\n');
  char letter = s[0];
  String word;
  int amount;
  Serial.println("");
  switch(letter) {
    case 'A':
      seeBankStatement(data, serial);
      break;
    case 'B':
      word = findWordAfterSpace(s);
      amount = word.toInt();
      depositMoney(amount, data, serial);
      break;
    case 'C':
      word = findWordAfterSpace(s);
      amount = word.toInt();
      withdrawMoney(amount, data, serial);
      break;
    case 'D':
      updatePassword(data, serial);
      break;
    case 'E':
      leaveProgram();
      break;
    default: 
      displayError(4);
      break;
  }
}