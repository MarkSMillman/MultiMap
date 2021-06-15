#pragma once
#include "NRC.h"
#include "MultiMap.h"
class NRC_Linbcg {
public:
	MULTIMAP_API ~NRC_Linbcg(void);
	MULTIMAP_API NRC_Linbcg(void);
	MULTIMAP_API void solve(VecDoub_I &b, VecDoub_IO &x, const Int itol, const Doub tol, const Int itmax, Int &iter, Doub &err);

private:
	virtual void asolve(VecDoub_I &b, VecDoub_O &x, const Int itrnsp) = 0;
	virtual void atimes(VecDoub_I &x, VecDoub_O &r, const Int itrnsp) = 0;
	Doub snrm(VecDoub_I &sx, const Int itol);
};