#include <REG52.h>
#include<intrins.h>
sbit K1=P1^2;
sbit ALAM = P2^5;                                                                                                //报警        
sbit open_led=P2^3;                                                                                                //开锁指示灯        
/*LCD接口定义*/
sbit LcdRS   = P2^0;                                                                                      //lcd数据/命令选择端  数据1命令0
sbit LcdRW   = P2^1;                                                                                      //lcd读/写选择端    读1写0
sbit LcdEn   = P2^2;                                                                                      //lcd使能控制端  1有效
sfr  LcdIO   = 0x80;                                                                                      //lcd数据接口 P0=0x80

unsigned char code a[]={0xFE,0xFD,0xFB,0xF7};                                         //控盘扫描控制表
               
unsigned char countt0,second;                                                                        //t0中断计数器,秒计数器
unsigned char code start_line[]        = {"password:       "};
unsigned char code name[]                  = {"===Coded Lock==="};                     //显示名称                 
unsigned char code Error[]           = {"      error     "};                  //输入错误
unsigned char code codepass[]        = {"      pass      "};
unsigned char code LockOpen[]        = {"      open      "};                        //OPEN

char InputData[6];                                                                                                //输入密码暂存区
unsigned char CurrentPassword[6]={6,5,4,3,2,1};                                 //当前密码值
unsigned char N=0;                                                                                                //密码输入位数记数
unsigned char ErrorCont;                                                                                //错误次数计数
unsigned char KEY_SCAN,NUM;

/****向LCD写入命令或数据部分****/
#define LCD_COMMAND         0         // 输出指令
#define LCD_DATA            1         // 输出数据
#define LCD_CLEAR_SCREEN    0x01      // 清屏指令
#define LCD_HOME            0x02      // 光标返回原点指令

//=====================16us短延时==============================
void Delay_short(unsigned int n)
{
         unsigned int i;
         for(i=0;i<n;i++)     
         {;}
}
//=====================长延时==============================
void Delay_long(unsigned char N)
{
        unsigned char i;
          unsigned int j;
           for(i=0;i<N;i++)
           {
                for(j=0;j<315;j++)  //一个循环16us，共5ms
              {;}
   }
}
//=====================5ms延时==============================
void Delay5Ms(void)
{
        unsigned int TempCyc = 5552;
        while(TempCyc--);
}
//===================400ms延时==============================
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

/*写LCD子程序                                          */
/*入口参数：数据style=1 指令, style=0 input：写入的内容*/

void LCD_Write(bit style, unsigned char input)  
{
    LcdRS=style;         //数据style=1  指令style=0
    LcdRW=0;             //写
    LcdIO=input;         //P0口输出
    Delay_short(10);     //延时
    LcdEn=1;             //lcd使能
    Delay_short(10);     //延时
    LcdEn=0;             //停止
}

/****初始化LCD程序****/
void LCD_Initial()
{
    Delay_long(6);                                     //延迟5*6=30ms
    LCD_Write(LCD_COMMAND,0x38);                       //8位数据端口,2行显示,5*7点阵
    LCD_Write(LCD_COMMAND,0x38);
    LCD_Write(LCD_COMMAND,0x38);                       //发送三遍
    Delay_short(2);                                    //延迟大于39us
    LCD_Write(LCD_COMMAND,0x0c);                            //显示模式设置：开启显示, 无光标 0x0c
    Delay_short(2);                             //延迟大于39us
    LCD_Write(LCD_COMMAND,LCD_CLEAR_SCREEN);           //清屏                          0x01
    Delay_short(100);                           //延迟大于1.53ms
    LCD_Write(LCD_COMMAND,0x06);                               //输入模式设置：AC递增, 画面不动 0x06
}


/*液晶字符输入的位置定位程序*/
/*入口参数：x范围：0-15     y范围：1，2*/
void GotoXY(unsigned char x, unsigned char y)  
{
    unsigned char address;
    if(y==1)
        {
        address=0x80+x;         //y=1显示在第一行
    }
        else
        {
        address=0xc0+x;      //y=2显示在第二行
    }
        LCD_Write(LCD_COMMAND, address);
}

/*将字符串输出到液晶显示函数*/
/*入口参数：字符串指针      */
void Print(unsigned char *str)
{
    while(*str!='\0')
    {
        LCD_Write(LCD_DATA,*str);
        str++;
    }
}
//*********************************************************************
//==============将按键值编码为数值=========================
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
//=====================按键检测并返回按键值===============================
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
                    row=P3^0xf0;          //确定行线
                        i=0;
                        P3=a[i];                  //精确定位
                        while(i<4)
                        {
                                 if((P3&0xf0)!=0xf0)
                                  {
                                           col=~(P3&0xff);          //确定列线
                                           break;            //已定位后提前退出   
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
                return (row|col);                         //行线与列线组合后返回
         }
         else
        {
                return 0xff;                                 //无键按下时返回0xff
        }
}
//***************************************************************************************/
//=======================一声提示音，表示有效输入========================
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

//========================二声提示音，表示操作成功========================
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
//========================三声提示音,表示错误========================
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
//=======================输入密码错误超过三过，报警并锁死键盘======================
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
//=======================取消所有操作============================================
void Cancel(void)
{        
        unsigned char i;
        GotoXY(0,2);
        Print(start_line);        
        TwoAlam();                                //提示音
        for(i=0;i<6;i++)
        {
                InputData[i]=0;
        }
        ALAM=1;                                        //报警关
        ErrorCont=0;                        //密码错误输入次数清零
        open_led=1;                                  //指示灯关闭
        N=0;                                        //输入位数计数器清零
}
//==========================确认键，并通过相应标志位执行相应功能===============================
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
                        if(ErrorCont==3)                        //错误输入计数达三次时，报警并锁定键盘
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

        if(i==6)   //密码正确输入
        {
                        GotoXY(0,2);
                        Print(codepass);
                        Delay400Ms();
                        Delay400Ms();
                        GotoXY(0,2);
                        Print(LockOpen);        
                        TwoAlam();                                           //操作成功提示音
                        //KEY_CLOCK=0;                                                                                        //开锁
                        //pass=1;                                                                                        //置正确标志位
                        open_led=0;                                                                                //开锁指示灯亮
                        for(j=0;j<6;j++)                                                                //将输入清除
                        {
                                InputData[i]=0;
                        }
                        while(1);
        }
        else
        {
        GotoXY(0,2);
                Print(Error);        
                 ThreeAlam();                                                                                //错误提示音
        Delay400Ms();
        GotoXY(0,2);
                Print(start_line);
        for(j=0;j<6;j++)                                                                //将输入清除
                {
                        InputData[i]=0;
                 }
    }
        N=0;                                                                                                        //将输入数据计数器清零，为下一次输入作准备
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
void keyscan()                                        //按键扫描程序
{
        static bit key_sign=0;        //按键自锁标志
        static bit sign=0;
        static unsigned char count=0;//计数变量                        
        unsigned char i;
        if(K1==0)                                        //检测按键如果为0
        {
                count++;                                        //消抖计数
                if((count>=250)&&(key_sign==0))
                {                        
                        key_sign=1;                        //按键自锁标志置1
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
        else                                                        //按键抬起
        {
                key_sign=0;                                //按键自锁标志清0
                count=0;                                        //消抖计数清0
        }
}

//==============================主函数===============================
void main(void)
{
         
        unsigned char j;
//        test_led();
//        test_alarm();
         Delay400Ms();         //启动等待，等LCM讲入工作状态
         LCD_Initial();         //LCD初始化

//        while(1)
//        {
//                test_key();
//        }

        GotoXY(0,1);//日历显示固定符号从第一行第0个位置之后开始显示
        Print(name);//向液晶屏写日历显示的固定符号部分
        GotoXY(0,2);//时间显示固定符号写入位置，从第2个位置后开始显示
        Print(start_line);//写显示时间固定符号，两个冒号
        GotoXY(9,2);        //设置光标位置
        LCD_Write(LCD_COMMAND,0x0f);        //设置光标为闪烁

//        while(1);

         P3=0xFF;
        timer_init();



         Delay5Ms(); //延时片刻(可不要)
        N=0;                                                                                                                //初始化数据输入位数
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
                                                case '*': Cancel();                                break;      //取消当前输入
                                                case '#': Ensure();                         break;           //确认键，
                                                default:
                                                {        
                                                        if(N<6)                                                           //当输入的密码少于6位时，接受输入并保存，大于6位时则无效。
                                                        {
//                                                                  unsigned        char temp;
                                                                OneAlam();                                                                //按键提示音                                                
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
                                                        else                                                                                //输入数据位数大于6后，忽略输入
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

//*********************************中断服务函数**************************************
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
                                ErrorCont=0;                        //密码错误输入次数清零
        GotoXY(0,2);
                                Print(start_line);
        TR0=0;                                //关定时器
                }
                        
           }
}