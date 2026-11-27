#include <unistd.h>
#endif

int caf_num_images = -1;

/* Implementation of secure_getenv() for targets where it is not
   provided. */
