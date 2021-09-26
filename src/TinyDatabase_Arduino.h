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

#ifndef TINY_DATABASE_ARDUINO
#define TINY_DATABASE_ARDUINO

#include <EEPROM.h>
#include <string.h>

/********************************************************************************************************************************/
/**
   @brief Define each memory size based on the board

*/
const int START_DB             = 1;                       // Where the database start
const int END_DB               = EEPROM.length() - 1;     // Where the database end
const int ADDR_SIZE_DB         = END_DB;                  // Address of numbers of data in database
const int ADDR_SIZE_METADATA   = ADDR_SIZE_DB - 2;        // Where meta-data end in database
const int ADDR_NB_TABLES       = ADDR_SIZE_METADATA - 2;  // Store address of the numbers of tables
const int ADDR_TABLES_METADATA = ADDR_NB_TABLES - 1;      // Where we store tables metadata
const int BYTES_TABLES_GEN_METADATA = 8;                  // Bytes used to store table data

/********************************************************************************************************************************/
/**
   @brief Execution status code

*/
const int STATUS_TABLE_CREATED = -200;  // Table creation succeeded
const int STATUS_TABLE_EXIST   = -201;  // Table already exist
const int STATUS_SUF_MEMORY    = -204;  // Sufficient memory - memory is available

const int STATUS_TABLE_NOT_FOUND  = -401;  // Table not found
const int STATUS_COL_NOT_FOUND    = -402;  // Column not found
const int STATUS_INSUF_MEMORY     = -404;  // Insufficient memory
const int STATUS_MAX_CAP_EXCEEDED = -405;  // Maximum of capacity exceeded

/********************************************************************************************************************************/
/**
   @brief: Some constants
*/
const uint8_t ZERO = 0;  // zero as uint8_t - to erase data from memory

const uint8_t ACTION_INSERT     = 0;
const uint8_t ACTION_UPDATE     = 1;
const uint8_t ACTION_DELETE     = 2;
const uint8_t ACTION_DELETE_ALL = 3;
const uint8_t ACTION_SELECT     = 4;
const uint8_t ACTION_WHERE      = 5;

const uint8_t MAX_CHAR = 30;
const uint8_t MAX_CAP  = 255;

/********************************************************************************************************************************/
/**
   @brief Filters. It is used to compare numerics data
*/
enum FILTER {
  isLessThan,
  isGreaterThan,
  isEqualTo,
  isLessOrEqualTo,
  isGreaterOrEqualTo,
  isNotEqualTo
};

/********************************************************************************************************************************/
/**
 * @brief Custom println function as key - value to give to data
 * a name
 * @tparam T Correspond to the data type you want to display
 * @param name is its key or name
 * @param data
 */
template <typename T>
static void PRINT(char name[], T data) {
  Serial.print(name);
  Serial.print(": ");
  Serial.println(data);
}

/**
 * @brief Each table's column has a type. So user must define the column
 * type with char sequence defined by the library. And each data type
 * has a "ID" to know how many uint8_t will be consumed by the data
 *
 * @param type Char sequence from the user to indicate column type
 * @return uint8_t
 */
uint8_t typeIDNum(char* type) {
  if (strcmp(type, "BOOL") == 0) return 1;
  if (strcmp(type, "BYTE") == 0) return 2;
  if (strcmp(type, "FLOAT") == 0) return 3;
  if (strcmp(type, "INT") == 0) return 4;
  if (strcmp(type, "LONG") == 0) return 5;
  // if ( strcmp(type, "STRING") == 0) return 6;
  if (strcmp(type, "UINT") == 0) return 7;
  if (strcmp(type, "ULONG") == 0) return 8;
  if (strncmp(type, "CHAR", 4) == 0) {
    if (strlen(type) == 4) return 9;
    char nb[] = {type[4], type[5]};
    return 9 + atoi(nb);
  }

  return 0;
}

/**
 * @brief When we read data type from column informations, we deduce the amount
 * of uint8_t which match
 *
 * @param type
 * @return uint8_t
 */
uint8_t deduceSizeof(uint8_t type) {
  if (type == 1) return sizeof(bool);
  if (type == 2) return sizeof(byte);
  if (type == 3) return sizeof(float);
  if (type == 4) return sizeof(int);
  if (type == 5) return sizeof(long);
  // if (type == 6) return sizeof(String);
  if (type == 7) return sizeof(unsigned int);
  if (type == 8) return sizeof(unsigned long);
  if (type == 9) return sizeof(char);
  if (type > 9) return (type - 9) * sizeof(char);

  return 0;
}

/********************************************************************************************************************************/
/**
   @brief This structure represent a column of a table that user
  instanciate to create a column
*/
struct Column {
  char  name[3];
  char* type;
};

/**
 * @brief Will be used by an instance of TableData
 * to hold easily column informations
 *
 */
struct AllColumn {
  char    name[3];
  uint8_t type;
};

/*
struct ArrayOfChar {
  char* value;
};
*/

/********************************************************************************************************************************/
/**
 * @brief TableData is the second most important class of this library.
 * It is used to work on a table: read a table data, display them
 * and delete operations. The main class MemoryManager has an instance of
 * this to work on the data base.
 *
 * We use inline methods for function definitions to kep all code in one file.
 *
 */
class TableData {
 public:
  TableData() {}

  /********************************************************************************************************************************/
  /**
   * @brief Construct a new Table Data object
   * We work on a table after getting its information's address
   * from Meta data parts. When we go to the Meta data parts
   * we can have all global informations about a table: size, capacity
   * where we start store its data in memory, columns name,...
   *
   *
   * @param c_addr Where the current table data informations storage started
   */
  TableData(int c_addr) {
    if (c_addr >= 0) {
      addr      = c_addr;
      int _addr = addr;

      name[0] = char(EEPROM.read(_addr - 0));
      name[1] = char(EEPROM.read(_addr - 1));
      name[2] = '\0';  // char(EEPROM.read(szMeta - 2));

      start = 0;
      EEPROM.get(_addr - 3, start);

      size = EEPROM.read(_addr - 5);

      capacity = EEPROM.read(_addr - 6);

      modSize = size % capacity;

      currentAction = ACTION_SELECT;
      indexElt      = (int*)malloc(capacity * sizeof(int));
      initIndexElt();

      nbCols = EEPROM.read(_addr - 7);

      _addr -= 8;

      cols = (AllColumn*)malloc(nbCols * sizeof(AllColumn));

      for (size_t i = 0; i < nbCols; i++) {
        cols[i].name[0] = char(EEPROM.read(_addr - 0));
        cols[i].name[1] = char(EEPROM.read(_addr - 1));
        cols[i].name[2] = '\0';  // char(EEPROM.read(szMeta - 2));

        cols[i].type = EEPROM.read(_addr - 3);

        _addr -= 4;
      }
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief We use chaining syntax (operations) with TableData to
   * manipulate its data. And we copy to the TableData instance in MemoryManager
   * all informations when we locate a table on what to work. So
   * we need copy assignment operator.
   *
   * @param other
   * @return TableData&
   */
  TableData& operator=(const TableData& other) {
    addr = other.addr;
    strcpy(name, other.name);
    capacity = other.capacity;
    size     = other.size;
    modSize  = other.modSize;
    start    = other.start;
    nbCols   = other.nbCols;

    currentAction = other.currentAction;
    indexElt      = (int*)malloc(capacity * sizeof(int));
    for (size_t i = 0; i < capacity; i++) {
      indexElt[i] = other.indexElt[i];
    }

    cols = (AllColumn*)malloc(nbCols * sizeof(AllColumn));
    for (size_t i = 0; i < nbCols; i++) {
      strcpy(cols[i].name, other.cols[i].name);
      cols[i].type = other.cols[i].type;
    }

    return *this;
  }

  ~TableData() {
    free(cols);
    free(indexElt);

    cols     = nullptr;
    indexElt = nullptr;
  }

  /********************************************************************************************************************************/
  /**
   * @brief Count amount of data available (like real size) in the table if
   * WHERE is applied or not
   *
   * @return uint8_t
   */
  uint8_t COUNT() { return amountElt(); }

  /********************************************************************************************************************************/
  /**
   * @brief Return the capacity or the maximum amount of data that the table can
   * hold
   *
   * @return uint8_t
   */
  uint8_t CAPACITY() { return capacity; }

  /********************************************************************************************************************************/
  /**
   * @brief This function is used to insert a new data at the end of
   * the table. If the capacity of data of the table is reached, we
   * re-write from the beginning of the table news data. Also we
   * use modulo function between table size and its capacity to be
   * sure that at any time we don't write outside of the table bound.
   *
   * @tparam T
   * @param c_name Column name
   * @param data Data use want to store - not a char
   * @return TableData& The current table instance
   */
  template <typename T>
  TableData& INSERT(char* c_name, const T* data) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);
      uint8_t type = cols[pos].type;

      int addrToWrite = jumpToColumn(pos, modSize);

      if (type <= 9) {
        EEPROM.put(addrToWrite, *data);
      }

      currentAction = ACTION_INSERT;
    }
    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief An overload version othe insert template function used for char
   * array
   *
   * @param c_name Column name
   * @param data Char sequence we want to store
   * @return TableData& The current table sequence
   */
  TableData& INSERT(char* c_name, char* data) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);
      uint8_t type = cols[pos].type;

      int addrToWrite = jumpToColumn(pos, modSize);

      if (type > 9) {
        uint8_t nbBytes = deduceSizeof(type);
        writeCharArray(addrToWrite, nbBytes, data);
      }

      currentAction = ACTION_INSERT;
    }
    return *this;
  }
  /********************************************************************************************************************************/
  /**
   * @brief An updata function is just an override methods over a data at
   * a position in the table. The amount of data doesn't increase or decrease,
   * just an override. - not char array
   *
   * @tparam T
   * @param c_name Column name
   * @param data Data to override
   * @param nth Position where to override
   * @return TableData& The current table sequence
   */
  template <typename T>
  TableData& UPDATE(char* c_name, const T* data, int nth) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);
      uint8_t type = cols[pos].type;

      int index       = indexMatch(nth);
      int addrToWrite = jumpToColumn(pos, index);

      if (type <= 9) {
        EEPROM.put(addrToWrite, *data);
      }

      currentAction = ACTION_UPDATE;
    }
    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief Used for char array
   *
   * @param c_name Column name
   * @param data Data to override
   * @param nth Position to update
   * @return TableData& The current table sequence
   */
  TableData& UPDATE(char* c_name, char* data, int nth) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);
      uint8_t type = cols[pos].type;

      int index       = indexMatch(nth);
      int addrToWrite = jumpToColumn(pos, index);

      if (type > 9) {
        uint8_t nbBytes = deduceSizeof(type);
        writeCharArray(addrToWrite, nbBytes, data);
      }

      currentAction = ACTION_UPDATE;
    }

    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief Retrieve a data from the table memory. Based on the data
   * type we can retrieve the exact data at position. It returns one
   * data per time - not char array
   *
   * @tparam T
   * @param c_name Column name
   * @param data Data to hold value
   * @param nth Position where to get value
   * @return TableData& The current table sequence
   */
  template <typename T>
  TableData& SELECT(char* c_name, T* data, int nth = 0) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);

      uint8_t amount = amountElt();
      if (amount != 0) {
        int index      = indexMatch(nth);
        int addrToRead = jumpToColumn(pos, index);

        uint8_t type = cols[pos].type;
        if (type <= 9) {
          EEPROM.get(addrToRead, *data);
        }

        currentAction = ACTION_SELECT;
      }
    } else {
      data = NULL;
    }
    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief For char array
   *
   * @param c_name Column name
   * @param data char array to get value
   * @param nth Position where to get value
   * @return TableData& The current table sequence
   */
  TableData& SELECT(char* c_name, char* data, int nth = 0) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      // PRINT(c_name, pos);

      uint8_t amount = amountElt();
      if (amount != 0) {
        int index      = indexMatch(nth);
        int addrToRead = jumpToColumn(pos, index);

        uint8_t type = cols[pos].type;
        if (type > 9) {
          uint8_t nbBytes = deduceSizeof(type);
          readCharArray(addrToRead, nbBytes, data);
        }

        currentAction = ACTION_SELECT;
      }
    } else {
      data = NULL;
    }
    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief Retireve all data possible from the table. If WHERE methods
   * is used, it returns only data that match the FILTER
   *
   * @tparam T
   * @param c_name Column name
   * @param data Array to hold data selected
   * @param nbData amount of data finds. Zero if no data is found or match.
   * @return TableData& The current table sequence
   */
  template <typename T>
  TableData& SELECT_ALL(char* c_name, T* data, int& nbData) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      uint8_t amount = amountElt();
      nbData         = amount;
      if (amount != 0) {
        uint8_t type = cols[pos].type;

        if (type <= 9) {
          for (size_t i = 0; i < amount; i++) {
            int index      = indexMatch(i);
            int addrToRead = jumpToColumn(pos, index);
            EEPROM.get(addrToRead, data[i]);
          }
        }

        currentAction = ACTION_SELECT;
      }
    } else {
      data = NULL;
    }

    return *this;
  }

  /********************************************************************************************************************************/

  /**
   * @brief For char array - Next update of library
   *
   */
  //    TableData& SELECT_ALL(char* c_name, ArrayOfChar* data, int& nbData) {
  //      int pos = isColumnExist(c_name);
  //      if (pos != STATUS_COL_NOT_FOUND) {
  //
  //        uint8_t amount = amountElt();
  //        nbData = amount;
  //        if ( amount != 0 ) {
  //          uint8_t type = cols[pos].type;
  //
  //          if (type  > 9 ) {
  //
  //            for (size_t i = 0; i < amount; i++) {
  //              int index = indexMatch(i);
  //              int addrToRead = jumpToColumn(pos, index);
  //
  //              uint8_t nbBytes = deduceSizeof(type);
  //              readCharArray(addrToRead, nbBytes, data[i].value);
  //            }
  //          }
  //
  //          currentAction = ACTION_SELECT;
  //        }
  //      }
  //
  //      return *this;
  //    }

  /********************************************************************************************************************************/

  /**
   * @brief We can delete a row of the table based on the position. If you apply
   * a WHERE method before, it will delete only data at the position wanted that
   * match the FILTER. One row per time is deleted here.
   *
   * @param nth
   * @return TableData& The current table sequence
   */
  TableData& DELETE(int nth = 0) {
    if (addr < 0) return *this;

    uint8_t amount = amountElt();
    if (amount != 0) {
      uint8_t index = indexMatch(nth);

      eraseRow(index);

      moveAfterDelete(index, amount, nth % amount);
      sizeAfterDelete(1);
      currentAction = ACTION_DELETE;

      debugDisplayindexElt();
    }

    return *this;
  }

  /********************************************************************************************************************************/
  /**
   * @brief Delete all data wanted (if WHERE methods is used before)
   *
   * @param nbData Amount of data deleted
   * @return TableData& The current table sequence
   */
  TableData& DELETE_ALL(int& nbData) {
    if (addr < 0) return *this;

    uint8_t amount = amountElt();
    nbData         = amount;
    if (amount != 0) {
      for (size_t i = 0; i < amount; i++) {
        uint8_t index = indexMatch(0);
        eraseRow(index);

        uint8_t currentAmount = amountElt();
        moveAfterDelete(index, currentAmount, 0);
        sizeAfterDelete(1);
      }

      debugDisplayindexElt();

      // size -= amount;
      currentAction = ACTION_DELETE_ALL;
    }

    return *this;
  }

  /********************************************************************************************************************************/

  /**
   * @brief It's used to filter data from the table. So user can decide to get
   * data that match a condition- numerics data
   *
   * @tparam T
   * @param c_name Column name
   * @param f A filter - enum value
   * @param data Data that column data will be compared
   * @return TableData&
   */
  template <typename T>
  TableData& WHERE(char* c_name, FILTER f, T* data) {
    if (addr < 0) return *this;

    int pos = isColumnExist(c_name);
    if (pos != STATUS_COL_NOT_FOUND) {
      int     addrToRead = jumpToColumn(pos, 0);
      uint8_t nbBytes    = deduceSizeof(cols[pos].type);
      uint8_t amount     = amountElt();

      int* copyIndexElt = malloc(capacity * sizeof(int));
      for (size_t j = 0; j < capacity; j++) {
        copyIndexElt[j] = -1;
      }
      size_t j = 0;

      if (cols[pos].type < 9) {
        for (size_t i = 0; i < amount; i++) {
          uint8_t index = indexMatch(i);

          T dataRead = 0;
          EEPROM.get(addrToRead + index * nbBytes, dataRead);
          // PRINT("--val--", dataRead);

          switch (f) {
            case FILTER::isLessThan: {
              if (dataRead < *data) {
                copyIndexElt[j] = index;
                j += 1;
              }
              break;
            }

            case FILTER::isGreaterThan: {
              if (dataRead > *data) {
                copyIndexElt[j] = index;
                j += 1;
              }

              break;
            }

            case FILTER::isEqualTo: {
              if (dataRead == *data) {
                copyIndexElt[j] = index;
                j += 1;
              }

              break;
            }

            case FILTER::isLessOrEqualTo: {
              if (dataRead <= *data) {
                copyIndexElt[j] = index;
                j += 1;
              }

              break;
            }

            case FILTER::isGreaterOrEqualTo: {
              if (dataRead >= *data) {
                copyIndexElt[j] = index;
                j += 1;
              }

              break;
            }

            case FILTER::isNotEqualTo: {
              if (dataRead != *data) {
                copyIndexElt[j] = index;
                j += 1;
              }

              break;
            }

            default:
              break;
          }
        }
      }
      for (size_t k = 0; k < capacity; k++) {
        indexElt[k] = copyIndexElt[k];
      }

      free(copyIndexElt);

      currentAction = ACTION_WHERE;
    }

    return *this;
  }

  /********************************************************************************************************************************/

  /**
   * @brief Each time that an operation is done on the table, some
   * informations must be updated to keep consistent the current table data
   * So user must be call this function all times he ue: INSERT / SELECT /
   * SELECT_ALL / UPDATE / DELETE / DELETE_ALL
   */
  void DONE() {
    if (currentAction == ACTION_INSERT) {  // INSERTION
      size += 1;

      if (size == (2 * capacity)) {
        size = capacity;
        // PRINT("capacity", capacity);
      }

      EEPROM.write(addr - 5, size);  // -5
    }

    currentAction = ACTION_SELECT;
    initIndexElt();
  }

  /********************************************************************************************************************************/
  //// UTILITIES FUNCTIONS ////
  /**
   * @brief Verify if a column exist based on the name give as argument
   *
   * @param c_name Column name
   * @return int A number superior to zero (as column position in the table) if
   * column is found and inferior if not.
   */
  int isColumnExist(char* c_name) {
    for (size_t i = 0; i < nbCols; i++) {
      if (strcmp(c_name, cols[i].name) == 0) return i;
    }

    return STATUS_COL_NOT_FOUND;
  }

 private:
  /********************************************************************************************************************************/
  /**
   * @brief Allow us to jump directly at a column position in the table
   *
   * @param indexCol Position of the column in the data base
   * @param currentPos Index in this column where we want to operate
   * @return int The column address in the memory
   */
  int jumpToColumn(size_t indexCol, int currentPos) {
    int pos = start + deduceSizeof(cols[indexCol].type) * currentPos;
    for (size_t i = 0; i < indexCol; i++) {
      pos += deduceSizeof(cols[i].type) * (int)capacity;
    }

    return pos;
  }

  /********************************************************************************************************************************/
  /**
   * @brief To retrieve data or delete them, we keep in an array their
   * index in table memory. So when WHERE is applied we just update these index
   * and we can jump to these index in each column for any kind of operation.
   * If WHERE is not applied, we just keep in their order (0 to capcity -1) data
   * index
   *
   */
  void initIndexElt() {
    uint8_t indexToStop = (size >= capacity) ? capacity : size;

    for (size_t i = 0; i < capacity; i++) {
      if (i < indexToStop)
        indexElt[i] = i;
      else
        indexElt[i] = -1;
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief We count the amount of data on which we can work. Read
   * "initIndexElt()" comments
   *
   * @return uint8_t
   */
  uint8_t amountElt() {
    uint8_t maxElt = 0;
    for (size_t i = 0; i < capacity; i++) {
      if (indexElt[i] >= 0) {
        maxElt += 1;
      }
    }

    return maxElt;
  }

  /********************************************************************************************************************************/
  /**
   * @brief We match a give index to the real amount data available to avoid
   * to be out of bound.
   *
   * @param index
   * @return uint8_t
   */
  uint8_t indexMatch(int index) {
    index = index % amountElt();

    return indexElt[index];
  }

  /********************************************************************************************************************************/
  /**
   * @brief In the case of char array, we do some stuff to store these
   * data and retrieve them, as is it is an array.
   *
   * @param from Address wehre we want to store char array
   * @param nbBytes Amount of bytes used to store the char array
   * @param value char array to store
   */
  void writeCharArray(int from, uint8_t nbBytes, char* value) {
    uint8_t len = strlen(value) > nbBytes ? nbBytes : strlen(value);

    for (size_t i = 0; i < len; i++) {
      EEPROM.write(from + i, value[i]);
    }
    EEPROM.write(from + len, '\0');
  }

  /********************************************************************************************************************************/
  /**
   * @brief In the case of char array, we do some stuff to store these
   * data and retrieve them, as is it is an array.
   *
   * @param from Address wehre we want to retrieve char array
   * @param nbBytes Amount of bytes used to retrieve the char array
   * @param value char array to retrieve
   */
  void readCharArray(int from, uint8_t nbBytes, char* value) {
    for (size_t i = 0; i < nbBytes; i++) {
      value[i] = (char)EEPROM.read(from + i);
      if (value[i] == '\0') break;
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief Allow us to move from an index to another data in the same column
   * We move uint8_t per uint8_t
   *
   * @param addrCol Column address
   * @param nbBytes Amount of bytes used
   * @param from Index of the data to move
   * @param to Index of te destination where to move on
   */
  void moveData(int addrCol, uint8_t nbBytes, uint8_t from, uint8_t to) {
    for (size_t i = 0; i < nbBytes; i++) {
      int     fromAddr = addrCol + from * nbBytes + i;
      uint8_t dataRead = EEPROM.read(fromAddr);

      EEPROM.write(addrCol + to * nbBytes + i, dataRead);
      EEPROM.write(fromAddr, ZERO);
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief We can move a full row of a table from a position to other.
   * By row we mean: all data of each column of an index
   * @param from Index of the row to move
   * @param to Index of where to move the row
   */
  void moveRow(uint8_t from, uint8_t to) {
    for (size_t i = 0; i < nbCols; i++) {
      int     pos        = isColumnExist(cols[i].name);
      int     addrToRead = jumpToColumn(pos, 0);
      uint8_t nbBytes    = deduceSizeof(cols[i].type);

      moveData(addrToRead, nbBytes, from, to);
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief After a delete operation we need to move data from an index to
   * another to keep the table data contiguous
   *
   * @param index
   * @param amount
   * @param currIndex
   */
  void moveAfterDelete(uint8_t index, uint8_t amount, uint8_t currIndex) {
    for (size_t i = index + 1; i < capacity; i++) {
      moveRow(i, i - 1);
    }

    reOrderIndexElt(amount, currIndex);
  }

  /********************************************************************************************************************************/
  /**
   * @brief Above we say, we keep only index to work on data, so after
   * a delete operation fro example, we re-order the array that contains these
   * index
   *
   * @param amount Amount of data (index)
   * @param currIndex The index of the element where we want to move on
   */
  void reOrderIndexElt(uint8_t amount, uint8_t currIndex) {
    for (size_t i = currIndex + 1; i < amount; i++) {
      indexElt[i - 1] = indexElt[i] - 1;
    }
    indexElt[amount - 1] = -1;
  }

  /********************************************************************************************************************************/
  /**
   * @brief We can delete a whole row by put at specific index ZER0 (as uint8_t)
   * based on the number of bytes need for each column type
   *
   * @param index
   */
  void eraseRow(uint8_t index) {
    for (size_t i = 0; i < nbCols; i++) {
      int     pos         = isColumnExist(cols[i].name);
      int     addrToWrite = jumpToColumn(pos, index);
      uint8_t nbBytes     = deduceSizeof(cols[i].type);

      for (size_t j = 0; j < nbBytes; j++) {
        EEPROM.put(addrToWrite + j, ZERO);
      }
    }
  }

  /********************************************************************************************************************************/
  /**
   * @brief After a delete operation, we must update the amount of data int he
   * table
   *
   * @param nbDelete Number of data deleted
   */
  void sizeAfterDelete(uint8_t nbDelete = 1) {
    size = size > capacity ? capacity : size;
    size = (size - nbDelete) < 0 ? 0 : (size - nbDelete);
    EEPROM.write(addr - 5, size);
  }

  /********************************************************************************************************************************/
  /**
   * @brief In development mode, we display current index of data on what we
   * operate
   *
   */
  void debugDisplayindexElt() {
    for (size_t i = 0; i < capacity; i++) {
      PRINT("--val--", indexElt[i]);
    }
  }

  /********************************************************************************************************************************/
  /// ATTRIBUTES ///

  int addr = STATUS_TABLE_NOT_FOUND;  // Address of the table in meta data section
                                      // memory - negative number means no table
  char    name[3];                    // Table name -  not more than 2 char
  uint8_t capacity = 0;               // Table capacity - maximum 255
  uint8_t size     = 0;               // The current amount of data stored and re-evaluated after
                                      // all table row are used - not more than "2*capacity"
  uint8_t    modSize = 0;             // size % capacity to not be bound of table capacity
  int        start   = -1;            // Where in storage memory section, table data are stored
  uint8_t    nbCols  = 0;             // Number of columns
  AllColumn* cols    = nullptr;       // Columns informations of the table

  int*    indexElt      = nullptr;        // Array to keep data index and not the data itself
  uint8_t currentAction = ACTION_SELECT;  // 0: nothing; 1: where search
};

/********************************************************************************************************************************/
/**
 * @brief MemoryManager is our main class. It manage memory byte, meta data about
 * the data base, the tables informations, and their data. We access table through its methods.
 * We use commonly its instance named "mem" to work on database
 *
 *
 */
class MemoryManager {
 public:
  /********************************************************************************************************************************/
  /**
   * @brief Construct a new Memory Manager object
   *
   */
  MemoryManager();

  /********************************************************************************************************************************/
  /**
   * @brief We init manager here. If it is the first time, we put in  END_DB the zero value
   *
   */
  void init();

  /********************************************************************************************************************************/
  /**
   * @brief We erase all things, let's clean all tables and their meta data
   *
   */
  void clearAll();

  /********************************************************************************************************************************/
  /**
   * @brief Retrieve the toatal amount of capacity (sum of all capacity)
   *
   * @return int
   */
  int size();

  /********************************************************************************************************************************/
  /**
   * @brief Retrieve where (address) meta data informations are finished
   *
   * @return int
   */
  int sizeMeta();

  /********************************************************************************************************************************/
  /**
   * @brief Retrieve the number of tables in the data abase
   *
   * @return uint8_t
   */
  uint8_t nbTables();

  /********************************************************************************************************************************/
  /**
   * @brief Check if they are enough memory available in the data base to store
   * table data and their meata data.
   *
   * @param need
   * @return int
   */
  int isMemoryAvailable(int need);

  /********************************************************************************************************************************/
  /**
   * @brief Print meta data informations with their name or key
   *
   */
  void printMetaData();

  /********************************************************************************************************************************/
  /**
   * @brief Create a Table object. With this we can create a new table to work on after

  * @param tableName The table name, no more than 2 char
  * @param capacity The maximum data that can be hold by this table. If this value is attended, we
  override old value from the beginning
  * @param col The number of columns of this table
  * @param tableCol The informations about columns: name and size
  * @return uint8_t We return a status: if table is created, or already existed, if capacity is
  exceeded or something else... - see status code
  */
  int CREATE_TABLE(char* tableName, uint8_t capacity, uint8_t col, Column tableCol[]);

  /********************************************************************************************************************************/
  /**
   * @brief Allow us to check the existence of a table by retrieving its addr. If negative address
   * is returned, that mean, not exist.
   *
   * @param tableName The name of table (max 2 char)
   * @return int Return its address. Positif if it existed and so where the table meta data started
   * and negative if it is not existed
   */
  int ON(char* tableName);

  /********************************************************************************************************************************/
  /**
   * @brief This function is used if we wan to set/alter data on the table, insert or update or
   * delete
   *
   * @param tableName The table name
   * @return TableData& A reference to a member data of MemoryAManager to allow chaining when
   * working on the table
   */
  TableData& TO(char* tableName);

  /********************************************************************************************************************************/
  /**
   * @brief Same thing as TO function but here we use it to get/retrieve informations from the table
   *
   * @param tableName Table name
   * @return TableData& TableData& A reference to a member data of MemoryAManager to allow chaining
   * when working on the table
   */
  TableData& FROM(char* tableName);

 private:
  /********************************************************************************************************************************/
  /**
   * @brief Set the size (sum of all capacity) of all tables
   *
   * @param nSize
   */
  void setSize(int nSize);

  /********************************************************************************************************************************/

  /**
   * @brief Set the meta data end emplacement after adding to the read value the new amount of meta
   * data used to store new table infos
   *
   * @param nSizeMeta
   */
  void setSizeMeta(int nSizeMeta);

  /********************************************************************************************************************************/

  /**
   * @brief Set the number of tables
   *
   * @param nbTab
   */
  void setNbTables(uint8_t nbTab);

  /********************************************************************************************************************************/
  /**
   * @brief It is a TableData used by MemoryManager that user can access to chain functions when
   * working on a table
   *
   */
  TableData T_DATA;
};

/********************************************************************************************************************************/
MemoryManager::MemoryManager() { init(); }

void MemoryManager::init() {
  int sz = 0;
  EEPROM.get(ADDR_SIZE_DB, sz);

  if (sz == 0) {
    EEPROM.put(ADDR_SIZE_METADATA, ADDR_TABLES_METADATA);
    setSize(1);
  }
}

void MemoryManager::clearAll() {
  for (int i = 0; i <= END_DB; i++) {
    EEPROM.write(i, ZERO);
  }
  Serial.println("...END...");
}

int MemoryManager::size() {
  int sz = 0;
  EEPROM.get(ADDR_SIZE_DB, sz);
  return sz;
}

void MemoryManager::setSize(int nSize) { EEPROM.put(ADDR_SIZE_DB, nSize); }

int MemoryManager::sizeMeta() {
  int sz = 0;
  EEPROM.get(ADDR_SIZE_METADATA, sz);
  return sz;
}

void MemoryManager::setSizeMeta(int nSizeMeta) { EEPROM.put(ADDR_SIZE_METADATA, nSizeMeta); }

uint8_t MemoryManager::nbTables() {
  uint8_t nbCols = 0;
  nbCols         = EEPROM.read(ADDR_NB_TABLES);
  return nbCols;
}

void MemoryManager::setNbTables(uint8_t nbTab) { EEPROM.write(ADDR_NB_TABLES, nbTab); }

int MemoryManager::isMemoryAvailable(int need) {
  if (need > (sizeMeta() - size())) return STATUS_INSUF_MEMORY;
  return STATUS_SUF_MEMORY;
}

void MemoryManager::printMetaData() {
  // Memory meta data infos
  PRINT("====================", "====================");
  PRINT("nb data", size());

  PRINT("meta", (ADDR_SIZE_DB - sizeMeta()));
  PRINT(" && start at", ADDR_SIZE_DB);
  PRINT(" && stop at", sizeMeta() + 1);

  PRINT("nb tables", nbTables());

  PRINT("-", "-");
  // Tables meta data
  int szMeta = ADDR_TABLES_METADATA;
  for (size_t i = 0; i < nbTables(); i++) {
    char t_name[3];
    t_name[0] = char(EEPROM.read(szMeta - 0));
    t_name[1] = char(EEPROM.read(szMeta - 1));
    t_name[2] = '\0';  // char(EEPROM.read(szMeta - 2));
    PRINT("Table", t_name);

    // PRINT("addr - start", szMeta - 3);
    int t_start = 0;
    EEPROM.get(szMeta - 3, t_start);
    PRINT("start at", t_start);

    // PRINT("addr - sz", szMeta - 5);
    uint8_t t_sz = EEPROM.read(szMeta - 5);
    PRINT("size", t_sz);

    // PRINT("addr - cap", szMeta - 6);
    uint8_t t_cap = EEPROM.read(szMeta - 6);
    PRINT("capacity", t_cap);

    uint8_t t_col = EEPROM.read(szMeta - 7);
    PRINT("nb col", t_col);

    szMeta -= 8;
    for (size_t k = 0; k < t_col; k++) {
      char c_name[3];
      c_name[0] = char(EEPROM.read(szMeta - 0));
      c_name[1] = char(EEPROM.read(szMeta - 1));
      c_name[2] = '\0';  // char(EEPROM.read(szMeta - 2));

      PRINT("column", k + 1);
      PRINT("name", c_name);

      uint8_t c_sz = EEPROM.read(szMeta - 3);
      PRINT("sizeof", deduceSizeof(c_sz));

      szMeta -= 4;
    }
    PRINT("-", "-");
  }

  PRINT("====================", "====================");
}

int MemoryManager::CREATE_TABLE(char* tableName, uint8_t capacity, uint8_t col, Column tableCol[]) {
  if (capacity > MAX_CAP) return STATUS_MAX_CAP_EXCEEDED;

  int dataMemoryNeed = 0;  // to calculate new space needs to stores this table data
  for (size_t i = 0; i < col; i++) {
    dataMemoryNeed += deduceSizeof(typeIDNum(tableCol[i].type));
  }

  dataMemoryNeed *= capacity;

  int metaMemoryNeed = 4 * col + BYTES_TABLES_GEN_METADATA;

  if (isMemoryAvailable(dataMemoryNeed + metaMemoryNeed) != STATUS_SUF_MEMORY) {
    return STATUS_INSUF_MEMORY;
  }

  if (ON(tableName) != STATUS_TABLE_NOT_FOUND) {
    return STATUS_TABLE_EXIST;
  }

  // Start storage
  int     sz     = size() + dataMemoryNeed;
  int     szMeta = sizeMeta();
  uint8_t nbT    = nbTables() + 1;

  // PRINT("szMeta", szMeta);

  // Store table name
  tableName[2] = '\0';
  for (int i = 0; i < 2; i++) {
    int pos = szMeta - i;
    // PRINT("pos", pos);
    EEPROM.write(pos, tableName[i]);
  }
  szMeta -= 3;

  // Store table start storage data
  // PRINT("t_start", szMeta);
  int start = size();
  EEPROM.put(szMeta, start);
  szMeta -= 2;

  // Store table size
  // PRINT("t_sz", szMeta);
  uint8_t t_sz = 0;
  EEPROM.write(szMeta, t_sz);
  szMeta -= 1;

  // Store table capacity
  // PRINT("cap", szMeta);
  EEPROM.write(szMeta, capacity);
  szMeta -= 1;

  // Store nb cols
  // PRINT("t_nb", szMeta);
  EEPROM.write(szMeta, col);
  szMeta -= 1;

  // cols data
  // PRINT("t_cols", szMeta);
  for (size_t i = 0; i < col; i++) {
    EEPROM.write(szMeta - 0, tableCol[i].name[0]);
    EEPROM.write(szMeta - 1, tableCol[i].name[1]);
    // EEPROM.write(szMeta - 2, '\0');

    EEPROM.write(szMeta - 3, typeIDNum(tableCol[i].type));

    szMeta -= 4;
  }
  // PRINT("new", szMeta);

  // Update memory manager meta data infos
  setSize(sz);
  setSizeMeta(szMeta);
  setNbTables(nbT);

  return STATUS_TABLE_CREATED;
}

int MemoryManager::ON(char* tableName) {
  int szMeta = ADDR_TABLES_METADATA;
  for (size_t i = 0; i < nbTables(); i++) {
    char t_name[3];
    t_name[0] = char(EEPROM.read(szMeta - 0));
    t_name[1] = char(EEPROM.read(szMeta - 1));
    t_name[2] = '\0';

    uint8_t t_col = EEPROM.read(szMeta - 7);

    // PRINT("taddr", szMeta);
    if (strcmp(t_name, tableName) == 0) return szMeta;

    szMeta -= (BYTES_TABLES_GEN_METADATA + t_col * 4);
  }

  // PRINT("found", STATUS_TABLE_NOT_FOUND);
  return STATUS_TABLE_NOT_FOUND;
}

TableData& MemoryManager::TO(char* tableName) {
  int result = ON(tableName);
  // PRINT("r", result);

  T_DATA = TableData(result);
  return T_DATA;
}

TableData& MemoryManager::FROM(char* tableName) {
  int result = ON(tableName);

  T_DATA = TableData(result);
  return T_DATA;
}

#endif
