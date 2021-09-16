/**
 * @file SELECT.ino
 * @author Tawaliou ALAO (alaotawaliou@gmail.com)
 * @remark Thanks to my brother ALIDOU Abdoul Majeed to help to define the database architecture 
 * @remark Thanks to YoupiLab: https://youpilab.com/ to support the project 
 * @brief  A tiny database to manipulate EEPROM easily
 * @version 1.0.0
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/**
 * In INSERT example,
 * We have inserted 3 data in the table of name EM
 * | ag | rt  |
 * ------------
 * | 25 | 8.3 |
 * | 30 | 7.9 |
 * | 21 | 8.63|
 * ------------
 * 
 * Now we want to select or retrieve these values
 */
 
#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // 1- create the data to hold values selected
  int em_ag = 0;
  float em_rt = 0.0;
  
  // 2- access to the table and select at a position one data
  // and don't forget to call DONE at the end of operation
  mem.FROM("EM")
    .SELECT("ag", &em_ag)
    .SELECT("rt", &em_rt)
    .DONE(); // we get the value at index 0 (first element) in the table

  // 3- you can use them may be display them
  PRINT("em age", em_ag);
  PRINT("em rate", em_rt);

  // again
  int another_age = 0;
  float another_rate = 0.0;

  mem.FROM("EM")
    .SELECT("ag", &another_age, 2)
    .SELECT("rt", &another_rate, 2)
    .DONE(); // we get the value at index 2 (third element) in the table
  
  PRINT("another em age", another_age);
  PRINT("another em rate", another_rate);
}

void loop() {
  // put your main code here, to run repeatedly:

}
