#include <Arduino.h>

#include <mailClientPrivate.h>
#include <mailClientCfg.h>

#include <ESP_Mail_Client.h>

SMTPSession smtp;                      // Declare the global used SMTPSession object for SMTP transport
Session_Config config;
SMTP_Message message;

//Does the configuration for Email sending. Returns: 0 if successful, 1 if error.
uint8 EmailConfig(void)
{
  Serial.println("Configuring e-mail log in values");
  MailClient.networkReconnect(true);
  //smtp.debug(1);

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 1;
  config.time.day_light_offset = 0;

  message.sender.name = AUTHOR_NAME;
  message.sender.email = AUTHOR_EMAIL;

  return 0;
}

//Tries to log into the e-mail account specified in EmailConfig.
uint8 EmailLogIn(void)
{
  Serial.print("Logging into e-mail account");
  int returner = 0;
  int connection_timer = 0;
  while ((!smtp.connect(&config)) and (connection_timer < 2))
  {
    connection_timer++;
    Serial.print(".");
    delay(500);
  }
  if (connection_timer > 1)
  {
    Serial.println();
    Serial.print("Timeout reached. ");
    ESP_MAIL_PRINTF("Could not connect to e-mail server. Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    returner = 1;
  }

  connection_timer = 0;
  while ((!smtp.isLoggedIn()) and (connection_timer < 2))
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
    while ((!smtp.isAuthenticated()) and (connection_timer < 2))
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
uint8 EmailSend(String subject, String textMsg)
{
  //Log in e-mail account
  if (EmailLogIn()) return 1;

  message.subject = subject;
  message.addRecipient(RECIPIENT_NAME, RECIPIENT_EMAIL);

  //Send raw text message
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Start sending Email and close the session */
  Serial.print("Sending e-mail");

  int send_counter = 0;
  while ((!MailClient.sendMail(&smtp, &message, true)) and (send_counter < 2))
  {
    send_counter++;
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  if (send_counter > 1)
  {
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return 1;
  }
  return 0;
}
