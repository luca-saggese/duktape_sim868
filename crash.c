#include "eat_type.h"
#include <eat_interface.h>
#include <eat_periphery.h>
#include <math.h>
#include "main.h"
#include "internal.h"


//#include "mma8452.h"

u8 eat_cal_static(u8 scale){
  float accelG[3]={0};
  float rot[3]={0};
  float sum=0;
  u8 i;
  eat_mma_read(scale, &accelG[0]);
  sum=sqrt(accelG[0]*accelG[0] + accelG[1]*accelG[1] + accelG[2]*accelG[2]);
  eat_trace("matrix1 X:%f Y:%f Z:%f",accelG[0],accelG[1],accelG[2]);
  if(sum < 0.97 || sum > 1.0){
    eat_trace("are we mooving (%f)??\ncalibration aborted", sum);
    return 0;
  }
  
  //B=Math.atan(acc.X/acc.Z)
  rot_matrix.B=atan(accelG[0]/accelG[2]);
  //rotazione intorno Y di B
  rot[1]=accelG[1];
  rot[0]=accelG[0] * cos(rot_matrix.B) - accelG[2] * sin(rot_matrix.B);
  rot[2]=accelG[0] * sin(rot_matrix.B) + accelG[2] * cos(rot_matrix.B);
  eat_trace("matrix2 X:%f Y:%f Z:%f",rot[0],rot[1],rot[2]);
  //var G=Math.atan(rot.Y/rot.Z)
  rot_matrix.G=atan(rot[1]/rot[2]);
  eat_trace("rot_matrix A:%f B:%f G:%f",rot_matrix.A/3.14*180, rot_matrix.B/3.14*180, rot_matrix.G/3.14*180);

  return 1;
}

u8 eat_acc_rotate(float *accelG, float*dest){
  float rot[3]={0};
  float rot1[3]={0};
  rot[1]=accelG[1];
  rot[0]=accelG[0] * cos(rot_matrix.B) - accelG[2] * sin(rot_matrix.B);
  rot[2]=accelG[0] * sin(rot_matrix.B) + accelG[2] * cos(rot_matrix.B);

  rot1[0]=rot[0];
  rot1[1]=rot[1] * cos(rot_matrix.G) - rot[2] * sin(rot_matrix.G);
  rot1[2]=rot[1] * sin(rot_matrix.G) + rot[2] * cos(rot_matrix.G);

  dest[2]=rot1[2];
  dest[0]=rot1[0] * cos(rot_matrix.A) - rot1[1] * sin(rot_matrix.A);
  dest[1]=rot1[0] * sin(rot_matrix.A) + rot1[1] * cos(rot_matrix.A);
  return 1;
}
