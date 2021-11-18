#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include "jpeglib.h"
 
// -- define v4l2 ---------------
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define VIDEO_DEVICE0 "/dev/video1"  // gray scale thermal image
#define FRAME_WIDTH0  160
#define FRAME_HEIGHT0 120
#define FRAME_FORMAT0 V4L2_PIX_FMT_GREY

struct v4l2_capability vid_caps0;
struct v4l2_format vid_format0;

size_t framesize0;
size_t linewidth0;

const char *video_device0=VIDEO_DEVICE0;
int fdwr0 = 0;
// -- end define v4l2 ---------------

#define VENDOR_ID 0x09cb
#define PRODUCT_ID 0x1996

static struct libusb_device_handle *devh = NULL;
int filecount=0;
struct timeval t1, t2;

// -- buffer for EP 0x85 chunks ---------------
#define BUF85SIZE 1048576
int buf85pointer = 0;
unsigned char buf85[BUF85SIZE];
  

double raw2temperature(unsigned short raw)
{

// mystery correction factor
raw *=4;

float E=0.95;
float OD=0.15;
float RTemp=22;
float ATemp=20;
float IRWTemp=25;
float IRT=1;
float RH=50;
float PR1=17549.801;
float PB=1435;
float PF=1;
float PO=-3004;
float PR2=0.0125;

float ATA1 = 0.006569;
float ATA2 = 0.01262;
float ATB1 = -0.002276;
float ATB2 = -0.00667;
float ATX = 1.9;

// transmission through window (calibrated)
float emiss_wind = 1 - IRT;
float refl_wind = 0;

// transmission through the air
float h2o = (RH / 100) * exp(
            1.5587
            + 0.06939 * (ATemp)
            - 0.00027816 * pow(ATemp,2)
            + 0.00000068455 * pow(ATemp,3)
        );
float tau1 = ATX * exp(-sqrt(OD / 2) * (ATA1 + ATB1 * sqrt(h2o))) + (1 - ATX) * exp(
            -sqrt(OD / 2) * (ATA2 + ATB2 * sqrt(h2o))
        );
float tau2 = ATX * exp(-sqrt(OD / 2) * (ATA1 + ATB1 * sqrt(h2o))) + (1 - ATX) * exp(
            -sqrt(OD / 2) * (ATA2 + ATB2 * sqrt(h2o))
        );


// radiance from the environment
float raw_refl1 = PR1 / (PR2 * (exp(PB / (RTemp + 273.15)) - PF)) - PO;
float raw_refl1_attn = (1 - E) / E * raw_refl1;
float raw_atm1 = PR1 / (PR2 * (exp(PB / (ATemp + 273.15)) - PF)) - PO;
float raw_atm1_attn = (1 - tau1) / E / tau1 * raw_atm1;
float raw_wind = PR1 / (PR2 * (exp(PB / (IRWTemp + 273.15)) - PF)) - PO;
float raw_wind_attn = emiss_wind / E / tau1 / IRT * raw_wind;
float raw_refl2 = PR1 / (PR2 * (exp(PB / (RTemp + 273.15)) - PF)) - PO;
float raw_refl2_attn = refl_wind / E / tau1 / IRT * raw_refl2;
float raw_atm2 = PR1 / (PR2 * (exp(PB / (ATemp + 273.15)) - PF)) - PO;
float raw_atm2_attn = (1 - tau2) / E / tau1 / IRT / tau2 * raw_atm2;

float raw_obj = (
            raw / E / tau1 / IRT / tau2
            - raw_atm1_attn
            - raw_atm2_attn
            - raw_wind_attn
            - raw_refl1_attn
            - raw_refl2_attn
        );

// temperature from radiance
float temp_celcius = PB / log(PR1 / (PR2 * (raw_obj + PO)) + PF) - 273.15;
return temp_celcius;
}



void startv4l2()
{
     int ret_code = 0;
     int i;
     int k=1;
     
     //open video_device0
     fdwr0 = open(video_device0, O_RDWR);
     assert(fdwr0 >= 0);

     ret_code = ioctl(fdwr0, VIDIOC_QUERYCAP, &vid_caps0);
     assert(ret_code != -1);

     memset(&vid_format0, 0, sizeof(vid_format0));

     ret_code = ioctl(fdwr0, VIDIOC_G_FMT, &vid_format0);

     linewidth0=FRAME_WIDTH0;
     framesize0=FRAME_WIDTH0*FRAME_HEIGHT0*1; // 8 Bit

     vid_format0.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
     vid_format0.fmt.pix.width = FRAME_WIDTH0;
     vid_format0.fmt.pix.height = FRAME_HEIGHT0;
     vid_format0.fmt.pix.pixelformat = FRAME_FORMAT0;
     vid_format0.fmt.pix.sizeimage = framesize0;
     vid_format0.fmt.pix.field = V4L2_FIELD_NONE;
     vid_format0.fmt.pix.bytesperline = linewidth0;
     vid_format0.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

     // set data format
     ret_code = ioctl(fdwr0, VIDIOC_S_FMT, &vid_format0);
     assert(ret_code != -1);

}


// unused
void closev4l2()
{
     close(fdwr0);
}


void vframe(char ep[],char EP_error[], int r, int actual_length, unsigned char buf[])
{
  
  unsigned char magicbyte[4]={0xEF,0xBE,0x00,0x00};
  
  if  ((strncmp (buf, magicbyte,4)==0 ) || ((buf85pointer + actual_length) >= BUF85SIZE))
    {
        buf85pointer=0;
    }
 
  memmove(buf85+buf85pointer, buf, actual_length);
  buf85pointer=buf85pointer+actual_length;
  
  if  ((strncmp (buf85, magicbyte,4)!=0 ))
    {
        //reset buff pointer
        buf85pointer=0;
        return;
    }
      
  // a quick and dirty job for gcc
  uint32_t FrameSize   = buf85[ 8] + (buf85[ 9] << 8) + (buf85[10] << 16) + (buf85[11] << 24);
  uint32_t ThermalSize = buf85[12] + (buf85[13] << 8) + (buf85[14] << 16) + (buf85[15] << 24);
  uint32_t JpgSize     = buf85[16] + (buf85[17] << 8) + (buf85[18] << 16) + (buf85[19] << 24);
  uint32_t StatusSize  = buf85[20] + (buf85[21] << 8) + (buf85[22] << 16) + (buf85[23] << 24);

  if ( (FrameSize+28) > (buf85pointer) ) 
  {
      return;
  }
  
  int i,v;

  filecount++;
  for (i = 0; i <  StatusSize; i++)
  {
      v=28+ThermalSize+JpgSize+i;
  }
		
  buf85pointer=0;
  
  unsigned short pix[160*120];
  int x, y;
  unsigned char *fb_proc,*fb_proc2; 
  fb_proc = malloc(160 * 120); // 8 Bit gray buffer really needs only 160 x 120
  
  int min = 0x10000, max = 0;

// Make a unsigned short array from what comes from the thermal frame
  int maxx, maxy;

  for (y = 0; y < 120; ++y) 
  {
    for (x = 0; x < 160; ++x)
    {
      if (x<80) 
         v = buf85[2*(y * 164 + x) +32]+256*buf85[2*(y * 164 + x) +33];
      else
         v = buf85[2*(y * 164 + x) +32+4]+256*buf85[2*(y * 164 + x) +33+4];   
      pix[y * 160 + x] = v;   // unsigned char!!

      if (v < min) min = v;
      if (v > max) { max = v; maxx = x; maxy = y; }
    }
  }

  
  for (y = 0; y < 120; ++y)
  {
    for (x = 0; x < 160; ++x)
    {
      int v = (pix[y * 160 + x]) >> 8;


  // fb_proc is the gray scale frame buffer

      fb_proc[y * 160 + x] = v;   // unsigned char!!
    }
  }

  printf("%.1f\n",raw2temperature(max));
  fflush(stdout);
    
  // write video to v4l2loopback(s)
   write(fdwr0, fb_proc, framesize0);  // gray scale Thermal Image
   free(fb_proc);
    
}

 static int find_lvr_flirusb(void)
 {
 	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
 	return devh ? 0 : -EIO;
 }
 
 void print_bulk_result(char ep[],char EP_error[], int r, int actual_length, unsigned char buf[])
 {
         int i;

         if (r < 0) {
                if (strcmp (EP_error, libusb_error_name(r))!=0)
                {       
                    strcpy(EP_error, libusb_error_name(r));
                    sleep(1);
                }
                //return 1;
        } 
 }       
 
 int main(void)
 {
      
 	int i,r = 1;
 	r = libusb_init(NULL);
 	if (r < 0) {
 		exit(1);
 	}
 	
  	r = find_lvr_flirusb();
 	if (r < 0) {
 		goto out;
 	}
	

    r = libusb_set_configuration(devh, 3);
    if (r < 0) {
        goto out;
    }
	
 
 	// Claiming of interfaces is a purely logical operation; 
        // it does not cause any requests to be sent over the bus. 
 	r = libusb_claim_interface(devh, 0);
 	if (r <0) {
 		goto out;
 	}	
	
	unsigned char buf[1048576]; 
        int actual_length;

 	char EP81_error[50]="", EP83_error[50]="",EP85_error[50]=""; 
 	unsigned char data[2]={0,0}; // only a bad dummy
 	
 	// don't forget: $ sudo modprobe v4l2loopback video_nr=0,1
 	startv4l2();
 	
 	int state = 1; 
 	int ct=0;

    while (1)
    {
    	
    switch(state) {
        
         case 1:
 	
            r = libusb_control_transfer(devh,1,0x0b,0,2,data,0,100);
            if (r < 0) {
                return r;
            }

            r = libusb_control_transfer(devh,1,0x0b,0,1,data,0,100);
            if (r < 0) {
                return r;
            } 
             	
         	r = libusb_control_transfer(devh,1,0x0b,1,1,data,0,100);
 	        if (r < 0) {
 		        return r;
 	        }
 	        state = 2;   // jump over wait stait 2
            break;
        
        
        case 2:  	    
            TRUE;
            int transferred = 0;
            char my_string[128];

            int length = 16;
            unsigned char my_string2[16]={0xcc,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x41,0x00,0x00,0x00,0xF8,0xB3,0xF7,0x00};
            int i;
    
            r = libusb_bulk_transfer(devh, 2, my_string2, length, &transferred, 0);
            strcpy(  my_string,"{\"type\":\"openFile\",\"data\":{\"mode\":\"r\",\"path\":\"CameraFiles.zip\"}}");
    
            length = strlen(my_string)+1;
    
            // avoid error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
            unsigned char *my_string1 = (unsigned char*)my_string;
            
            r = libusb_bulk_transfer(devh, 2, my_string1, length, &transferred, 0);
            length = 16;
            unsigned char my_string3[16]={0xcc,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x33,0x00,0x00,0x00,0xef,0xdb,0xc1,0xc1};
            r = libusb_bulk_transfer(devh, 2, my_string3, length, &transferred, 0);

            strcpy(  my_string,"{\"type\":\"readFile\",\"data\":{\"streamIdentifier\":10}}");
            length = strlen(my_string)+1;
    
            // avoid error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
            my_string1 = (unsigned char*)my_string;
            
            r = libusb_bulk_transfer(devh, 2, my_string1, length, &transferred, 0);
            state = 3;           
            break;
    

        case 3:
            r = libusb_control_transfer(devh,1,0x0b,1,2,data, 2,200);
            state = 4;
            break;

        case 4:
            // endless loop 
            // poll Frame Endpoints 0x85 
            // don't change timeout=100ms !!
            r = libusb_bulk_transfer(devh, 0x85, buf, sizeof(buf), &actual_length, 100); 
            if (actual_length > 0)
                vframe("0x85",EP85_error, r, actual_length, buf);   
            break;      

        }    

        // poll Endpoints 0x81, 0x83
        r = libusb_bulk_transfer(devh, 0x81, buf, sizeof(buf), &actual_length, 10); 
        r = libusb_bulk_transfer(devh, 0x83, buf, sizeof(buf), &actual_length, 10); 

    }
    
    // never reached ;-)
 	libusb_release_interface(devh, 0);
 	
 out:
    //close the device
 	libusb_reset_device(devh);
 	libusb_close(devh);
 	libusb_exit(NULL);
 	return r >= 0 ? r : -r;
 }
