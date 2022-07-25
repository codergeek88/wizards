#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

//implement updating

const char FILENAME[21] = "wizards.dat";
const char CATEGORY_TITLES[101] = "(First name, Last name, House, Alive (status), Good (status))\n";
const char WIZARD_FORMAT_WRITE[41] = "%s, %s, %s, %i, %i, %i, %i\n";
const char WIZARD_FORMAT_READ[51] = "%[^,], %[^,], %[^,], %i, %i, %i, %i";
const char WIZARD_FORMAT_PRINT[41] = "%i. %s, %s, %s, %i, %i\n";
const int N = 10;

typedef struct wizard
{
  char fname[21];
  char lname[21];
  char house[11];
  int alive;
  int good;
  //for keeping track of locations
  int fname_slot;
  int lname_slot;
} wizard;

void addTitles(FILE *db)
//add category titles
{
  fprintf(db, "%s", CATEGORY_TITLES);
  printf("Category titles added to database.\n");
}

bool isEmpty(FILE *db)
//returns true if db csv file is empty, false otherwise
{
  fseek(db, 0, SEEK_END);
  int size = ftell(db);
  return size == 0;
}

struct wizard buildWizard()
//get all wizard data from user input and return wizard
{
  struct wizard wizard;
  printf("Enter wizard data below.\n");
  printf("First name: ");
  scanf("%s", wizard.fname);
  printf("Last name: ");
  scanf("%s", wizard.lname);
  printf("House: ");
  scanf("%s", wizard.house);
  printf("Alive (status): ");
  scanf("%i", &wizard.alive);
  printf("Good (status): ");
  scanf("%i", &wizard.good);
  return wizard;
}

void recordWizard(FILE *db, struct wizard wizard)
//record wizard in database
{
  fprintf(db, WIZARD_FORMAT_WRITE, wizard.fname, wizard.lname, wizard.house,
          wizard.alive, wizard.good, wizard.fname_slot, wizard.lname_slot);
}

void printWizard(struct wizard wizard, int wiz_rank)
//print wizard in user-readable format
//wiz_rank is the order of appearance of wizard when printing
//leave out slot indices (only necessary for locating names)
{
  printf(WIZARD_FORMAT_PRINT, wiz_rank, wizard.fname, wizard.lname, wizard.house,
          wizard.alive, wizard.good);
}

bool isReal(struct wizard wizard)
//return true if wizard is real, false otherwise
{
  return (wizard.alive != -1);
}


int getNumWizards(struct wizard *wizards)
//get the number of real wizards in array
{
  int num_wizards = 0;
  while ( isReal(wizards[num_wizards]) )
  {
    num_wizards++;
  }
  return num_wizards;
}

void printWizards(struct wizard *wizards)
//print all wizards in an array
{
  //get number of real wizards
  int num_wizards = getNumWizards(wizards);
  if (num_wizards == 0)
  {
    //this if statement will never be triggered if the function is called by printWizMessages
    //it is only here to handle the case in which the function is directly called outside of printWizMessages
    printf("No wizards found.\n");
  }
  //print all real wizards
  for (int i = 0; i < num_wizards; i++)
  {
    //wiz_rank = i + 1 (wiz_rank starts indexing at 1)
    printWizard(wizards[i], i + 1);
  }
}

void printWizMessages(struct wizard *wizards, char *empty_msg, char *found_msg)
//print wizards along with messages that facilitate reading the wizard information
//empty_msg printed if there are no real wizards in the wizards array
//found_msg printed if one or more real wizards are found in the wizards array
{
  //check if the first wizard is a phantom (see getWizard)
  //if it is, then there are no wizards with a name match
  printf("\n");
  if ( !isReal(wizards[0]) )
  {
    printf("%s\n", empty_msg);
  }
  //else print out all wizards that are not phantoms
  else
  {
    printf("%s\n", found_msg);
    printf("%s", CATEGORY_TITLES);
    printWizards(wizards);
  }
  printf("\n");
}

void createSpace(FILE *db, int item_len, int num_slots)
{
  //create blank spaces to store items of data
  //create num_slots of blank slots for data storage
  //for each item, allow for item_len storage space
  fseek(db, 0, SEEK_SET);
  for (int i = 0; i < item_len * num_slots; i++)
  {
    fprintf(db, " ");
  }
  printf("Database created.\n");
}

int hashIndex(char *name, int num_slots)
{
  //create an index of item storage using a name
  //initialize index at 1
  double i = 1;
  for (int k = 0; k < sizeof(name); k++)
  {
    //break loop if the loop hits the end of the name string
    if (name[k] == 0)
    {
      break;
    }
    //choose to multiply or divide by character if the index is odd or even
    if (k % 2 == 0)
    {
      i = i * name[k];
    } else {
      i = i / name[k];
    }
  }
  //convert double to int after multiplying by num_slots
  int ind = (int) (i * num_slots);
  //standardize ind at num_slots and then apply mod function
  //should get an even spread across all indices
  ind = ind % num_slots;
  return ind;
}

char getSlotFirst(FILE *db, int slot_ind, int item_len)
//get the first character in a slot
{
  char slot_first;
  fseek(db, slot_ind * item_len, SEEK_SET);
  fscanf(db, "%c", &slot_first);
  return slot_first;
}

bool isOpen(FILE *db, int slot_ind, int item_len)
//check if the current slot is open for data storage
{
  //check if the first character in the slot is a space
  char slot_first = getSlotFirst(db, slot_ind, item_len);
  return (slot_first == ' ');
}

bool isDeleted(FILE *db, int slot_ind, int item_len)
//check if current slot has a deleted wizard
{
  //check if the first character in the slot is "/"
  char slot_first = getSlotFirst(db, slot_ind, item_len);
  return (slot_first == '/');
}

int assignNameSlot(FILE *db, struct wizard wizard, char *title, int item_len, int num_slots)
//assign slot_ind for provided name (either fname or lname)
{
  //specify which name of the wizard to use in assigning an index (fname or lname)
  char *name;
  if (strcmp(title, "fname") == 0)
  {
    name = wizard.fname;
  }
  if (strcmp(title, "lname") == 0)
  {
    name = wizard.lname;
  }
  //get slot index of the item
  int slot_ind = hashIndex(name, num_slots);
  //if the slot is taken by a wizard, look for storage in the adjacent slots until
  //an open or deleted slot is found
  while (!isOpen(db, slot_ind, item_len) && !isDeleted(db, slot_ind, item_len))
  {
    slot_ind++;
  }
  return slot_ind;
}

char** generateTitles()
//creat titles to iterate over for recording at two indices in database
{
  char** titles = malloc(2 * sizeof(char*));
  for (int i = 0; i < 2; i++)
  {
    titles[i] = malloc(6 * sizeof(char));
  }
  titles[0] = "fname";
  titles[1] = "lname";
  return titles;
}

void writeWizardAtSlot(FILE *db, struct wizard wizard, int slot_ind, int item_len)
//write wizard at given slot
{
  fseek(db, slot_ind * item_len, SEEK_SET);
  recordWizard(db, wizard);
}

void writeWizard(FILE *db, struct wizard wizard, int item_len, int num_slots)
//write wizard into database at fname and lname indices
{
  //get slot_inds for fname and lname storage
  int fname_slot = assignNameSlot(db, wizard, "fname", item_len, num_slots);
  int lname_slot = assignNameSlot(db, wizard, "lname", item_len, num_slots);
  //assign fname_slot and lname_slot to wizard
  wizard.fname_slot = fname_slot;
  wizard.lname_slot = lname_slot;
  //write wizard at fname_slot and lname_slot
  writeWizardAtSlot(db, wizard, fname_slot, item_len);
  writeWizardAtSlot(db, wizard, lname_slot, item_len);
}

void makeWizard(FILE *db, int item_len, int num_slots)
//build and record wizard
{
  struct wizard wizard = buildWizard();
  writeWizard(db, wizard, item_len, num_slots);
}

void createWizard(FILE *db, int item_len, int num_slots)
//create wizard entry
{
  makeWizard(db, item_len, num_slots);
  printf("Wizard recorded!\n\n");
}

bool nameMatches(FILE *db, char*name, int slot_ind, int item_len, int num_slots)
//check if the entered name matches a wizard whose information is recorded at the given slot_ind
{
  //initialize empty wizard
  struct wizard wizard;
  //scan for and record wizard data (assume slot isn't empty)
  fseek(db, slot_ind * item_len, SEEK_SET);
  fscanf(db, WIZARD_FORMAT_READ, wizard.fname, wizard.lname, wizard.house,
        &wizard.alive, &wizard.good, &wizard.fname_slot, &wizard.lname_slot);
  //check if wizard's first or last name matches entered name
  return ( strcmp(wizard.fname, name) == 0 || strcmp(wizard.lname, name) == 0 );
}

void markPhantomWizards(struct wizard *wizards, int num_wizards)
//mark each phantom wizard in the wizards array with an alive status of -1
//to make them easy to identify
{
  for (int i = num_wizards; i < N; i++)
  {
    wizards[i].alive = -1;
  }
}

struct wizard *findWizards(FILE *db, char *name, int item_len, int num_slots)
//find wizards with a name (fname or lname) that matches provided name
{
  //initialize an empty wizard array and start index num_wizards at 0
  struct wizard *wizards = malloc(N * sizeof(wizard));
  int num_wizards = 0;
  //run the name through the hash function to locate where the wizard with that name should be stored
  int slot_ind = hashIndex(name, num_slots);
  //if the slot is open, no wizard with that name exists
  //otherwise, the wizard may be stored there or somewhere adjacent

  //while adjacent slots are taken and wizards isn't filled up, parse through
  //the slots to find name matches and add to wizards
  while ( (!isOpen(db, slot_ind, item_len)) && (num_wizards < N) )
  {
    //if the slot is not deleted, check for a name match, else don't check for a name match
    //because a deleted slot has an incompatible format for checking for name matches
    if ( !isDeleted(db, slot_ind, item_len) )
    {
      //if there is a name match, fill out wizard information at current wizards index
      bool name_match = nameMatches(db, name, slot_ind, item_len, num_slots);
      if (name_match)
      {
        fseek(db, slot_ind * item_len, SEEK_SET);
        fscanf(db, WIZARD_FORMAT_READ, wizards[num_wizards].fname, wizards[num_wizards].lname,
              wizards[num_wizards].house, &wizards[num_wizards].alive,
              &wizards[num_wizards].good, &wizards[num_wizards].fname_slot,
              &wizards[num_wizards].lname_slot);
        num_wizards++;
      }
    }
    //move to the next slot to check for a name match
    slot_ind++;
  }
  //mark all phantom wizards in the array with an alive status of -1
  markPhantomWizards(wizards, num_wizards);
  return wizards;
}

struct wizard *getWizards(FILE *db, int item_len, int num_slots)
{
  //get wizard from user inputted name
  printf("Enter below the first or last name of the wizard for which you are searching.\n");
  char name[21];
  printf("Name: ");
  scanf("%s", name);
  return findWizards(db, name, item_len, num_slots);
}

struct wizard *listReadWizards(FILE *db, int item_len, int num_slots)
//retrieve and print an array of wizards from a user-inputted name
{
  //get wizards from user inputted name
  struct wizard *wizards = getWizards(db, item_len, num_slots);
  //print wizards
  char empty_msg[101] = "No wizard found with that name.";
  char found_msg[101] = "Wizard(s) found!";
  printWizMessages(wizards, empty_msg, found_msg);
  return wizards;
}

void readWizards(FILE *db, int item_len, int num_slots)
{
  struct wizard *wizards = listReadWizards(db, item_len, num_slots);
}

int *genWizardTracker(int num_wizards)
//generate an array of false values (0s) to use to track which wizards have been selected from an array
{
  //initialize wiz_tracker
  int *wiz_tracker = malloc(num_wizards * sizeof(int));
  //initialize all values as 0 (false)
  for (int i = 0; i < num_wizards; i++)
  {
    wiz_tracker[i] = 0;
  }
  return wiz_tracker;
}

struct wizard *selectWizards(struct wizard *wizards)
//select wizards from wizards array based on wiz_rank
{
  int num_wizards = getNumWizards(wizards);
  //use wiz_tracker to track which wizards have already been selected by user
  int *wiz_tracker = genWizardTracker(num_wizards);
  //initialize array of selected wizards and number of selected wizards
  struct wizard *selected = malloc(num_wizards * sizeof(wizard));
  int num_selected = 0;
  //ask user to select indices of desired wizards
  printf("Enter the ranks associated with the wizards you'd like to select.\n");
  printf("Enter 0 to select all wizards.\n");
  printf("Enter -1 when you are done.\n\n");
  while (true)
  {
    //ask user for wizard (by wiz_ind)
    int wiz_ind;
    scanf("%i", &wiz_ind);
    if (wiz_ind == 0)
    {
      return wizards;
    }
    if (wiz_ind == -1)
    {
      break;
    }
    //if wizard is not listed, skip
    if (wiz_ind < 1 || wiz_ind > num_wizards)
    {
      printf("Invalid. Enter a number for a wizard listed above.\n");
    }
    else
    {
      //decrease wiz_ind by 1 for indexing purposes
      wiz_ind--;
      //if wizard has already been picked, skip
      if (wiz_tracker[wiz_ind])
      {
        printf("Invalid. This wizard has already been selected.\n");
      }
      //if wizard has not been picked add it to selected and mark it as selected in wiz_tracker
      else
      {
        //add wizard to selected wizards
        selected[num_selected] = wizards[wiz_ind];
        //mark wizard as selected (true) in wiz_tracker
        wiz_tracker[wiz_ind] = 1;
        //increement num_selected
        num_selected++;
      }
    }
    //if all wizards have been selected, cease inquiring the user for further selections
    if (num_selected == num_wizards)
    {
      break;
    }
  }
  //mark phantom wizards
  markPhantomWizards(selected, num_selected);
  return selected;
}

struct wizard *listSelectedWizards(struct wizard *wizards)
//retrieve and print user-selected array of wizards
{
  //select desired wizards
  struct wizard *selected = selectWizards(wizards);
  //print wizards
  char empty_msg[101] = "You didn't select any wizards.";
  char found_msg[101] = "Here are the wizards you selected.";
  printWizMessages(selected, empty_msg, found_msg);
  return selected;
}

struct wizard *getSelected(FILE *db, int item_len, int num_slots)
{
  //get wizard from user inputted name
  struct wizard *wizards = listReadWizards(db, item_len, num_slots);
  //initialize selected
  struct wizard *selected;
  //if no wizards are found from inputted name, return a garbage array with one phantom wizard
  if ( !isReal(wizards[0]) )
  {
    selected = malloc(sizeof(int));
    selected[0].alive = -1;
  }
  //else, select wizards from nonempty list
  else
  {
    selected = listSelectedWizards(wizards);
  }
  return selected;
}

void deleteName(FILE *db, struct wizard wizard, int item_len, char *title)
//delete wizard at slot given by name (either fname or lname)
{
  //specify slot to clear
  int slot_ind;
  if ( strcmp(title, "fname") == 0 )
  {
    slot_ind = wizard.fname_slot;
  }
  if ( strcmp(title, "lname") == 0 )
  {
    slot_ind = wizard.lname_slot;
  }
  //to delete the name, clear wizard info (replace with spaces)
  //then print "/" at the first space in the slot as a deleted flat
  fseek(db, slot_ind * item_len, SEEK_SET);
  for (int i = 0; i < item_len; i++)
  {
    fprintf(db, " ");
  }
  fseek(db, slot_ind * item_len, SEEK_SET);
  fprintf(db, "/");
}

void deleteWizard(FILE *db, struct wizard wizard, int item_len)
//delete wizard information at both name indices in db (fname and lname)
{
  deleteName(db, wizard, item_len, "fname");
  deleteName(db, wizard, item_len, "lname");
}

bool isConfirmed(char* confirm_msg)
//get confirmation to go through with task detailed by confirm_msg
{
  char conf[2];
  printf("%s (y/n): ", confirm_msg);
  scanf("%s", conf);
  return (strcmp(conf, "y") == 0);
}

void deleteWizards(FILE *db, int item_len, int num_slots)
{
  struct wizard *selected = getSelected(db, item_len, num_slots);
  //delete wizards if selected array has one or more real wizards
  if ( isReal(selected[0]) )
  {
    //ask for deletion confirmation
    char confirm_msg[101] = "Would you like to delete these wizards?";
    bool del_confirm = isConfirmed(confirm_msg);
    if (del_confirm)
    {
      //delete all real wizards in selected
      int i = 0;
      while ( isReal(selected[i]) )
      {
        deleteWizard(db, selected[i], item_len);
        i++;
      }
      printf("Selected wizard(s) deleted!\n");
    }
    else
    {
      printf("Ok. No wizards were deleted.\n");
    }
    printf("\n");
  }
}

void updateWizard(FILE *db, struct wizard wizard, int item_len, int num_slots, int rank)
//update given wizard
{
  //print current wizard information for reference (along with list rank)
  printf("Current wizard:\n");
  printWizard(wizard, rank);
  //delete old wizard and create new entry for updated wizard
  //make new wizard before deleting old one to prevent information loss if user
  //quits update partway through generating new wizard information
  makeWizard(db, item_len, num_slots);
  deleteWizard(db, wizard, item_len);
  printf("Wizard updated!\n\n");
}

void updateWizards(FILE *db, int item_len, int num_slots)
//update user-picked wizards
{
  struct wizard *selected = getSelected(db, item_len, num_slots);
  //check if there are any real selected wizards
  if ( isReal(selected[0]) )
  {
    //update all real selected wizards
    int i = 0;
    while ( isReal(selected[i]) )
    {
      //wizard list rank = i + 1
      updateWizard(db, selected[i], item_len, num_slots, i + 1);
      i++;
    }
    printf("All wizards updated!\n\n");
  }
}

void askForUserAction(FILE *db, int item_len, int num_slots)
{
  printf("Welcome to the wizard database!\n");
  printf("Here you can store and retrieve information about wizards!\n");
  printf("Type 'end' after the action prompt to terminate program.\n\n");
  while (true)
  {
    //get user action
    char action[7];
    printf("What would you like to do (create/read/update/delete/end)?\n");
    printf("Action: ");
    scanf("%s", action);
    printf("\n");
    if ( strcmp(action, "end") == 0 )
    {
      break;
    }
    //catch invalid user commands and redirect to prompt
    if ( strcmp(action, "create") != 0 && strcmp(action, "read") != 0 && strcmp(action, "delete") != 0
        && strcmp(action, "update") != 0 && strcmp(action, "end") != 0 )
    {
      printf("Select valid database function.\n\n");
    }
    else
    {
      //perform database function until user wants to change function or end program
      while (true)
      {
        if ( strcmp(action, "create") == 0 )
        {
          createWizard(db, item_len, num_slots);
        }
        if ( strcmp(action, "read") == 0 )
        {
          readWizards(db, item_len, num_slots);
        }
        if ( strcmp(action, "delete") == 0)
        {
          deleteWizards(db, item_len, num_slots);
        }
        if ( strcmp(action, "update") == 0)
        {
          updateWizards(db, item_len, num_slots);
        }

        //check if user wants to change function or end program
        char change[7];
        printf("Change action or terminate program (y/n): ");
        scanf("%s", change);
        if ( strcmp(change, "y") == 0 )
        {
          break;
        }
      }
    }
  }
}

int main(void)
{
  //create file pointer and open file
  FILE *db;
  int item_len = 100, num_slots = 10000;
  db = fopen(FILENAME, "r+");
  //check if file exists
  if (db == NULL)
  {
    printf("%s does not exist\n", FILENAME);
    return -1;
  }
  //create space for data storage if file is empty
  if (isEmpty(db))
  {
    createSpace(db, item_len, num_slots);
  }
  //ask for user input
  askForUserAction(db, item_len, num_slots);
  //close file
  fclose(db);
  return 0;
}
