#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// escape sequence for terminal color
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

#define MAX_LIST_SIZE 100

typedef struct {
  char matId[10];
  char name[50];
  char unit[10];
  int qty;    // quantity in storage
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

void readValidLine(char *buffer, size_t size, char *announce, char *valueType);
void readInt(int *number, char *announce, char *valueType);

int findMaterialIndexById(Material *m, char *id, int materialCount);

void createNewMaterial(Material **materials, int *materialCount);
void updateMaterial(Material *materials, int materialCount);
void updateMaterialStatus(Material *materials, int materialCount);
int readStatusWithDefault();
void findByID(Material *materials, int materialCount);
void findByName(Material *materials, int materialCount);

void displayMaterialList(Material *materials, int materialCount);
void showCurrentInfo(Material *materials, int idx);

// ======= MENU =======
void displayMenu() {
  printf(
      GREEN
      "==================== MATERIAL MANAGEMENT ====================\n" RESET);
  printf(YELLOW " 1. Add new materials\n" RESET);
  printf(YELLOW " 2. Update material info\n" RESET);
  printf(YELLOW " 3. Update material status\n" RESET);
  printf(YELLOW " 4. Find material by ID\n" RESET);
  printf(YELLOW "11. Display material list\n" RESET);
  printf(YELLOW "10. Exit\n" RESET);
  printf(
      GREEN
      "=============================================================\n" RESET);
}

// ======= MAIN =======
int main() {
  Material *materials = NULL;
  int materialCount = 0;

  int choice;
  do {
    displayMenu();
    readInt(&choice, "Enter your choice: ", "choice");

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
      findByID(materials, materialCount);
      break;
    }
    case 5: {
      findByName(materials, materialCount);
      break;
    }
    case 11: {
      displayMaterialList(materials, materialCount);
      break;
    }
    case 10: {
      printf(BLUE "Exiting program...\n" RESET);
      break;
    }
    default: {
      printf(RED "Invalid choice, please try again.\n\n" RESET);
      break;
    }
    }
  } while (choice != 10);

  free(materials);
  return 0;
}

// ======= INPUT/OUTPUT helper =======
void readValidLine(char *buffer, size_t size, char *announce, char *valueType) {
  while (1) {
    printf("%s", announce);

    if (fgets(buffer, size, stdin) == NULL) {
      printf(RED "Error reading input\n" RESET);
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
  bool isValid = false;

  while (!isValid) {
    char numberTest[1000];

    printf("%s", announce);
    if (fgets(numberTest, sizeof(numberTest), stdin) == NULL) {
      printf(RED "Error reading %s, please try again.\n" RESET, valueType);
      continue;
    }

    numberTest[strcspn(numberTest, "\n")] = '\0';

    if (sscanf(numberTest, "%d", number) == 1) {
      if (*number >= 0) {
        isValid = true;
      } else {
        printf(RED
               "%s must be greater or equal zero, please type again.\n" RESET,
               valueType);
      }
    } else {
      printf(RED "Invalid %s, please type again.\n" RESET, valueType);
    }
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
    printf(RED "Allocate failed\n" RESET);
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
      printf(RED "\nID must not duplicate existing material ID, "
                 "please try again!\n" RESET);
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

  printf(BLUE "\nAdd new material successfully\n\n" RESET);
}

// ======= Update material via ID =======
void updateMaterial(Material *materials, int materialCount) {
  if (materialCount == 0) {
    printf(RED "Material list is empty. Nothing to update.\n\n" RESET);
    return;
  }

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to update: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
    return;
  }

  // show current info
  showCurrentInfo(materials, idx);
  int mode;
  do {
    printf(GREEN "=============== EDIT MENU ===============\n" RESET);
    printf(YELLOW "1. Edit name\n" RESET);
    printf(YELLOW "2. Edit unit\n" RESET);
    printf(YELLOW "3. Edit quantity\n" RESET);
    printf(YELLOW "4. Edit all\n" RESET);
    printf(YELLOW "5. Exit\n" RESET);
    printf(GREEN "=========================================\n" RESET);

    readInt(&mode, "Enter info to edit: ", "mode");
    switch (mode) {
    case 1: {
      readValidLine(materials[idx].name, sizeof(materials[idx].name),
                    "Enter new name: ", "Name");
      printf(BLUE "\nUpdate name of material with ID %s successfully.\n" RESET,
             id);
      showCurrentInfo(materials, idx);
      break;
    }
    case 2: {
      readValidLine(materials[idx].unit, sizeof(materials[idx].unit),
                    "Enter new unit: ", "Unit");
      printf(BLUE "\nUpdate unit of material with ID %s successfully.\n" RESET,
             id);
      showCurrentInfo(materials, idx);
      break;
    }
    case 3: {
      readInt(&materials[idx].qty, "Enter new quantity: ", "quantity");
      printf(BLUE
             "\nUpdate quantity of material with ID %s successfully.\n" RESET,
             id);
      showCurrentInfo(materials, idx);
      break;
    }
    case 4: {
      readValidLine(materials[idx].name, sizeof(materials[idx].name),
                    "Enter new name: ", "Name");
      readValidLine(materials[idx].unit, sizeof(materials[idx].unit),
                    "Enter new unit: ", "Unit");
      readInt(&materials[idx].qty, "Enter new quantity: ", "quantity");

      printf(BLUE "\nUpdate material with ID %s successfully.\n" RESET, id);
      showCurrentInfo(materials, idx);
      break;
    }
    case 5: {
      break;
    }
    default: {
      printf(RED "Invalid option, please choose 1-5.\n" RESET);
      break;
    }
    }
  } while (mode != 5);
}

// ==== UPDATE STATUS ====
void updateMaterialStatus(Material *materials, int materialCount) {
  if (materialCount == 0) {
    printf(RED "Material list is empty.\n\n" RESET);
    return;
  }

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to toggle status: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
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
      printf(RED "Error reading input. Try again.\n" RESET);
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
    printf(RED "Status must be 0 or 1. Please type again.\n" RESET);
  }
}

// ===== Find material by id ===== ( absolute id )
void findByID(Material *materials, int materialCount) {
  if (materialCount == 0) {
    printf(RED "Material list is empty.\n\n" RESET);
    return;
  }

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to find: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
    return;
  }

  showCurrentInfo(materials, idx);
}

// ==== Find material by name (substring, case-insensitive) ====
int charToLower(int c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 'a';
  return c;
}

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

void findByName(Material *materials, int materialCount) {
  if (materialCount == 0) {
    printf(RED "Material list is empty.\n\n" RESET);
    return;
  }

  char keyword[50];
  readValidLine(keyword, sizeof(keyword), "Enter name to search: ", "Name");

  int found = 0;
  printf(GREEN "\nSearch results:\n" RESET);

  for (int i = 0; i < materialCount; i++) {
    if (containsIgnoreCase(materials[i].name, keyword)) {
      showCurrentInfo(materials, i);
      found = 1;
    }
  }

  if (!found) {
    printf(RED "No material matched this name.\n\n" RESET);
  }
}

// show current material info
void showCurrentInfo(Material *materials, int idx) {
  printf(GREEN "\nCurrent information:\n" RESET);
  printf("ID   : %s\n", materials[idx].matId);
  printf("Name : %s\n", materials[idx].name);
  printf("Unit : %s\n", materials[idx].unit);
  printf("Qty  : %d\n", materials[idx].qty);
  printf("Status: %s\n\n", (materials[idx].status == 1) ? "Active" : "Expired");
}

// check valid id
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
    printf(RED "\nMaterial list is empty.\n\n" RESET);
    return;
  }

  int page = 0;
  int pageSize = 10;
  int totalPages = (materialCount + pageSize - 1) / pageSize;

  while (1) {
    system("clear");

    printMaterialPage(materials, materialCount, page, pageSize);

    printf("1. Next page\n");
    printf("2. Previous page\n");
    printf("0. Exit\n");

    int choice;
    readInt(&choice, "Your choice: ", "Choice");

    if (choice == 1) {
      if (page < totalPages - 1)
        page++;
    } else if (choice == 2) {
      if (page > 0)
        page--;
    } else if (choice == 0) {
      break;
    }
  }
}
