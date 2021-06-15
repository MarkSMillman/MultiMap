#pragma once
#include "NRC.h"
#include "NRC_Linbcg.h"

class NRC_Laplace : NRC_Linbcg {
public:
	MatDoub &mat;
	Int ii;
	Int jj;
	Int nn;
	Int iter;
	VecDoub b;
	VecDoub y;
	VecDoub mask;

	MULTIMAP_API NRC_Laplace(MatDoub_IO &matrix);
	MULTIMAP_API double solve(Doub tol = 1.e-6, Int itmax = -1);

private:
	void asolve(VecDoub_I &b, VecDoub_O &x, const Int itrnsp);
	void atimes(VecDoub_I &x, VecDoub_O &r, const Int itrnsp);


};
