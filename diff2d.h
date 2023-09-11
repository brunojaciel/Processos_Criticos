float LUT[256];

struct thread_args
{
  int      imax;
  float    ht;        /* time step size, >0, e.g. 0.5 */
  float    lambda;    /* contrast parameter */
  long     nx;        /* image dimension in x direction */
  long     ny;        /* image dimension in y direction */
  float    **f;
  int indexMat;
};

float dco 
      (float v,         /* value at one point */
       float w,         /* value at the other point */
       float lambda);   /* contrast parameter */

float dco_LUT 
      (float i);   /* index */

void diff2d 
     (float    ht,        /* time step size */
      float    lambda,    /* contrast parameter */
      long     nx,        /* image dimension in x direction */ 
      long     ny,        /* image dimension in y direction */ 
      float    **f);      /* input: original image ;  output: smoothed */

void *diff2d_thread_half
      (void *threadarg);
      

