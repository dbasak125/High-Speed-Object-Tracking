#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<process.h>
#include<math.h>
#include<cv.h>
#include<highgui.h>

void elips(int *y[1000],int *y2[1000],int b,float e,int r,int c,int *cn1,int *cn2)
{
    int x=-b,j,cnt1=0,cnt2,cnt3=0,temp;    
    y[cnt1][0]=sqrt((b*b-x*x)/(1-e*e));
    y[cnt1][1]=x;
    y2[cnt3][0]=y[cnt1][0];
    y2[cnt3][1]=y[cnt1][1]; //printf("\n%d\t%d\t%d",cnt3,y2[cnt3][0],y2[cnt3][1]); getch();
    cnt1++; cnt3++;
    for(x=-b+1;x<=b;x++)
    {
        y[cnt1][0]=sqrt((b*b-x*x)/(1-e*e));
        y[cnt1][1]=x;
        y2[cnt3][0]=y[cnt1][0];
        y2[cnt3][1]=y[cnt1][1];        
        //printf("\n%d\t%d\t%d",cnt3,y2[cnt3][0],y2[cnt3][1]); getch();
        cnt3++;
        if(y[cnt1][0]-y[cnt1-1][0]>1)
        {
             temp=y[cnt1][0];
             for(cnt2=y[cnt1-1][0]+1;cnt2<=temp;cnt2++)
             {
                  y[cnt1][0]=cnt2;
                  y[cnt1][1]=x;
                  cnt1++;
             }
             continue;
        }
        if(y[cnt1-1][0]-y[cnt1][0]>1)
        {
             temp=y[cnt1][0];
             for(cnt2=y[cnt1-1][0]-1;cnt2>temp;cnt2--)
             {
                  y[cnt1][0]=cnt2;
                  y[cnt1][1]=x-1;
                  cnt1++;
             }
             y[cnt1][0]=temp;
             y[cnt1][1]=x;
             cnt1++;
             continue;
        }
        cnt1++; 
    }
    for(j=cnt1-2;j>0;j--)
    {
        y[cnt1][0]=-y[j][0];
        y[cnt1][1]=y[j][1]; //printf("\n%d\t%d\t%d",cnt1,y[cnt1][0],y[cnt1][1]); getch();       
        cnt1++;
    }     
    for(j=0;j<cnt1;j++)
    {
        y[j][0]=r+y[j][0];
        y[j][1]=c+y[j][1];
        //printf("\n%d\t%d\t%d",cnt1,y[j][0],y[j][1]); getch();
    }
    for(j=0;j<cnt3;j++)
    {
        y2[j][0]=r+y2[j][0];
        y2[j][1]=c+y2[j][1];
    }
    //printf("cnt1=%d\n\n",cnt1);
//    printf("cnt3=%d\n\n",cnt3);
//    for(j=0;j<cnt3;j++)
//    {
//         printf("%d\t%d",y2[j][0],y2[j][1]);
//         printf("\n");
//    }
//    getch();


    *cn1=cnt1; *cn2=cnt3;
    return;
}        

void gradientx(IplImage *frame_GRADx,IplImage *frame_GRADy,IplImage *frame_GRAD)
{
    uchar *ptrs4,*ptrs5,*ptrs6;
    float *ptrs1,*ptrs2,*ptrs3;
    int y,x;
    float Theta;
    for(y=0;y<frame_GRADx->height;y++)                                                 
    {                                                                                   
          (uchar*)ptrs4=(uchar*)(frame_GRADx->imageData+y*frame_GRADx->widthStep); ptrs1=(float*)ptrs4;                 
          (uchar*)ptrs5=(uchar*)(frame_GRADy->imageData+y*frame_GRADy->widthStep); ptrs2=(float*)ptrs5;
          (uchar*)ptrs6=(uchar*)(frame_GRAD->imageData+y*frame_GRAD->widthStep); ptrs3=(float*)ptrs6;   
          for(x=0;x<frame_GRADx->width;x++)                                           
          {                                                                            
                  ptrs3[2*x+0]=sqrt(ptrs1[x]*ptrs1[x]+ptrs2[x]*ptrs2[x]);
                  Theta=atan(ptrs2[x]/(ptrs1[x]+0.00000001));
                  Theta=(180/3.142)*Theta;
                  if(ptrs1[x]>=0)
                      Theta=fmod(360-Theta,360);
                  else
                      Theta=180-Theta;
                  ptrs3[2*x+1]=Theta; 
                  //printf("\n%.2f\t%.2f\t%d\t%.2f\t%.2f",ptrs1[x],ptrs2[x],x,ptrs3[2*x+0],ptrs3[2*x+1]); getch();                                                          
          }                                                                                       
    }
    return;
}

float correlate(float *x,float *y,int n)
{
    float sxx,syy,sxy,mula,mulcx,mulcy,sumx,sumy,rho;
    int i;
    mulcx=mulcy=mula=sumx=sumy=0.0;
    for(i=0;i<n;i++)
    {
         mulcx+=x[i]*x[i];
         mulcy+=y[i]*y[i];
         mula+=x[i]*y[i];
         sumx+=x[i];
         sumy+=y[i];
    }
    sxx=mulcx-((sumx*sumx)/n);
    syy=mulcy-((sumy*sumy)/n);
    sxy=mula-((sumx*sumy)/n);
    rho=sxy/sqrt(sxx*syy);
    return(rho);
}
         
void transfer(int xcoord,int ycoord)
{
    int val,xcoord0,xcoord1,ycoord0,ycoord1;
    if(xcoord>255)
	{
		xcoord0=255;
		xcoord1=xcoord-255;
	}
	else
	{
		xcoord0=xcoord;
		xcoord1=0;
	}

	if(ycoord>255)
	{
		ycoord0=255;
		ycoord1=ycoord-255;
	}
	else
	{
		ycoord0=ycoord;
		ycoord1=0;
	}
    sendval(xcoord0);
    printf("\nx0:%3d   ",xcoord0);
    sendval(xcoord1);
    printf("x1:%3d      ",xcoord1);            
    sendval(ycoord0);
    printf("y0:%3d   ",ycoord0);
    sendval(ycoord1);
    printf("y1:%3d",ycoord1);
}

