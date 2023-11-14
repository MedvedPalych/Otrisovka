// *******************************************************************
// RisMO.h
// Отрисовка модели через Open GL.
// лаб. 0144 2016г.
// *******************************************************************

#ifndef RisMO_H
#define RisMO_H

namespace RisMO {

bool Otkryt(HWND ahwnd, int W, int H);
void Otrisovka(double usC[3][3], double *Vx, double H, double Lsf, double Nst,
               double deltaRN, double deltaEVL, double deltaEVR, double Q);
void Zakryt();
void FonVysoty(int H);
}

#endif
