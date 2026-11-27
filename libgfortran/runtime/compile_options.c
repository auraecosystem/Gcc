extern void set_options (int , int []);
export_proto(set_options);

/* Thread-local storage which image we are in.  */

__thread int _gfortrani_caf_this_image = -1;

void
set_options (int num, int options[])
{
