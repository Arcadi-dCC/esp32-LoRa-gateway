#ifndef mailClientPrivate_H
#define mailClientPrivate_H

#include <platformTypes.h>
#include <Arduino.h>

uint8 EmailConfig(void);
uint8 EmailSend(String subject, String textMsg);

#endif //mailClientPrivate_H