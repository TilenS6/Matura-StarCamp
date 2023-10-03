#include "phisics/phisics.h"

PhMuscle::PhMuscle(FastCont<PhPoint>* points, int a, int b, double spring_koef = 100, double damp_koef = 10, double originalLength = 0) : PhLink(points, a, b, spring_koef, damp_koef, originalLength) {
    minLenPow2 = lenPow2;
    maxLenPow2 = lenPow2;
    setRange(.3);
}
void PhMuscle::setRange(double a) {
    double len = sqrt(orgLenPow2);
    double min = pow(len * (1 - a), 2);
    double max = pow(len * (1 + a), 2);
    minLenPow2 = min;
    maxLenPow2 = max;
}
void PhMuscle::expand() {
    lenPow2 = maxLenPow2;
}
void PhMuscle::contract() {
    lenPow2 = minLenPow2;
}
void PhMuscle::relax() {
    lenPow2 = orgLenPow2;
}
void PhMuscle::setMuscle(double a) { // min(0)-max(1)
    lenPow2 = minLenPow2 * (1 - a) + maxLenPow2 * a;
}
