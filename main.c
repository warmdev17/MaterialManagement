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
  int status; // 1. active | 2. expired
} Material;

typedef struct {
  char transId[20];
  char matId[10];
  char type[5];  // input/output
  char date[15]; // transaction time
} Transaction;

void readValidLine(char *buffer, size_t size, char *announce, char *valueType);
int findMaterialIndexById(Material *m, char *id, int materialCount);
void readInt(int *number, char *announce, char *valueType);
void clearBuffer();
void createNewMaterial(Material **materials, int *materialCount,
                       int *indexOfMaterial);
void updateMaterial(Material *materials, int materialCount);
void updateMaterialStatus(Material *materials, int materialCount);
void findByID(Material *materials, int materialCount);
void findByName(Material *materials, int materialCount);
void showMaterialByID(Material *materials, char *id, char materialCount);
void debugML(Material **materials, int *materialCount, int *indexOfMaterial);
void showCurrentInfo(Material *materials, int idx);
void getMaterialID(Material *materials, int materialCount);

void displayMenu() {
  printf(GREEN "========================\n" RESET);
  printf(YELLOW "1. Add new materials\n" RESET);
  printf(YELLOW "2. Update materials\n" RESET);
  printf(GREEN "========================\n" RESET);
}

int main() {
  Material *materials = NULL;
  int materialCount = 0;
  int indexOfMaterial = 0;

  int choice;
  do {
    displayMenu();
    printf("Enter your choice: ");
    if (scanf("%d", &choice) != 1) {
      clearBuffer();
      printf(RED "Invalid choice, please try again!!\n" RESET);
      choice = 0;
      continue;
    }
    clearBuffer();

    switch (choice) {
    case 1: {
      createNewMaterial(&materials, &materialCount, &indexOfMaterial);
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
    case 11: {
      debugML(&materials, &materialCount, &indexOfMaterial);
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

      // isspace return non-zero integer if agurment is space (\n, \t, \f, \r,
      // \v, ' ') in ASCII

      // return 0 if argument is whitespace
      if (!isspace((unsigned char)buffer[i])) {
        isEmpty = 0;
        return;
      }
    }

    if (isEmpty) {
      printf(RED "%s cannot empty. Please type again!!!\n" RESET, valueType);
      continue;
    }

    return; // break if buffer is not empty
  }
}

void readInt(int *number, char *announce, char *valueType) {
  bool isValid = false;

  while (!isValid) {
    printf("%s", announce);

    if (scanf("%d", number) == 1) {
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

    clearBuffer();
  }
}

void clearBuffer() {
  char c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

// ======= Material management helper =======
void createNewMaterial(Material **materials, int *materialCount,
                       int *indexOfMaterial) {
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

  // get material id
  do {
    readValidLine((*materials + idxMaterial)->matId,
                  sizeof((*materials + idxMaterial)->matId),
                  "Enter id of material: ", "ID");

    if (findMaterialIndexById(*materials, (*materials + idxMaterial)->matId,
                              *indexOfMaterial) != -1) {
      printf(RED "\nID must not match the ID of the product in stock, please "
                 "try again!\n" RESET);
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
  (*materials + idxMaterial)->status = 1;

  (*indexOfMaterial)++;

  printf(BLUE "\nAdd new materials successfully\n\n" RESET);
}

// ======= Update material via ID =======
void updateMaterial(Material *materials, int materialCount) {
  if (materialCount == 0) {
    printf(RED "Material list is empty. Nothing to update.\n" RESET);
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
    printf(GREEN "===============\n" RESET);
    printf(YELLOW "1. Edit name\n" RESET);
    printf(YELLOW "2. Edit unit\n" RESET);
    printf(YELLOW "3. Edit quantity\n" RESET);
    printf(YELLOW "4. Edit all\n" RESET);
    printf(YELLOW "5. Exit\n" RESET);
    printf(GREEN "===============\n" RESET);
    readInt(&mode, "Enter info to edit: ", "Mode");
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
      printf(RED "Invalid info please choose name/unit/quantity/all\n" RESET);
      break;
    }
    }
  } while (mode != 5);
}

// ==== UPDATE STATUS ====
void updateMaterialStatus(Material *materials, int materialCount) {

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to update: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
    return;
  }

  (materials + idx)->status = 0;

  printf(BLUE "Update status material with %s successfully!" RESET, id);
}

// ===== Find material by id ===== ( absolute id )
void findByID(Material *materials, int materialCount) {

  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to find: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
    return;
  } else {
    showMaterialByID(materials, id, materialCount);
  }
}

// ==== Find material by name ====
void findByName(Material *materials, int materialCount) {}

// show current material info
void showCurrentInfo(Material *materials, int idx) {
  printf(GREEN "\nCurrent information:\n" RESET);
  printf("ID   : %s\n", materials[idx].matId);
  printf("Name : %s\n", materials[idx].name);
  printf("Unit : %s\n", materials[idx].unit);
  printf("Qty  : %d\n\n", materials[idx].qty);
}

// show material by id
void showMaterialByID(Material *materials, char *id, char materialCount) {
  for (int i = 0; i < materialCount; i++) {
    if (strcmp((materials + i)->matId, id) == 0) {
      printf(GREEN "\nCurrent information:\n" RESET);
      printf("ID   : %s\n", materials[i].matId);
      printf("Name : %s\n", materials[i].name);
      printf("Unit : %s\n", materials[i].unit);
      printf("Qty  : %d\n\n", materials[i].qty);
    }
  }
}

// get material ID
void getMaterialID(Material *materials, int materialCount) {
  char id[10];
  readValidLine(id, sizeof(id), "Enter material ID to update: ", "ID");

  int idx = findMaterialIndexById(materials, id, materialCount);
  if (idx == -1) {
    printf(RED "Material with this ID was not found.\n\n" RESET);
    return;
  }
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

// debug materials list
void debugML(Material **materials, int *materialCount, int *indexOfMaterial) {
  for (int i = 0; i < *materialCount; i++) {
    printf("id: %s\n", (*materials + i)->matId);
    printf("Name: %s\n", (*materials + i)->name);
    printf("quantity: %d\n", (*materials + i)->qty);
    printf("unit: %s\n", (*materials + i)->unit);
  }
}
