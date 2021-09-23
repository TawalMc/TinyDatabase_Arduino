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
 * In CREATE_TABLE example,
 * a table of name "EM" was created with 02 columns: "ag" and "rt"
 * We'll populate this tables of 3 data like this
 *
 * | ag | rt  |
 * ------------
 * | 25 | 8.3 |
 * | 30 | 7.9 |
 * | 21 | 8.63|
 * ------------
 */
#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // 1- create data to store
  int   em1_ag = 25;
  float em1_rt = 8.3;

  // 2- access to the table with TO function and insert them
  // and don't forget to call DONE at the end
  mem.TO("EM").INSERT("ag", &em1_ag).INSERT("rt", &em1_rt).DONE();

  // again
  int   em2_ag = 30;
  float em2_rt = 7.9;

  mem.TO("EM").INSERT("ag", &em2_ag).INSERT("rt", &em2_rt).DONE();

  // again
  int   em3_ag = 21;
  float em3_rt = 8.63;

  mem.TO("EM").INSERT("ag", &em3_ag).INSERT("rt", &em3_rt).DONE();

  // 3- you can count the amount of data in the table
  uint8_t count = mem.FROM("EM").COUNT();
  PRINT("amount/count", count);  // PRINT is a custom function to
                                 // display value with a name or an ID
}

void loop() {
  // put your main code here, to run repeatedly:
}
