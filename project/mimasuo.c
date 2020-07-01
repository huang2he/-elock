//包含头文件
#include <REG51.h>
#include<intrins.h>
//宏定义
#define LCM_Data  P0		  //将P0口定义为LCM_Data
#define uchar unsigned char 
#define uint  unsigned int


#define LCD_DATA            1         // 输出数据 GAIGAIGIAGIAGIAGIAGIAGIGAIA


//1602的控制脚
sbit lcd1602_rs=P2^5;
sbit lcd1602_rw=P2^6;
sbit lcd1602_en=P2^7;

sbit Scl=P3^4;			//24C02串行时钟
sbit Sda=P3^5;			//24C02串行数据

sbit ALAM = P2^1;		//报警	
sbit KEY = P3^6;		//开锁

bit  pass=0;			//密码正确标志
bit  ReInputEn=0;		//重置输入允许标志	
bit  s3_keydown=0;		//3秒按键标志位
bit  key_disable=0;		//锁定键盘标志

unsigned char countt0,second;	//t0中断计数器,秒计数器

void Delay5Ms(void);		//声明延时函数

unsigned char code a[]={0xFE,0xFD,0xFB,0xF7}; 				//控盘扫描控制表
//液晶显示数据数组
unsigned char code start_line[]	= {"password=       "};
unsigned char code name[] 	 	= {"  BY HZ AND LZP "};	//显示名称
unsigned char code Correct[] 	= {"     success    "};		//输入正确
unsigned char code Error[]   	= {"      fail      "};  	//输入错误
unsigned char code codepass[]	= {"      pass      "}; 
unsigned char code LockOpen[]	= {"      open      "};		//OPEN
unsigned char code SetNew[] 	= {"reset password  "};
unsigned char code Input[]   	= {"input:          "};		//INPUT
unsigned char code ResetOK[] 	= {"password changed"};
unsigned char code initword[]	= {"Init password..."};
unsigned char code Er_try[]		= {"error,try again!"};
unsigned char code again[]		= {"again           "};
unsigned char code admin[]      ={"admin mode"};

unsigned char InputData[6];									//输入密码暂存区
unsigned char CurrentPassword[6]={0,0,0,0,0,0}; 			//读取EEPROM密码暂存数组
unsigned char TempPassword[6];
unsigned char N=0;				//密码输入位数记数
unsigned char ErrorCont;			//错误次数计数
unsigned char CorrectCont;			//正确输入计数
unsigned char ReInputCont; 			//重新输入计数
unsigned char code initpassword[6]={0,0,0,0,0,0};			//输入管理员密码后将密码初始为000000
unsigned char code adminpassword[6]={8,8,8,8,8,8};			//输入管理员密码后将密码初始为000000


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

//=============================================================================================
//================================24C02========================================================
//=============================================================================================

void mDelay(uint t) //延时
{ 
	uchar i;
   	while(t--)
   	{
   		for(i=0;i<125;i++)
   		{;}
   	}
}
   

void Nop(void)		  //空操作
{
 	_nop_();		  //仅作延时用一条语句大约1us
 	_nop_();
 	_nop_();
 	_nop_();
}

/*****24c02程序参照24c02时序图*****/
/*起始条件*/

void Start(void)
{
 	Sda=1;
 	Scl=1;
 	Nop();
 	Sda=0;
 	Nop();
}


 /*停止条件*/
void Stop(void)
{
 	Sda=0;
 	Scl=1;
 	Nop();
 	Sda=1;
 	Nop();
}

/*应答位*/
void Ack(void)
{
	Sda=0;
	Nop();
	Scl=1;
	Nop();
	Scl=0;
}

/*反向应答位*/
void NoAck(void)
{
 	Sda=1;
 	Nop();
 	Scl=1;
 	Nop();
 	Scl=0;
}

 /*发送数据子程序，Data为要求发送的数据*/
void Send(uchar Data)
{
   	uchar BitCounter=8;
   	uchar temp;
   	do
   	{
   		temp=Data;					   //将待发送数据暂存temp
   		Scl=0;
   		Nop();
   		if((temp&0x80)==0x80)		   //将读到的数据&0x80
   		Sda=1;
   		else 
   		Sda=0;
   		Scl=1;
   		temp=Data<<1;				   //数据左移
   		Data=temp;					   //数据左移后重新赋值Data
   		BitCounter--;				   //该变量减到0时，数据也就传送完成了
   	}
   	while(BitCounter);				   //判断是否传送完成
   	Scl=0;
}

/*读一字节的数据，并返回该字节值*/
uchar Read(void)
{
    uchar temp=0;
	uchar temp1=0;
	uchar BitCounter=8;
	Sda=1;
	do
	{
		Scl=0;
		Nop();
		Scl=1;
		Nop();
		if(Sda)				   //数据位是否为1
			temp=temp|0x01;	   //为1 temp的最低位为1（|0x01，就是将最低位变为1）
		else				   //如果为0
			temp=temp&0xfe;	   //temp最低位为0（&0xfe（11111110）最低位就是0）
		if(BitCounter-1)	   //BitCounter减1后是否为真
		{
			temp1=temp<<1;	   //temp左移
			temp=temp1;
		}
		BitCounter--;		   //BitCounter减到0时，数据就接收完了
	}
	while(BitCounter);		   //判断是否接收完成
	return(temp);
}

void WrToROM(uchar Data[],uchar Address,uchar Num)
{
  uchar i;
  uchar *PData;
  PData=Data;
  for(i=0;i<Num;i++)
  {
  Start();
  Send(0xa0);
  Ack();
  Send(Address+i);
  Ack();
  Send(*(PData+i));
  Ack();
  Stop();
  mDelay(20);
  }
}

void RdFromROM(uchar Data[],uchar Address,uchar Num)
{
  uchar i;
  uchar *PData;
  PData=Data;
  for(i=0;i<Num;i++)
  {
  Start();
  Send(0xa0);
  Ack();
  Send(Address+i);
  Ack();
  Start();
  Send(0xa1);
  Ack();
  *(PData+i)=Read();
  Scl=0;
  NoAck();
  Stop();
  }
}


//==================================================================================================
//=======================================LCD1602====================================================
//==================================================================================================

#define yi 0x80 //LCD第一行的初始位置,因为LCD1602字符地址首位D7恒定为1（100000000=80）
#define er 0x80+0x40 //LCD第二行初始位置（因为第二行第一个字符位置地址是0x40）


//----------------延时函数，后面经常调用----------------------
void delay(uint xms)//延时函数，有参函数
{
	uint x,y;
	for(x=xms;x>0;x--)
	 for(y=110;y>0;y--);
}

//--------------------------写指令---------------------------
void write_1602com(uchar com)//****液晶写入指令函数****
{
	lcd1602_rs=0;//数据/指令选择置为指令
	lcd1602_rw=0; //读写选择置为写
	P0=com;//送入数据
	delay(1);
	lcd1602_en=1;//拉高使能端，为制造有效的下降沿做准备
	delay(1);
	lcd1602_en=0;//en由高变低，产生下降沿，液晶执行命令
}

//-------------------------写数据-----------------------------
void write_1602dat(uchar dat)//***液晶写入数据函数****
{
	lcd1602_rs=1;//数据/指令选择置为数据
	lcd1602_rw=0; //读写选择置为写
	P0=dat;//送入数据
	delay(1);
	lcd1602_en=1; //en置高电平，为制造下降沿做准备
	delay(1);
	lcd1602_en=0; //en由高变低，产生下降沿，液晶执行命令
}

//-------------------------初始化-------------------------
void lcd_init(void)
{
	write_1602com(0x38);//设置液晶工作模式，意思：16*2行显示，5*7点阵，8位数据
	write_1602com(0x0c);//开显示不显示光标
	write_1602com(0x06);//整屏不移动，光标自动右移
	write_1602com(0x01);//清显示
}
//========================================================================================
//=========================================================================================
//==============将按键值编码为数值=========================
unsigned char coding(unsigned char m)	 
{
	unsigned char k;
	switch(m)
	{
		case (0x11): k=1;break;
		case (0x21): k=2;break;
		case (0x41): k=3;break;
		case (0x81): k='A';break;
		case (0x12): k=4;break;
		case (0x22): k=5;break;
		case (0x42): k=6;break;
		case (0x82): k='B';break;
		case (0x14): k=7;break;
		case (0x24): k=8;break;
		case (0x44): k=9;break;
		case (0x84): k='C';break;
		case (0x18): k='*';break;
		case (0x28): k=0;break;
		case (0x48): k='#';break;
		case (0x88): k='D';break;
	}
	return(k);
}
//=====================按键检测并返回按键值===============================
unsigned char keynum(void)
{
 	unsigned char row,col,i;
 	P1=0xf0;
 	if((P1&0xf0)!=0xf0)
 	{
	   	Delay5Ms();
        Delay5Ms();
   		if((P1&0xf0)!=0xf0)
		{
    		row=P1^0xf0;          //确定行线
			i=0;
			P1=a[i];	          //精确定位
			while(i<4)
			{
	 			if((P1&0xf0)!=0xf0)
	  			{
	   				col=~(P1&0xff);	  //确定列线
	   				break;            //已定位后提前退出   
	  			}
				else 
	  			{
	   				i++;
	   				P1=a[i];
	  			}
			}
		}
		else 
		{
			return 0;
		}
		while((P1&0xf0)!=0xf0);
		return (row|col);	 		//行线与列线组合后返回
 	}
 	else return 0;	         		//无键按下时返回0
}
//=======================一声提示音，表示有效输入========================
void OneAlam(void)
{
	ALAM=0;
	Delay5Ms();
    ALAM=1;
}
//========================二声提示音，表示操作成功========================
void TwoAlam(void)
{
	ALAM=0;
	Delay5Ms();
    ALAM=1;
    Delay5Ms();
	ALAM=0;
	Delay5Ms();
    ALAM=1;
}
//========================三声提示音,表示错误========================
void ThreeAlam(void)
{
	ALAM=0;
	Delay5Ms();
    ALAM=1;
    Delay5Ms();
	ALAM=0;
	Delay5Ms();
    ALAM=1;
    Delay5Ms();
	ALAM=0;
	Delay5Ms();
    ALAM=1;
}
//=======================显示提示输入=========================
void DisplayChar(void)
{
	unsigned char i;
	if(pass==1)
	{
		//DisplayListChar(0,1,LockOpen);
		write_1602com(er);				   //在二行开始显示
		for(i=0;i<16;i++)
		{
			write_1602dat(LockOpen[i]);	   //显示open 开锁成功
		}
	}
	else
	{
		if(N==0)
		{
			//DisplayListChar(0,1,Error);
			write_1602com(er);
			for(i=0;i<16;i++)
			{
				write_1602dat(Error[i]);	//显示错误
			}
		}
		else
		{
			//DisplayListChar(0,1,start_line);	
			write_1602com(er);
			for(i=0;i<16;i++)
			{
				write_1602dat(start_line[i]);//显示开始输入	
			}
		}
	}
}

//========================重置密码==================================================
//==================================================================================
void ResetPassword(void)
{
	unsigned char i;	
	unsigned char j;
	if(pass==0)
	{
		pass=0;			   
		DisplayChar();	   //显示错误
		ThreeAlam();	   //没开锁时按下重置密码报警3声
	}
	else				   //开锁状态下才能进行密码重置程序
	{
    	if(ReInputEn==1)   //开锁状态下，ReInputEn置1，重置密码允许
		{
			if(N==6)	   //输入6位密码
			{
				ReInputCont++;			//密码次数计数	
				if(ReInputCont==2)		//输入两次密码
				{
					for(i=0;i<6;)
					{
						if(TempPassword[i]==InputData[i])	//将两次输入的新密码作对比
							i++;
						else								//如果两次的密码不同
						{
							//DisplayListChar(0,1,Error);
							write_1602com(er);
							for(j=0;j<16;j++)
							{
								write_1602dat(Error[j]);	//显示错误Error
							}
							ThreeAlam();			//错误提示	
							pass=0;					//关锁
							ReInputEn=0;			//关闭重置功能，
							ReInputCont=0;
							DisplayChar();
							break;
						}
					} 
					if(i==6)
					{
						//DisplayListChar(0,1,ResetOK);
						write_1602com(er);
						for(j=0;j<16;j++)
						{
							write_1602dat(ResetOK[j]);	  //密码修改成功，显示
						}

						TwoAlam();				//操作成功提示
					 	WrToROM(TempPassword,0,6);		//将新密码写入24C02存储
						ReInputEn=0;
					}
					ReInputCont=0;
					CorrectCont=0;
				}
				else					  //输入一次密码时
				{
					OneAlam();
					//DisplayListChar(0, 1, again); 		//显示再次输入一次
					write_1602com(er);
					for(j=0;j<16;j++)
					{
						write_1602dat(again[j]);			//显示再输入一次
					}					
					for(i=0;i<6;i++)
					{
						TempPassword[i]=InputData[i];		//将第一次输入的数据暂存起来						
					}
				}

			N=0;						//输入数据位数计数器清零
		   }
	    }
	}
}
//=======================输入密码错误超过三过，报警并锁死键盘======================
void Alam_KeyUnable(void)
{
	P1=0x00;
	{
		ALAM=~ALAM;				 //蜂鸣器一直闪烁鸣响
		Delay5Ms();
	}
}
//=======================取消所有操作============================================
void Cancel(void)
{	
	unsigned char i;
	unsigned char j;
	//DisplayListChar(0, 1, start_line); 
	write_1602com(er);
	for(j=0;j<16;j++)
	{
		write_1602dat(start_line[j]);	  //显示开机输入密码界面
	}
	TwoAlam();				//提示音
	for(i=0;i<6;i++)
	{
		InputData[i]=0;		//将输入密码清零
	}
	KEY=1;					//关闭锁
	ALAM=1;					//报警关
	pass=0;					//密码正确标志清零
	ReInputEn=0;			//重置输入充许标志清零
	ErrorCont=0;			//密码错误输入次数清零
	CorrectCont=0;			//密码正确输入次数清零
	ReInputCont=0;			//重置密码输入次数清零 
	s3_keydown=0;
	key_disable=0;			//锁定键盘标志清零
	N=0;					//输入位数计数器清零
}

//==========================确认键，并通过相应标志位执行相应功能===============================
void Ensure(void)
{	
	unsigned char i,j;
	RdFromROM(CurrentPassword,0,6); 					//从24C02里读出存储密码
    if(N==6)
	{
	    if(ReInputEn==0)							//重置密码功能未开启
		{
			for(i=0;i<6;)
   			{					
				if(CurrentPassword[i]==InputData[i])	//判断输入密码和24c02中的密码是否相同
				{
					i++;								//相同一位 i就+1
				}
				else 									//如果有密码不同
				{										
					ErrorCont++;						//错误次数++
					if(ErrorCont==3)			//错误输入计数达三次时，报警并锁定键盘
					{
						write_1602com(er);
						for(i=0;i<16;i++)
						{
							write_1602dat(Error[i]);	
						}
						do
						Alam_KeyUnable();
						while(1);
					}
					else					 //错误次数小于3次时，锁死键盘3秒，然后重新可以输入
					{
						TR0=1;				//开启定时
						key_disable=1;			//锁定键盘
						pass=0;					//pass位清零
						break;					//跳出
					}
				}
			}

			if(i==6)					 //密码输入对时
			{
				if((InputData[0]==adminpassword[0])&&(InputData[1]==adminpassword[1])&&(InputData[2]==adminpassword[2])&&(InputData[3]==adminpassword[3])&&(InputData[4]==adminpassword[4])&&(InputData[5]==adminpassword[5]))
		  	 	{
					WrToROM(initpassword,0,6); 				//强制将初始密码写入24C02存储
					write_1602com(er);
					for(j=0;j<16;j++)
					{
						write_1602dat(initword[j]);				//显示初始化密码
					}
					TwoAlam();									//成功提示音
					Delay400Ms();								//延时400ms
					TwoAlam();									//成功提示音
					N=0;										//输入位数计数器清零
				}
				else 
				{
					CorrectCont++;					//输入正确变量++
					if(CorrectCont==1)				//正确输入计数，当只有一次正确输入时，开锁
					{
						//DisplayListChar(0,1,LockOpen);
						write_1602com(er);
						for(j=0;j<16;j++)
						{
							write_1602dat(LockOpen[j]);		  //显示open开锁画面
						}
						TwoAlam();			//操作成功提示音
						KEY=0;											//开锁
						pass=1;											//置正确标志位
						TR0=1;											//开启定时
						for(j=0;j<6;j++)								//将输入清除
						{
							InputData[i]=0;								//开锁后将输入位清零
						}
					}	
					else												//当两次正确输入时，开启重置密码功能
					{
						write_1602com(er);
						for(j=0;j<16;j++)
						{
							write_1602dat(SetNew[j]);					//显示重置密码界面
						}
						TwoAlam();									    //操作成功提示
						ReInputEn=1;									//允许重置密码输入
						CorrectCont=0;									//正确计数器清零
					}
				}
	  		}
	
			else			//=========================当第一次使用或忘记密码时可以用131420对其密码初始化============
			{
				if((InputData[0]==adminpassword[0])&&(InputData[1]==adminpassword[1])&&(InputData[2]==adminpassword[2])&&(InputData[3]==adminpassword[3])&&(InputData[4]==adminpassword[4])&&(InputData[5]==adminpassword[5]))
		  	 	{
					WrToROM(initpassword,0,6); 				//强制将初始密码写入24C02存储
					write_1602com(er);
					for(j=0;j<16;j++)
					{
						write_1602dat(initword[j]);				//显示初始化密码
					}
					TwoAlam();									//成功提示音
					Delay400Ms();								//延时400ms
					TwoAlam();									//成功提示音
					N=0;										//输入位数计数器清零
				}
				else											//密码输入错误
				{
					write_1602com(er);
					for(j=0;j<16;j++)
					{
						write_1602dat(Error[j]);				 //显示错误信息
					}
 					ThreeAlam();										//错误提示音
					pass=0;	
				}
			}
		}

		else											//当已经开启重置密码功能时，而按下开锁键，
		{
			//DisplayListChar(0,1,Er_try);
			write_1602com(er);
			for(j=0;j<16;j++)
			{
				write_1602dat(Er_try[j]);			  //错误，请重新输入
			}
			ThreeAlam();							  //错误提示音
		}
	}

	else				   //密码没有输入到6位时，按下确认键时
	{
		//DisplayListChar(0,1,Error);
		write_1602com(er);
		for(j=0;j<16;j++)
		{
			write_1602dat(Error[j]);		 //显示错误
		}

 		ThreeAlam();										//错误提示音
		pass=0;	
	}
	
	N=0;													//将输入数据计数器清零，为下一次输入作准备
}

 
 void keyscan()                                        //按键扫描程序
{
        static bit key_sign=0;        //按键自锁标志
        static bit sign=0;
        static unsigned char count=0;//计数变量                        
        unsigned char i,NUM;
        if(pass==0)                                        //检测按键如果为0
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
                                        write_1602com(er);
                                        write_1602dat(NUM+0x30);
                                }
                        }
                        else
                        {
                                for(i=0;i<N;i++)
                                {
                                        write_1602com(er);
                                        write_1602dat('*');
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
 	unsigned char KEY,NUM;
	unsigned char i,j;
 	P1=0xFF; 				   //P1口复位
	TMOD=0x11;				   //定义工作方式
 	TL0=0xB0;
 	TH0=0x3C;				   //定时器赋初值
 	EA=1;					   //打开中断总开关
 	ET0=1;					   //打开中断允许开关
 	TR0=0;					   //打开定时器开关
 	Delay400Ms(); 	//启动等待，等LCM讲入工作状态
 	lcd_init(); 	//LCD初始化
	write_1602com(yi);//日历显示固定符号从第一行第0个位置之后开始显示
	for(i=0;i<16;i++)
	{
		write_1602dat(name[i]);//向液晶屏写开机画面
	}
	write_1602com(er);
	for(i=0;i<16;i++)
	{
		write_1602dat(start_line[i]);//写输入密码等待界面
	}
	write_1602com(er+9);	//设置光标位置
	write_1602com(0x0f);	//设置光标为闪烁
 	Delay5Ms(); //延时片刻(可不要)

 	N=0;														//初始化数据输入位数
 	while(1)		 //进入循环
 	{
		if(key_disable==1)						//锁定键盘标志为1时
			Alam_KeyUnable();					//报警键盘锁
		else
			ALAM=1;								//关报警

		KEY=keynum();							//读按键的位置码
		if(KEY!=0)								//当有按键按下时
		{	
			if(key_disable==1)					//锁定键盘标志为1时
			{
				second=0;						//秒清零
			}
			else								//没有锁定键盘时
			{
				NUM=coding(KEY);				//根据按键的位置将其编码，编码值赋值给NUM
				{
					switch(NUM)					//判断按键值
					{
						case ('A'): 	if(N>=1) N--;	OneAlam();								//按键提示音						
									//DisplayOneChar(6+N,1,'*');
									for(j=N;j<16;j++)
									{
										write_1602com(er+6+j);
										write_1602dat(' ');
									}	; 					break;
						case ('B'):	write_1602com(er);
		                            for(j=0;j<16;j++)
		                           {
			                        write_1602dat(admin[j]);		 //显示错误
	                            	}	;     				break;
						case ('C'):
									
								
									for(j=0;j<N;j++)
									{
										write_1602com(er+6+j);				//显示位数随输入增加而增加
										write_1602dat(InputData[j]+0x30);					//但不显示实际数字，用*代替
									}  
									InputData[N]=N+4;
									break;		//
						case ('D'): ResetPassword();		break;      //重新设置密码
						case ('*'): Cancel();				break;      //取消当前输入
						case ('#'): Ensure(); 				break;   	//确认键，
						default: 										//如果不是功能键按下时，就是数字键按下
						{	
							//DisplayListChar(0,1,Input);
							if(N<6)                   					//当输入的密码少于6位时，接受输入并保存，大于6位时则无效。
							{ 
								write_1602com(er);
								for(i=0;i<16;i++)
								{
									write_1602dat(Input[i]);				//显示输入画面
								} 
								OneAlam();								//按键提示音	
						 		for(j=0;j<=N;j++)
								{
									write_1602com(er+6+j);				//显示位数随输入增加而增加
									write_1602dat('*');					//但不显示实际数字，用*代替
								}
								InputData[N]=NUM;						//将数字键的码赋值给InputData[]数组暂存
								N++;									//密码位数加
							}
							else										//输入数据位数大于6后，忽略输入
							{
								N=6;									//密码输入大于6位时，不接受输入
						 		break;
							}
						}
					}
				}
			}
	 	} 
	}
}

//*********************************中断服务函数**************************************
void  time0_int(void) interrupt 1 		   //定时器T0
{
 	TL0=0xB0;
 	TH0=0x3C;		  //定时器重新赋初值
 	//TR0=1;
 	countt0++;			   //计时变量加，加1次时50ms
  	if(countt0==20)		   //加到20次就是1s
   	{
		countt0=0;		   //变量清零
		second++;		   //秒加
		if(pass==1)		   //开锁状态时
		{
			if(second==1)			//秒加到1s时
			{
				TR0=0;				//关定时器
				TL0=0xB0;
 				TH0=0x3C;			//再次赋初值
				second=0;			//秒清零
			}
		}
		else						//不在开锁状态时
		{
			if(second==3)			//秒加到3时
			{
				TR0=0;				//关闭定时器
				second=0;			//秒清零
				key_disable=0;		//锁定键盘清零
				s3_keydown=0;	
				TL0=0xB0;
				TH0=0x3C;			//重新赋初值
			}
			else
			    TR0=1;				//打开定时器
		}
			
   	}
}

