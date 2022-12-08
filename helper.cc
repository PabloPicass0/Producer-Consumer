# include "helper.h"

bool validInput(int argc, char** argv) {
  //Checks if command line inputs are 5 in total
  if (argc != 5) {
    cerr << "Error msg: number of command line arguments must be 5\n";
    return false;
  }

  //Checks if all of those inputs all are of numeric value (exluding the executable)
  for (int count = 1; count < argc; count++) {
    char *string = argv[count];
    while (*string != '\0') {
      if (!isdigit(*string)) {
	cerr << "Error msg: " << argv[count] << " is not a numeric character\n";
	return false;
      }
      string++;
    }
  }
  
  return true;
}
