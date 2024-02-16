/********************************************************/
/*************LoRa VALUES AND DECLARATIONS  *************/
/********************************************************/

#include <SPI.h>
#include <LoRa.h>

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BUFFER_SIZE 255 //Maximum number of bytes per packet
#define NET_ID 0x53AC //ID that all emmiters must use to communicate with gateway

int LoRaConfig(int sck, int miso, int mosi, int ss, int rst, int dio0, int freq, int sf, int bw);
int LoRaReading(byte* values_str, int* values_pos);
void onReceive(int packetSize);
void printStr(byte* str, int len);

byte values[BUFFER_SIZE];
int num_values = 0;


/********************************************************/
/***********InfluxDB VALUES AND DECLARATIONS  ***********/
/********************************************************/

#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include <InfluxDbClient.h>

#define WIFI_SSID "LAB243_exp"
#define WIFI_PASSWORD "123456789"
#define INFLUXDB_URL "http://10.20.255.254:8086/"
#define INFLUXDB_TOKEN "RjBjj-Q5BDBV5ndi3_GNqEwc2EX8KDcErqzsBHk_Rf5t5QMVX1FFiYOcPsMGJYrZZsQw-W8fywxJR7Pb5A7JTw=="
#define INFLUXDB_ORG "1d5669401414b39c"
#define INFLUXDB_BUCKET "esp32_tests"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

int WiFiConnection(const char *WiFiSSID, const char *WiFiPassword);
int InfluxServerConnection(void);
int uploadValue(const String &field, int value);
void SwReset(int seconds);

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point sensor("LoRa"); // Data point (_measurement)
RTC_DATA_ATTR int new_value = 31;
RTC_DATA_ATTR int old_value = 0;
RTC_DATA_ATTR int discontinuity = 0;
RTC_DATA_ATTR int upload_error = 0;



/********************************************************/
/***********   EMAIL VALUES AND DECLARATIONS  ***********/
/********************************************************/

#include <ESP_Mail_Client.h>

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "espsender1@gmail.com"
#define AUTHOR_PASSWORD "yajhwpxuyhmmhrdd"

#define RECIPIENT_EMAIL "espsender1@gmail.com" // Recipient's email

SMTPSession smtp;                      // Declare the global used SMTPSession object for SMTP transport
Session_Config config;
SMTP_Message message;

int EmailConfig(Session_Config* conf, SMTP_Message* msg, const char* smtp_host, int smtp_port, String author_name, const char* author_email, const char* author_password);
int EmailLogIn(Session_Config* conf, SMTPSession* smtpp);
int EmailSend(Session_Config* conf, SMTPSession* smtpp, SMTP_Message* msg, String recipient_name, const char* recipient_email, String subject, String textMsg);


/********************************************************/
/*********************  MAIN CODE  **********************/
/********************************************************/

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Gateway");

  //Connect to WiFi
  if (WiFiConnection(WIFI_SSID, WIFI_PASSWORD))
  {
    SwReset(10);
    //while(1);
  }

  //Update the RTC
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  //Connect to InfluxDB server
  if (InfluxServerConnection())
  {
    SwReset(10);
    //while (1);
  }

  // Add tags
  sensor.addTag("test", "LoRa_2minutes");
  sensor.addTag("try", "20240215_1");

  //Configure and log into e-mail account
  if (EmailConfig(&config, &message, SMTP_HOST, SMTP_PORT, "esp32 sender", AUTHOR_EMAIL, AUTHOR_PASSWORD))
  {
    SwReset(10);
  }

  if (LoRaConfig(SCK, MISO, MOSI, SS, RST, DIO0, 868E6, 7, 250E3))
  {
    SwReset(10);
  }
  LoRa.onReceive(onReceive);
  LoRa.receive();
  delay(1000);
  Serial.println("Listening");
}

void loop() {

  //interrupt called
  if (num_values != 0)
  {
    Serial.println();
    printStr(values, num_values);
    Serial.println();
    Serial.println(num_values);
   
    if(num_values == 3)
    {
      
      old_value = new_value;
      new_value = values[2];

      if (((old_value + 1) % 32) != new_value)
      {
        discontinuity++;
      }

      if(uploadValue("received_value", new_value))
      {
        upload_error++;
      }
      if(new_value == 31)
      {
        (void)EmailSend(&config, &smtp, &message, "esp32 sender", RECIPIENT_EMAIL, "20240214_1 test", "31 has been reached again");
      }
      
    }
    else
    {
      Serial.println("More than 3 bytes received");
    }
    num_values = 0;
  }
}




/********************************************************/
/****************  FUNCTION DEFINITIONS  ****************/
/********************************************************/

//Encapsules the whole LoRa configuration. Returns 0 if successful, 1 if error.
int LoRaConfig(int sck, int miso, int mosi, int ss, int rst, int dio0, int freq, int sf, int bw)
{
  Serial.println("Configuring LoRa");
  //SPI LoRa pins
  SPI.begin(sck, miso, mosi, ss);

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    return 1;
  }

  LoRa.setSpreadingFactor(sf);        //LoRa Spreading Factor configuration.
  LoRa.setSignalBandwidth(bw);     //LoRa Bandwidth configuration.
  //LoRa.setPreambleLength(8);
  //LoRa.enableInvertIQ();
  //LoRa.setSyncWord(0x00);
  return 0;
}

//Returns 0 if no new data, 1 if there is something new. Data and data lenght is returned through parameters
int LoRaReading(byte* values_str)
{
  for (int w = 0; w < num_values; w++) values_str[w] = LoRa.read();
  return 1;
}

//ISR when receiving LoRa signals
void onReceive(int packetSize)
{
  int w = 0;
  //Checks if the packet is long enough for an ID to fit
  if(packetSize > 2)
  {
    //Reads the ID values and compares to the established ID
    for(w = 0;w<2;w++) values[w] = LoRa.read();
    if(values[0] == (((NET_ID & 0xFF00) >> 8)) and (values[1] == (NET_ID & 0x00FF)))
    {
      //Continues if ID is correct
      if(packetSize > BUFFER_SIZE) num_values = BUFFER_SIZE;
      else num_values = packetSize;
      
      for(w = 2; w < num_values; w++) values[w] = LoRa.read();
    }
  }
}

void printStr(byte* str, int len)
{
  Serial.print("| ");
  for (int w = 0; w < len; w++)
  {
    Serial.print(str[w]);
    Serial.print(" | ");
  }
}

//Tries to connect to WiFi. Returns: 0 if connection successful, 1 if 30 s timeout reached.
int WiFiConnection(const char *WiFiSSID, const char *WiFiPassword)
{
  delay(1000);
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WiFiSSID, WiFiPassword);

  byte connection_timer = 0;
  while ((wifiMulti.run() != WL_CONNECTED) and (connection_timer < 19)) {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (connection_timer > 18)
  {
    Serial.println("Could not connect to WiFi (timeout reached).");
    return 1;
  }
  else return 0;
}

//Connects to the InfluxDB server. Returns: 0 if connection successful, 1 if error.
int InfluxServerConnection(void)
{
  Serial.print("Connecting to InfluxDB local server");
  int returner = 0;
  byte connection_timer = 0;
  while ((!client.validateConnection()) and (connection_timer < 2))
  {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (connection_timer > 1)
  {
    Serial.print("Timeout reached. InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    returner = 1;
  }

  else
  {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  }
  return returner;
}

//Uploads a value to InfluxDB server with specified field. Returns: 0 if successful, 1 if error.
int uploadValue(const String &field, int value)
{
  Serial.print("Uploading new value: ");
  Serial.print(new_value);
  Serial.print(" ");

  sensor.clearFields();
  sensor.addField(field, value);
  sensor.addField("discontinuity", discontinuity); //TEMP!
  sensor.addField("upload_error", upload_error); //TEMP!

  int upload_timer = 0;
  while ((!client.writePoint(sensor)) and (upload_timer < 2))
  {
    upload_timer++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (upload_timer > 1)
  {
    Serial.print("Timeout reached. InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
    return 1;
  }
  else
  {
    return 0;
  }
}

//Performs a software reset after warning through the serial output and doing a countdown.
void SwReset(int countdown)
{
  Serial.print("Reseting the MCU in ");
  for (int w = countdown; w > 0; w--)
  {
    Serial.print(w);
    Serial.print(" ");
    delay(1000);
  }
  ESP.restart();
}

//Does the configuration for Email sending. Returns: 0 if successful, 1 if error.
int EmailConfig(Session_Config* conf, SMTP_Message* msg, const char* smtp_host, int smtp_port, String author_name, const char* author_email, const char* author_password)
{
  Serial.println("Configuring e-mail log in values");
  MailClient.networkReconnect(true);
  //smtpp->debug(1);
  //smtpp->callback(smtpCallback);

  conf->server.host_name = SMTP_HOST;
  conf->server.port = SMTP_PORT;
  conf->login.email = AUTHOR_EMAIL;
  conf->login.password = AUTHOR_PASSWORD;
  conf->login.user_domain = "";

  conf->time.ntp_server = F("pool.ntp.org,time.nist.gov");
  conf->time.gmt_offset = 1;
  conf->time.day_light_offset = 0;

  msg->sender.name = author_name;
  msg->sender.email = author_email;

  return 0;
}

//Tries to log into the e-mail account specified in EmailConfig.
int EmailLogIn(Session_Config* conf, SMTPSession* smtpp)
{
  Serial.print("Logging into e-mail account");
  int returner = 0;
  int connection_timer = 0;
  while ((!smtpp->connect(conf)) and (connection_timer < 2))
  {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  if (connection_timer > 1)
  {
    Serial.println();
    Serial.print("Timeout reached. ");
    ESP_MAIL_PRINTF("Could not connect to e-mail server. Status Code: %d, Error Code: %d, Reason: %s", smtpp->statusCode(), smtpp->errorCode(), smtpp->errorReason().c_str());
    returner = 1;
  }

  connection_timer = 0;
  while ((!smtpp->isLoggedIn()) and (connection_timer < 2))
  {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  if (connection_timer > 1)
  {
    Serial.println();
    Serial.println("Timeout reached. Not logged in mail account.");
    returner = 1;
  }
  else
  {
    connection_timer = 0;
    while ((!smtpp->isAuthenticated()) and (connection_timer < 2))
    {
      connection_timer++;
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    if (connection_timer > 1)
    {
      Serial.println("Timeout reached. Connected to mail service with no Auth.");
      returner = 1;
    }
  }
  return returner;
}

//Tries to send an e-mail. Returns 1 if successful, 0 if error.
int EmailSend(Session_Config* conf, SMTPSession* smtpp, SMTP_Message* msg, String recipient_name, const char* recipient_email, String subject, String textMsg)
{
  //Log in e-mail account
  if (EmailLogIn(conf, smtpp)) return 1;

  msg->subject = subject;
  msg->addRecipient(recipient_name, recipient_email);

  //Send raw text message
  msg->text.content = textMsg.c_str();
  msg->text.charSet = "us-ascii";
  msg->text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  msg->priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  msg->response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Start sending Email and close the session */
  Serial.print("Sending e-mail");

  int send_counter = 0;
  while ((!MailClient.sendMail(smtpp, msg, true)) and (send_counter < 2))
  {
    send_counter++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (send_counter > 1)
  {
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtpp->statusCode(), smtpp->errorCode(), smtpp->errorReason().c_str());
    return 1;
  }
  return 0;
}
