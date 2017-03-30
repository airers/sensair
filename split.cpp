#if FALSE

#include <iostream>
#include <cstring>
#include <stdlib.h>     /* malloc, free, rand */
using namespace std;


char * input = "ABD65,123,ASD,EEE\0GARBAGE";

/**
 * Simulates doing a input.split(',')[section]
 * @param  output          output memory space (must be allocated)
 * @param  input           input memory (char*) assumes it's a string
 * @param  section         which section you want after "splitting"
 */
void getSplitSection(char* output, char * input, int section ) {
  int start = 0;
  int end = 0;
  int i = 0;
  for (int s = 0; s < section + 1; s++ ) {
    i++; // Move one index up. Assumes the first char is not a comma
    start = !end?0:end+1; // Set the start as the previous end + 1
    while(input[i] && input[i] != ',') i++; // Move iterator to next pointer
    end = i; // Set the end index
    if (!input[i]) break; // Break on null terminator
  }
  if ( start == end ) return;
  memcpy(output, input + start, end - start); // Do actual copying
  output[end - start] = '\0'; // Null terminate the string
}

int main() {
  char * output = (char*)malloc(10);
  cout << (char*)input << endl;
  getSplitSection(output, input, 0);
  cout << output << endl;

  getSplitSection(output, input, 1);
  cout << atoi(output) + 2 << endl;

}

#endif
