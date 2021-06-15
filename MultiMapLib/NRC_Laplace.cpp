#include "MultiMap.h"
#include "NRC_Laplace.h"
/**
 * Construct using matrix with sparce data and 1.e99 where values are not known.
 * Then call solve (the default values will usually work).
 * Quite decent results are obtained for smooth functions on 300x300 matricies in which
 * a random 10% of grid points have known values, with 90% interpolation.
 * @see Chapter 3.8 Laplace Interpolation in Numerical Recipes
**/
MULTIMAP_API NRC_Laplace::NRC_Laplace(MatDoub_IO &matrix) :
mat(matrix), ii(mat.nrows()), jj(mat.ncols()), nn(ii*jj), iter(0), b(nn), y(nn), mask(nn) {
	Int i, j, k;
	Doub vl = 0.;
	for (k = 0; k < nn; k++) {
		i = k / jj;
		j = k - i*jj;
		if (mat[i][j] < 1.e99) {
			b[k] = y[k] = vl = mat[i][j];
			mask[k] = 1;
		}
		else {
			b[k] = 0.;
			y[k] = vl;
			mask[k] = 0;
		}
	}
}
MULTIMAP_API double NRC_Laplace::solve(Doub tol, Int itmax) {
	Int i, j, k;
	Doub err;
	if (itmax <= 0) itmax = 2 * MAX(ii, jj);
	NRC_Linbcg::solve(b, y, 1, tol, itmax, iter, err);
	for (k = 0, i = 0; i<ii; i++) for (j = 0; j<jj; j++) mat[i][j] = y[k++];
	return err;
}
void NRC_Laplace::asolve(VecDoub_I &b, VecDoub_O &x, const Int itrnsp) {
	Int i, n = b.size();
	for (i = 0; i<n; i++) x[i] = b[i];
}

void NRC_Laplace::atimes(VecDoub_I &x, VecDoub_O &r, const Int itrnsp) {
	Int i, j, k, n = r.size(), jjt, it;
	Doub del;
	for (k = 0; k<n; k++) r[k] = 0.;
	for (k = 0; k<n; k++) {
		i = k / jj;
		j = k - i*jj;
		if (mask[k]) {
			r[k] += x[k];
		}
		else if (i>0 && i<ii - 1 && j>0 && j<jj - 1) {
			if (itrnsp) {
				r[k] += x[k];
				del = -0.25*x[k];
				r[k - 1] += del;
				r[k + 1] += del;
				r[k - jj] += del;
				r[k + jj] += del;
			}
			else {
				r[k] = x[k] - 0.25*(x[k - 1] + x[k + 1] + x[k + jj] + x[k - jj]);
			}
		}
		else if (i>0 && i<ii - 1) {
			if (itrnsp) {
				r[k] += x[k];
				del = -0.5*x[k];
				r[k - jj] += del;
				r[k + jj] += del;
			}
			else {
				r[k] = x[k] - 0.5*(x[k + jj] + x[k - jj]);
			}
		}
		else if (j>0 && j<jj - 1) {
			if (itrnsp) {
				r[k] += x[k];
				del = -0.5*x[k];
				r[k - 1] += del;
				r[k + 1] += del;
			}
			else {
				r[k] = x[k] - 0.5*(x[k + 1] + x[k - 1]);
			}
		}
		else {
			jjt = i == 0 ? jj : -jj;
			it = j == 0 ? 1 : -1;
			if (itrnsp) {
				r[k] += x[k];
				del = -0.5*x[k];
				r[k + jjt] += del;
				r[k + it] += del;
			}
			else {
				r[k] = x[k] - 0.5*(x[k + jjt] + x[k + it]);
			}
		}
	}
}
