#include <REG52.h>
#include<intrins.h>
sbit K1=P1^2;
sbit ALAM = P2^5;                                                                                                //����        
sbit open_led=P2^3;                                                                                                //����ָʾ��        
/*LCD�ӿڶ���*/
sbit LcdRS   = P2^0;                                                                                      //lcd����/����ѡ���  ����1����0
sbit LcdRW   = P2^1;                                                                                      //lcd��/дѡ���    ��1д0
sbit LcdEn   = P2^2;                                                                                      //lcdʹ�ܿ��ƶ�  1��Ч
sfr  LcdIO   = 0x80;                                                                                      //lcd���ݽӿ� P0=0x80

unsigned char code a[]={0xFE,0xFD,0xFB,0xF7};                                         //����ɨ����Ʊ�
               
unsigned char countt0,second;                                                                        //t0�жϼ�����,�������
unsigned char code start_line[]        = {"password:       "};
unsigned char code name[]                  = {"===Coded Lock==="};                     //��ʾ����                 
unsigned char code Error[]           = {"      error     "};                  //�������
unsigned char code codepass[]        = {"      pass      "};
unsigned char code LockOpen[]        = {"      open      "};                        //OPEN

char InputData[6];                                                                                                //���������ݴ���
unsigned char CurrentPassword[6]={6,5,4,3,2,1};                                 //��ǰ����ֵ
unsigned char N=0;                                                                                                //��������λ������
unsigned char ErrorCont;                                                                                //�����������
unsigned char KEY_SCAN,NUM;

/****��LCDд����������ݲ���****/
#define LCD_COMMAND         0         // ���ָ��
#define LCD_DATA            1         // �������
#define LCD_CLEAR_SCREEN    0x01      // ����ָ��
#define LCD_HOME            0x02      // ��귵��ԭ��ָ��

//=====================16us����ʱ==============================
void Delay_short(unsigned int n)
{
         unsigned int i;
         for(i=0;i<n;i++)     
         {;}
}
//=====================����ʱ==============================
void Delay_long(unsigned char N)
{
        unsigned char i;
          unsigned int j;
           for(i=0;i<N;i++)
           {
                for(j=0;j<315;j++)  //һ��ѭ��16us����5ms
              {;}
   }
}
//=====================5ms��ʱ==============================
void Delay5Ms(void)
{
        unsigned int TempCyc = 5552;
        while(TempCyc--);
}
//===================400ms��ʱ==============================
void Delay400Ms(void)
{
         unsigned char TempCycA = 5;
         unsigned int TempCycB;
         while(TempCycA--)
         {
                  TempCycB=7269;
                  while(TempCycB--);
         }
}

/*дLCD�ӳ���                                          */
/*��ڲ���������style=1 ָ��, style=0 input��д�������*/

void LCD_Write(bit style, unsigned char input)  
{
    LcdRS=style;         //����style=1  ָ��style=0
    LcdRW=0;             //д
    LcdIO=input;         //P0�����
    Delay_short(10);     //��ʱ
    LcdEn=1;             //lcdʹ��
    Delay_short(10);     //��ʱ
    LcdEn=0;             //ֹͣ
}

/****��ʼ��LCD����****/
void LCD_Initial()
{
    Delay_long(6);                                     //�ӳ�5*6=30ms
    LCD_Write(LCD_COMMAND,0x38);                       //8λ���ݶ˿�,2����ʾ,5*7����
    LCD_Write(LCD_COMMAND,0x38);
    LCD_Write(LCD_COMMAND,0x38);                       //��������
    Delay_short(2);                                    //�ӳٴ���39us
    LCD_Write(LCD_COMMAND,0x0c);                            //��ʾģʽ���ã�������ʾ, �޹�� 0x0c
    Delay_short(2);                             //�ӳٴ���39us
    LCD_Write(LCD_COMMAND,LCD_CLEAR_SCREEN);           //����                          0x01
    Delay_short(100);                           //�ӳٴ���1.53ms
    LCD_Write(LCD_COMMAND,0x06);                               //����ģʽ���ã�AC����, ���治�� 0x06
}


/*Һ���ַ������λ�ö�λ����*/
/*��ڲ�����x��Χ��0-15     y��Χ��1��2*/
void GotoXY(unsigned char x, unsigned char y)  
{
    unsigned char address;
    if(y==1)
        {
        address=0x80+x;         //y=1��ʾ�ڵ�һ��
    }
        else
        {
        address=0xc0+x;      //y=2��ʾ�ڵڶ���
    }
        LCD_Write(LCD_COMMAND, address);
}

/*���ַ��������Һ����ʾ����*/
/*��ڲ������ַ���ָ��      */
void Print(unsigned char *str)
{
    while(*str!='\0')
    {
        LCD_Write(LCD_DATA,*str);
        str++;
    }
}
//*********************************************************************
//==============������ֵ����Ϊ��ֵ=========================
unsigned char coding(unsigned char m)         
{
        unsigned char k;
        switch(m)
        {
//                case (0x18): k=3;break;
//                case (0x28): k=7;break;
//                case (0x48): k='#';break;
//                case (0x88): k='D';break;
                case (0x14): k=3;break;
                case (0x24): k=6;break;
                case (0x44): k=9;break;
                case (0x84): k='#';break;
                case (0x12): k=2;break;
                case (0x22): k=5;break;
                case (0x42): k=8;break;
                case (0x82): k=0;break;
                case (0x11): k=1;break;
                case (0x21): k=4;break;
                case (0x41): k=7;break;
                case (0x81): k='*';break;
        }
        return(k);
}
//=====================������Ⲣ���ذ���ֵ===============================
unsigned char keynum(void)
{
         unsigned char row,col,i;
         P3=0xf0;
         if((P3&0xf0)!=0xf0)
         {
                   Delay5Ms();
              Delay5Ms();
                   if((P3&0xf0)!=0xf0)
                {
                    row=P3^0xf0;          //ȷ������
                        i=0;
                        P3=a[i];                  //��ȷ��λ
                        while(i<4)
                        {
                                 if((P3&0xf0)!=0xf0)
                                  {
                                           col=~(P3&0xff);          //ȷ������
                                           break;            //�Ѷ�λ����ǰ�˳�   
                                  }
                                else
                                  {
                                           i++;
                                           P3=a[i];
                                  }
                        }
                }
                else
                {                 
                        return 0xff;        
                }
                while((P3&0xf0)!=0xf0);
                return (row|col);                         //������������Ϻ󷵻�
         }
         else
        {
                return 0xff;                                 //�޼�����ʱ����0xff
        }
}
//***************************************************************************************/
//=======================һ����ʾ������ʾ��Ч����========================
void OneAlam(void)
{
        unsigned char y;
        for(y=0;y<30;y++)
        {
        ALAM=0;
        Delay_short(50);
          ALAM=1;
        Delay_short(50);
        }

}

//========================������ʾ������ʾ�����ɹ�========================
void TwoAlam(void)
{
        OneAlam();
        Delay5Ms();
    ALAM=1;
          Delay5Ms();
        OneAlam();
        Delay5Ms();
    ALAM=1;
}
//========================������ʾ��,��ʾ����========================
void ThreeAlam(void)
{
        OneAlam();
        Delay5Ms();
    ALAM=1;
        Delay5Ms();
        OneAlam();
        Delay5Ms();
    ALAM=1;
          Delay5Ms();
        OneAlam();
        Delay5Ms();
    ALAM=1;
}
//=======================����������󳬹���������������������======================
void Alam_KeyUnable(void)
{
        P3=0x00;
        {
                OneAlam();
                Delay5Ms();
            ALAM=1;
                Delay5Ms();
        }
}
//=======================ȡ�����в���============================================
void Cancel(void)
{        
        unsigned char i;
        GotoXY(0,2);
        Print(start_line);        
        TwoAlam();                                //��ʾ��
        for(i=0;i<6;i++)
        {
                InputData[i]=0;
        }
        ALAM=1;                                        //������
        ErrorCont=0;                        //������������������
        open_led=1;                                  //ָʾ�ƹر�
        N=0;                                        //����λ������������
}
//==========================ȷ�ϼ�����ͨ����Ӧ��־λִ����Ӧ����===============================
void Ensure(void)
{        
        unsigned char i,j;
        for(i=0;i<6;)
        {                                       
                if(CurrentPassword[i]==InputData[i])
                {
                        i++;
                }
                else
                {                        
                        ErrorCont++;
                        if(ErrorCont==3)                        //�����������������ʱ����������������
                        {
                                GotoXY(0,2);
                                Print("  KeypadLocked! ");
                                TR0=1;
                                do
                                    Alam_KeyUnable();
                                while(TR0);
                return ;
                        }
            break;
                }
        }

        if(i==6)   //������ȷ����
        {
                        GotoXY(0,2);
                        Print(codepass);
                        Delay400Ms();
                        Delay400Ms();
                        GotoXY(0,2);
                        Print(LockOpen);        
                        TwoAlam();                                           //�����ɹ���ʾ��
                        //KEY_CLOCK=0;                                                                                        //����
                        //pass=1;                                                                                        //����ȷ��־λ
                        open_led=0;                                                                                //����ָʾ����
                        for(j=0;j<6;j++)                                                                //���������
                        {
                                InputData[i]=0;
                        }
                        while(1);
        }
        else
        {
        GotoXY(0,2);
                Print(Error);        
                 ThreeAlam();                                                                                //������ʾ��
        Delay400Ms();
        GotoXY(0,2);
                Print(start_line);
        for(j=0;j<6;j++)                                                                //���������
                {
                        InputData[i]=0;
                 }
    }
        N=0;                                                                                                        //���������ݼ��������㣬Ϊ��һ��������׼��
}

void test_led()
{
        while(1)
        {
                open_led=0;
                Delay400Ms();
                open_led=1;
                Delay400Ms();
        }
}

void test_alarm()
{
    while(1)
        {
                ALAM=1;
                Delay_short(60);
                ALAM=0;
                Delay_short(60);
        }
}
void timer_init()
{
        TMOD=0x11;
         TL0=0xB0;
         TH0=0x3C;
         EA=1;
         ET0=1;        
         TR0=0;
}

void test_key()
{
        unsigned char code ascii[]={"0123456789ABCDEF"};
        KEY_SCAN=keynum();
        if(KEY_SCAN!=0xff)
        {
                GotoXY(0,1);
                Print("key_code:0x");
                LCD_Write(LCD_DATA, ascii[KEY_SCAN/16]);
                LCD_Write(LCD_DATA, ascii[KEY_SCAN%16]);
        }
}
void keyscan()                                        //����ɨ�����
{
        static bit key_sign=0;        //����������־
        static bit sign=0;
        static unsigned char count=0;//��������                        
        unsigned char i;
        if(K1==0)                                        //��ⰴ�����Ϊ0
        {
                count++;                                        //��������
                if((count>=250)&&(key_sign==0))
                {                        
                        key_sign=1;                        //����������־��1
                        sign=~sign;
                        if(sign==1)
                        {
                                for(i=0;i<N;i++)
                                {
                                        GotoXY(9+i,2);
                                        LCD_Write(LCD_DATA,InputData[i]+0x30);
                                }
                        }
                        else
                        {
                                for(i=0;i<N;i++)
                                {
                                        GotoXY(9+i,2);
                                        LCD_Write(LCD_DATA,'*');
                                }
                        }
                }
        }
        else                                                        //����̧��
        {
                key_sign=0;                                //����������־��0
                count=0;                                        //����������0
        }
}

//==============================������===============================
void main(void)
{
         
        unsigned char j;
//        test_led();
//        test_alarm();
         Delay400Ms();         //�����ȴ�����LCM���빤��״̬
         LCD_Initial();         //LCD��ʼ��

//        while(1)
//        {
//                test_key();
//        }

        GotoXY(0,1);//������ʾ�̶����Ŵӵ�һ�е�0��λ��֮��ʼ��ʾ
        Print(name);//��Һ����д������ʾ�Ĺ̶����Ų���
        GotoXY(0,2);//ʱ����ʾ�̶�����д��λ�ã��ӵ�2��λ�ú�ʼ��ʾ
        Print(start_line);//д��ʾʱ��̶����ţ�����ð��
        GotoXY(9,2);        //���ù��λ��
        LCD_Write(LCD_COMMAND,0x0f);        //���ù��Ϊ��˸

//        while(1);

         P3=0xFF;
        timer_init();



         Delay5Ms(); //��ʱƬ��(�ɲ�Ҫ)
        N=0;                                                                                                                //��ʼ����������λ��
         while(1)
         {
        if (!TR0)
                {
                    KEY_SCAN=keynum();
                }
        else
                {
            KEY_SCAN = 0xff;
        }

                if(KEY_SCAN!=0xff)
                {                                   
                                        NUM=coding(KEY_SCAN);
                                        switch(NUM)
                                        {
                                                case 'A':         ;                                         break;
                                                case 'B':        ;                                     break;
                                                case 'C':         ;                                         break;
                                                case 'D':         ;                                        break;   
                                                case '*': Cancel();                                break;      //ȡ����ǰ����
                                                case '#': Ensure();                         break;           //ȷ�ϼ���
                                                default:
                                                {        
                                                        if(N<6)                                                           //���������������6λʱ���������벢���棬����6λʱ����Ч��
                                                        {
//                                                                  unsigned        char temp;
                                                                OneAlam();                                                                //������ʾ��                                                
                                                                for(j=N;j<=N;j++)
                                                                {
                                                                        GotoXY(9+j,2);
                                                                        LCD_Write(LCD_DATA,'*');
                                                                }
/*                                                           if(K1==0)
                                                           {
                                                                        GotoXY(9+N,2);
                                                                        temp=NUM+0x30           ;
                                                                        LCD_Write(LCD_DATA,temp);
                                                                                                  
                                                                }*/
                                                                InputData[N]=NUM;
                                                                N++;
                                                        }
                                                        else                                                                                //��������λ������6�󣬺�������
                                                        {
                                                                N=6;
                                                                 
                                                        }
                            break;
                                                }
                                        }
                        }
                keyscan();
        }
}

//*********************************�жϷ�����**************************************
void  time0_int(void) interrupt 1
{
         TL0=0xB0;
         TH0=0x3C;
         countt0++;
          if(countt0==20)
           {
                countt0=0;
                second++;
                if(second==10)
                {
                                P3=0xf0;
                                TL0=0xB0;
                                 TH0=0x3C;
                                second=0;
                                ErrorCont=0;                        //������������������
        GotoXY(0,2);
                                Print(start_line);
        TR0=0;                                //�ض�ʱ��
                }
                        
           }
}