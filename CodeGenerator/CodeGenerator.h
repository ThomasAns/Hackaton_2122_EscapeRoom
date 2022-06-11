#pragma once
#include <math.h>
#define PI 3.14159265359
class CodeGenerator
{
public:
	static int* getRandomCode(int input);
private:
	static double getAbs(double num);
};