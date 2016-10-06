//tracking
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<process.h>
#include<cv.h>
#include<highgui.h>
#include<windows.h>

extern void sendval();
void transfer(int xcoord,int ycoord);

HANDLE hComm;
OVERLAPPED osWrite = {0};
DWORD dwWritten;

int main(int argc, char* argv[])
{
    
    int num = 200, ret, lp;
    int* num_pntr = &num;
    
    char toWrite [6];
   toWrite[0] = 0x01;
   toWrite[1] = 0x02;
   toWrite[2] = 0x04;
   toWrite[3] = 0x08;
   toWrite[4] = 0x10;
   toWrite[5] = 0x20;


        //const WCHAR FileFullPath[] = {L"COM1"} ;


        hComm = CreateFile( "COM1",
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            0);

        if (hComm == INVALID_HANDLE_VALUE) {
            printf("Invalid value: %d\r\n", GetLastError());
        }
        
        DCB dcb = {0};
        dcb.DCBlength = sizeof(DCB); 
        if (GetCommState (hComm,&dcb) == 0)
        {  
            printf( "Failed to Get Comm State Reason:    %d",GetLastError()); 
        }
        else
        {
            dcb.StopBits = 0;
            dcb.BaudRate = 9600;
            dcb.Parity = 0;
            dcb.ByteSize = 8;
            if (SetCommState (hComm,&dcb) == 0)
            {  
                printf( "Failed to Set Comm State Reason:    %d",GetLastError()); 
            }
            if (GetCommState (hComm,&dcb) == 0)
            {  
                printf( "Failed to Get Comm State Reason:    %d",GetLastError()); 
            }
            printf("Baud Rate %d  Parity %d Byte Size %d Stop Bits %d", dcb.BaudRate,dcb.Parity,dcb.ByteSize,dcb.StopBits );
        }
        
        /*ret = WriteFile(hComm,&toWrite,1,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);
        ret = WriteFile(hComm,&toWrite,2,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);
        ret = WriteFile(hComm,&toWrite,3,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);
        ret = WriteFile(hComm,&toWrite,4,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);
        ret = WriteFile(hComm,&toWrite,5,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);
        ret = WriteFile(hComm,&toWrite,6,&dwWritten,&osWrite);
        printf("\n1");
        for(lp=0;lp<=100000;lp++);*/
          
    
    int i,j,c,cog_x,cog_y,total,count;
    int height,width,step,channels,channelsr,stepr;
    /*__asm__("push ebp;"
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
            "pop ebp;");*/
    
    printf("\nWelcome to point tracking program... Press any key to launch\n");
    getch();    
    IplImage *frame;
    IplImage *result=cvCreateImage(cvSize(320,240),8,1);    
    uchar *data,*datar;
    CvCapture* capture=cvCreateCameraCapture(0);
    cvNamedWindow("Result",1);
    cvNamedWindow("original", CV_WINDOW_AUTOSIZE);
    
    frame=cvQueryFrame(capture);
    
    height = frame->height;
    width = frame->width;
    step =frame->widthStep;
    channels = frame->nChannels;
    data = (uchar *)frame->imageData;

    stepr=result->widthStep;
    channelsr=result->nChannels;
    datar = (uchar *)result->imageData;    

    while(1)
    {
            i=j=cog_x=cog_y=total=count=0;
            frame=cvQueryFrame(capture);
            if(frame!=0)
            {
                for(i=0;i < (height);i=i+2) for(j=0;j <(width);j=j+2)
                if(((data[i*step+j*channels+2]) > (20+data[i*step+j*channels])) && ((data[i*step+j*channels+2]) > (50+data[i*step+j*channels+1])))
                {
                    datar[(i/2)*stepr+(j/2)*channelsr]=255;
                    cog_x=cog_x+(j/2); cog_y=cog_y+(i/2); total=total+1;
                    count++;
                }
                else
                    datar[(i/2)*stepr+(j/2)*channelsr]=0;
                cvShowImage("original",frame);
                //cvErode(result,result,0,1);
                //cvDilate(result,result,0,1);
                if(count<40)
                {
                    cog_x=160; cog_y=120;
                }
                else   
                {
                    cog_x=(int)(cog_x/(total+1)); cog_y=(int)(cog_y/(total+1));
                }
                
                cvLine(result,cvPoint(cog_x,0),cvPoint(cog_x,240),CV_RGB(255,255,255),1,4,0);
                cvLine(result,cvPoint(0,cog_y),cvPoint(320,cog_y),CV_RGB(255,255,255),1,4,0);
                cvShowImage("Result",result);
                
                transfer(cog_x,cog_y);
            }
            c=cvWaitKey(3);
            if(c==27)
                     break;     
    }
    cvReleaseCapture(&capture);
    cvDestroyWindow("Result");
    cvDestroyWindow("original");
    return 0;
}

void transfer(int xcoord,int ycoord)
{
    int val,xcoord0,xcoord1,ycoord0,ycoord1,ret,lp;
    char tochar;
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
    //sendval(xcoord0);
    tochar = (char)xcoord0;
    ret = WriteFile(hComm,&tochar,1,&dwWritten,&osWrite);
    for(lp=0;lp<=400000;lp++);
    printf("\nx0:%3d   ",xcoord0);
    //sendval(xcoord1);
    tochar = (char)xcoord1;
    ret = WriteFile(hComm,&tochar,1,&dwWritten,&osWrite);
    for(lp=0;lp<=400000;lp++);
    printf("x1:%3d      ",xcoord1);            
    //sendval(ycoord0);
    tochar = (char)ycoord0;
    ret = WriteFile(hComm,&tochar,1,&dwWritten,&osWrite);
    for(lp=0;lp<=400000;lp++);
    printf("y0:%3d   ",ycoord0);
    //sendval(ycoord1);
    tochar = (char)ycoord1;
    ret = WriteFile(hComm,&tochar,1,&dwWritten,&osWrite);
    for(lp=0;lp<=400000;lp++);
    printf("y1:%3d",ycoord1);
}
