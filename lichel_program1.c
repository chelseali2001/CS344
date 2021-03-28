#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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
  char **languages;
  int numLang;
  char *rating;
  struct movie *next;
};

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

  // The next token is the languages.
  token = strtok_r(NULL, "[]", &saveptr);
  currMovie->languages = (char**)malloc(sizeof(char*));
  currMovie->numLang = 0;
  
  // The next subtokens are the languages.
  for (char *x = token;; x = NULL) {
    char *subtoken = strtok_r(x, ";", &saveptr0);
    
    if (subtoken == NULL)
      break;
    
    currMovie->languages = (char**)realloc(currMovie->languages, (currMovie->numLang + 1) * sizeof(char*));
    currMovie->languages[currMovie->numLang] = calloc(strlen(subtoken) + 1, sizeof(char));
    strcpy(currMovie->languages[currMovie->numLang], subtoken);
    currMovie->numLang++;
  }
  
  // The last token is the rating value.
  token = strtok_r(NULL, ",\n", &saveptr);
  currMovie->rating = calloc(strlen(token) + 1, sizeof(char));
  strcpy(currMovie->rating, token);
  
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
 */
struct movie *processFile(char *file) {
  // Open the specified file for reading only.
  FILE *movieFile = fopen(file, "r");

  char *currLine = NULL;
  size_t len = 0;
  ssize_t nread = getline(&currLine, &len, movieFile);
  int total = 0;
  
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
    
    total += 1;
  }

  printf("Processed file %s and parsed data for %d movies\n", file, total);
  free(currLine);
  fclose(movieFile);
  
  return head;
}

/*
 * This function should show the movies released in the specified year.
 *
 * Params:
 *  list - list of movies
 *  year - the user specified year.
 */
void printYear(struct movie *list, int year) {
  int num = 0;
  
  // Parsing through the linked list and printing the movies released in the specified year.
  while (list != NULL) {
    if (atoi(list->year) == year) {
      printf("%s\n", list->title);
      num++;
    }
    
    list = list->next;
  }
  
  // If there are no movies released in the specified year.
  if (num == 0)
    printf("No data about movies released in the year %d\n", year);
}

/*
 * This function should show the highest rated movie for each year.
 *
 * Params:
 *  list - list of movies.
 */
void printRatings(struct movie *list) {
  char **years = (char**)malloc(sizeof(char*)); // Stores the years.
  char **ratings = (char**)malloc(sizeof(char*)); // Stores the highest rating for each year.
  char **titles = (char**)malloc(sizeof(char*)); // Stores the title of the highest rating movie of each year.
  char *ptr;
  int size = 0;

  // Parsing through the linked list.
  while (list != NULL) {
    int in = 0;
    
    // Checks if there are other movies of the same year.
    for (int x = 0; x < size; x++) {
      if (atoi(list->year) == atoi(years[x])) {
        in = 1;

        // If the movie rating is higher than the currently stored rating, then replace it.
        if (strtod(list->rating, &ptr) > strtod(ratings[x], &ptr)) {
          ratings[x] = list->rating;
          titles[x] = list->title;
        }
      }
    }

    // If there are no other movies of the same year, append it to the lists.
    if (in == 0) {
      years = (char**)realloc(years, (size + 1) * sizeof(char*));
      years[size] = list->year;
    
      ratings = (char**)realloc(ratings, (size + 1) * sizeof(char*));
      ratings[size] = list->rating;

      titles = (char**)realloc(titles, (size + 1) * sizeof(char*));
      titles[size] = list->title;
      
      size++;
    }
    
    list = list->next;
  }
  
  // Printing the highest rating movies of each year.
  for (int x = 0; x < size; x++) 
    printf("%s %.1f %s\n", years[x], strtod(ratings[x], &ptr), titles[x]);
  
  free(years);
  free(ratings);
  free(titles);
}

/*
 * This function should show the movies and their year of release for a specific language.
 *
 * Params:
 *  list - list of movies.
 * language - the user specified language.
 */
void printLanguage(struct movie *list, char *language) {
  int num = 0;

  // Parsing through the linked list.
  while (list != NULL) {    
    // Checking if the movie is in the specified language.
    for (int x = 0; x < list->numLang; x++) {
      // If the movie is in the specified language.
      if (strcmp(list->languages[x], language) == 0) {
        printf("%s %s\n", list->year, list->title);
        num++;
      }
    }

    list = list->next;
  }

  // If there are no movies in the specified language.
  if (num == 0)
    printf("No data about movies released in %s\n", language);
}

/*
 * This function should free the memory associated with a linked list.
 *
 * Params:
 *   list - the movie list or linked list to be destroyed.
 */
void freeMovies(struct movie *list) {
  struct movie *curr;
  
  // Parsing through the linked list.
  while (list) {
    curr = list;
    list = list->next;
    free(curr->title);
    free(curr->year);
  
    for (int x = 0; x < curr->numLang; x++)
      free(curr->languages[x]);
    
    free(curr->languages);
    free(curr->rating);
    free(curr);
  }
}

int main(int argc, char **argv) {
  // Checking if the command line input has only 1 file inputed.
  if (argc == 1 || argc > 2) {
    printf("Error: one file is required.\n");
  } else {
    // Creating the list of movies extracted from the file.
    struct movie *list = processFile(argv[1]);
    int choice = 0, year;
    char language[20];
    
    // The menu of interactive choices (entering 4 will exit the program).
    while (choice != 4) {
      printf("\n1. Show movies released in the specified year\n");
      printf("2. Show highest rated movie for each year\n");
      printf("3. Show the title and year of release of all movies in a specific language\n");
      printf("4. Exit from the program\n\n");
      
      printf("Enter a choice from 1 to 4: ");
      scanf("%d", &choice);
      
      // Checking if the entered choice is valid and executing the chosen function.
      if (choice < 1 || choice > 4) {
        printf("You entered an incorrect choice. Try again.\n");
      } else if (choice == 1) {
        printf("Enter the year for which you want to see movies: ");
        scanf("%d", &year);
        printYear(list, year);
      } else if (choice == 2) {
        printRatings(list);
      } else if (choice == 3) {
        printf("Enter the language for which you want to see movies: ");
        scanf("%s", language);
        printLanguage(list, language);
      }
    }
  
    freeMovies(list);
  }
  
  return 0;
}
