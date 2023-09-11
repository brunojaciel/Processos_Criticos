#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pgmfiles.h"
#include "diff2d.h"
#include <inttypes.h>
#include <x86intrin.h>
#include <stdint.h>
//PTHREAD
#include <pthread.h>

#pragma intrinsic(__rdtsc)

//gcc -o fda pgmtolist.c pgmfiles.c diff2d.c main.c -lm

float  **matrix;

void threadFuction (long imax, float lambda, eightBitPGMImage *PGMImage, int indexMat){

  int aStatus;
  int bStatus;
  //PTHREADS
  pthread_t thread_idA;
  pthread_t thread_idB;
  void * thread_res;

  /* --- MATRIX THREAD A ---*/
  float **matrixA;
  float **matrixB;
  int i, j;
  matrixA = (float **) malloc ((PGMImage->x - indexMat) * sizeof(float *));

  if (matrixA == NULL)
    { 
      printf("not enough storage available\n");
      exit(1);
    } 
  for (i=0; i<PGMImage->x - indexMat; i++)
    {
      matrixA[i] = (float *) malloc (PGMImage->y * sizeof(float));
      if (matrixA[i] == NULL)
        { 
	  printf("not enough storage available\n");
	  exit(1);
        }
    }
  
  /* ---- read image data into matrix ---- */
  
 for (i=0; i<(PGMImage->x - indexMat); i++)
    for (j=0; j<PGMImage->y; j++)
      matrixA[i][j] = matrix[i][j];

  /* --- MATRIX THREAD B ---*/

  matrixB = (float **) malloc ((PGMImage->x - indexMat) * sizeof(float *));

  if (matrixB == NULL)
    { 
      printf("not enough storage available\n");
      exit(1);
    } 
  for (i=0; i<PGMImage->x - indexMat; i++)
    {
      matrixB[i] = (float *) malloc (PGMImage->y * sizeof(float));
      if (matrixB[i] == NULL)
        { 
	  printf("not enough storage available\n");
	  exit(1);
        }
    }
  
  /* ---- read image data into matrix ---- */
  
 for (i=0; i<(PGMImage->x - indexMat); i++)
    for (j=0; j<PGMImage->y; j++)
      matrixB[i][j] = matrix[i + indexMat][j]; 

  /* --- MATRIX STRUCT A ---*/
  struct thread_args thread_dataA;

  thread_dataA.ht = 0.5;
  thread_dataA.lambda = lambda;
  thread_dataA.nx = PGMImage->x;
  thread_dataA.ny = PGMImage->y;
  thread_dataA.f = matrixA;
  thread_dataA.imax = imax;
  thread_dataA.indexMat = indexMat;

  /* --- MATRIX STRUCT B ---*/
  struct thread_args thread_dataB;

  thread_dataB.ht = 0.5;
  thread_dataB.lambda = lambda;
  thread_dataB.nx = PGMImage->x;
  thread_dataB.ny = PGMImage->y;
  thread_dataB.f = matrixB;
  thread_dataB.imax = imax;
  thread_dataB.indexMat = indexMat;
  
  /*tenta iniciar o thread, indicando a função 'diff2d' e passando como argumento a struct "thread_args"*/
  aStatus = pthread_create (&thread_idA, NULL, diff2d_thread_half, (void *) &thread_dataA);
  bStatus = pthread_create (&thread_idB, NULL, diff2d_thread_half, (void *) &thread_dataB);
 
  /*verificar se ocorreu algum erro na chamada de pthread_create*/
  
  if(aStatus || bStatus != 0)
  {
    printf ("Erro ao criar o thread.\n");
    exit(EXIT_FAILURE);
  }
  
  printf ("Thread criado com sucesso!\n");

  aStatus = pthread_join (thread_idA, &thread_res);

  if(aStatus != 0)
  {
    printf ("Erro ao aguardar finalização do thread1.\n");
    exit(EXIT_FAILURE);
  }

  printf ("Thread finalizado! %s\n", (char *)thread_res);
  
  bStatus = pthread_join (thread_idB, &thread_res);
 
  if(bStatus != 0)
  {
    printf ("Erro ao aguardar finalização do thread2.\n");
    exit(EXIT_FAILURE);
  }
  
  printf ("Thread finalizado! %s\n", (char *)thread_res);
  
  /* ---- write matrixA/matrixB data into matrix ---- */
  
 for (i=0; i<PGMImage->x; i++)
    for (j=0; j<PGMImage->y; j++){
      if(i<indexMat){
        matrix[i][j] = matrixA[i][j]; 
      } else {
        matrix[i][j] = matrixB[i-indexMat][j];
      } 
    }
       
}

void main (int argc, char **argv) {

  //TIME
  uint64_t beginTP, endTP, cycles_spentTP;
  uint64_t beginResponseTimeDiff, endResponseTimeDiff, cycles_spentResponseTimeDiff;
  uint64_t beginReadImage, endReadImage, cycles_spentReadImage;
  uint64_t beginWriteImage, cycles_spentWriteImage;
  beginTP = __rdtsc();
  //----------
  
  beginReadImage = __rdtsc();
  char   row[80];
  //float  **matrix;
  int i, j;
  FILE   *inimage, *outimage;
  long   imax;
  float  lambda;
  int result;
  eightBitPGMImage *PGMImage;
  
  /* ---- read image name  ---- */
  
  PGMImage = (eightBitPGMImage *) malloc(sizeof(eightBitPGMImage));
	
  if (!argv[1])
  {
	  printf("name of input PGM image file (with extender): ");
    scanf("%s", PGMImage->fileName);
  }
  else
  {
    strcpy(PGMImage->fileName, argv[1]);
  }

  result = read8bitPGM(PGMImage);

  if(result < 0) 
    {
      printPGMFileError(result);
      exit(result);
    }

  /* ---- allocate storage for matrix ---- */
  
  matrix = (float **) malloc (PGMImage->x * sizeof(float *));
  if (matrix == NULL)
    { 
      printf("not enough storage available\n");
      exit(1);
    } 
  for (i=0; i<PGMImage->x; i++)
    {
      matrix[i] = (float *) malloc (PGMImage->y * sizeof(float));
      if (matrix[i] == NULL)
        { 
	  printf("not enough storage available\n");
	  exit(1);
        }
    }
  
  /* ---- read image data into matrix ---- */
  
 for (i=0; i<PGMImage->x; i++)
    for (j=0; j<PGMImage->y; j++)
      matrix[i][j] = (float) *(PGMImage->imageData + (i*PGMImage->y) + j); 
  

  endReadImage = __rdtsc();
  /* ---- process image ---- */
  
  //printf("contrast paramter lambda (>0) : ");
  //~ gets(row);  sscanf(row, "%f", &lambda);
  //scanf("%f", &lambda);
  lambda = 15;
  //printf("number of iterations: ");
  //~ gets(row);  sscanf(row, "%ld", &imax);
  imax = 500;
  //scanf("%ld", &imax);
  beginResponseTimeDiff = __rdtsc();
  /*-----------------------------------------------------*/
  // LUT
  
  for (i=0; i<256; i++){
    LUT[i] =  (1.0 - exp(-8.0 * 0.5 * dco_LUT(i))) / 8.0;
    //printf("%f\n", LUT[i]);
  }
  
  /*-----------------------------------------------------*/
  
  threadFuction (imax, lambda, PGMImage, 128);

  /*
  for (i=1; i<=imax; i++)
    {
      //printf("iteration number: %3d \n", i);
      diff2d (0.5, lambda, PGMImage->x, PGMImage->y, matrix); 
    }
  */
  endResponseTimeDiff = __rdtsc();
   
  /*-----------------------------------------------------*/
  beginWriteImage = __rdtsc();
  /* copy the Result Image to PGM Image/File structure */

  for (i=0; i<PGMImage->x; i++)
    for (j=0; j<PGMImage->y; j++)
      *(PGMImage->imageData + i*PGMImage->y + j) = (char) matrix[i][j];

  /* ---- write image ---- */
  
  if (!argv[2])
  {
    printf("name of output PGM image file (with extender): ");
    scanf("%s", PGMImage->fileName);
  }
  else
  {
    strcpy(PGMImage->fileName, argv[2]);
  }

  write8bitPGM(PGMImage);
  
  /* ---- disallocate storage ---- */
  
  for (i=0; i<PGMImage->x; i++)
    free(matrix[i]);
  free(matrix);

  free(PGMImage->imageData);
  free(PGMImage);

  //TIME
  endTP = __rdtsc();
  
  cycles_spentTP = (endTP - beginTP);
  cycles_spentResponseTimeDiff = (endResponseTimeDiff - beginResponseTimeDiff);
  cycles_spentReadImage = (endReadImage - beginReadImage);
  cycles_spentWriteImage = (endTP - beginWriteImage);
  printf("\nTotal: %" PRIu64 "\n", cycles_spentTP);
  printf("Processamento imagem: %" PRIu64 "\n", cycles_spentResponseTimeDiff);
  printf("Leitura imagem: %" PRIu64 "\n", cycles_spentReadImage);
  printf("Escrita imagem: %" PRIu64 "\n", cycles_spentWriteImage);
}


