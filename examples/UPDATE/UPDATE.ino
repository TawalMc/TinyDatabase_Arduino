/**
 * @author Tawaliou ALAO Engineer at YoupiLab (sales@youpilab.com)
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
 * Now we want to update the second value (index 1)
 * to become 31 and 8.2
 */

#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // It's like INSERT methods but here you can
  // specify the index or the position where you want to write
  int   new_age  = 31;
  float new_rate = 8.2;

  mem.TO("EM").UPDATE("ag", &new_age, 1).UPDATE("rt", &new_rate, 1).DONE();
}

void loop() {
  // put your main code here, to run repeatedly:
}
