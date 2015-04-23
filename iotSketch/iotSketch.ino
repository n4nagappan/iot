/*
  Web client

 This sketch connects to a website 
 using Wi-Fi functionality on MediaTek LinkIt platform.

 Change the macro WIFI_AP, WIFI_PASSWORD, WIFI_AUTH and SITE_URL accordingly.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 modified 20 Aug 2014
 by MediaTek Inc.
 */

#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LGPS.h>

#include "Wire.h"

// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"

#define WIFI_AP "AndroidAP"
#define WIFI_PASSWORD "mafj3221"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.
#define SITE_URL "www.fa9aa48d.ngrok.io"

LWiFiClient c;

/*GPS info variables*/
gpsSentenceInfoStruct info;
char buff[256];
double latitude;
double longitude;
double latno, latdegree, longno, longdegree;

/*IMU related variables*/
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU9250 accelgyro;
I2Cdev   I2C_M;

uint8_t buffer_m[6];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t   mx, my, mz;

float heading;
float tiltheading;

float Axyz[3];
float Gxyz[3];
float Mxyz[3];

#define sample_num_mdate  5000      

volatile float mx_sample[3];
volatile float my_sample[3];
volatile float mz_sample[3];

static float mx_centre = 0;
static float my_centre = 0;
static float mz_centre = 0;

volatile int mx_max =0;
volatile int my_max =0;
volatile int mz_max =0;

volatile int mx_min =0;
volatile int my_min =0;
volatile int mz_min =0;

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

void parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */
  int tmp, hour, minute, second, num ;
  char latDirection, longDirection;
  Serial.println(GPGGAstr);
  if(GPGGAstr[0] == '$')
  {
    tmp = getComma(1, GPGGAstr);
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
    
    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(3, GPGGAstr);
    latDirection = GPGGAstr[tmp];
    tmp = getComma(4, GPGGAstr);
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    tmp = getComma(5, GPGGAstr);
    longDirection = GPGGAstr[tmp];
    sprintf(buff, "latitude = %10.4f%c, longitude = %10.4f%c", latitude, latDirection, longitude, longDirection);
    Serial.println(buff); 
    
    tmp = getComma(7, GPGGAstr);
    num = getIntNumber(&GPGGAstr[tmp]);    
    sprintf(buff, "satellites number = %d", num);
    Serial.println(buff); 
  }
  else
  {
    Serial.println("Not get data"); 
  }
}

void setup()
{
  /*Power on WiFi module*/
  LTask.begin();
  LWiFi.begin();
  
  Serial.begin(115200);
  
  // /* IMU Setup */
  // // join I2C bus (I2Cdev library doesn't do this automatically)
  // Wire.begin();

  // // initialize device
  // Serial.println("Initializing I2C devices...");
  // accelgyro.initialize();

  // // verify connection
  // Serial.println("Testing device connections...");
  // Serial.println(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");
 
  /*Power on GPS module*/
  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ..."); 
  delay(3000);
  
  // keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
    Serial.println("Trying again...");
  }
}

boolean disconnectedMsg = false;

//Rounds down (via intermediary integer conversion truncation)
String doubleToString(double input,int decimalPlaces){
  if(decimalPlaces!=0){
  String string = String((int)(input*pow(10,decimalPlaces)));
  if(abs(input)<1){
    if(input>0)
      string = "0"+string;
    else if(input<0)
      string = string.substring(0,1)+"0"+string.substring(1);
  }
  return string.substring(0,string.length()-decimalPlaces)+"."+string.substring(string.length()-decimalPlaces);
  }
  else {
    return String((int)input);
  }
}

void sendGpsData(){
  // keep retrying until connected to website
  Serial.println("Connecting to WebSite");
  while (0 == c.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

  // send HTTP request, ends with 2 CR/LF
  Serial.println("send HTTP GET request");
//  Serial.println("lat="+doubleToString(latno,5)+"&long="+doubleToString(longno,5));
  String gpsData = "lat="+doubleToString(latitude,5)+"&long="+doubleToString(longitude,5);
  Serial.println(gpsData);
  c.println("GET http://fa9aa48d.ngrok.io/gps?"+gpsData+" HTTP/1.1");
  c.println("Host: " SITE_URL);
  c.println("Connection: keep-alive");
  c.println();

  // waiting for server response
  Serial.println("waiting HTTP response:");
  while (!c.available())
  {
    delay(100);
  }
  
  // Read the data here
  // Make sure we are connected, and dump the response content to Serial
  while (c)
  {
    int v = c.read();
    if (v != -1)
    {
      Serial.print((char)v);
    }
    else
    {
      Serial.println("no more content, disconnect");
      c.stop();
    }
  }

  if (!disconnectedMsg)
  {
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
}

void sendIMUData(){
  // keep retrying until connected to website
  Serial.println("Connecting to WebSite for IMU");
  while (0 == c.connect(SITE_URL, 80))
  {
    Serial.println("Re-Connecting to WebSite");
    delay(1000);
  }

  // send HTTP request, ends with 2 CR/LF
  Serial.println("send HTTP GET request");
//  Serial.println("lat="+doubleToString(latno,5)+"&long="+doubleToString(longno,5));
  String imuData = "ax="+doubleToString(Axyz[0],3)+"&ay="+doubleToString(Axyz[1],3)+"&az="+doubleToString(Axyz[2],3);
  Serial.println(imuData);
  c.println("GET http://722b4490.ngrok.io/imu?"+imuData+" HTTP/1.1");
  c.println("Host: " SITE_URL);
  c.println("Connection: keep-alive");
  c.println();

  // waiting for server response
  Serial.println("waiting HTTP response:");
  while (!c.available())
  {
    delay(100);
  }
  
  // Read the data here
  // Make sure we are connected, and dump the response content to Serial
  while (c)
  {
    int v = c.read();
    if (v != -1)
    {
      Serial.print((char)v);
    }
    else
    {
      Serial.println("no more content, disconnect");
      c.stop();
    }
  }

  if (!disconnectedMsg)
  {
    Serial.println("disconnected by server");
    disconnectedMsg = true;
  }
}

void getIMUData()
{
  getAccel_Data();
	getGyro_Data();
	getCompassDate_calibrated(); // compass data has been calibrated here 
	getHeading();				//before we use this function we should run 'getCompassDate_calibrated()' frist, so that we can get calibrated data ,then we can get correct angle .					
	getTiltHeading();           
	
	Serial.println("calibration parameter: ");
	Serial.print(mx_centre);
	Serial.print("         ");
	Serial.print(my_centre);
	Serial.print("         ");
	Serial.println(mz_centre);
	Serial.println("     ");
	
	
	Serial.println("Acceleration(g) of X,Y,Z:");
	Serial.print(Axyz[0]); 
	Serial.print(",");
	Serial.print(Axyz[1]); 
	Serial.print(",");
	Serial.println(Axyz[2]);
        Serial.print("Total Acceleration(g): "); 
        Serial.println(sqrt(Axyz[0]*Axyz[0]+Axyz[1]*Axyz[1]+Axyz[2]*Axyz[2]));
	Serial.println("Gyro(degress/s) of X,Y,Z:");
	Serial.print(Gxyz[0]); 
	Serial.print(",");
	Serial.print(Gxyz[1]); 
	Serial.print(",");
	Serial.println(Gxyz[2]); 
	Serial.println("Compass Value of X,Y,Z:");
	Serial.print(Mxyz[0]); 
	Serial.print(",");
	Serial.print(Mxyz[1]); 
	Serial.print(",");
	Serial.println(Mxyz[2]);
	Serial.println("The clockwise angle between the magnetic north and X-Axis:");
	Serial.print(heading);
	Serial.println(" ");
	Serial.println("The clockwise angle between the magnetic north and the projection of the positive X-Axis in the horizontal plane:");
	Serial.println(tiltheading);
	Serial.println("   ");
	Serial.println("   ");
    Serial.println("   ");



	delay(300);
}
void loop()
{
  /*Get GPS data*/
  LGPS.getData(&info);
  Serial.println((char*)info.GPGGA); 
  parseGPGGA((const char*)info.GPGGA);
  
  // every 5 seconds
  sendGpsData();
  delay(5000);
  
  // getIMUData();
  // sendIMUData();
}

void getHeading(void)
{
  heading=180*atan2(Mxyz[1],Mxyz[0])/PI;
  if(heading <0) heading +=360;
}

void getTiltHeading(void)
{
  float pitch = asin(-Axyz[0]);
  float roll = asin(Axyz[1]/cos(pitch));

  float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
  float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
  float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
  tiltheading = 180 * atan2(yh, xh)/PI;
  if(yh<0)    tiltheading +=360;
}



void Mxyz_init_calibrated ()
{
	
	Serial.println(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
	Serial.print("  ");
	Serial.println(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
	Serial.print("  ");
	Serial.println(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
	while(!Serial.find("ready"));	
	Serial.println("  ");
	Serial.println("ready");
	Serial.println("Sample starting......");
	Serial.println("waiting ......");
	
	get_calibration_Data ();
	
	Serial.println("     ");
	Serial.println("compass calibration parameter ");
	Serial.print(mx_centre);
	Serial.print("     ");
	Serial.print(my_centre);
	Serial.print("     ");
	Serial.println(mz_centre);
	Serial.println("    ");
}


void get_calibration_Data ()
{
		for (int i=0; i<sample_num_mdate;i++)
			{
			get_one_sample_date_mxyz();
			/*
			Serial.print(mx_sample[2]);
			Serial.print(" ");
			Serial.print(my_sample[2]);                            //you can see the sample data here .
			Serial.print(" ");
			Serial.println(mz_sample[2]);
			*/


			
			if (mx_sample[2]>=mx_sample[1])mx_sample[1] = mx_sample[2];			
			if (my_sample[2]>=my_sample[1])my_sample[1] = my_sample[2]; //find max value			
			if (mz_sample[2]>=mz_sample[1])mz_sample[1] = mz_sample[2];		
			
			if (mx_sample[2]<=mx_sample[0])mx_sample[0] = mx_sample[2];
			if (my_sample[2]<=my_sample[0])my_sample[0] = my_sample[2];//find min value
			if (mz_sample[2]<=mz_sample[0])mz_sample[0] = mz_sample[2];
						
			}
			
			mx_max = mx_sample[1];
			my_max = my_sample[1];
			mz_max = mz_sample[1];			
					
			mx_min = mx_sample[0];
			my_min = my_sample[0];
			mz_min = mz_sample[0];
	

	
			mx_centre = (mx_max + mx_min)/2;
			my_centre = (my_max + my_min)/2;
			mz_centre = (mz_max + mz_min)/2;	
	
}






void get_one_sample_date_mxyz()
{		
		getCompass_Data();
		mx_sample[2] = Mxyz[0];
		my_sample[2] = Mxyz[1];
		mz_sample[2] = Mxyz[2];
}	


void getAccel_Data(void)
{
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Axyz[0] = (double) ax / 16384;//16384  LSB/g
  Axyz[1] = (double) ay / 16384;
  Axyz[2] = (double) az / 16384; 
}

void getGyro_Data(void)
{
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
  Gxyz[0] = (double) gx * 250 / 32768;//131 LSB(��/s)
  Gxyz[1] = (double) gy * 250 / 32768;
  Gxyz[2] = (double) gz * 250 / 32768;
}

void getCompass_Data(void)
{
	I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
	delay(10);
	I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);
	
    mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
	my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
	mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;	
	
	//Mxyz[0] = (double) mx * 1200 / 4096;
	//Mxyz[1] = (double) my * 1200 / 4096;
	//Mxyz[2] = (double) mz * 1200 / 4096;
	Mxyz[0] = (double) mx * 4800 / 8192;
	Mxyz[1] = (double) my * 4800 / 8192;
	Mxyz[2] = (double) mz * 4800 / 8192;
}

void getCompassDate_calibrated ()
{
	getCompass_Data();
	Mxyz[0] = Mxyz[0] - mx_centre;
	Mxyz[1] = Mxyz[1] - my_centre;
	Mxyz[2] = Mxyz[2] - mz_centre;	
}
