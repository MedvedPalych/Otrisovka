// *******************************************************************
// RisMO.cpp
// Отрисовка модели изделия через Open GL.
// все в один исходник, ибо в комплексе их и так уже много
// лаб. 0144 2016г.
// *******************************************************************

#include <math.h>
#include <vcl.h>
#include <glu.h>
#include <stdio.h>
#pragma hdrstop
#include "RisMO.h"

#define RAZMER 1. // коэффициент увеличения моделей
//#define TEXTURY   // использование текстур

using namespace RisMO;
namespace RisMO {

#ifndef TEXTURY
  void glTexCoord2f(float, float) {}
#endif

  // системные дескрипторы
  HWND hwnd;  // окно
  HDC hdc;    // устройство
  HGLRC hrc;  // контекст рендеринга

  GLUquadricObj *quadObj;

  // списки отрисовки
  #define SPISKOV 5
  GLuint spSpisok=0,
    spHvost,
    spKorpus,
    spRN,
    spEVR,
    spEVL,
    texture[2];

#ifdef TEXTURY
  // *******************************************************************
  struct BMP {
    unsigned char header[54]; // Каждый БМП начинается с заголовка в 54 байта
    unsigned int dataPos,     // Смещение данных в файле (позиция данных)
      width, height,
      imageSize;              // Ширина * высота * 3
    unsigned char *data;      // RGB data 
    void LoadBMP(const char *path);
  } zvezda, shashki;

  void BMP::LoadBMP(const char *path) {
  // загрузка БМП файла (строго 24-х битных!)
    FILE *f = fopen(path, "rb");
    if (!f)
      throw ("нет файла текстуры");
    // Читаем заголовок файла
    if (fread(header, 1, 54, f) != 54 || header[0] != 'B' || header[1] != 'M')
      throw ("файл текстуры - не BMP");
    dataPos   =*(int*)&(header[0x0A]); // смещение данных изображения в файле
    imageSize =*(int*)&(header[0x22]);
    width     =*(int*)&(header[0x12]);
    height    =*(int*)&(header[0x16]);
    // бывает, что поля imageSize и dataPos нулевые в файле, надо это учесть
    if (imageSize == 0) imageSize=width*height*3;
    if (dataPos   == 0) dataPos=54;
    // зная размер изображения, выделим область памяти под него
    data = new unsigned char[imageSize];  // буфер
    fread(data,1,imageSize,f);         // читаем из файла в буфер
    fclose(f);                         // больше файл не нужен
  }
  // *******************************************************************
#endif

  // *******************************************************************
  double PodgonRulei(double rm) {  // для отображения рулей
    return(45*sqrt(fabs(rm)));
  }
  // *******************************************************************

  // *******************************************************************
  void MnaV(double m[3][3], double v[3], double r[3]) {
  // умножение матрицы <m> на вектор <v> и запись результата в <r>
    for (int i=0; i < 3; i++) {
      r[i]=0;
      for (int j=0; j < 3; j++)
        r[i]+=m[j][i]*v[j];
    }
  }
  // *******************************************************************

  // *******************************************************************
  void M33v44(double m33[3][3], double m44[4][4]) { // матрицу 3х3 в матрицу 4х4
    for (int i=0; i < 3; i++)
      for (int j=0; j < 3; j++)
        m44[i][j]=m33[j][i]; // матрица транспонирована
    m44[0][3]=0;
    m44[1][3]=0;
    m44[2][3]=0;
    m44[3][0]=0;
    m44[3][1]=0;
    m44[3][2]=0;
    m44[3][3]=1;
  }
  // *******************************************************************

  // *******************************************************************
  void NormV(double v[3]) { // нормировка вектора
  double mod=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
    for (int i=0; i<3; i++)
      v[i]=v[i]/mod;
  }
  // *******************************************************************

  // *******************************************************************
  //*** цвет небесного фона в зависимости от высоты **/
  void FonVysoty(int H) {
  static const double
    R[5]={ 1.4783e-19,-1.7030e-14,-7.1163e-12, 3.8315e-05,-0.3551},
    G[5]={ 5.6115e-19,-1.0043e-13, 5.8714e-09,-1.3457e-04, 1.6246},
    B[5]={-6.7906e-19, 1.1804e-13,-7.2528e-09, 1.8401e-04,-0.7265};

    if (H <= 16000)
      glClearColor(0.196078, 0.6, 0.8, 1);
    else if (H <  80000)
      glClearColor((((R[0]*H + R[1])*H + R[2])*H + R[3])*H + R[4],
                   (((G[0]*H + G[1])*H + G[2])*H + G[3])*H + G[4],
                   (((B[0]*H + B[1])*H + B[2])*H + B[3])*H + B[4],1);
    else
      glClearColor(0.0, 0.0, 0.2, 1);
  }
  // *******************************************************************

  // *******************************************************************
  void Trafaret() { // списки отрисовки
    spSpisok=glGenLists(SPISKOV);
    spHvost=spSpisok;
    glNewList(spHvost, GL_COMPILE);
      glBegin(GL_TRIANGLES);
        glVertex3f(-1.0*RAZMER, 1.00*RAZMER, 0.00*RAZMER);
        glVertex3f( 2.0*RAZMER,-0.10*RAZMER, 0.00*RAZMER);  // вертикальное оперение лево
        glVertex3f(-1.0*RAZMER, 0.00*RAZMER,-0.05*RAZMER);

        glVertex3f( 2.0*RAZMER,-0.10*RAZMER, 0.00*RAZMER);  // вертикальное оперение право
        glVertex3f(-1.0*RAZMER, 1.00*RAZMER, 0.00*RAZMER);
        glVertex3f(-1.0*RAZMER, 0.00*RAZMER, 0.05*RAZMER);
      glEnd();
    glEndList();

    spKorpus=spHvost+1;
    glNewList(spKorpus, GL_COMPILE);
      glBegin(GL_TRIANGLES);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);  // гор. плат. верх лево
        glTexCoord2f(0.5f, 0.4f); glVertex3f(-1.0*RAZMER,-0.1*RAZMER,-1.00*RAZMER);
        glTexCoord2f(1.0f, 0.4f); glVertex3f(-1.0*RAZMER, 0.0*RAZMER,-0.05*RAZMER);

        glTexCoord2f(0.0f, 1.0f); glVertex3f( 2.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);  // гор. плат. верх право
        glTexCoord2f(0.0f, 0.4f); glVertex3f(-1.0*RAZMER, 0.0*RAZMER, 0.05*RAZMER);
        glTexCoord2f(0.5f, 0.4f); glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 1.00*RAZMER);

        glTexCoord2f(0.5f, 0.8f); glVertex3f( 2.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);  // гор. плат. низ
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 1.00*RAZMER);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0*RAZMER,-0.1*RAZMER,-1.00*RAZMER);

        glVertex3f(-1.0*RAZMER, 0.0*RAZMER,-0.05*RAZMER); // задние стенки корпуса
        glVertex3f(-1.0*RAZMER,-0.1*RAZMER,-1.00*RAZMER); // грани у них нам не нужны,
        glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);

        glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 1.00*RAZMER);
        glVertex3f(-1.0*RAZMER, 0.0*RAZMER, 0.05*RAZMER);
        glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);
      glEnd();
      glBegin(GL_QUADS);
        glVertex3f(-1.0*RAZMER, 0.0*RAZMER, 0.05*RAZMER);
        glVertex3f(-1.0*RAZMER, 1.0*RAZMER, 0.00*RAZMER);
        glVertex3f(-1.0*RAZMER, 0.0*RAZMER,-0.05*RAZMER);
        glVertex3f(-1.0*RAZMER,-0.1*RAZMER, 0.00*RAZMER);
      glEnd();
    glEndList();

    spRN=spKorpus+1;
    glNewList(spRN, GL_COMPILE);
      glBegin(GL_QUADS); // руль направления левая сторона
        glVertex3f(0.0*RAZMER,  1.0*RAZMER, 0.00*RAZMER);
        glVertex3f(0.0*RAZMER,  0.0*RAZMER,-0.05*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.3*RAZMER, 0.00*RAZMER);
        glVertex3f(-0.3*RAZMER, 1.0*RAZMER, 0.00*RAZMER);
        // руль направления правая сторона
        glVertex3f(0.0*RAZMER,  0.0*RAZMER, 0.05*RAZMER);
        glVertex3f(0.0*RAZMER,  1.0*RAZMER, 0.00*RAZMER);
        glVertex3f(-0.3*RAZMER, 1.0*RAZMER, 0.00*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.3*RAZMER, 0.00*RAZMER);
      glEnd();
      glBegin(GL_TRIANGLES); // руль направления нижняя толщина
        glVertex3f( 0.0*RAZMER, 0.0*RAZMER, 0.05*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.3*RAZMER, 0.00*RAZMER);
        glVertex3f( 0.0*RAZMER, 0.0*RAZMER,-0.05*RAZMER);
        // руль направления задняя толщина
        glVertex3f( 0.0*RAZMER, 0.0*RAZMER,-0.05*RAZMER);
        glVertex3f( 0.0*RAZMER, 1.0*RAZMER, 0.00*RAZMER);
        glVertex3f( 0.0*RAZMER, 0.0*RAZMER, 0.05*RAZMER);
      glEnd();
    glEndList();

    spEVL=spRN+1;
    glNewList(spEVL, GL_COMPILE);
      glBegin(GL_QUADS);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-1.00*RAZMER);
        glVertex3f(-0.3*RAZMER,-0.05*RAZMER,-1.00*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.00*RAZMER,-0.30*RAZMER);
        glVertex3f( 0.0*RAZMER, 0.05*RAZMER,-0.05*RAZMER); // левый закрылок верхняя часть

        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-0.05*RAZMER); // левый закрылок нижняя часть
        glVertex3f(-0.3*RAZMER, 0.00*RAZMER,-0.30*RAZMER);
        glVertex3f(-0.3*RAZMER,-0.05*RAZMER,-1.00*RAZMER);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-1.00*RAZMER);
      glEnd();
      glBegin(GL_TRIANGLES);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-0.05*RAZMER); // левый закрылок задняя толщина
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-1.00*RAZMER);
        glVertex3f( 0.0*RAZMER, 0.05*RAZMER,-0.05*RAZMER);

        glVertex3f( 0.0*RAZMER, 0.05*RAZMER,-0.05*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.00*RAZMER,-0.30*RAZMER); // левый закрылок правая толщина
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER,-0.05*RAZMER);
      glEnd();
    glEndList();

    spEVR=spEVL+1;
    glNewList(spEVR, GL_COMPILE);
      glBegin(GL_QUADS);
        glVertex3f( 0.0*RAZMER, 0.05*RAZMER, 0.05*RAZMER); // правый закрылок верхняя часть
        glVertex3f(-0.3*RAZMER, 0.00*RAZMER, 0.30*RAZMER);
        glVertex3f(-0.3*RAZMER,-0.05*RAZMER, 1.00*RAZMER);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 1.00*RAZMER);

        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 0.05*RAZMER); // правый закрылок нижняя часть
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 1.00*RAZMER);
        glVertex3f(-0.3*RAZMER,-0.05*RAZMER, 1.00*RAZMER);
        glVertex3f(-0.3*RAZMER, 0.00*RAZMER, 0.30*RAZMER);
      glEnd();
      glBegin(GL_TRIANGLES);
        glVertex3f( 0.0*RAZMER, 0.05*RAZMER, 0.05*RAZMER); // правый закрылок задняя толщина
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 1.00*RAZMER);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 0.05*RAZMER);

        glVertex3f(-0.3*RAZMER, 0.00*RAZMER, 0.30*RAZMER); // правый закрылок левая толщина
        glVertex3f( 0.0*RAZMER, 0.05*RAZMER, 0.05*RAZMER);
        glVertex3f( 0.0*RAZMER,-0.05*RAZMER, 0.05*RAZMER);
      glEnd();
    glEndList();
  }
  // *******************************************************************

  // *******************************************************************
  void Otrisovka(double usC[3][3], double *V, double H, double Lsf, double Nst,
                 double deltaRN, double deltaEVL, double deltaEVR, double Q) {
  double rul1, rul2, rul3;
  double m44[4][4];
  double V1[3]; // скорость в ССК
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // сброс буферов
    FonVysoty(H);
    M33v44(usC, m44);  // матрицу 3х3 в GL 4х4
    glLoadMatrixd(*m44); // текущей видовой матрицей принимается наша МНК

    if (Nst == 3) { // самолётик
      // *** корпус
      double zakrut=180*Lsf/(3.14*6371); // по формуле длины дуги сектора  (движ. по сфере)
        glColor3f(0.8,0.3,0.2);
        glRotatef(zakrut+5.35, 0.0, 0.0, 1.0); // доворот в плоскость крыла
  #ifdef TEXTURY
        glEnable(GL_TEXTURE_2D);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,zvezda.width,zvezda.height,0,0x80E0,GL_UNSIGNED_BYTE,zvezda.data);
  #endif
        glCallList(spKorpus);
        glCallList(spHvost);
  #ifdef TEXTURY
        glDisable(GL_TEXTURE_2D);
  #endif

      // *** Рули
      if (deltaRN < 0)               // рули отклоняются
        rul1=-PodgonRulei(deltaRN);  // в пределах 1 градуса
      else                           // рисуем с масштабированием
        rul1=PodgonRulei(deltaRN);   // 1 градус = 45 на рисунке
      if (deltaEVL < 0)
        rul2=-PodgonRulei(deltaEVL);
      else
        rul2=PodgonRulei(deltaEVL);
      if (deltaEVR < 0)
        rul3=-PodgonRulei(deltaEVR);
      else
        rul3=PodgonRulei(deltaEVR);
      if (rul1 > 90)         // искуственные
        rul1 = 90;           // упоры
      else if (rul1 < -90)   // реальный другой
        rul1 =-90;           // рисуем 90
      if (rul2 > 90)
        rul2 = 90;
      else if (rul2 < -90)
        rul2 =-90;
      if (rul3 > 90)
        rul3 = 90;
      else if (rul3 < -90)
        rul3 =-90;
      // руль направления
      glPushMatrix();
        glTranslatef(-1.05*RAZMER, 0.0*RAZMER, 0.0*RAZMER);
        glRotatef(rul1, 0,1,0);
        glColor3f(0.8,0.3,0.2);
        glCallList(spRN);
      glPopMatrix();
      // левый закрылок
      glPushMatrix();
        glTranslatef(-1.05*RAZMER,-0.05*RAZMER, 0.0*RAZMER);
        glRotatef(rul2, 0,0,1);
        glColor3f(0.8,0.3,0.2);
        glCallList(spEVL);
      glPopMatrix();
      // правый закрылок
      glPushMatrix();
        glTranslatef(-1.02*RAZMER,-0.05*RAZMER, 0.0*RAZMER);
        glRotatef(rul3, 0,0,1);
        glColor3f(0.8,0.3,0.2);
        glCallList(spEVR);
      glPopMatrix();

      // вектор скорости,
      MnaV(usC, V, V1);  // скорость из ИСК в ССК
      NormV(V1);
      glColor3f(1,1,1);
    glBegin(GL_LINES);
      glVertex3f(2.0*RAZMER,-0.1*RAZMER,0.0*RAZMER); // начало в носике,
      if (Q < 0.01) Q=0.01;
      glVertex3f(V1[0]*0.3*log(Q)*RAZMER,     // конец с умножением на
                 V1[1]*0.3*log(Q)*RAZMER,     // коэф.
                 V1[2]*0.3*log(Q)*RAZMER);
    glEnd();
    } else { // батон
      glColor3f(0.22,0.26,0.13);
      glRotatef(90, 0,1,0);
      glTranslatef(0*RAZMER,0*RAZMER,-1.7*RAZMER);
  #ifdef TEXTURY
      glEnable(GL_TEXTURE_2D);
      glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,shashki.width,shashki.height,0,0x80E0,GL_UNSIGNED_BYTE,shashki.data);
  #endif
      gluQuadricTexture(quadObj, GL_TRUE);
      gluQuadricDrawStyle(quadObj, GLU_FILL);
      gluCylinder(quadObj, 0.5*RAZMER, 0.5*RAZMER, 2.9*RAZMER, 12, 12);
      glTranslatef(0*RAZMER,0*RAZMER,2.9*RAZMER);
      gluCylinder(quadObj, 0.5*RAZMER, 0.0*RAZMER, 1.3*RAZMER, 12, 1);
  #ifdef TEXTURY
      glDisable(GL_TEXTURE_2D);
  #endif

    }
    SwapBuffers(hdc); // выводим буфер на экран, а во втором отрисовываем след. картинку
  }  // Otrisovka
  
  // *******************************************************************
  bool Otkryt(HWND ahwnd, int W, int H) { // запуск OpenGL и подготовка картинки
  PIXELFORMATDESCRIPTOR pfd;
  int pixelformat;
    hwnd=ahwnd;
    hdc=GetDC(hwnd);

    memset(&pfd,0,sizeof(pfd));
    pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion=1;
    pfd.dwFlags=PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.dwLayerMask=PFD_MAIN_PLANE;
    pfd.iPixelType=PFD_TYPE_RGBA;  // формат указания цвета
    pfd.cColorBits=24; // глубина цвета
    pfd.cDepthBits=32; // размер Z-буфера
    pixelformat=ChoosePixelFormat(hdc, &pfd);
    if ((pixelformat == 0) || (SetPixelFormat(hdc, pixelformat, &pfd) == FALSE))
      return false;
    hrc=wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);
    quadObj=gluNewQuadric();
    gluQuadricDrawStyle(quadObj, GLU_FILL);

    glMatrixMode(GL_PROJECTION);     // матрица видового преобразования
    glLoadIdentity();                // заменяет матрицу на единичную
    glViewport(0, 0, W, H);
    // плоскости отсечения (left, right, bottom, top, near, far)
    if (W > H)
      glFrustum(-W/(double)H,W/(double)H,-1,1, 2,10);
    else
      glFrustum(-1,1,-H/(double)W,H/(double)W, 2,10);
    gluLookAt(0,0,5, 0,0,0, 0,1,0);  // параметры камеры: куда смотрим, откуда и направление осей

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // -----------СВЕТ------------------------
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);     // разрешили Z-буфер
    glEnable(GL_COLOR_MATERIAL); // разрешили цвет объектам
    glEnable(GL_LIGHT0);
    GLfloat light_dif[]={1.0, 1.0, 1.0, 1.0}; // цвет света
    GLfloat light_dir[]={-0.5, 1.5, 1.0, 1.0}; // направление света
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glLightfv(GL_LIGHT0, GL_POSITION, light_dir);
    glColorMaterial(GL_FRONT,GL_SPECULAR);
    glColorMaterial(GL_FRONT,GL_DIFFUSE);
    glMaterialf(GL_FRONT,GL_SHININESS, 105);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
#ifdef TEXTURY
    shashki.LoadBMP("shashki.bmp");  // некрасиво
    zvezda.LoadBMP("zvezda.bmp");    // поэтому не дефайним текстуры
    glGenTextures(2, texture);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
#endif
    Trafaret();
    return true;
  }

  // *******************************************************************
  void Zakryt() { // конец работы с OpenGL
    gluDeleteQuadric(quadObj);
    // А текстуры ??
    if (spSpisok) {
      glDeleteLists(spSpisok,SPISKOV);
      spSpisok=0;
#ifdef TEXTURY
      glDeleteTextures(2, texture);
      delete zvezda.data;
      delete shashki.data;
#endif
    }
    if (hrc) {
      wglMakeCurrent(hdc,0);
      wglDeleteContext(hrc);
      hrc=0;
    }
    if (hdc) {
      ReleaseDC(hwnd, hdc);
      hdc=0;
    }
  }

} // namespace
