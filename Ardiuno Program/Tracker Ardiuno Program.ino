#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <avr/wdt.h>

SoftwareSerial sim(10,11);
SoftwareSerial mySerial(3, 4);
TinyGPS gps;

String completeSMS="";
String number="+91 XXXXXXXXXX"; //Enter Your Number Here
String sendSMS="";
String _buffer;
int _timeout;
int i,flagSMS=1;

float flat, flon;
unsigned long age;
bool flag,newdata;
String latitude="";
String longitude="";
String getValues=""; 
void gpsdump(TinyGPS &gps);
void printFloat(double f, int digits = 2);

void setup(){
	// Oploen serial communications and wait for port to open:
	Serial.begin(9600);
	// set the data rate for the SoftwareSerial port
	Serial.println("\nSystem Started...");
	Serial.println("GPS Started");
	_buffer.reserve(50);
	sim.begin(9600);
	mySerial.begin(9600);
	Serial.println("in sim");
}
void loop() // run over and over{
	delay(1000);
	sim.listen();
	// Serial.println("sim listen");
	secondSim();
}

void firstGPS(){
	while(latitude.length()<=8 && longitude.length()<=8){
		if (mySerial.available()){
			//Serial.println("ATAT");
			char c = mySerial.read();
			//Serial.print(c);  // uncomment to see raw GPS data
			if (gps.encode(c)){
				gpsdump(gps);
			flag=false;
			Serial.println("\n\nCo-ordinates : "+latitude+","+longitude);
		}
		}
		if(latitude.length()==8 && longitude.length()==8){
			//latitude="";
			//longitude="";
			break;
		}
	}
}

void secondSim(){
	if (sim.available() > 0){
		Serial.println ("SIM800L Read an SMS");
		delay (1000);
		sim.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
		delay(1000);
		Serial.write ("Unread Message done\n"); 
		completeSMS=sim.readString();
		Serial.print(completeSMS);
		if(completeSMS.length()>1){
			sim.end();
			delay(1000);
			mySerial.listen();
			firstGPS();
			mySerial.end();
			Serial.println("PK");
			sim.begin(9600);
			sim.listen();
			trackMyDevice();
			completeSMS="";
		}
	}
}
void gpsdump(TinyGPS &gps){
	gps.f_get_position(&flat, &flon, &age);
	printFloat(flat, 5);
	printFloat(flon, 5);
}

void printFloat(double number, int digits){
	bool getValuesFlag;
	// Handle negative numbers
	if (number < 0.0)
		number = -number;
	if(number==flat)
		getValuesFlag=true;
	else if(number==flon)
		getValuesFlag=false;
	
	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i)
		rounding /= 10.0;
	
	number += rounding;
	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	getValues=getValues+int_part;
	// Print the decimal point, but only if there are digits beyond
	if (digits > 0)
		getValues=getValues+".";
	// Extract digits from the remainder one at a time
	while (digits-- > 0){
		remainder *= 10.0;
		int toPrint = int(remainder);
		getValues=getValues+toPrint;
		remainder -= toPrint;
	}
	if(getValuesFlag==true)
		latitude=getValues;
	else
		longitude=getValues;
	getValues="";
}

void trackMyDevice(){
	Serial.println("TRACK MY DEVICE");
	number="+91XXXXXXXXXX";
	sendSMS=latitude+" "+longitude;
	Serial.println ("Sending Message");
	sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
	delay(1000);
	Serial.println ("Set SMS Number"+number);
	sim.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
	delay(1000);
	Serial.println("Almost done");
	sim.println(sendSMS);
	Serial.println("SMS Sent");
	delay(100);
	sim.println((char)26);// ASCII code of CTRL+Z
	delay(1000);
	Serial.println("Done..!");
	//sendSMS="";
	flag=0;
	_buffer = _readSerial();
	delay(4000);
	wdt_disable();
	wdt_enable(WDTO_15MS);
	while (1) {}
}

String _readSerial(){
	_timeout = 0;
	while  (!sim.available() && _timeout < 12000  )
	{
		delay(13);
		_timeout++;
	}
	if (sim.available()) {
		Serial.print("1111\n");
		Serial.print(sim.readString());
		return sim.readString();
	}
}