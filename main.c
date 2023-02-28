#include <msp430.h>
#include "in430.h"
#include "DS1302.h"

#define CS BIT0  //2.0 is CS
#define MOSI BIT7  //1.7 is SPI MOSI
#define SCLK BIT5  //1.5 is SPI clock


#define P1_4 BIT4
#define P1_6 BIT6
#define P2_4 BIT4
#define P2_5 BIT5
#define P1_0 BIT0
#define P1_1 BIT1
#define P1_2 BIT2
#define P1_3 BIT3
volatile char LEN=4;
unsigned char matrix[4][4] ={'1','2','3','A',
                             '4','5','6','B',
                             '7','8','9','C',
                             '*','0','#','D'};

void ScanKey(void);

unsigned int sicaklik_ham,sicaklik,sicaklik_temp;

void sicaklik_init(void);
void deger_goster(unsigned int,unsigned int);
void sicaklik_oku(void);
void integer_yaz(unsigned int,char);

unsigned int sicaklik=0;
volatile char tick;
unsigned int c;
int d1=1,d2=2,d3=3,d4=4,ad=0;

struct _DS1302 DS1302; // Saat/tarih bilgilerini tutan yapi
unsigned char bGecikmeSayac=0,bMenu=0;  // Gecikme sayaci
void Gun_Yazdir(unsigned char);
void Sayi_Yazdir(unsigned char);
void Saat_Tarih_Goster(void);



void Init_MAX7219(void);
void SPI_Init(void); //SPI initialization
void SPI_Write2(unsigned char, unsigned char);
unsigned char disp1[10]={0x7E,0x30,0x6D,0x79,0x33,0x5B,0x5F,0x70,0x7F,0x7B};  


void SPI_Init(void) //SPI initialization
{
  P2DIR |= CS;  //cs is output
  P1SEL |= MOSI + SCLK; //spi init
  P1SEL2 |= MOSI + SCLK; //spi init
  UCB0CTL1 = UCSWRST;
  UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCCKPH; // 3-pin, 8-bit SPI master
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  UCB0BR0 = 10;                          // spi speed is smclk/10
  UCB0BR1 = 0;                              //
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     __enable_interrupt(); // enable all interrupts
}
void SPI_Write2(unsigned char MSB, unsigned char LSB) //SPI write one byte
{
       P2OUT &= ~CS;
       __delay_cycles(50);
       UCB0TXBUF = MSB ;
       while (UCB0STAT & UCBUSY);
       UCB0TXBUF = LSB ;
       while (UCB0STAT & UCBUSY);
       P2OUT |= CS;
}
void Init_MAX7219(void)

{
       SPI_Write2(0x09, 0x00);       //
       SPI_Write2(0x0A, 0x0F);       //
       SPI_Write2(0x0B, 0x0F);       //
       SPI_Write2(0x0C, 0x01);       //
       SPI_Write2(0x0F, 0x0F);       //
       SPI_Write2(0x0F, 0x00);       //
}

void sicaklik_init()
{
  ADC10CTL0 &= ~ENC;
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + MSC + ADC10IE;  
  ADC10CTL1 = INCH_10 + ADC10SSEL_0 + ADC10DIV_0 + CONSEQ_2;
  ADC10DTC1 = 32;
  __delay_cycles(256);

}


void sicaklik_oku()
{
 char i;
  float yeni=0;
  unsigned int toplam=0;
  unsigned int dma_buf[32];
  
  ADC10SA = (unsigned int)dma_buf;
  ADC10CTL0 |= ENC + ADC10SC;        
  __bis_SR_register(CPUOFF + GIE);    
  
  for(i=0;i<32;i++)
    toplam+=dma_buf[i];
  
  toplam >>= 5;
  
  sicaklik_ham = toplam;
  yeni=(((sicaklik_ham - 673) * 423) / 1024.0f)*100;
  sicaklik=(unsigned int)yeni;  
}

/*void Sayi_Yazdir(unsigned char Sayi)
{
if(Sayi<100){
SPI_Write2(0X06,Sayi/10+0x30);
SPI_Write2(0X07,Sayi%10+0x30);
}}*/

void Saat_Tarih_Goster(void){
DS1302_Saat_Tarih_Oku(&DS1302); // DS1302'den saat/tarih bilgisini oku
SPI_Write2(0x08,disp1[DS1302.Saat/10]); // Dakikanin onlar basamagini yazdir.
SPI_Write2(0x07,disp1[DS1302.Saat%10]); // Dakikanin birler basamagini yazdir.

SPI_Write2(0x06,disp1[DS1302.Dakika/10]); // Dakikanin onlar basamagini yazdir.
SPI_Write2(0x05,disp1[DS1302.Dakika%10]); // Dakikanin birler basamagini yazdir.


}



int main(void)

{
          WDTCTL = WDTPW | WDTHOLD;
       BCSCTL1 = CALBC1_1MHZ; // Dahili osilatörü 1MHz'e ayarla.
       DCOCTL = CALDCO_1MHZ; // Dahili osilatörü 1MHz'e ayarla.
       
       
        P2DIR |= (BIT4 + BIT5); // Output Direction p2.0,p2.1,p2.2,p2.3
        P1DIR |= (BIT4 + BIT6);
        P2OUT &= ~(BIT4 + BIT5); // Put output to low
        P2OUT &= ~(BIT4 + BIT6);

         P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3); //Input Direction p1.3,p1.4,p1.5,p1.7
         P1REN |= (BIT0 + BIT1 + BIT2 + BIT3);//enable resistor p1.3,p1.4,p1.5,p1.7
         P1OUT |= (BIT0 + BIT1 + BIT2 + BIT3);// set resistor pull up
         P1IES |= (BIT0 + BIT1 + BIT2 + BIT3);//high to low
         P1IE |= (BIT0 + BIT1 + BIT2 + BIT3); //enable interupt

        P1IFG = 0;                                  //Clear Port1 IFG
         P2IFG = 0;                                  //Clear Port2 IFG
    
       DS1302_Ayarla(); // DS1302 ayarlari 
       TA0CCTL0 = CCIE; // Timer0 CCR0 ayarlari
      
       TA0CCR0 = 50000; // Timer0 kesme periyodu ~50ms
       TA0CTL = TASSEL_2 + TACLR + MC_2;

       sicaklik_init();
      
       //_BIS_SR(GIE); // Kesmeleri aç

       __bis_SR_register(GIE);
       TA0CTL |= MC_1; //Timer Baslatiliyor...
       
       SPI_Init();
       __delay_cycles(100000);
       Init_MAX7219();
      __delay_cycles(1000);
      
       while(1)
       {
           Saat_Tarih_Goster();
  
         
         sicaklik_temp=sicaklik;
         
          d4=sicaklik_temp%10;
          sicaklik_temp=sicaklik_temp/10;

          d3=sicaklik_temp%10;
          sicaklik_temp=sicaklik_temp/10;
          
          d2=sicaklik_temp%10;
          sicaklik_temp=sicaklik_temp/10;

          d1=sicaklik_temp%10;

    
       }

}
void ScanKey()
{
    unsigned char row_sel=0;
    unsigned char keyrow=0;
    unsigned char i=0;
    unsigned char j=0;


    for (i = 0 ; i < LEN ; i ++)// each output
    {
        switch(i)
        {
        case 0:
            P1OUT |= BIT4; // output p1.4 is high
            break;
        case 1:
            P1OUT |= BIT6;// output p1.6 is high
            break;
        case 2:
            P2OUT |= BIT4;// output p2.4 is high
            break;
        case 3:
            P2OUT |= BIT5;// output p2.5 is high
            break;
        }

        __delay_cycles(100);

        if((P1IN & BIT3))           // find the pressed button row
            row_sel|=0x08;
        if((P1IN & BIT2))
            row_sel|=0x04;
        if((P1IN & BIT1))
            row_sel|=0x02;
        if((P1IN & BIT0))
            row_sel|=0x01;
        keyrow = BIT0;

        for (j = 0 ; j< LEN ; j++)
        {
            if ((row_sel & keyrow) == 1)
            {
                P2DIR |= (BIT4 + BIT5); // Output Direction p2.0,p2.1,p2.2,p2.3
                P1DIR |= (BIT4 + BIT6);
                P2OUT &= ~(BIT4 + BIT5); // Put output to low
                P1OUT &= ~(BIT4 + BIT6);
               
                if ( matrix[i][j] == '1')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x05);
                  DS1302_Veri_Yaz(0x82,0x35);
                  Saat_Tarih_Goster();
                 
                }
                
                 if ( matrix[i][j] == '2')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x06);
                  DS1302_Veri_Yaz(0x82,0x36);
                  Saat_Tarih_Goster();
                  
                }
                 if ( matrix[i][j] == '3')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x07);
                  DS1302_Veri_Yaz(0x82,0x37);
                  Saat_Tarih_Goster();
                  
                }
                 if ( matrix[i][j] == '3')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x08);
                  DS1302_Veri_Yaz(0x82,0x38);
                  Saat_Tarih_Goster();
                  
                }
                
                  if ( matrix[i][j] == '4')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x09);
                  DS1302_Veri_Yaz(0x82,0x39);
                  Saat_Tarih_Goster();
                  
                }
                
                 if ( matrix[i][j] == '5')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x10);
                  DS1302_Veri_Yaz(0x82,0x40);
                  Saat_Tarih_Goster();
                  
                }
                 if ( matrix[i][j] == '6')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x11);
                  DS1302_Veri_Yaz(0x82,0x41);
                  Saat_Tarih_Goster();
                  
                }
                 if ( matrix[i][j] == '7')
                {
                  
                  DS1302_Veri_Yaz(0x84,0x12);
                  DS1302_Veri_Yaz(0x82,0x42);
                  Saat_Tarih_Goster();
                  
                }



                 // return matrix[i][j];
            }

            keyrow = keyrow >> 1;
        }
        row_sel=0;

        switch(i)
                {
                case 0:
                    P1OUT &= ~BIT4;// output p1.4 is low
                    break;
                case 1:
                    P1OUT &= ~BIT6;// output p1.6 is low
                    break;
                case 2:
                    P2OUT &= ~BIT4;// output p2.4 is low
                    break;
                case 3:
                    P2OUT &= ~BIT5;// output p2.5 is low
                    break;
                }

    }


                P2DIR |= (BIT4 + BIT5); // Output Direction p2.0,p2.1,p2.2,p2.3
                P1DIR |= (BIT4 + BIT6);
                P2OUT &= ~(BIT4 + BIT5); // Put output to low
                P1OUT &= ~(BIT4 + BIT6);
 
}

#pragma vector=PORT1_VECTOR
__interrupt void port_1(void)
{

    ScanKey();
    P1IFG = 0;                                  //Clear Port1 IFG
    P2IFG = 0;                                  //Clear Port2 IFG
}

#pragma vector=PORT2_VECTOR
__interrupt void port_2(void)
{

    ScanKey();
    P1IFG = 0;                                  //Clear Port1 IFG
    P2IFG = 0;                                  //Clear Port2 IFG
}





// Timer_A0 Zamanlayicisi CCR0 kesme vektörü
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  if(++bGecikmeSayac>=20){ // ~1sn oldu mu?
bGecikmeSayac=0; // Sayaci sifirla
__bic_SR_register_on_exit(CPUOFF); // Islemciyi uykudan uyandir.


    sicaklik_oku();
  
if(ad==0)
{
  SPI_Write2(0x01,0x4E);
  
}
else if(ad==1)
{
  SPI_Write2(0x02,disp1[d3]);
  
}
else if(ad==2)
{
  SPI_Write2(0x03,disp1[d2]+0x80);
  
  
  
}
else if(ad==3)
{
  SPI_Write2(0x04,disp1[d1]);
  ad=-1;
}
 ad++;
 
    
  }
TA0CCR0 += 50000; // Zamanlayiciyi yeniden kur.
}


// ADC10 Kesme Vektörü
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

