#pragma once
#include "NRC.h"

class NRC_BaseInterp
{
public:
	NRC_BaseInterp::~NRC_BaseInterp();

	NRC_BaseInterp::NRC_BaseInterp(VecDoub_I &x, int xsize, const Doub *y, Int m);
	NRC_BaseInterp::NRC_BaseInterp(const Doub *x, int xsize, const Doub *y, Int m);

	Doub NRC_BaseInterp::interp(Doub x);
	Int NRC_BaseInterp::locate(const Doub x);
	Int NRC_BaseInterp::hunt(const Doub x);

	Doub virtual rawinterp(Int jlo, Doub x) = 0;

	Int n, mm, jsav, cor, dj;
	const Doub *xx, *yy;
};
