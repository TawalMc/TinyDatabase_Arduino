/**
 * @file DELETE.ino
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
 * Now we want to 
 * - delete the second data (index 1) in the table
 * - delete all data where rate is greater or equal to 8.3
 */

#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  //1- delete the second data: access to the table and precise the index
  mem.TO("EM")
    .DELETE(1)
    .DONE();

  // 2- delete all data where >= 8.3
  float min_val = 8.3;
  int nbValueDelete = 0; // to know how many data is deleted
  
  mem.TO("EM")
    .WHERE("rt", FILTER::isGreaterOrEqualTo, &min_val)
    .DELETE_ALL(nbValueDelete)
    .DONE(); 

  PRINT("nbValueDelete", nbValueDelete); // you can display the number of data deleted
}

void loop() {
  // put your main code here, to run repeatedly:

}
