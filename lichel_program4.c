#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Buffer 1, shared resource between input thread and line separator thread
char *buffer_1[50];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between line separator thread and replace pluses thread
char *buffer_2[50];
// Number of items in the buffer
int count_2 = 0;
// Index where the input thread will put the next item
int prod_idx_2 = 0;
// Index where the line separator thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

// Buffer 3, shared resource between replace pluses thread and output thread
char *buffer_3[50];
// Number of items in the buffer
int count_3 = 0;
// Index where the input thread will put the next item
int prod_idx_3 = 0;
// Index where the replace pluses thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

// Coordinate Threads 1 and 4
// Number of items in the buffer
int count_4 = 0;
// Index where the input thread will put the next item
int prod_idx_4 = 0;
// Index where the replace pluses thread will pick up the next item
int con_idx_4 = 0;
// Initialize the mutex for buffer 3
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_4 = PTHREAD_COND_INITIALIZER;

/*
Put an item in buff_1
*/
void put_buff_1(char *item){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_1);
  // Put the item in the buffer
  buffer_1[prod_idx_1] = item;
  // Increment the index where the next item will be put.
  prod_idx_1 = prod_idx_1 + 1;
  count_1++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_1);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
}

/*
Function that the input thread will run.
Get input from the user.
This function doesn't perform any error checking.
Put the item in the buffer shared with the line separator thread.
*/
void* get_input(){
  char buffer[1000];
  int stop = 0;
  
  while (stop == 0) {
    // Get the input into a local buffer
		fgets(buffer, 1000, stdin);   
    put_buff_1(buffer);
    
    // Wait until the print thread has processed this line
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_4);
    
    while (count_4 == 0)
      pthread_cond_wait(&full_4, &mutex_4);
    
    count_4++;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_4);
    
		// If we've passed on "STOP" to the next thread, we can exit
		if (!strncmp(buffer, "STOP\n", 5))
			stop = 1;
	}
 
  return NULL;
}

/*
Get the next item from buffer 1
*/
char* get_buff_1(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_1);
  while (count_1 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_1, &mutex_1);
  char *item = buffer_1[con_idx_1];
  // Increment the index from which the item will be picked up
  con_idx_1 = con_idx_1 + 1;
  count_1--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_1);
  // Return the item
  return item;
}

/*
Put an item in buff_2
*/
void put_buff_2(char *item){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_2);
  // Put the item in the buffer
  buffer_2[prod_idx_2] = item;
  // Increment the index where the next item will be put.
  prod_idx_2 = prod_idx_2 + 1;
  count_2++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_2);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
}

/*
Function that replaces all occurances of '\n' with a space.
Consume an item from the buffer shared with the input thread.
Replace newline with a space.
Produce an item in the buffer shared with the replace pluses thread.
*/
void *replace_seperators(void *args) {
  char *item;
  char newitem[1000] = "";
  int stop = 0;
  
  // Loop that replaces newline with a space.
  while (stop == 0) {
    item = get_buff_1();
    
    // If we've passed on "STOP" to the next thread, we can exit
		if (!strncmp(item, "STOP\n", 5))
			stop = 1;
    else
      item[strlen(item) - 1] = ' ';
    
    put_buff_2(item);
  }
  
  return NULL;
}

/*
Get the next item from buffer 2
*/
char* get_buff_2(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_2);
  while (count_2 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_2, &mutex_2);
  char *item = buffer_2[con_idx_2];
  // Increment the index from which the item will be picked up
  con_idx_2 = con_idx_2 + 1;
  count_2--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_2);
  // Return the item
  return item;
}

/*
Put an item in buff_3
*/
void put_buff_3(char *item){
  // Lock the mutex before putting the item in the buffer
  pthread_mutex_lock(&mutex_3);
  // Put the item in the buffer
  buffer_3[prod_idx_3] = item;
  // Increment the index where the next item will be put.
  prod_idx_3 = prod_idx_3 + 1;
  count_3++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&full_3);
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
}

/*
Function that replaces all occurances of "++" with "^".
Consume an item from the buffer shared with the line separator thread.
Replace "++" with "^".
Produce an item in the buffer shared with the output thread.
*/
void *replace_pluses(void *args) {
  char *item;
  int stop = 0;
  
  // Loop to replace "++" with "^".
  while (stop == 0) {
    item = get_buff_2();
    
    // If we've passed on "STOP" to the next thread, we can exit
		if (!strncmp(item, "STOP\n", 5)) {
			stop = 1;    
    } else {
      int x = 0, length = strlen(item);
      
      // Check the item for buffer for any "++".
      while (x < length) {
        // If a "++" is found, it'll be replaced with "^".
        if (item[x] == '+' && x < length - 1) {
          if (item[x + 1] == '+') {
              item[x] = '^';
              
              for (int i = x + 1; i < length - 1; i++) {
                item[i] = item[i + 1];
              }
              
              item[length - 1] = '\0';
          }
        }
  
        x++;
      }
    }
    
    put_buff_3(item);
  }
}

/*
Get the next item from buffer 3
*/
char* get_buff_3(){
  // Lock the mutex before checking if the buffer has data
  pthread_mutex_lock(&mutex_3);
  while (count_3 == 0)
    // Buffer is empty. Wait for the producer to signal that the buffer has data
    pthread_cond_wait(&full_3, &mutex_3);
  char* item = buffer_3[con_idx_3];
  // Increment the index from which the item will be picked up
  con_idx_3 = con_idx_3 + 1;
  count_3--;
  // Unlock the mutex
  pthread_mutex_unlock(&mutex_3);
  // Return the item
  return item;
}

/*
Function that the output thread will run. 
Consume an item from the buffer shared with the replace pluses thread.
Print the item.
*/
void *write_output(void *args) {
  char *item;
  char line[1000];
  int i = 0, stop = 0;
  
  // Loop that prints out the item.
  while (stop == 0) {
    item = get_buff_3();
    
    // If we've passed on "STOP" to the next thread, we can exit
		if (!strncmp(item, "STOP\n", 5)) {
			stop = 1; 
    } else {   
      // Checks length of the item.
      for (int x = 0; x < strlen(item); x++) {
        line[i] = item[x];
        i++;
        
        // Whenever there's at least 80 characters for an output line, the output line will be produced.
        if (i == 80) {
          line[i] = '\n';
          line[i+1] = '\0';
          fprintf(stdout, line);
          fflush(stdout);
          i = 0;
          line[0] = '\0';
        }
      } 
     
    }
    
    // Let input thread know the line is printed
    pthread_mutex_lock(&mutex_4);
    count_4--;
    pthread_cond_signal(&full_4);
    pthread_mutex_unlock(&mutex_4);
  }
}

int main(int argc, char *argv[]) {
  srand(time(0));
  pthread_t input_t, line_seperator_t, plus_sign_t, output_t;
  
  // Create the threads
  pthread_create(&input_t, NULL, get_input, NULL);
  pthread_create(&line_seperator_t, NULL, replace_seperators, NULL);
  pthread_create(&plus_sign_t, NULL, replace_pluses, NULL);
  pthread_create(&output_t, NULL, write_output, NULL);
  
  // Wait for the threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(line_seperator_t, NULL);
  pthread_join(plus_sign_t, NULL); 
  pthread_join(output_t, NULL); 
  
  return EXIT_SUCCESS;
}