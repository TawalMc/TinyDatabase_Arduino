/**
 * @file SELECT_ALL.ino
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
 * Now we want to retieve all value instead of select one by one
 */

#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // 1- create an array of data type that'll hold selected data
  int age[6];
  float rate[6];

  // 2- a variable to hold the amount of data selected
  int nbValue = 0;
  
  // 2- access to the table and use select - to get data
  // use FROM and write data use TO. BUt they do the same thing
  
  mem.FROM("EM")
    .SELECT_ALL("ag", age, nbValue)
    .SELECT_ALL("rt", rate, nbValue)
    .DONE();

  // 3- you can use them or display them by iterating
  for (size_t i = 0; i < nbValue; i++) {
    PRINT("-read-", i);
    PRINT("age", age[i]);
    PRINT("rate", rate[i]);
    
    Serial.println();
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
