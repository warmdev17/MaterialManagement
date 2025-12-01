#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

// escape sequence for terminal color
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

#define USE_MATERIAL_TEST_DATA 0
#define USE_TRANSACTION_TEST_DATA 0

#define MAX_LIST_SIZE 100
#define MAX_TRANS_SIZE 500

typedef struct {
  char matId[10];
  char name[50];
  int qty; // quantity in storage
  char unit[10];
  int status; // 1. active | 0. expired
} Material;

typedef struct {
  char transId[20];
  char matId[10];
  char type[5];  // input/output
  char date[15]; // transaction time
} Transaction;

// ======= PROTOTYPES =======
void displayMenu();
void initTestMaterialData(Material **materials, int *materialCount);
void initTestTransData(Transaction **transactions, int *transCount);

void readValidLine(char *buffer, size_t size, char *announce, char *valueType);
void readInt(int *number, char *announce, char *valueType);

void createNewMaterial(Material **materials, int *materialCount);
void updateMaterial(Material *materials, int materialCount);
void updateMaterialStatus(Material *materials, int materialCount);
int readStatusWithDefault();
int findMaterialByID(Material *materials, int materialCount, char *target);
void findMaterialByName(Material *materials, int materialCount, char *target);
int findMaterialIndexById(Material *m, char *id, int materialCount);
void findMaterialByIdOrName(Material *materials, int materialCount);
void sortMaterial(Material *materials, int materialCount);

void displayMaterialList(Material *materials, int materialCount);
void printMaterialPage(Material *materials, int materialCount, int page,
                       int pageSize);
void showCurrentInfo(Material *materials, int idx);

void createNewTransaction(Transaction **transactions, int *transactionCount,
                          Material *materials, int materialCount,
                          char *transID);
void transferMaterial(Transaction **transactions, Material *materials,
                      int *transactionCount, int materialCount, char *id,
                      int type,
                      char *transID); // type 1: import | type 2: export
void displayTransactionByID(Transaction *transactions, int transactionCount);
void findTransactionByID(Transaction *transactions, int transactionCount);
Transaction generateTransferHistory(char *matID, char *transID, int type);

// ======= Log with color =======
void logToConsole(char *type, char *log) {
  if (strcmp(type, "error") == 0) {
    printf(RED "%s" RESET, log);
  } else if (strcmp(type, "choosen") == 0) {
    printf(YELLOW "%s" RESET, log);
  } else if (strcmp(type, "border") == 0) {
    printf(GREEN "%s" RESET, log);
  } else if (strcmp(type, "announce") == 0) {
    printf(BLUE "%s" RESET, log);
  }
}

// ======= MENU =======
void displayMenu() {
  logToConsole(
      "border",
      "=============================================================\n");
  logToConsole("choosen", " 1. Add new material\n");
  logToConsole("choosen", " 2. Update material info\n");
  logToConsole("choosen", " 3. Update material status\n");
  logToConsole("choosen", " 4. Find material by ID/Name\n");
  logToConsole("choosen", " 5. Display material list\n");
  logToConsole("choosen", " 6. Sort material list\n");
  logToConsole("choosen", " 7. Make a transfer\n");
  logToConsole("choosen", " 8. View transaction history\n");
  logToConsole("choosen", " 9. Clear screen\n");
  logToConsole("choosen", "10. Exit\n");
  logToConsole(
      "border",
      "=============================================================\n");
}

// ======= MAIN =======
int main() {
  Material *materials = NULL;
  Transaction *transaction = NULL;

  char initTransID[20] = "T000";

  int materialCount = 0;
  int transactionCount = 0;

#if USE_MATERIAL_TEST_DATA
  initTestMaterialData(&materials, &materialCount);
#endif

#if USE_TRANSACTION_TEST_DATA
  initTestTransData(&transaction, &transactionCount);
#endif

  if (transactionCount > 0) {
    char lastID[20];
    strcpy(lastID, transaction[transactionCount - 1].transId);

    int number = atoi(lastID + 1);
    number++;
    char prefix = lastID[0];

    sprintf(initTransID, "%c%03d", prefix, number);
  }

  int choice;
  do {
    displayMenu();
    readInt(&choice, "Enter your choice: ", "Choice");

    switch (choice) {
    case 1: {
      createNewMaterial(&materials, &materialCount);
      break;
    }
    case 2: {
      updateMaterial(materials, materialCount);
      break;
    }
    case 3: {
      updateMaterialStatus(materials, materialCount);
      break;
    }
    case 4: {
      findMaterialByIdOrName(materials, materialCount);
      break;
    }
    case 5: {
      displayMaterialList(materials, materialCount);
      break;
    }
    case 6: {
      sortMaterial(materials, materialCount);
      break;
    }
    case 7: {
      createNewTransaction(&transaction, &transactionCount, materials,
                           materialCount, initTransID);
      break;
    }
    case 8: {
      findTransactionByID(transaction, transactionCount);
      break;
    }
    case 9: {
      system("clear");
      break;
    }
    case 10: {
      logToConsole("announce", "Exiting program...\n");
      break;
    }
    default: {
      logToConsole("error", "Invalid choice, please try again.\n\n");
      break;
    }
    }
  } while (choice != 10);

  free(materials);
  free(transaction);
  return 0;
}

// ======= INPUT/OUTPUT helper =======
void readValidLine(char *buffer, size_t size, char *announce, char *valueType) {
  while (1) {
    printf("%s", announce);

    if (fgets(buffer, size, stdin) == NULL) {
      logToConsole("error", "Error reading input.\n");
      continue;
    }

    size_t len = strlen(buffer);

    // buffer does not contain '\n'
    // this mean user maybe type more than (size - 1) characters.
    if (len > 0 && buffer[len - 1] != '\n') {
      int ch;
      int tooLong = 0;

      // flush the remaining characters in stdin
      // if we see anything before '\n', the input was actually too long.
      while ((ch = getchar()) != '\n' && ch != EOF) {
        tooLong = 1;
      }

      if (tooLong) {
        printf(
            RED
            "%s is too long (max %zu characters). Please type again.\n" RESET,
            valueType, size - 1);
        continue; // ask user to re-enter
      }
      // if 'tooLong' is 0 user type exactly (size - 1) chars + Enter
      // valid : continue processing normally
    }

    // Remove '\n' if it exists
    buffer[strcspn(buffer, "\n")] = '\0';

    // check if the input is empty or only space
    int isEmpty = 1;
    for (int i = 0; buffer[i] != '\0'; i++) {
      if (!isspace((unsigned char)buffer[i])) {
        isEmpty = 0;
        break;
      }
    }

    if (isEmpty) {
      printf(RED "%s cannot be empty. Please type again.\n" RESET, valueType);
      continue;
    }

    return;
  }
}

void readInt(int *number, char *announce, char *valueType) {
  while (1) {
    char input[1000];
    char extra; // to check char after number

    printf("%s", announce);

    if (fgets(input, sizeof(input), stdin) == NULL) {
      printf(RED "Error reading %s, please try again.\n" RESET, valueType);
      continue;
    }

    input[strcspn(input, "\n")] = '\0';

    if (strlen(input) == 0) {
      printf(RED "%s cannot be empty, please type again.\n" RESET, valueType);
      continue;
    }

    if (sscanf(input, "%d %c", number, &extra) != 1) {
      printf(RED "Invalid %s, please type again.\n" RESET, valueType);
      continue;
    }

    if (*number < 0) {
      printf(RED "%s must be greater or equal zero, please type again.\n" RESET,
             valueType);
      continue;
    }

    break;
  }
}

// ======= Material management helper =======
void createNewMaterial(Material **materials, int *materialCount) {
  if (*materialCount >= MAX_LIST_SIZE) {
    printf(RED "Material list reached max size (%d). Cannot add more.\n" RESET,
           MAX_LIST_SIZE);
    return;
  }

  int idxMaterial = *materialCount;
  (*materialCount)++;

  // reallocate
  Material *temp = realloc(*materials, *materialCount * sizeof(Material));
  if (temp == NULL) {
    logToConsole("error", "Allocate failed\n");
    (*materialCount)--;
    return;
  }
  *materials = temp;

  // get material id (must be unique)
  do {
    readValidLine((*materials + idxMaterial)->matId,
                  sizeof((*materials + idxMaterial)->matId),
                  "Enter id of material: ", "ID");

    if (findMaterialIndexById(*materials, (*materials + idxMaterial)->matId,
                              idxMaterial) != -1) {
      logToConsole("error", "\nID must not duplicate existing material ID, "
                            "please try again!\n");
    } else {
      break;
    }
  } while (1);

  // get material name
  readValidLine((*materials + idxMaterial)->name,
                sizeof((*materials + idxMaterial)->name),
                "Enter name of material: ", "Name");

  // get materials inventory quantity
  readInt(&(*materials + idxMaterial)->qty,
          "Enter inventory quantity ( must be greater than 0 ): ", "quantity");

  // get material unit
  readValidLine((*materials + idxMaterial)->unit,
                sizeof((*materials + idxMaterial)->unit),
                "Enter unit of materials: ", "Unit");

  // default status 1 is active
  (*materials + idxMaterial)->status = readStatusWithDefault();

  logToConsole("announce", "\nAdd new material successfully\n\n");
}

// ======= Create new transaction =======
void createNewTransaction(Transaction **transactions, int *transactionCount,
                          Material *materials, int materialCount,
                          char *transID) {
  int mode;
  char id[10];

  while (1) {
    logToConsole("border", "====================\n");
    logToConsole("choosen", "1. Import material\n");
    logToConsole("choosen", "2. Export material\n");
    logToConsole("choosen", "3. Exit to main menu\n");
    logToConsole("border", "====================\n");

    readInt(&mode, "Enter mode: ", "mode");

    // back to main menu
    if (mode == 3) {
      system("clear");
      break;
    }

    // invalid mode
    if (mode != 1 && mode != 2) {
      logToConsole("error", "Invalid mode. Please choose 1, 2 or 3.\n");
      continue;
    }

    // import/export
    while (1) {
      readValidLine(id, sizeof(id), "Enter id of material: ", "ID");

      int idx = findMaterialIndexById(materials, id, materialCount);
      if (idx == -1) {
        logToConsole("error", "ID not found in material list\n");
        continue;
      }

      // 0/expired -> cannot transfer
      if (materials[idx].status == 0) {
        logToConsole(
            "error",
            "This material is locked/expired. Cannot make a transaction.\n");
        continue; // enter other id
      }

      transferMaterial(transactions, materials, transactionCount, materialCount,
                       id, mode, transID);
      break;
    }
  }
}

// ======= Transfer material =======
void transferMaterial(Transaction **transactions, Material *materials,
                      int *transactionCount, int materialCount, char *id,
                      int type, char *transId) {
  if (*transactionCount >= MAX_TRANS_SIZE) {
    printf(RED "Transaction list reached max size (%d). Cannot add more!" RESET,
           MAX_LIST_SIZE);
    return;
  }
  int idxTransaction = *transactionCount;
  (*transactionCount)++;

  // reacollate transaction list
  Transaction *temp =
      realloc(*transactions, *transactionCount * sizeof(Transaction));
  if (temp == NULL) {
    logToConsole("error", "Allocate failed\n");
    (*transactionCount)--;
    return;
  }
  *transactions = temp;

  int transCount = 0;

  for (int i = 0; i < materialCount; i++) {
    if (strcasecmp(materials[i].matId, id) == 0) {
      if (type == 1) {
        showCurrentInfo(materials, i);
        // import
        do {
          readInt(
              &transCount,
              "Enter amount of material to import ( must be greater than 0 ): ",
              "Amount of material");
          if (transCount <= 0) {
            logToConsole("error", "Amount must be greater than zero.\n");
          }
        } while (transCount <= 0);
        materials[i].qty += transCount;
        (*transactions)[idxTransaction] =
            generateTransferHistory(materials[i].matId, transId, type);
        showCurrentInfo(materials, i);
      } else {
        // export
        do {
          showCurrentInfo(materials, i);

          readInt(
              &transCount,
              "Enter amount of material to export ( must be greater than 0 ): ",
              "Amount of material");
          if (transCount <= 0) {
            logToConsole("error", "Amount must be greater than zero.\n");
            continue;
          }
          if (transCount > materials[i].qty) {
            logToConsole("error", "The quantity of materials exceeds the "
                                  "quantity on hand. Please type again!\n");
            continue;
          } else {
            materials[i].qty -= transCount;
            (*transactions)[idxTransaction] =
                generateTransferHistory(materials[i].matId, transId, type);
            showCurrentInfo(materials, i);
            break;
          }
        } while (1);
      }
    }
  }
}

// ======= Generate transfer history ========
Transaction generateTransferHistory(char *matID, char *transID, int type) {
  Transaction transactions;

  char prefix = transID[0];
  int number = atoi(transID + 1);
  number++;
  sprintf(transID, "%c%03d", prefix, number);
  strcpy(transactions.transId, transID);

  strcpy(transactions.matId, matID);
  if (type == 1) {
    strcpy(transactions.type, "IN");
  } else {
    strcpy(transactions.type, "OUT");
  }

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  char dateStr[11];
  strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", t);

  strcpy(transactions.date, dateStr);

  return transactions;
}

// ======= Update material via ID =======
void updateMaterial(Material *materials, int materialCount) {
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty. Nothing to update.\n\n");
    return;
  }

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to update: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    logToConsole("error", "Material with this ID was not found.\n\n");
    return;
  }

  // show current info
  showCurrentInfo(materials, idx);

  readValidLine(materials[idx].name, sizeof(materials[idx].name),
                "Enter new name: ", "Name");
  readValidLine(materials[idx].unit, sizeof(materials[idx].unit),
                "Enter new unit: ", "Unit");
  readInt(&materials[idx].qty, "Enter new quantity: ", "quantity");

  printf(BLUE "\nUpdate material with ID %s successfully.\n" RESET, id);

  showCurrentInfo(materials, idx);
}

// ==== UPDATE STATUS ====
void updateMaterialStatus(Material *materials, int materialCount) {
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty.\n\n");
    return;
  }

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to toggle status: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    logToConsole("error", "Material with this ID was not found.\n\n");
    return;
  }

  materials[idx].status = !materials[idx].status;

  printf(BLUE "Status toggled successfully! New status: %s\n" RESET,
         (materials[idx].status ? "Active" : "Expired"));
}

// ===== read material status with validation =====
int readStatusWithDefault() {
  char line[20];

  while (1) {
    printf("Enter status (0 = expired, 1 = active, empty = default active): ");

    if (fgets(line, sizeof(line), stdin) == NULL) {
      logToConsole("error", "Error reading input. Try again.\n");
      continue;
    }

    // remove '\n'
    line[strcspn(line, "\n")] = '\0';

    // empty -> default
    if (strlen(line) == 0) {
      return 1; // default Active
    }

    // check valid 0 or 1
    if (strcmp(line, "0") == 0)
      return 0;
    if (strcmp(line, "1") == 0)
      return 1;

    // invalid input
    logToConsole("error", "Status must be 0 or 1. Please type again.\n");
  }
}

// ===== Find by ID or Name ====
void findMaterialByIdOrName(Material *materials, int materialCount) {
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty.\n\n");
    return;
  }

  char target[50];
  readValidLine(target, sizeof(target), "Enter ID or Name to find: ", "target");

  int idx = findMaterialByID(materials, materialCount, target);

  if (idx != -1) {
    showCurrentInfo(materials, idx);
  } else {
    findMaterialByName(materials, materialCount, target);
  }
}

// ===== Find material by id ===== ( absolute id )
int findMaterialByID(Material *materials, int materialCount, char *target) {
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty.\n\n");
    return -1;
  }

  int idx = findMaterialIndexById(materials, target, materialCount);
  if (idx == -1) {
    logToConsole("error", "Material with this ID was not found.\n\n");
    return -1;
  }

  showCurrentInfo(materials, idx);
  return idx;
}

// char to lowercase
// subtring checking
int containsIgnoreCase(char *haystack, char *needle) {
  if (*needle == '\0')
    return 1;

  size_t lenH = strlen(haystack);
  size_t lenN = strlen(needle);
  if (lenN > lenH)
    return 0;

  for (size_t i = 0; i <= lenH - lenN; i++) {
    size_t j = 0;
    while (j < lenN && tolower((unsigned char)haystack[i + j]) ==
                           tolower((unsigned char)needle[j])) {
      j++;
    }
    if (j == lenN)
      return 1;
  }
  return 0;
}

// ==== Find material by name (substring, case-insensitive) ====
void findMaterialByName(Material *materials, int materialCount, char *target) {
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty!");
    return;
  }
  Material *m = malloc(materialCount * sizeof(Material));
  if (m == NULL) {
    logToConsole("error", "Memory allocation failed.\n");
    return;
  }

  int count = 0;

  logToConsole("border", "\nSearch results:\n");

  for (int i = 0; i < materialCount; i++) {
    if (containsIgnoreCase(materials[i].name, target)) {
      m[count] = materials[i];
      count++;
    }
  }

  if (count > 0) {
    displayMaterialList(m, count);
  } else {
    logToConsole("error", "No material matched this name.\n\n");
  }
  free(m);
}

// show current material info
void showCurrentInfo(Material *materials, int idx) {
  logToConsole("border", "\nCurrent information:\n");
  printf("ID     : %s\n", materials[idx].matId);
  printf("Name   : %s\n", materials[idx].name);
  printf("Unit   : %s\n", materials[idx].unit);
  printf("Qty    : %d\n", materials[idx].qty);
  printf("Status : %s\n\n",
         (materials[idx].status == 1) ? "Active" : "Expired");
}

// find exist material id
int findMaterialIndexById(Material *m, char *id, int count) {
  for (int i = 0; i < count; i++) {
    if (strcmp(m[i].matId, id) == 0) {
      return i;
    }
  }
  return -1;
}

// ===== Display material list =====
void printMaterialPage(Material *materials, int materialCount, int page,
                       int pageSize) {
  int start = page * pageSize;
  int end = start + pageSize;

  if (start >= materialCount)
    return;
  if (end > materialCount)
    end = materialCount;

  printf("\n+------+------------+-----------------------------------+----------"
         "+------------+------------+\n");
  printf("|  No  |  Mat ID    | Name                              |   Qty    | "
         "  Unit     |  Status    |\n");
  printf("+------+------------+-----------------------------------+----------+-"
         "-----------+------------+\n");

  for (int i = start; i < end; i++) {
    char *result = (materials[i].status == 1) ? "Active" : "Expired";
    printf("| %4d | %-10s | %-33s | %8d | %-10s | %-10s |\n", i + 1,
           materials[i].matId, materials[i].name, materials[i].qty,
           materials[i].unit, result);
  }

  printf("+------+------------+-----------------------------------+----------+-"
         "-----------+------------+\n");
  printf("Page %d / %d\n\n", page + 1,
         (materialCount + pageSize - 1) / pageSize);
}

void displayMaterialList(Material *materials, int materialCount) {
  if (materialCount == 0) {
    logToConsole("error", "\nMaterial list is empty.\n\n");
    return;
  }

  int pageSize = 10; // 2, 3, 5
  int totalPages = (materialCount + pageSize - 1) / pageSize;

  int currentPage = 1;

  while (1) {
    system("clear");

    logToConsole("border", "MATERIAL LIST\n");
    printf("Total materials: %d\n", materialCount);

    printMaterialPage(materials, materialCount, currentPage - 1, pageSize);

    printf("You are on page %d of %d.\n", currentPage, totalPages);

    int pageToView;
    readInt(&pageToView, "Enter page to view (0 = back to menu): ", "page");

    if (pageToView == 0) {
      break;
    }

    if (pageToView < 0 || pageToView > totalPages) {
      printf(RED "Page must be between 1 and %d.\n" RESET, totalPages);
      printf("Press Enter to continue...");
      getchar();
      continue;
    }

    currentPage = pageToView;
  }
}

// ===== Display transaction list =====
void printTransactionPage(Transaction *transactions, int transactionCount,
                          int page, int pageSize) {
  int start = page * pageSize;
  int end = start + pageSize;

  if (start >= transactionCount)
    return;
  if (end > transactionCount)
    end = transactionCount;

  printf("\n+------+------------+------------+------------+--------+\n");
  printf("| No   | Trans ID   | Mat ID     | Date       | Type   |\n");
  printf("+------+------------+------------+------------+--------+\n");

  for (int i = start; i < end; i++) {
    printf("| %4d | %-10s | %-10s | %-10s | %-6s |\n", i + 1,
           transactions[i].transId, transactions[i].matId, transactions[i].date,
           transactions[i].type);
  }

  printf("+------+------------+------------+------------+--------+\n");
  printf("Page %d / %d\n\n", page + 1,
         (transactionCount + pageSize - 1) / pageSize);
}

void displayTransactionByID(Transaction *transactions, int transactionCount) {
  if (transactionCount == 0) {
    logToConsole("error", "\nTransaction list is empty.\n\n");
    return;
  }

  int pageSize = 3; // 2, 3, 5
  int totalPages = (transactionCount + pageSize - 1) / pageSize;

  int currentPage = 1;

  while (1) {
    system("clear");

    logToConsole("border", "TRANSACTION LIST\n");
    printf("Total transaction: %d\n", transactionCount);

    printTransactionPage(transactions, transactionCount, currentPage - 1,
                         pageSize);

    printf("You are on page %d of %d.\n", currentPage, totalPages);

    int pageToView;
    readInt(&pageToView, "Enter page to view (0 = back to menu): ", "page");

    if (pageToView == 0) {
      break;
    }

    if (pageToView < 0 || pageToView > totalPages) {
      printf(RED "Page must be between 1 and %d.\n" RESET, totalPages);
      printf("Press Enter to continue...");
      getchar();
      continue;
    }

    currentPage = pageToView;
  }
}

// ===== sortMaterial ===== (by name, by quantity)
void sortMaterial(Material *materials, int materialCount) {
  int mode;
  do {
    logToConsole("border", "===============\n");
    logToConsole("choosen", "1. Sort by name (a-z)\n");
    logToConsole("choosen", "2. Sort by quantity ( ascending )\n");
    logToConsole("choosen", "3. Back to main menu\n");
    logToConsole("border", "===============\n");
    readInt(&mode, "Enter mode to sort: ", "mode");
    switch (mode) {

    // sort by name with bubble sort
    case 1: {
      for (int i = 0; i < materialCount - 1; i++) {
        bool isSwap = false;
        for (int j = 0; j < materialCount - i - 1; j++) {
          if (strcasecmp(materials[j].name, materials[j + 1].name) > 0) {
            Material tmp = materials[j];
            materials[j] = materials[j + 1];
            materials[j + 1] = tmp;
            isSwap = true;
          }
        }
        if (!isSwap) {
          break;
        }
      }
      displayMaterialList(materials, materialCount);
      break;
    }

    // sort by quantity with bubble sort
    case 2: {
      for (int i = 0; i < materialCount - 1; i++) {
        bool isSwap = false;
        for (int j = 0; j < materialCount - i - 1; j++) {
          if (materials[j].qty > materials[j + 1].qty) {
            Material tmp = materials[j];
            materials[j] = materials[j + 1];
            materials[j + 1] = tmp;
            isSwap = true;
          }
        }
        if (!isSwap) {
          break;
        }
      }
      displayMaterialList(materials, materialCount);
      break;
    }
    case 3: {
      system("clear");
      return;
    }
    default: {
      logToConsole("error", "Invalid mode, please type again.\n");
      break;
    }
    }
  } while (mode != 3);
}

void findTransactionByID(Transaction *transactions, int transactionCount) {
  if (transactionCount == 0) {
    logToConsole("error", "\nTransaction list is empty.\n\n");
    return;
  }

  char matId[50];
  readValidLine(
      matId, sizeof(matId),
      "Enter material ID to find transaction history: ", "Material ID");

  Transaction *trans = malloc(transactionCount * sizeof(Transaction));
  if (trans == NULL) {
    logToConsole("error", "Memory allocation failed.\n");
    return;
  }

  int count = 0;

  for (int i = 0; i < transactionCount; i++) {
    if (strcasecmp(transactions[i].matId, matId) == 0) {
      trans[count] = transactions[i];
      count++;
    }
  }

  if (count > 0) {
    displayTransactionByID(trans, count);
  } else {
    logToConsole("error", "No transaction found for this material ID.\n\n");
  }

  free(trans);
}

void initTestMaterialData(Material **materials, int *materialCount) {
  Material testData[] = {
      {"M001", "Bolt 8mm", 120, "pcs", 1},
      {"M002", "Bolt 10mm", 95, "pcs", 1},
      {"M003", "Nut 8mm", 200, "pcs", 1},
      {"M004", "Nut 10mm", 180, "pcs", 1},
      {"M005", "Steel Plate A3", 50, "kg", 1},
      {"M006", "Steel Plate A4", 30, "kg", 1},
      {"M007", "Cable Type-C", 60, "pcs", 1},
      {"M008", "Cable Type-A", 40, "pcs", 0},
      {"M009", "Pipe 20mm", 75, "m", 1},
      {"M010", "Pipe 30mm", 45, "m", 1},

      {"M011", "Washer 8mm", 300, "pcs", 1},
      {"M012", "Washer 10mm", 260, "pcs", 0},
      {"M013", "Screw 3cm", 500, "pcs", 1},
      {"M014", "Screw 5cm", 350, "pcs", 1},
      {"M015", "Iron Bar 6mm", 80, "m", 1},
      {"M016", "Iron Bar 8mm", 70, "m", 1},
      {"M017", "Iron Bar 10mm", 65, "m", 0},
      {"M018", "Paint Red", 20, "l", 1},
      {"M019", "Paint Blue", 25, "l", 1},
      {"M020", "Paint White", 15, "l", 0},

      {"M021", "PVC Glue", 12, "bottle", 1},
      {"M022", "Contact Glue", 7, "bottle", 1},
      {"M023", "Bearing 608", 44, "pcs", 1},
  };

  int testCount = sizeof(testData) / sizeof(testData[0]);

  Material *tmp = malloc(testCount * sizeof(Material));
  if (tmp == NULL) {
    printf(RED "Allocate test data failed\n" RESET);
    return;
  }

  memcpy(tmp, testData, testCount * sizeof(Material));

  *materials = tmp;
  *materialCount = testCount;
}

void initTestTransData(Transaction **transactions, int *transCount) {
  Transaction testData[] = {
      {"T001", "M001", "in", "01/02/2025"},
      {"T002", "M002", "out", "01/02/2025"},
      {"T003", "M003", "in", "02/02/2025"},
      {"T004", "M005", "in", "03/02/2025"},
      {"T005", "M007", "out", "03/02/2025"},
      {"T006", "M010", "in", "04/02/2025"},
      {"T007", "M011", "out", "04/02/2025"},
      {"T008", "M014", "in", "05/02/2025"},
      {"T009", "M018", "out", "05/02/2025"},
      {"T010", "M023", "in", "06/02/2025"},
  };

  int count = sizeof(testData) / sizeof(testData[0]);

  Transaction *tmp = malloc(count * sizeof(Transaction));
  if (tmp == NULL) {
    printf(RED "Allocate transaction test data failed\n" RESET);
    return;
  }

  memcpy(tmp, testData, count * sizeof(Transaction));

  *transactions = tmp;
  *transCount = count;
}
