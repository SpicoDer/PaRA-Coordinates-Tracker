#include <stdio.h>
#include <string.h>


#define DEBUG true
#define MODE_1A

#define DTR_PIN 9
#define RI_PIN 8

#define LTE_PWRKEY_PIN 5
#define LTE_RESET_PIN 6
#define LTE_FLIGHT_PIN 7

String token = "MdV20wkp6ZLD_iYNf9m7ObJ5rsM_PKnr";
String from_usb = "";
String loc = "";
String longitude = "";
String lattitude = ""; 

//FUNCTION TO PASS AT+ COMMAND

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  Serial1.println(command);
  
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {
      char c = Serial1.read();
      response += c;
    }
  }
  if (debug)
  {
    SerialUSB.print(response);
  }
  return response;
}

//FUNCTION TO CHECK SIGNAL STRENGTH

int check_signal(void)
{
  while(1)
  {
    String sig = sendData("AT+CSQ",3000,DEBUG);
    int i=0;
    String strength;
    while(sig[i]!=':')i++;

    String loc_2 = sig.substring(i+2);

    i=0;
    while(loc_2[i]!=',')i++;

    strength = loc_2.substring(0,i);

    int strength_1 = strength.toInt();
    SerialUSB.println(strength_1);

    return strength_1;
  }
}

//FUNCTION TO GET LATITUDE AND LONGITUDE STRING

void gpsLocation(String local)
{
  while(1)
  {
    int i=0;
    while(local[i]!=':')i++;

    String loc_2 = local.substring(i+2);

    i=0;
    while(loc_2[i]!=',')i++;

    lattitude = loc_2.substring(0,i);
    SerialUSB.println(lattitude);

    int j = i+3;
    int k = j;

    while(loc_2[k]!=',')k++;

    longitude = loc_2.substring(j,k);
    SerialUSB.println(longitude);

    return;
    
  }
}

//CHECK IF NO GPS DETECTED
//+CGPSINFO: ,,,,,,,,
void gpsDetect(String local){
  while(1){
    int i=0;
    while(local[i]!=':')i++;

    String loc_2 = local.substring(i+2);

    if(loc_2[0]==','){
      String noGPS = "EMPTY";
      SerialUSB.println("EMPTY COORDINATES");
      String http_str = "AT+HTTPPARA=\"URL\",\"https://jparatest.000webhostapp.com/location/updateLoc.php?loc=0\"\r\n";
      SerialUSB.println(http_str);  
      sendData("AT+HTTPINIT\r\n", 3000, DEBUG);
      sendData(http_str, 3000, DEBUG);
      sendData("AT+HTTPACTION=0\r\n", 3000, DEBUG);
      sendData("AT+HTTPTERM\r\n", 3000, DEBUG);
    }
    else{
    gpsLocation(local);
    }
    return;
  }
}


//CONVERSION OF GIVEN LATTITUDE FORMAT FROM NMEA TO PARSER

String conversion(String local)
{
    //Get the digits before decimal

      int dec = 0;
      while(local[dec]!='.')dec++;
      String str_init = local.substring(0,dec);

    //Get the initial value digits

      int val = str_init.toInt();
      
    //Get the decimal value digits

      String str_dec = local.substring(dec-2);
      double deci_val = str_dec.toDouble() ;

    //Get the converted value

      double new_value = (val/100)+(deci_val / 60);

      SerialUSB.println(new_value,6);

      String final_val = String(new_value,6);

      return final_val;
}

//FUNCTION TO PASS LOCATION IN REST API

void map_loc(String lat1,String lon1)
{
    String lat = conversion(lat1);
    String lon = conversion(lon1);

    SerialUSB.print("lattitude = ");SerialUSB.println(lat);
    SerialUSB.print("longitude = ");SerialUSB.println(lon);

    String http_str = "AT+HTTPPARA=\"URL\",\"https://jparatest.000webhostapp.com/location/updateLoc.php?lat=" + lat + "&lon=" + lon + "\"\r\n";
    SerialUSB.println(http_str);  
    sendData("AT+HTTPINIT\r\n", 3000, DEBUG);
    sendData(http_str, 3000, DEBUG);
    sendData("AT+HTTPACTION=0\r\n", 3000, DEBUG);
    sendData("AT+HTTPTERM\r\n", 3000, DEBUG);
}

void setup(){
  SerialUSB.begin(115200);
  //while (!SerialUSB)
//  {
    ; // wait for Arduino serial Monitor port to connect
//  }

  delay(100);

  Serial1.begin(115200);

  //INITIALIZING GSM MODULE

  pinMode(LTE_RESET_PIN, OUTPUT);
  digitalWrite(LTE_RESET_PIN, LOW);

  pinMode(LTE_PWRKEY_PIN, OUTPUT);
  digitalWrite(LTE_RESET_PIN, LOW);
  delay(100);
  digitalWrite(LTE_PWRKEY_PIN, HIGH);
  delay(2000);
  digitalWrite(LTE_PWRKEY_PIN, LOW);

  pinMode(LTE_FLIGHT_PIN, OUTPUT);
  digitalWrite(LTE_FLIGHT_PIN, LOW); //Normal Mode
  // digitalWrite(LTE_FLIGHT_PIN, HIGH);//Flight Mode

  SerialUSB.println("Maduino Zero 4G Test Start!");

  

  SerialUSB.println(sendData("AT+CGMM\r\n", 3000, DEBUG));
  sendData("AT+CPIN?\r\n",3000,DEBUG);
  
  sendData("AT+COPS?\r\n",3000,DEBUG);

  sendData("AT+CNUM\r\n",3000,DEBUG);

  //INITIALIZING GPS MODULE
  sendData("AT+CGPS=1",3000,DEBUG);
  delay(60000);

  
}

void loop(){

    //CHECKING SIGNAL STRENGTH
    
    if(check_signal()>=10)
    {
      loc = sendData("AT+CGPSINFO\r\n",3000,DEBUG);
      gpsDetect(loc);
      if(lattitude!=""&& longitude!="")
      {
        map_loc(lattitude,longitude);
      }
      delay(5000);  
    }    
}
