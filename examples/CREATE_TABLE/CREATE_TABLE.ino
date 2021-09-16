/**
 * @file CREATE_TABLE.ino
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
 * We want to create a this table
 * table name: EM
 * capacity (maximum of data): 15
 * columns: 2 
 *          -ag (age) of type int
 *          -rt (rate) of type float
 * 
 * With TinyDatabase, there are some predifined types:
 * 
 * int    -> "INT"
 * float  -> "FLOAT"
 * 
 * and more - get them at  
 * table name and columns are not more than 2 char
 */

#include <TinyDatabase_Arduino.h>

// MemoryManager object to access data base
MemoryManager mem;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(9600);

  // 1- create array of columns
  Column myCols[] = {{"ag", "INT"}, {"rt", "FLOAT"}};

  // 2- create table and provide capaciyt, number of columns and table name
  int isTbaleCreated = mem.CREATE_TABLE("EM", 15, 2, myCols);

  // 3- You can verify if table is created 
  // by compare the returned value to a status code
  if (isTbaleCreated != STATUS_TABLE_CREATED) {
    Serial.println("Table is not created");   
  }

  // 4- You can display all informations about the meta data of the database
  // (tables infos, amount of data
  mem.printMetaData();
}

void loop() {
  // put your main code here, to run repeatedly:
}
