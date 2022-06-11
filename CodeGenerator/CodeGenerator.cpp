#include "CodeGenerator.h"
int* CodeGenerator::getRandomCode(int input) {

    const double multiplication = 1034.8123;
    double vorigGegenereerdGetal;

    //getal 1
    double nieuwGetal = getAbs(sin(input)) * multiplication * PI;
    int getal1 = floor(nieuwGetal);
    getal1 %= 10;
    vorigGegenereerdGetal = nieuwGetal;

    //getal2
    nieuwGetal = getAbs(cos(vorigGegenereerdGetal)) * multiplication;
    int getal2 = floor(nieuwGetal);
    getal2 %= 10;
    vorigGegenereerdGetal = nieuwGetal;
    //getal3
    nieuwGetal = log(vorigGegenereerdGetal) * multiplication;
    int getal3 = floor(nieuwGetal);
    getal3 %= 10;
    vorigGegenereerdGetal = nieuwGetal;

    //getal4
    nieuwGetal = getAbs(tan(sqrt(vorigGegenereerdGetal)) * multiplication);
    int getal4 = floor(nieuwGetal);
    getal4 %= 10;

    static int output[5];
    output[0] = getal1;
    output[1] = getal2;
    output[2] = getal3;
    output[3] = getal4;
    double num = getAbs((getal1 + getal2) * (getal3 + getal4) + vorigGegenereerdGetal - nieuwGetal) * (multiplication / PI);
    output[4] = (int)num % 255;


    return output;
};
double CodeGenerator::getAbs(double num) {
    return num > 0 ? num : -num;
}