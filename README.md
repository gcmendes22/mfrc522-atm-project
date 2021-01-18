# mfrc522-atm-project

Simple ATM with rfid mfrc522 and lcd16x4

### Used Material

For the realization of this project, was necessary:

- Pic32 (Digilent Microchip UNO32)
- RFID - MFRC522
- RFID Tag
- BreadBoard
- Connecting wires
  To complement the project, we used a LCD16x4 to display all the main messages to
  the user. To adjust the illumination of the screen, was used a 10k𝛀 potentiometer.

### Application Flow

This application consists of a simple ATM. In this ATM we can deposit money,
withdraw money and see the status of the account.
The application proceeds in the following order:
The Rfid reader searches for a new tag. If the tag is present on the reader, then
verify if the tag is valid. If it is, ask for the password and then print the UID serial
number and select that tag to talk. After that, display the menu on the screen and
ask the user to choose the option. Execute the operation according to the selected
option. This procedure happens every time that the user swipe de tag into the
reader.

### Bugs and Reports

Although the application works correctly, there were some errors and problems:

1. We only have 1.5 seconds to start writing the option.
2. Everything we write on the keyboard is not written on the screen.
   The card or tag, must be present to the reader during the entire operation.
