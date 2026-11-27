/* Static variables.  */

__thread int _gfortrani_caf_this_image = -1;
int caf_num_images = -1;
pthread_barrier_t sync_all_barrier;
pthread_t *tidlist;

