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

#define USE_TEST_DATA 1

#define MAX_LIST_SIZE 100

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
void initTestData(Material **materials, int *materialCount);

void readValidLine(char *buffer, size_t size, char *announce, char *valueType);
void readInt(int *number, char *announce, char *valueType);

void createNewMaterial(Material **materials, int *materialCount);
void updateMaterial(Material *materials, int materialCount);
void updateMaterialStatus(Material *materials, int materialCount);
int readStatusWithDefault();
int findByID(Material *materials, int materialCount, char *target);
void findByName(Material *materials, int materialCount, char *target);
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
void displayTransactionList(Transaction *transactions, int transactionIdx);
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
  logToConsole("choosen", " 8. Search transactions\n");
  logToConsole("choosen", "10. Exit\n");
  logToConsole("choosen", "11. Clear screen\n");
  logToConsole(
      "border",
      "=============================================================\n");
}

// ======= MAIN =======
int main() {
  Material *materials = NULL;
  Transaction *transaction = NULL;

  char firstTransID[20] = "T001";

  int materialCount = 0;
  int transactionCount = 0;

#if USE_TEST_DATA
  initTestData(&materials, &materialCount);
#endif

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
                           materialCount, firstTransID);
      break;
    }
    case 8: {
      displayTransactionByID(transaction, transactionCount);
      break;
    }
    case 9: {
      displayTransactionList(transaction, transactionCount);
      break;
    }
    case 11: {
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
      logToConsole("error", "Error reading input\n");
      continue;
    }

    // replace \n at the end of string by '\0'
    buffer[strcspn(buffer, "\n")] = '\0';

    int isEmpty = 1;
    for (int i = 0; buffer[i] != '\0'; i++) {
      if (!isspace((unsigned char)buffer[i])) {
        isEmpty = 0;
        return; // valid (has non-space char)
      }
    }

    if (isEmpty) {
      printf(RED "%s cannot be empty. Please type again!!!\n" RESET, valueType);
      continue;
    }

    return;
  }
}

void readInt(int *number, char *announce, char *valueType) {
  while (1) {
    char input[1000];

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

    if (sscanf(input, "%d", number) != 1) {
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
          "Enter inventory quantity: ", "quantity");

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
  if (*transactionCount > MAX_LIST_SIZE) {
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
          readInt(&transCount,
                  "Enter amount of material to import: ", "Amount of material");
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

          readInt(&transCount,
                  "Enter amount of material to export: ", "Amount of material");
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
  sprintf(transID, "%c%03d", prefix, number);
  number++;
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

  int idx = findByID(materials, materialCount, target);

  if (idx != -1) {
    showCurrentInfo(materials, idx);
  } else {
    findByName(materials, materialCount, target);
  }
}

// ===== Find material by id ===== ( absolute id )
int findByID(Material *materials, int materialCount, char *target) {
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
int charToLower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 'a';
  return c;
}

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
    while (j < lenN && charToLower((unsigned char)haystack[i + j]) ==
                           charToLower((unsigned char)needle[j])) {
      j++;
    }
    if (j == lenN)
      return 1;
  }
  return 0;
}

// ==== Find material by name (substring, case-insensitive) ====
void findByName(Material *materials, int materialCount, char *target) {
  Material m[100];
  int count = 0;
  if (materialCount == 0) {
    logToConsole("error", "Material list is empty.\n\n");
    return;
  }

  int found = 0;
  logToConsole("border", "\nSearch results:\n");

  for (int i = 0; i < materialCount; i++) {
    if (containsIgnoreCase(materials[i].name, target)) {
      found = 1;
      m[count] = materials[i];
      count++;
    }
  }

  if (!found) {
    logToConsole("error", "No material matched this name.\n\n");
  } else {
    displayMaterialList(m, count);
  }
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

// find exist transaction id
int findTransactionIndexById(Transaction *t, char *id, int count) {
  for (int i = 0; i < count; i++) {
    if (strcmp(t[i].matId, id) == 0) {
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
    }
  } while (mode != 3);
}

void displayTransactionList(Transaction *transactions, int transactionIdx) {
  for (int i = 0; i < transactionIdx; i++) {
    printf("transID: %s\n", transactions[i].transId);
    printf("matID: %s\n", transactions[i].matId);
    printf("type: %s\n", transactions[i].type);
    printf("date: %s\n", transactions[i].date);
  }
}

void displayTransactionByID(Transaction *transactions, int transactionCount) {
  char id[10];
  readValidLine(id, sizeof(id),
                "Enter material id to see transfer history: ", "Material ID");
  for (int i = 0; i < transactionCount; i++) {
    if (strcmp(transactions[i].matId, id) == 0) {
      printf(GREEN "\n\nTransfer history of material with ID: %s\n" RESET, id);
      printf("TransID: %s\n", transactions[i].transId);
      printf("MatID  : %s\n", transactions[i].matId);
      printf("Type   : %s\n", transactions[i].type);
      printf("Date   : %s\n", transactions[i].date);
      printf("\n");
      return;
    }
  }
  printf("Material with ID: %s is not yet imported or exported", id);
}

void initTestData(Material **materials, int *materialCount) {
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
