#ifndef USCI_LIB
#define USCI_LIB

#define SDA_PIN BIT6                                  // msp430FR5739 UCB0SDA pin 1.6
#define SCL_PIN BIT7                                  // msp430FR5739 UCB0SCL pin 1.7

/*void TI_USCI_I2C_receiveinit(unsigned char slave_address, unsigned char prescale);
void TI_USCI_I2C_transmitinit(unsigned char slave_address, unsigned char prescale);

void TI_USCI_I2C_receive(unsigned char byteCount, unsigned char *field);
void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field);

unsigned char TI_USCI_I2C_slave_present(unsigned char slave_address);
unsigned char TI_USCI_I2C_notready();*/

void receiveI2C_init(unsigned char slave_address);
void transmitI2C_init(unsigned char slave_address);
void transmitI2C(unsigned char byteCount, unsigned char *field);
void receiveI2C(unsigned char byteCount, unsigned char *field);
unsigned char TI_USCI_I2C_notready();

#endif
