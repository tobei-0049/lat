#

MPI_CC=mpicc

lat: lat.c
	${MPI_CC}  -o lat lat.c 

#	${MPI_CC}  -o lat lat.c -I/opt/mpi/include -L/opt/mpi/lib/pa1.1 -lmpi   


