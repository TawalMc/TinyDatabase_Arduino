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

#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);

  // If it is a new project or first use be sure to
  // clear the whole data base
  mem.clearAll();

  // display meta data infos
  mem.printMetaData();
}

void loop() {
  // put your main code here, to run repeatedly:
}
