/*
 compile: 
 cc -nx -o lat lat.c -I/usr/local/mpi/include -L/usr/local/mpi/lib/paragon/nx -lmpi

 start program:
 isub -sz 2 ./lat 
 */


#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#define   NUM_TESTS       100   /* # tests for one masurement */
#define   BAND_TESTS      100   /* # masurement for bandwith average */
#define   LATENCY_TESTS   1000  /* # masurement for latency average */
#define   MAX_LEN         (1024L*1024L*16L)

 /*  ___________________________________________________  */
   static void lat_s  ( int partner );
   static void lat_r  ( int partner );
   static void band_s ( int partner );
   static void band_r ( int partner );
 /*  ___________________________________________________  */
static void  print_status ( int Node_number )
{
   time_t   zeit;

   zeit = time (NULL);

   printf ( "This ist Rank %d   %s", Node_number, ctime (&zeit) );
   system ( "uname -a" );
   printf ( "\n" );
}

 /*  ___________________________________________________  */
static char   *Buf;
static int     Numnode;
static int     Mynode;

 /*  ___________________________________________________  */
static void print_decription ( void )
{
  char *text1[] = {
"This Program tries to masure the bandwidth and latency of this",
"system using the MPI library. The Program needs to run on two nodes.",
"All timings are in milli seconds, bandwidth in MBytes per second",
  NULL };

  char *text2[] = { 
   NULL           };

   int x;

   for ( x = 0; text1[x] != NULL; x++ )
     printf ( "  %s\n" , text1[x] );

   for ( x = 0; text2[x] != NULL; x++ )
     printf ( "  %s\n" , text2[x] );

}

int main ( int argc, char *argv[] )
{

  int x;
  char *c_ptr;

  if ( ( Buf = malloc ( MAX_LEN ) ) == NULL )
    {
       fprintf ( stderr, " can not alloc memory \n" );
       exit ( 1 );
    }

  for ( c_ptr = Buf, x = 0; x < MAX_LEN; x++, c_ptr++ )
    *c_ptr = 'a';


  MPI_Init ( &argc, &argv );
  MPI_Comm_size ( MPI_COMM_WORLD, &Numnode );
  MPI_Comm_rank ( MPI_COMM_WORLD, &Mynode );

  printf ( "  This ist Node %d of %d nodes\n", Mynode, Numnode );

  if ( Numnode < 2 || (Numnode % 2) == 1 )
    MPI_Abort ( MPI_COMM_WORLD, 17 );
    
  print_status ( Mynode );
  if ( Mynode == 0 )
    {
      print_decription ();
    }


  MPI_Barrier ( MPI_COMM_WORLD );

  if ( (Mynode % 2) == 0 )
    {
      lat_s  ( Mynode+1 );
      band_s ( Mynode+1 );
    }
  else
    {
      lat_r  ( Mynode-1 );
      band_r ( Mynode-1 );
    }

  MPI_Finalize ();
  return ( 0 );

}

/*  __________________ LATENCY _____________________________ */

void print_lat ( double sum, double max, double min )
{
  printf(
"     average: %9.5lf   min: %9.5lf max: %9.5lf\n",
      (sum*1000.0)/( LATENCY_TESTS * ( 2 * NUM_TESTS) ),
      (min*1000.0)/(2*NUM_TESTS),
      (max*1000.0)/(2*NUM_TESTS)
	);
    
}

static void lat_s ( int partner )
{
  double  beg, end, min, max, s_time;
  long    cnt;
  long    mpt;
  long    x;
  double  z;
  MPI_Status  MPI_stat;

  printf ( "  Latency:\n" );

  s_time = max = 0.0;
  min = 99999000.0;

  for ( x = 0; x < LATENCY_TESTS; x++ )
    {
      MPI_Send ( Buf, 0, MPI_BYTE, partner, 4711, MPI_COMM_WORLD );
      MPI_Recv ( Buf, 0, MPI_BYTE, partner, 4712, MPI_COMM_WORLD, &MPI_stat );

      beg = MPI_Wtime ();
      for ( cnt = 0; cnt < NUM_TESTS; cnt++ )
        {
           MPI_Send ( Buf, 0, MPI_BYTE, partner, 4713, MPI_COMM_WORLD );
           MPI_Recv ( Buf, 0, MPI_BYTE, partner, 4714, MPI_COMM_WORLD, &MPI_stat );
        }
      end = MPI_Wtime ();
      z = end - beg;
      s_time += z;
      if ( z > max )
        max = z;
      if ( z < min )
        min = z;
    }

  print_lat ( s_time, max, min );

}

static void lat_r ( int partner )
{
  double  beg, end, min, max, s_time;
  long    cnt;
  long    mpt;
  long    x;
  double  z;
  MPI_Status  MPI_stat;


  s_time = max = 0.0;
  min = 99999000.0;

  for ( x = 0; x < LATENCY_TESTS; x++ )
    {
      MPI_Recv ( Buf, 0, MPI_BYTE, partner, 4711, MPI_COMM_WORLD, &MPI_stat );
      MPI_Send ( Buf, 0, MPI_BYTE, partner, 4712, MPI_COMM_WORLD );

      beg = MPI_Wtime ();
      for ( cnt = 0; cnt < NUM_TESTS; cnt++ )
        {
           MPI_Recv ( Buf, 0, MPI_BYTE, partner, 4713, MPI_COMM_WORLD, &MPI_stat );
           MPI_Send ( Buf, 0, MPI_BYTE, partner, 4714, MPI_COMM_WORLD );
        }
      end = MPI_Wtime ();
      z = end - beg;
      s_time += z;
      if ( z > max )
        max = z;
      if ( z < min )
        min = z;
    }

}

/* _________________________ BANDWITH  __________________________ */

void print_band ( long len, double sum, double max, double min )
{
  double  n_mb;

  n_mb = ( (double)len ) / ((double)(1024L*1024L));

  printf( "  % 10ld     %9.5lf    %9.5lf    %9.5lf         %.5lf\n"
      , len, 
      n_mb / ( sum / ( BAND_TESTS * ( 2 * NUM_TESTS) ) ) ,
      n_mb / ( max / (2*NUM_TESTS) ),
      n_mb / ( min / (2*NUM_TESTS) ),
      ( (sum * 1000.0) / ( BAND_TESTS * ( 2 * NUM_TESTS) ) )
	);
    
}

static void band_s ( int p )
{
  double   beg, end, min, max, s_time;
  long     cnt, loop, len;
  MPI_Status  MPI_stat;

  printf ( "\n" );
  printf ( "  Bandwidth:\n" );
  printf (
"     msg len       average        min          max        avr.-time 1 msg\n"
"      bytes        mbyte/s      mbyte/s      mbyte/s        milli sec\n"
	 );


  for ( len = 1; len <= MAX_LEN; len *= 2 )
    {
      long x;
      double  z;

      s_time = max = 0.0;
      min = 99999000.0;

      for ( x = 0; x < BAND_TESTS; x++ )
	{
          MPI_Barrier ( MPI_COMM_WORLD );
	  MPI_Send ( Buf, 0, MPI_BYTE, p, 4811, MPI_COMM_WORLD );
	  MPI_Recv ( Buf, 0, MPI_BYTE, p, 4812, MPI_COMM_WORLD, &MPI_stat );

          beg = MPI_Wtime ();
          for ( cnt = 0; cnt < NUM_TESTS; cnt++ )
            {
		MPI_Send ( Buf, len, MPI_BYTE, p, 4713, MPI_COMM_WORLD );
		MPI_Recv ( Buf, len, MPI_BYTE, p, 4714, MPI_COMM_WORLD, &MPI_stat );
            }
          end = MPI_Wtime ();

	  z = end - beg;
	  s_time += z;
	  if ( z > max )
	    max = z;
	  if ( z < min )
	    min = z;
	}

      print_band (  len, s_time, max, min );
    }
}

static void band_r ( int p )
{
  double   beg, end, min, max, s_time;
  long     cnt, loop, len;
  MPI_Status  MPI_stat;

  for ( len = 1; len <= MAX_LEN; len *= 2 )
    {
      long x;
      double  z;

      s_time = max = 0.0;
      min = 99999000.0;

      for ( x = 0; x < BAND_TESTS; x++ )
	{
          MPI_Barrier ( MPI_COMM_WORLD );
	  MPI_Recv ( Buf, 0, MPI_BYTE, p, 4811, MPI_COMM_WORLD, &MPI_stat );
	  MPI_Send ( Buf, 0, MPI_BYTE, p, 4812, MPI_COMM_WORLD );

          beg = MPI_Wtime ();
          for ( cnt = 0; cnt < NUM_TESTS; cnt++ )
            {
		MPI_Recv ( Buf, len, MPI_BYTE, p, 4713, MPI_COMM_WORLD, &MPI_stat );
		MPI_Send ( Buf, len, MPI_BYTE, p, 4714, MPI_COMM_WORLD );
            }
          end = MPI_Wtime ();

	  z = end - beg;
	  s_time += z;
	  if ( z > max )
	    max = z;
	  if ( z < min )
	    min = z;
	}
    }
}

