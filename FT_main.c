//Face Tracking
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<process.h>
#include<cv.h>
#include<highgui.h>
#include<math.h>

#define MAXIM 1000
#define factor 2
#define sw_size 5
#define fw_var 5

#define rescale    for(y=0;y<frame->height;y=y+factor)                                                 \
                   {                                                                                   \
                          (uchar*)ptr1=(uchar*)(frame->imageData+y*frame->widthStep);                  \
                          (uchar*)ptr2=(uchar*)(frame_RGB->imageData+y/factor*frame_RGB->widthStep);   \
                          for(x=0;x<frame->width;x=x+factor)                                           \
                          {                                                                            \
                                    ptr2[3*x/factor+0]=(uchar) ptr1[3*x+0];                            \
                                    ptr2[3*x/factor+1]=(uchar) ptr1[3*x+1];                            \
                                    ptr2[3*x/factor+2]=(uchar) ptr1[3*x+2];                            \
                          }                                                                            \
                   }                                                                            

extern void sendval();
extern void transfer(int xcoord,int ycoord);
extern void elips(int *y[MAXIM],int *y2[MAXIM],int b,float e,int r,int c,int *cn1,int *cn2); 
extern void gradientx(IplImage *frame_GRADx,IplImage *frame_GRADy,IplImage *frame_GRAD);
extern float correlate(float *x,float *y,int n);

int main(int argc,char* argv[])
{
    __asm__("push ebp;"
            "mov ebp,esi;"
            "mov dx,0x03fb;"
            "mov al,0x83;"
            "out dx,al;"
            "mov dx,0x03f8;"
            "mov al,0x0c;"
            "out dx,al;"
            "mov dx,0x03f9;"
            "mov al,0x00;"
            "out dx,al;"
            "mov dx,0x03fb;"
            "mov al,0x03;"
            "out dx,al;"
            "mov dx,0x03fa;"
            "mov al,0x07;"
            "out dx,al;"
            "mov dx,0x03fc;"
            "mov al,0x00;"
            "out dx,al;"
            "pop ebp;");
    
    printf("\nWelcome to blob tracking program... Press any key to launch\n");
    getch();    
    int flag=0,step,channels,step2,channels2;
    uchar *data,*data2;
    float alpha=0.99,sum_grad;
    float binsH_M[256],binsS_M[256],binsV_M[256],binsH_I[256],binsS_I[256],binsV_I[256],Phi_g_M,Phi_g_I;
    float Match_scores[(2*sw_size+1)*(2*sw_size+1)*fw_var][6];
    float binsH_Log[(2*sw_size+1)*(2*sw_size+1)*fw_var][256];
    float binsS_Log[(2*sw_size+1)*(2*sw_size+1)*fw_var][256];
    float binsV_Log[(2*sw_size+1)*(2*sw_size+1)*fw_var][256];
    float rho_H,rho_S,rho_V,Hist_match,PC_scores;
    
    CvCapture *capture=cvCreateCameraCapture(0);
    IplImage *frame,*frame_RGB,*frame_HSV,*frame_GRAY,*frame_GRAY2,*frame_GRAD,*frame_GRADx,*frame_GRADy;
    frame=cvQueryFrame(capture);
    frame_RGB=cvCreateImage(cvSize(frame->width/factor,frame->height/factor),IPL_DEPTH_8U,3);
    frame_HSV=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_8U,3);
    frame_GRAY=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_8U,1);
    frame_GRAY2=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_32F,1);
    frame_GRAD=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_32F,2);
    frame_GRADx=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_32F,1);
    frame_GRADy=cvCreateImage(cvSize(frame_RGB->width,frame_RGB->height),IPL_DEPTH_32F,1); 
    
    step=frame_GRAD->widthStep;
    channels=frame_GRAD->nChannels;
    //data=(uchar *)frame_GRAD->imageData;    
    step2=frame_HSV->widthStep;
    channels2=frame_HSV->nChannels;
    data2=(uchar *)frame_HSV->imageData;    
    
    uchar* ptr1;
    uchar* ptr2;    
    float* ptr3;    
    int x,y,i,j,b,s_r,s_c;      
    char c;
    
    int *p[MAXIM],*p2[MAXIM],fw_size=30,rtloc=150,ctloc=200,sp,spa,delY,delX,cnt1,cnt2,ind1,ind2,ind3,ind4,SrNo;
    float e=0.5,Theta,del_Theta,dot_product,max_Score,max_Hist,min_Hist,max_Grad,min_Grad;//,sum;
    
    cvNamedWindow("UI",CV_WINDOW_AUTOSIZE);    
    
    CvMat kernelx,kernely;
    float valsx[]={0.0,0.0,0.0,-1.0,0.0,1.0,0.0,0.0,0.0};
    float valsy[]={0.0,-1.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0};
    kernelx=cvMat(3,3,CV_32FC1,valsx);
    kernely=cvMat(3,3,CV_32FC1,valsy);

    
    //|==================================================|//
    //|========== MODEL INITIALIZATION ROUTINE ==========|//
    //|==================================================|//
    while(flag==0)
    {
            sum_grad=0.0; //sum=0.0;
            cnt1=0;
            for(i=0;i<256;i++)
            {
                 binsH_M[i]=0.0;
                 binsS_M[i]=0.0;
                 binsV_M[i]=0.0;
            }
            frame=cvQueryFrame(capture);
            rescale;
            cvCvtColor(frame_RGB,frame_HSV,CV_RGB2HSV);
            cvCvtColor(frame_RGB,frame_GRAY,CV_RGB2GRAY);
            cvConvertScale(frame_GRAY,frame_GRAY2,1.0,0.0);
            cvFilter2D(frame_GRAY2,frame_GRADx,&kernelx,cvPoint(-1,-1));
            cvFilter2D(frame_GRAY2,frame_GRADy,&kernely,cvPoint(-1,-1));                     
            gradientx(frame_GRADx,frame_GRADy,frame_GRAD);
            for(i=0;i<7*fw_size;i++)
            {
                 p[i]=(int *)malloc(2*sizeof(int));
                 p2[i]=(int *)malloc(2*sizeof(int));
            }
            elips(p,p2,fw_size,e,rtloc,ctloc,&sp,&spa); //printf("\n%d",sp); getch();
            for(i=0;i<sp;i++)
            {
                 (uchar*)ptr1=(uchar*)(frame_RGB->imageData+p[i][0]*frame_RGB->widthStep);
                 ptr1[3*p[i][1]+1]=255;
            }
            
            //GRADIENT MODULE//
            for(i=0;i<sp;i++)
            {
                 ind1=fmod(sp+i-6,sp);
                 ind2=fmod(sp+i+6,sp);
                 ind3=fmod(sp+i+6,sp);
                 ind4=fmod(sp+i-6,sp);
                 delY=p[ind1][0]-p[ind2][0];
                 delX=p[ind3][1]-p[ind4][1];
                 Theta=atan(delY/(delX+0.0000001));
                 Theta=(180/3.142)*Theta;
                 if(delX>=0)
                     Theta=fmod(360-Theta+90,360);
                 else
                     Theta=180-Theta+90;
                 //printf("\n%f",data[p[i][0]*step+p[i][1]*channels+1]); getch();
                 (uchar*)data=(uchar*)(frame_GRAD->imageData+p[i][0]*step); ptr3=(float*)data; 
                 del_Theta=abs(Theta-ptr3[p[i][1]*channels+1]);
                 dot_product=ptr3[p[i][1]*channels+0]*cos(del_Theta*(3.142/180));
                 sum_grad+=dot_product;
                 //printf("\n%f",cos(del_Theta*(3.142/180))); getch();
            }
            
            //for(i=0;i<7*fw_size;i++)
//            {                 
//                 free(*(p+i));
//                 //free(*(p2+i));
//            }
 
            
            //COLOR MODULE//
            for(i=1;i<spa-1;i++)
            {
                 for(j=2*rtloc-p2[i][0];j<=p2[i][0];j++)
                 {
                      binsH_M[data2[j*step2+p2[i][1]*channels2+0]]+=1;
                      binsS_M[data2[j*step2+p2[i][1]*channels2+1]]+=1;
                      binsV_M[data2[j*step2+p2[i][1]*channels2+2]]+=1;
                      cnt1+=1;
                 }
            } 
            Phi_g_M=sum_grad/sp;
            for(i=0;i<256;i++)
            {
                 binsH_M[i]=binsH_M[i]/cnt1;
                 binsS_M[i]=binsS_M[i]/cnt1;
                 binsV_M[i]=binsV_M[i]/cnt1;
                 //printf("\n%f\t%f\t%f",binsH_M[i],binsS_M[i],binsV_M[i]); 
            }
            //getch();
            //for(i=0;i<256;i++)
//            {
//                 sum+=binsH_M[i];
//            }
            //printf("\n%f",Phi_g_M);                    
            cvShowImage("UI",frame_RGB);
            c=cvWaitKey(1);
            if (c==116) {flag=1; break;}      //t
            if (c==119) rtloc=rtloc-1;        //w
            if (c==115) rtloc=rtloc+1;        //s
            if (c==97) ctloc=ctloc-1;         //a
            if (c==100) ctloc=ctloc+1;        //d
            if (c==111) fw_size=fw_size+1;    //o
            if (c==108) fw_size=fw_size-1;    //l
    }      
    
    
    
    //|===============================================|//
    //|============== SEARCH PROCESS =================|//
    //|===============================================|//    
    while(1)
    {                   
            frame=cvQueryFrame(capture);
            rescale;
            cvCvtColor(frame_RGB,frame_HSV,CV_RGB2HSV);
            cvCvtColor(frame_RGB,frame_GRAY,CV_RGB2GRAY);
            cvConvertScale(frame_GRAY,frame_GRAY2,1.0,0.0);
            cvFilter2D(frame_GRAY2,frame_GRADx,&kernelx,cvPoint(-1,-1));
            cvFilter2D(frame_GRAY2,frame_GRADy,&kernely,cvPoint(-1,-1));                     
            gradientx(frame_GRADx,frame_GRADy,frame_GRAD);
            cnt2=0;
            max_Score=0.0;
            for(s_r=rtloc-sw_size;s_r<=rtloc+sw_size;s_r++)
            {
                 for(s_c=ctloc-sw_size;s_c<=ctloc+sw_size;s_c++)
                 {
                      for(b=fw_size-2;b<=fw_size+2;b++)
                      {
                            sum_grad=0.0;
                            cnt1=0;
                            for(i=0;i<256;i++)
                            {
                                 binsH_I[i]=0.0;
                                 binsS_I[i]=0.0;
                                 binsV_I[i]=0.0;
                            }
                            for(i=0;i<7*b;i++)
                            {
                                 p[i]=(int *)malloc(2*sizeof(int));
                                 p2[i]=(int *)malloc(2*sizeof(int));
                            }
                            elips(p,p2,b,e,s_r,s_c,&sp,&spa);
            
                            //GRADIENT MODULE//
                            for(i=0;i<sp;i++)
                            {
                                 ind1=fmod(sp+i-6,sp);
                                 ind2=fmod(sp+i+6,sp);
                                 ind3=fmod(sp+i+6,sp);
                                 ind4=fmod(sp+i-6,sp);
                                 delY=p[ind1][0]-p[ind2][0];
                                 delX=p[ind3][1]-p[ind4][1];
                                 Theta=atan(delY/(delX+0.0000001));
                                 Theta=(180/3.142)*Theta;
                                 if(delX>=0)
                                     Theta=fmod(360-Theta+90,360);
                                 else
                                     Theta=180-Theta+90; 
                                 (uchar*)data=(uchar*)(frame_GRAD->imageData+p[i][0]*step); ptr3=(float*)data; 
                                 del_Theta=abs(Theta-ptr3[p[i][1]*channels+1]);
                                 dot_product=ptr3[p[i][1]*channels+0]*cos(del_Theta*(3.142/180));
                                 sum_grad+=dot_product;
                            } 
                            //for(i=0;i<7*b;i++)
//                            {
//                                 free(p+i);
//                                 free(p2+i);
//                            }     
                      
                            //COLOR MODULE//
                            for(i=1;i<spa-1;i++)
                            {
                                 for(j=2*rtloc-p2[i][0];j<=p2[i][0];j++)
                                 {
                                      binsH_I[data2[j*step2+p2[i][1]*channels2+0]]+=1;
                                      binsS_I[data2[j*step2+p2[i][1]*channels2+1]]+=1;
                                      binsV_I[data2[j*step2+p2[i][1]*channels2+2]]+=1;
                                      cnt1+=1;
                                 }
                            } 
                            Phi_g_I=sum_grad/sp;
                            for(i=0;i<256;i++)
                            {
                                 binsH_I[i]=binsH_I[i]/cnt1;
                                 binsS_I[i]=binsS_I[i]/cnt1;
                                 binsV_I[i]=binsV_I[i]/cnt1;
                            }
                            rho_H=correlate(binsH_I,binsH_M,256);
                            rho_S=correlate(binsS_I,binsS_M,256);
                            rho_V=correlate(binsV_I,binsV_M,256);
                            Hist_match=0.33*rho_H+0.33*rho_S+0.33*rho_V;
                            Match_scores[cnt2][0]=s_r;
                            Match_scores[cnt2][1]=s_c;
                            Match_scores[cnt2][2]=b;
                            Match_scores[cnt2][3]=Phi_g_I;
                            Match_scores[cnt2][4]=Hist_match; //printf("\n%f",Match_scores[cnt2][0]); getch();
                            for(i=0;i<256;i++)
                            {
                                 binsH_Log[cnt2][i]=binsH_I[i];
                                 binsS_Log[cnt2][i]=binsS_I[i];
                                 binsV_Log[cnt2][i]=binsV_I[i];
                            }
                            cnt2++;
                      }
                 }
            }
            max_Grad=min_Grad=Match_scores[0][3];
            max_Hist=min_Hist=Match_scores[0][4];
            for(i=1;i<cnt2;i++)
            {
                 if(Match_scores[i][3]>max_Grad)
                    max_Grad=Match_scores[i][3];
                 if(Match_scores[i][3]<min_Grad)
                    min_Grad=Match_scores[i][3];
                 if(Match_scores[i][4]>max_Hist)
                    max_Hist=Match_scores[i][4];
                 if(Match_scores[i][4]<min_Hist)
                    min_Hist=Match_scores[i][4];
            }
            for(i=0;i<cnt2;i++)
            {
                 Match_scores[i][5]=(((Match_scores[i][3]-min_Grad)/(max_Grad-min_Grad))*50)+(((Match_scores[i][4]-min_Hist)/(max_Hist-min_Hist))*50);
                 //printf("\n%f\t%f\t%f\t%f\t%f",Match_scores[i][0],Match_scores[i][1],Match_scores[i][2],Match_scores[i][3],Match_scores[i][5]); getch();
                 if(Match_scores[i][5]>max_Score)
                 {
                     max_Score=Match_scores[i][5];            
                     SrNo=i;
                 }
            }
            //getch();
            rtloc=(int)Match_scores[SrNo][0];
            ctloc=(int)Match_scores[SrNo][1];
            fw_size=(int)Match_scores[SrNo][2];
            Phi_g_M=Match_scores[SrNo][3];
            for(i=0;i<256;i++)
            {
                 binsH_M[i]=alpha*binsH_M[i]+(1-alpha)*binsH_Log[SrNo][i];
                 binsS_M[i]=alpha*binsS_M[i]+(1-alpha)*binsS_Log[SrNo][i];
                 binsV_M[i]=alpha*binsV_M[i]+(1-alpha)*binsV_Log[SrNo][i];
            }           
            
            
            //=== overlay ellipse ===// 
            for(i=0;i<7*fw_size;i++)
            {
                 p[i]=(int *)malloc(2*sizeof(int));
                 p2[i]=(int *)malloc(2*sizeof(int));
            }
            elips(p,p2,fw_size,e,rtloc,ctloc,&sp,&spa);
            for(i=0;i<sp;i++)
            {
                 (uchar*)ptr1=(uchar*)(frame_RGB->imageData+p[i][0]*frame_RGB->widthStep);
                 ptr1[3*p[i][1]+1]=255;
            }
            //=======================//                            
            
            transfer(ctloc,rtloc);
            cvShowImage("UI",frame_RGB);           
            c=cvWaitKey(1);
            if (c==27) break;
    }
        
    cvReleaseCapture(&capture);    
    cvDestroyWindow("UI");
    return 0;
}
