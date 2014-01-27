/* Requires Arduino IDE and MIRF library
 * MIRF lib: https://github.com/aaronds/arduino-nrf24l01/tree/master/Mirf
 * NOTE: It may be necessary to modify MirfHardwareSpiDriver.cpp with; 
 * SPI.setClockDivider(SPI_CLOCK_DIV2);
 * to increase stability on Freescale K series uC.
 * NOTE: added flushRX() function declare to HardwareSerial.h & function to HardwareSerial3.cpp
 * flushRX() is the old <1.0 flush() function to clear the Serial buffer.
 */
 
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

//PwrKey SIM900
int pwrKey = 5;

//GSM driver pins
//byte gsmDriverPin[3] = {3,4,5};

//Xively API Key
char xApiKey[49] = "CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS";
//byte data[Mirf.payload]; // data buffer - unused currently
//float data; // recieved data

void setup(){
  
  //setup GSM driver pins
  /*for (int i = 0 ; i < 3; i++){
    pinMode(gsmDriverPin[i],OUTPUT);
  }
  
  digitalWrite(5,HIGH);//Output GSM Timing 
  delay(1500);
  digitalWrite(5,LOW);
  delay(5000);  
  digitalWrite(3,LOW);//Enable the GSM mode
  digitalWrite(4,HIGH);//Disable the GPS mode
  delay(2000);
  */
  
  pinMode(pwrKey, OUTPUT);
  digitalWrite(pwrKey, HIGH);
  delay(3000);
  digitalWrite(pwrKey, LOW);
  
  Serial3.begin(9600);
  delay(5000);//call ready & wait for comms 
  delay(5000);

  
  Serial3.println("AT+CMEE=1");
  delay(2000); 
  
  Serial3.println("AT+CGATT=1"); //attach gprs service
  delay(2000);
  
  Serial3.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); //Set bearer connection type
  delay(2000);
  
  Serial3.println("AT+SAPBR=3,1,\"APN\",\"wap.o2.co.uk\""); //set bearer mode
  delay(2000);
  
  Serial3.println("AT+SAPBR=3,1,\"USER\",\"o2wap\""); //set bearer user
  delay(2000);
  
  Serial3.println("AT+SAPBR=3,1,\"PWD\",\"password\""); //Set bearer pass
  delay(2000);
  
  //Teensy pin config
  Mirf.cePin = 15;
  Mirf.csnPin = 20;   
  
  // Set the SPI Driver.
  Mirf.spi = &MirfHardwareSpi;
  
  // Setup pins / SPI
  Mirf.init();
  
  // Configure reciving address.   
  Mirf.setRADDR((byte *)"tbmn1");
  
  // payload on client and server must be the same. 
  Mirf.payload = sizeof(float);
  
  // Write channel and payload config then power up reciver.
  Mirf.config();
  
  // Get IP address and post it
  getIP();
}

// main loop
void loop(){
 
 postData(pollSensor());
 
 delay(5000);
 
}

// function to get the current IP and update xevily
void getIP() {
  //char* ipAddr[];
  char serData;
  String stringIpAddr = "";
  
  Serial3.println("AT+SAPBR=1,1"); //bring up connection
  delay(3000);
  
  Serial3.println("AT+HTTPINIT"); //Init HTTP engine
  delay(1500);
 
  Serial3.flush(); 
  Serial3.println("AT+SAPBR=2,1"); //get IP address
  delay(1000);
  
  while (Serial3.available() > 0) {
    serData = Serial3.read();
    stringIpAddr += serData;
  }
  
  if (stringIpAddr.indexOf("\"") > 0) {
    //Serial3.println(stringIpAddr);
    stringIpAddr = stringIpAddr.substring(stringIpAddr.indexOf("\"") + 1, stringIpAddr.lastIndexOf("\"") - 1); 
  }
  
  Serial3.print("AT+HTTPPARA=\"URL\",\"ec2-54-242-171-87.compute-1.amazonaws.com/xively/xivelyPut.php?X-ApiKey=CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS&chan=IPAddr&DATA=");

  Serial3.print(stringIpAddr);
  Serial3.println("\"");
  delay(1000);
  
  Serial3.println("AT+HTTPREAD");
  delay(2000);
  
  Serial3.println("AT+HTTPACTION=0"); //do HTTP get
  delay(6000);  

  Serial3.println("AT+HTTPTERM");
  delay(1000);

}

float pollSensor(){
 
  float data;
  
  /*
   * If a packet has been recived.
   * isSending also restores listening mode when it 
   * transitions from true to false.
   */
   
  if(!Mirf.isSending() && Mirf.dataReady()){
    
    // load the packet into the buffer.        
    Mirf.getData((byte *)&data);
   
    // Set the send address.
    Mirf.setTADDR((byte *)"volt1");
    
    // Send the data back to the client.     
    Mirf.send((byte *)&data);
    
    /*
     * Wait untill sending has finished
     * NB: isSending returns the chip to receving after returning true.
     */
    
    return data;
  }
  
}

void postData(float senseData){
//  char serDat[25];
    
  Serial3.println("AT+SAPBR=1,1"); //bring up connection
  delay(3000);
  
  Serial3.println("AT+HTTPINIT"); //Init HTTP engine
  delay(1500);

  //set URL
  Serial3.print("AT+HTTPPARA=\"URL\",\"ec2-54-242-171-87.compute-1.amazonaws.com/xively/xivelyPut.php?X-ApiKey=CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS&chan=volts&DATA=");
  Serial3.print(senseData);
  Serial3.println("\"");
  delay(1000);

  Serial3.println("AT+HTTPACTION=0"); //do HTTP get
  delay(6000);

  Serial3.println("AT+HTTPREAD");
  delay(2000);

  Serial3.println("AT+HTTPTERM");
  delay(1000);
 
}
