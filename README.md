TinyDatabase - TinyDatabase_Arduino
============

![TinyDatabase logo](TinyDatabase-logo-270.png)

## [Documentation available](https://tinydatabase-doc.vercel.app/)

TinyDatabase is a library that allow the management of persistent internal memory (EEPROM) of microncontroller. With a syntax near to the SQL queries, it encapsulates the writing/reading of EEPROM data which was done with their address by providing a more friendly and simply API. The project is still under development and is tested only on Arduino boards with AVR ATmega 328p. We'are working to support more board and some othersdevices with EEPROM memory like the RTC-DS321,..

## Support
Here is the current list of boards supported:
- Arduino UNO (ATmega 328p)

## Version
The library still in bêta version for testing and some corrections, so you can use the API in dev or some cool projects and provide us feedbacks.

## Installation
As TinyDataabse is not yet stable and well documented and tested, you can't download it through Arduino Libraries manager but you can download the zip in the github and add it to your Arduino IDE and to the sketch.

## API
- TinyDatabase API use a syntax like SQL queries to work on data base (tables, column, row,...). More details and explications will add soon in the official documentation website of the library.

- We don't implement yet `SELECT_ALL` functions for char sequence.

- Don't forget to clear the EEPROM memory the first time you use the library.

## Examples
Test the examples of this library to learn more

# Contributor
`This library was built and maintained by` [Tawaliou ALAO](https://github.com/TawalMc)` (this account)`. 

- _Thanks to my brother ALIDOU Abdoul Majeed who help me to define the database architecture_

- _Thanks to [YoupiLab](https://youpilab.com/) to support the project_

- [Here is the documentation](https://tinydatabase-doc.vercel.app/)