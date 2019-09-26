#include <SoftwareSerial.h>
#define RX 2
#define TX 3
String AP = "developers";
String PASS = "pr0t39ere";
String HOST = "192.168.103.19";
String PORT = "6789";
String field = "Id";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
int lflag = 0;
boolean port = false;
long int counter;
long int rtime;
SoftwareSerial esp8266(RX, TX);

String Data = "";
void setup() {

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT", 5, "OK", false);
  esp8266.println("AT+UART_DEF=9600,8,1,0,0");
  delay(1000);
  esp8266.end();
  esp8266.begin(9600);

  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);

  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  counter = 0;


  ConnectToWifi();
}


void loop() {
  if (lflag == 1) {

    //getData+="GET " + uri + " HTTP/1.0\n" +
    //"Host: " + HOST + "\n" +
    //"Accept: application/json\n" +
    //"Content-Type: application/json\n" +
    //"Connection: Keep-Alive\n" +
    //"\n";



    //getData+="HTTP/1.1 200 OK\r\n Content-Type:text/plain:charset=UTF-8";

    while (port && lflag == 1) {
      if (counter >= 2000) {
        if (!sendCommand("AT+CIFSR", 5, "192", true)) {
          resetConnection();
          continue;
        }
        port=sendCommand("AT+CIPSERVER=1,80", 5, "OK", false);
        if(!port){
          resetConnection();
          continue;
        }
        counter = 0;
      }

      counter++;

      lflag = 0;



      delay(10);
      String getData = "";
      getData += "HTTP/1.1 200 OK\r\n Connection:Keep-Alive \r\n Content-Type: text/html; charset=utf-8\r\n Content-Length : 5\r\n\n";
      String data = "";
      int connectionId = -1;
      if (esp8266.available()) {
        connectionId = esp8266.read() - 48;
      }
      if (connectionId == 0) {
        String conId = "";
        conId += connectionId;

        esp8266.find("pin=");
        delay(5);
        int pinNumber1 = (esp8266.read() - 48);
        int pinNumber2 = (esp8266.read() - 48);
        int pinNumber = -1;

        if (pinNumber1 >= 0)
        {
          if (pinNumber2 >= 0) {
            pinNumber = pinNumber1 * 10 + pinNumber2;
          } else {
            pinNumber = pinNumber1;
          }
        }


        if (pinNumber >= 0) {
          digitalWrite(pinNumber, !digitalRead(pinNumber));
        }
        data = "pin:";
        data += pinNumber;

        getData += data;



        boolean sendFlag = sendCommand("AT+CIPSEND=" + conId + "," + String(getData.length()), 5, ">", false);
        if (!sendFlag) {
          resetConnection();
          continue;
        }


        if (sendFlag) {
          sendFlag = sendCommand(getData, 5, "OK", false);
          if (!sendFlag) {
            resetConnection();
            continue;
          }
          countTrueCommand++;
          sendFlag = sendCommand("AT+CIPCLOSE=" + conId, 5, "OK", false);
          if (!sendFlag) {
            resetConnection();
            continue;
          }

        }

      }

      lflag = 1;
    }
  }
}

bool ConnectToWifi() {

  sendCommand("AT", 5, "OK", false);
  sendCommand("AT+CWMODE=1", 5, "OK", false);
  boolean isConnected = sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 10, "OK", false);
  if (isConnected)
  {
    sendCommand("AT+CIFSR", 5, "", true);
    OpenPort();
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
    lflag = 1;
    return true;
  } else {
    resetConnection();
  }

}

bool OpenPort() {
  sendCommand("AT+CIPMUX=1", 5, "OK", false);
  port = sendCommand("AT+CIPSERVER=1,80", 5, "OK", false);
  return port;
}

bool sendCommand(String command, int maxTime, char readReplay[], boolean isGetData) {
  boolean result = false;

  //Test Purpose
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);
    delay(100);
    if (esp8266.find(readReplay)) //ok
    {
      if (isGetData)
      {

        while (esp8266.available())
        {
          char character = esp8266.read();
          Data.concat(character);
          if (character == '\n')
          {
            Serial.print("Received: ");
            Serial.println(Data);
            delay(1);
            Data = "";
          }

        }

      }
      result = true;
      break;
    }
    countTimeCommand++;
  }

  if (result == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (result == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
  return result;
}

void resetConnection() {
  Serial.println("reseting connection");

  lflag = 0;
  digitalWrite(4, LOW);
  delay(1000);
  digitalWrite(4, HIGH);

  ConnectToWifi();

  lflag = 1;
}
