#pragma once
#include "Glacier/Math/SVector3.h"
struct EditBone
{
	SVector3 headPosition;
	SVector3 tailPosition;
	float length;
	EditBone* parent;
	float roll;
};