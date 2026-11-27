the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
#include <unistd.h>
#endif

int caf_num_images = -1;

/* Implementation of secure_getenv() for targets where it is not
   provided. */
