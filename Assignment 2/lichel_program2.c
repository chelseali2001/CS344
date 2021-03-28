#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

/*
 * This stucture is used to store the data for a movie.
 * This includes the movie title (title), movie year (year), 
 * the languages the movie is in (languages), the number of 
 * languages the movie is in (numLang), and the movie rating
 * (rating). This also represent a single link in a singly-linked
 * list.
 */
struct movie {
  char *title;
  char *year;
  struct movie *next;
};

/*
 * This function should free the memory associated with a linked list.
 *
 * Params:
 *   movie - the movie list or linked list to be destroyed.
 */
void freeMov(struct movie *movie) {
  struct movie *curr;
  
  // Parsing through the linked list.
  while (movie) {
    curr = movie;
    movie = movie->next;
    free(curr->title);
    free(curr->year);
    free(curr);
  }
}

/*
 * This function should free the memory associated with a dynamic array of linked lists.
 *
 * Params:
 *   movieFiles - the array of linked lists to be destroyed.
 *   size - size of the array.
 */
void freeMovies(struct movie **movieFiles, int size) {
  // Destroying each linked list in the array.
  for (int x = 0; x < size; x++)
    freeMov(movieFiles[x]);
  
  free(movieFiles);
}

/*
 * This function should allocate and initialize a new, empty movie struct
 * and return a pointer to it.
 * 
 * Params:
 *  currLine - the current line in the file being processed.
 */
struct movie *createMovie(char *currLine) {
  struct movie *currMovie = malloc(sizeof(struct movie));

  // For use with strtok_r.
  char *saveptr, *saveptr0;

  // The first token is the title.
  char *token = strtok_r(currLine, ",", &saveptr);
  currMovie->title = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->title, token);

  // The next token is the year.
  token = strtok_r(NULL, ",", &saveptr);
  currMovie->year = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->year, token);

  // Set the next node to NULL in the newly created movie entry.
  currMovie->next = NULL;
  
  return currMovie;
}

/* 
 * This function should return a linked list of movies by parsing data
 * from each line of the specified file.
 *
 * Params:
 *  file - the name of the file to be processed.
 *  total - size of the file.
 */
struct movie *processFile(char *file, int *total) {
  // Open the specified file for reading only.
  FILE *movieFile = fopen(file, "r");

  char *currLine = NULL;
  size_t len = 0;
  ssize_t nread = getline(&currLine, &len, movieFile);
  
  // The head of the linked list.
  struct movie *head = NULL;
  // The tail of the linekd list.
  struct movie *tail = NULL;

  // Read the file line by line.
  while ((nread = getline(&currLine, &len, movieFile)) != -1) {
    // Get a new movie node corresponding to the current line.
    struct movie *mov = createMovie(currLine);
    
    // Check if this is the first node in the linked list.
    if (head == NULL) {
      // This is the first node in the linked link
      // Set the head and the tail to this node
      head = mov;
      tail = mov;
    } else {
      // This is not the first node.
      // Add this node to the list and advance the tail
      tail->next = mov;
      tail = mov;
    }
  }

  // Calculate file size
  fseek(movieFile, 0L, SEEK_END); 
  *total = ftell(movieFile);

  free(currLine);
  fclose(movieFile);
  
  return head;
}

/*
 * This function should create a new directory.
 *
 * Params:
 *   movie - the list of movies.
 *   file - name of file being processed.
 */
void newDirectory(struct movie *movie, char *file) {
  // Creating new directory name.
  int n = rand() % 100000;
  char result[256];
  sprintf(result, "%d", n);
  char *newFile = malloc(256 * sizeof(char));
  strcpy(newFile, "lichel.movies.");
  strcat(newFile, result);

  // Checking if the randomly generated directory already exists.
  while (mkdir(newFile, 0750) == -1) {
    n = rand() % 100000;
    sprintf(result, "%d", n);
    strcpy(newFile, "lichel.movies.");
    strcat(newFile, result);
  }
  
  FILE *fptr;
  char *f = malloc(256 * sizeof(char));
  
  // Creating a file for each year in the directory.
  while (movie) {
    strcpy(f, newFile);
    strcat(f, "/");
    strcat(f, movie->year);
    strcat(f, ".txt");
    
    int fileDes = open(f, O_RDWR | O_CREAT, 0640);
    fptr = fopen(f, "a");
    fputs(movie->title, fptr);
    fputs("\n", fptr);
    fclose(fptr);
    
    movie = movie->next;
  }

  printf("New processing the chosen file named %s\n", file);
  printf("Created directory with name %s\n\n", newFile);
  
  free(newFile);
  free(f);
}

/*
 * This function finds the largest file and creates a directory for it.
 *
 * Params:
 *   movieFiles - the array of movie files.
 *   fileNames - the array of movie file names.
 *   fileSizes - the array of the sizes of movie files.
 *   size - number of movie files.
 */
void largest(struct movie **movieFiles, char **fileNames, int *fileSizes, int size) {  
  int max = 0, index = 0;
  
  // Finding the largest file.
  for (int x = 0; x < size; x++) {
    if (fileSizes[x] > max) {
      max = fileSizes[x];
      index = x;
    }
  }
  
  // Creating a new directory for the file.
  newDirectory(movieFiles[index], fileNames[index]);
}

/*
 * This function finds the largest file and creates a directory for it.
 *
 * Params:
 *   movieFiles - the array of movie files.
 *   fileNames - the array of movie file names.
 *   fileSizes - the array of the sizes of movie files.
 *   size - number of movie files.
 */
void smallest(struct movie **movieFiles, char **fileNames, int *fileSizes, int size) {
  int min = 0, index = 0;
  
  // Finding the smallest file.
  for (int x = 0; x < size; x++) {
    if (fileSizes[x] < min) {
      min = fileSizes[x];
      index = x;
    }
  }
  
  // Creating a new directory for the file.
  newDirectory(movieFiles[index], fileNames[index]);
}

/*
 * This function creates a new directory for a specified file.
 */
int specify() {
  char *fileName = malloc(256 * sizeof(char));
  FILE *fptr;
  
  printf("Enter the complete file name: ");
  scanf("%s", fileName);

  // Creating a new directory for a specified file (if it exists).
  if (fptr = fopen(fileName, "r")) {
    fclose(fptr);
    int total = 0;
    struct movie *list = processFile(fileName, &total);
    newDirectory(list, fileName);
    freeMov(list);
  } else {
    printf("The file %s was not found. Try again\n", fileName);
    free(fileName);
    
    return 0;
  }
  
  free(fileName);
  
  return 1;
}

/*
 * This function provides the program functions for the user.
 *
 * Params:
 *   movieFiles - the array of movie files.
 *   fileNames - the array of movie file names.
 *   fileSizes - the array of the sizes of movie files.
 *   size - number of movie files.
 */
void menu(struct movie **movieFiles, char **fileNames, int *fileSizes, int size) {
  int fileChoice = 0, index = 0, check = 0;
  
  // User picks what file they want to process.
  printf("\nWhich file you want to process?\n");
  printf("Enter 1 to pick the larget file\n");
  printf("Enter 2 to pick the smallest file\n");
  printf("Enter 3 to specify the name of a file\n");
  
  printf("\nEnter a choice from 1 to 3: ");
  scanf("%d", &fileChoice);

  if (fileChoice == 1) {
    largest(movieFiles, fileNames, fileSizes, size);
  } else if (fileChoice == 2) {
    smallest(movieFiles, fileNames, fileSizes, size);
  } else if (fileChoice == 3) {
    if (specify() == 0)
      menu(movieFiles, fileNames, fileSizes, size);
  } else {
    printf("Error: You can only enter 1, 2, or 3\n");
    menu(movieFiles, fileNames, fileSizes, size);
  }
}

int main() {
  srand(time(NULL));
  int choice = 0;
  struct dirent *de;
  DIR *dr = opendir("."); 
  struct movie **movieFiles = (struct movie**)malloc(sizeof(struct movie*));
  char **fileNames = (char**)malloc(sizeof(char*));
  int *fileSizes = (int*)malloc(sizeof(int));
  int size = 0;
  char *point;
  
  // Collecting the data from all of the files starting with "movies_".
  while ((de = readdir(dr)) != NULL) {
    if (strlen(de->d_name) >= 7) {
      if((point = strrchr(de->d_name,'.')) != NULL ) {
        if(strcmp(point,".csv") == 0) {
          char token[8];
          strncpy(token, de->d_name, 7);
          token[7] = '\0';
          
          if (strcmp(token, "movies_") == 0) {
            int total = 0;
            struct movie *list = processFile(de->d_name, &total);
            
            // Storing the movies of a file in an array.
            movieFiles = (struct movie**)realloc(movieFiles, (size + 1) * sizeof(struct movie*));
            movieFiles[size] = list;
            
            // Storing the file names in an array
            fileNames = (char**)realloc(fileNames, (size + 1) * sizeof(char*));
            fileNames[size] = calloc(strlen(de->d_name) + 1, sizeof(char));
            strcpy(fileNames[size], de->d_name);
            
            // Storing the file sizes in an array.
            fileSizes = (int*)realloc(fileSizes, (size + 1) * sizeof(int));
            fileSizes[size] = total;
            
            size++;
          }
        }
      }
    }
  }

  closedir(dr); 

  // User selects a process or exits the program.
  while (choice != 2) {
    printf("1. Select file to process\n");
    printf("2. Exit the program\n"); 
    
    printf("\nEnter a choice 1 or 2: ");
    scanf("%d", &choice);
    
    if (choice == 1)      
      menu(movieFiles, fileNames, fileSizes, size);
    else if (choice != 1 || choice != 2)
      printf("Error: You can either choose 1 or 2\n\n");
  }

  // Freeing data
  for (int x = 0; x < size; x++)
    free(fileNames[x]);
  
  free(fileNames);
  free(fileSizes);
  freeMovies(movieFiles, size);

  return 0;
}