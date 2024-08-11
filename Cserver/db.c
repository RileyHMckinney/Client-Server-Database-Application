//Assignment 6 - Riley Mckinney - rhm220001
#include <stdio.h>     // for sscanf, fprintf, perror
#include <stdint.h>    // for int32_t
#include <assert.h>    // for assert
#include <sys/types.h> // for read, write, lseek
#include <fcntl.h>     // for read, write, lseek
#include <unistd.h>    // for read, write, lseek
#include <inttypes.h>  // for SCNd32
#include "sr.h"	       // the student record struct


// Get the values of all the fields of student record sr from stdin
// Put the record in the appropriate offset of the file described by fd
void
put(int32_t fd)
{
	sr s;

	printf("Enter the student name: ");	
	
	// WRITE THE CODE to read the name from stdin
	// store it in s.name
	// use fgets()
	// fgets doesnt remove newline. replace '\n' with '\0' in s.name. strlen() will be useful
	if (fgets(s.name, sizeof(s.name), stdin) != NULL) {
	  int i;
	  for(i = 0; s.name[i] != '\0'; i++){
	    if(s.name[i] == '\n') {
	      s.name[i] = '\0';
	      break;
	    }		
	  }  
	}else {
          fprintf(stderr, "Error Reading Input\n");
  	  return;
	}

	printf("Enter the student id: ");
	//
	// WRITE THE CODE to read student id from stdin
	// store it in s.id
	if(scanf("%" SCNd32 , &s.id) != 1) {
	  fprintf(stderr, "Error Reading Input\n");
	  return;
	}

	printf("Enter the record index: ");
	//
	// WRITE THE CODE to read record index from stdin
	// store it in s.index
	if(scanf("%" SCNd32 , &s.index) != 1) {
	  fprintf(stderr, "Error Reading Input\n");	
	  return;
	}

	//This is extra code warning to user when they choose a high index value:
	if(s.index >= 1000) {
	  printf("Keep in mind that the index you entered is high, and may take up more file space.\n");
	}
	
	// WRITE THE CODE to seek to the appropriate offset in fd (lseek(), sizeof() will be useful)
	off_t offset = lseek(fd, sizeof(s)* s.index, SEEK_SET);
	if(offset == (off_t) -1) {
	  perror("lseek failure");
	  return;
	}

	// WRITE THE CODE to write record s to fd
	ssize_t bytes_written = write(fd, &s, sizeof(s));
	if(bytes_written == -1) {
	  perror("Failure to Write to file");
	  return;
	}else if (bytes_written < sizeof(s)) {
	  perror("Incomplete file write");
	  return;
	}
}

// read the student record stored at position index in fd
void
get(int32_t fd)
{
	sr s;
	int32_t index;

	printf("Enter the record index: ");
	//
	// WRITE THE CODE to get record index from stdin and store in it index
        if(scanf("%" SCNd32 , &index) != 1) {
	  fprintf(stderr, "Error Reading Input\n");
          return;
        }
    // WRITE THE CODE to seek to the appropriate offset in fd
    // The record index may be out of bounds. If so, 
    // print appropriate message and return
    // Check SEEK_DATA on the man page for lseek 
        off_t offset = lseek(fd, sizeof(s)* index, SEEK_SET);
	if(offset == (off_t) -1) {
           perror("lseek failure");
	   return;
	}		           

    // WRITE THE CODE to read record s from fd
    // If the record has not been put already, print appropriate message
    // and return
	ssize_t bytes_read = read(fd, &s, sizeof(s));	
	if(bytes_read == -1) {
	  perror("Failed to read from file");
	  return;
	} else if (bytes_read < sizeof(s)){
	  fprintf(stderr, "Index is out of bounds\n");
	  return;
	}

	printf("Student name %s \n", s.name);	
	printf("Student id: %d \n", s.id);
	printf("Record index: %d \n", s.index);

	assert(index == s.index);
}
