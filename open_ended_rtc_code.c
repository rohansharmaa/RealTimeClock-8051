#include<reg51.h>
#include<string.h>
#define lcd P1	  //Port 1 is used for LCD data
sbit rs=P2^0;	  //Register Select pin
sbit e=P2^1;	  //Enable pin

sbit c=P0^0;	  //Serial Clock bit of RTC
sbit d=P0^1;	  //Serial Data bit of RTC

//Function headers
void delay (int);
void cmd (unsigned char);
void display (unsigned char);
void string (char *);
void init (void);

void i2c_start (void);
void i2c_stop (void);
void i2c_write (unsigned char);
unsigned char i2c_read (void);
void i2c_ack (void);
void i2c_noack (void);
void write (unsigned char, unsigned char, unsigned char);
unsigned char read (unsigned char, unsigned char);
void set_time (unsigned char, unsigned char, unsigned char);

//ASCII values for 0-9
int no[10]={48,49,50,51,52,53,54,55,56,57};

unsigned int temp1[3]=0;

void delay (int d)		//Delay function 
{
	unsigned char i=0;
	for(;d>0;d--)
	{
		for(i=250;i>0;i--);
		for(i=248;i>0;i--);
	}
}

void cmd (unsigned char c)	   //Function to send commands to LCD display
{
	lcd=c;
	rs=0;
	e=1;
	delay(5);
	e=0;
}
void display (unsigned char c)	 //Function to display data on LCD 
{
	lcd=c;
	rs=1;
	e=1;
	delay(5);
	e=0;
}
void string (char *p)			//Function to display string on LCD
{
	while(*p)
	{
		display(*p++);
	}
}
void init (void)			   //Initiating the LCD display
{
	cmd(0x38);
    cmd(0x0c);
	cmd(0x01);
    cmd(0x80);
}


void i2c_start (void)		  //d switches from high to low before c switches
{
    c=1;
	d=1;
	delay(1);
	d=0;
	delay(1);
}

void i2c_stop (void)		//d switches from low to high after c switches from low to high
{
	c=0;
	delay(1);
	d=0;
	delay(1);
	c=1;
	delay(1);
	d=1;
	delay(1);
}
void i2c_write (unsigned char a)  //signals required to write data
{
	char j;
	for(j=0;j<8;j++)
	{
		c=0;
		d=(a&(0x80>>j))?1:0;	  //sending data bitwise
		c=1;
	}
}
unsigned char i2c_read (void)	 //reading signal from data line and returning it
{								 
	char j;
	unsigned char temp=0;
	for(j=0;j<8;j++)
	{
		c=0;
		if(d)
		temp=temp|(0x80>>j);	 //reading data bitwise
		c=1;
	}
	return temp;
}
void i2c_ack (void)			   //acknowledgement signal after operation is performed
{
	c=0;
	delay(1);
	d=1;
	delay(1);
	c=1;
	delay(1);
	while(d==1);
	c=0;
	delay(1);
}
void i2c_noack (void)		 //non-acknowledgement signal, if opertaion is not performed
{
	c=0;
	delay(1);
	d=1;
	delay(1);
	c=1;
	delay(1);
	
}

void write (unsigned char sa, unsigned char w_addr, unsigned char dat)   //writing data dat to w_addr register in RTC memory
{																		 // sa--> slave address, fixed in our case as we have only 1 slave
	i2c_start();
	i2c_write(sa);
	i2c_ack();
	i2c_write(w_addr);
    i2c_ack();
    i2c_write(dat);
    i2c_ack();
	i2c_stop();
    delay(10);
}
unsigned char read (unsigned char sa, unsigned char w_addr)			  //reading data from RTC memory's 'w_addr' register
{																	  //sa--> slave address, fixed in our case as we have only 1 slave
	unsigned char buf=0;
	i2c_start();	
	i2c_write(sa);
	i2c_ack();
	i2c_write(w_addr);
	i2c_ack();
	i2c_start();
	i2c_write(sa|0x01);
	i2c_ack();
	buf=i2c_read();
	i2c_noack();
	i2c_stop();
  return buf;
}
void set_time (uns rigned char hour, unsigned char min, unsigned char sec)	//Setting time in the RTC memory
{
	unsigned int temp[3];
	temp[0]=(((unsigned char)(sec/10))<<4)|((unsigned char)(sec%10));		//seconds
	temp[1]=(((unsigned char)(min/10))<<4)|((unsigned char)(min%10));		//minutes
	temp[2]=(((unsigned char)(hour/10))<<4)|((unsigned char)(hour%10));		//hours
	write(0xd0,0x00,temp[0]);			 //writing to 0x00 register
	write(0xd0,0x01,temp[1]);			 //writing to 0x01 register
	write(0xd0,0x02,temp[2]);			 //writing to 0x02 register
}


void main()
{
    unsigned char temp=0;
	init();
    string("  Digital Clock ");
	cmd(0xc0);
	string("   using 8051   ");
		
	delay(3000);
	cmd(0x01);
    set_time(12,59,51);	   //Setting initial time
	delay(500);
	while(1)
	{
	cmd(0x80);
	temp=read(0xd0,0x00);
	temp1[0]=((temp&0x7F)>>4)*10 + (temp&0x0F);		//0x7F to eleminate sign bit
	temp=read(0xd0,0x01);
	temp1[1]=(temp>>4)*10 + (temp&0x0F);
	temp=read(0xd0,0x02);	
	temp1[2]=(temp>>4)*10 + (temp&0x0F);
		
    display(no[(temp1[2]/10)%10]);
	display(no[temp1[2]%10]);
	display(':');
	display(no[(temp1[1]/10)%10]);
	display(no[temp1[1]%10]);
	display(':');
	display(no[(temp1[0]/10)%10]);
	display(no[temp1[0]%10]);
		
  }
	while(1);	  //precaution
}