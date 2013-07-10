// © 2013 the Search Authors under the MIT license. See AUTHORS for the list of authors.

#include "leastsquares.hpp"

// The lapack routine.
extern "C" void dgels_( char* trans, int* m, int* n, int* nrhs, double* a, int* lda, double* b, int* ldb, double* work, int* lwork, int* info );

bool leastsquares(int nrows, int ncols, double Ain[], double bin[], double x[]) {
	bool res = false;
	char trans[] = "No transpose";

	// Copy A over, and convert it to column-major.
	double *A = new double[nrows*ncols];
	for (int i = 0; i < nrows; i++)
	for (int j = 0; j < ncols; j++)
		A[j*nrows+i] = Ain[i*ncols+j];

	double *b = new double[nrows];
	for (int i = 0; i < nrows; i++)
		b[i] = bin[i];

	int nrhs = 1;
	int lda = nrows;
	int ldb = nrows;
	int info;

	// Get a good work size.
	int lwork = -1;
	double wksz;
	double *work = 0;
	dgels_(trans, &nrows, &ncols, &nrhs, A, &lda, b, &ldb, &wksz,
		&lwork, &info);
	if (info > 0)
		goto out;
	lwork = (int) wksz;
	work = new double[lwork];

	// Solve.
	dgels_(trans, &nrows, &ncols, &nrhs, A, &lda, b, &ldb, work,
		&lwork, &info);
	if (info > 0)
		goto out;

	// Copy the solution over.
	for (int i = 0; i < ncols; i++)
		x[i] = b[i];

	res = true;
out:
	if (work)
		delete []work;
	delete []A;
	delete []b;
	return res;
}