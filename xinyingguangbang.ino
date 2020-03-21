#include <EEPROM.h>
#include <Arduino.h>
//----------------------------------------------------//
//存储地址：1-20作为状态量
//(1-20待定，因为可能不需要状态量)
//21-740存储颜色，741-764存放频率，871-883存放每组最大数（883是自由组合模式下），
//771-795,796-820,821-845,846-870存放特殊设计程式(每8bit存放4个数值)，
//891-938存放显示颜色，945-1000存放注释，941到944存放特殊设计程式长度
//----------------------------------------------------//
#define LED_R 6  //红灯
#define LED_G 5 //绿灯
#define LED_B 11  //蓝灯
#define LED_W 3  //白灯
#define LED_S 2//LED颜色指示灯负极
#define bluetoothin 8  //蓝牙开关
//#define battery 4
#define vScanFunction 7
//--------------------//
bool ledOpen = 0;//led是否开启
bool straightTurnOff = 0;//一下关机
byte mode1 = 0;//发光模式量
byte groupMax;//颜色最大数
byte colorDirection = 0;//组内颜色序号
byte r[16];
byte g[16];
byte b[16];
byte w[16];
//===========================================================//
void setup()
{
	timer_set();
	//OCR0A,OCR0B,OCR2A,OCR2B分别是红绿蓝白
	Serial.begin(9600);//设置波特率
	pinMode(LED_R, OUTPUT); //设置输出
	pinMode(LED_G, OUTPUT); //同上
	pinMode(LED_B, OUTPUT); //同上
	pinMode(LED_W, OUTPUT);
	pinMode(LED_S, OUTPUT);
	pinMode(bluetoothin, INPUT);
	pinMode(vScanFunction, OUTPUT);
	//pinMode( battery,INPUT);
	//analogReference(DEFAULT); //设置PWM基准
	digitalWrite(LED_S, HIGH);
	Show(255, 0, 0, 0);
	wait(500);
	Show(0, 255, 0, 0);
	wait(500);
	Show(0, 0, 255, 0);
	wait(500);
	Show(0, 0, 0, 0);
}
//=======================================================//
void timer_set()//定时器初始化
{
	TCCR0A = 0XA3; TCCR0B = 0X02;
	TCCR2A = 0XA3; TCCR2B = 0X02;
	TCCR1A = 0X00; TCCR1B = 0X05; TCCR1C = 0X00; TIMSK1 = 0X00;
	OCR0A = 0; OCR0B = 0; OCR2A = 0; OCR2B = 0;
}
//=========================================================//
int ScanFunction(bool a1, bool a2, bool a3,
	bool mode11, bool mode12, bool mode13,
	unsigned long time1, unsigned long time2, unsigned long time3)
{
	while (1)
	{
		TCNT1 = 0X00;
		while (analogRead(A0) > 600 && analogRead(A1) > 600 && analogRead(A2) > 600
			&& analogRead(A3) > 600)
		{
			if (TCNT1 / 16 >= time1 || TCNT1 / 16 >= time2 || TCNT1 / 16 >= time3);
		}
		unsigned long switchdowntime = TCNT1 / 16;
		if (analogRead(A0) < 200 && a1 == 1)
		{
			while (analogRead(A0) < 200);
			if (mode11 == 0)
				return 1;
			else if (mode11 == 1 && switchdowntime < time1)
				return 1;
			else 
				return 11;
		}
		else if (analogRead(A1) < 200 && a2 == 1) 
		{
			while (analogRead(A1) < 200);
			if (mode12 == 0)
				return 2;
			else if (mode12 == 1 && switchdowntime < time2)
				return 2;
			else 
				return 12;
		}
		else if (analogRead(A2) < 200 && a3 == 1) 
		{
			while (analogRead(A2) < 200);
			if (mode13 == 0)
				return 3;
			else if (mode13 == 1 && switchdowntime < time3)
				return 3;
			else
				return 13;
		}
	}
}
//============================================================//
int ScanFunctionBotton()
{
	if (analogRead(A0) > 600 && analogRead(A1) > 600/*&&analogRead(A2)>600
	&&analogRead(A3)>600*/ && analogRead(A4) > 600 && analogRead(A5) > 600)
		return 0;
	return 1;
}
//===========================================================//
int voltageenough()
{
	digitalWrite(vScanFunction, HIGH);
	int v = analogRead(A6);
	if (v >= 800)
		return 1;
	return 0;
}
//===========================================================//
void wait(unsigned long times)
{
	TCNT1 = 0X00;
	while (TCNT1 < times*15.625);
}
//=============================================================//
void eepromwrite(long pin, int num)
{
	if (EEPROM.read(pin) != num)
		EEPROM.write(pin, num);
}
//===========================================================//
void Show(int rpin, int gpin, int bpin, int wpin)
{
	analogWrite(6, rpin);
	analogWrite(5, gpin);
	analogWrite(11, bpin);
	analogWrite(3, wpin);
}
//=======================================================//
//发光模块
void LightMode(int r1, int g1, int b1, int w1, int f1, int f2, int modenumber)//f1，f2单位是次/百毫秒，值域0至255
{
	if (modenumber == 0)
	{
		Show(r1, g1, b1, w1);
		while (ScanFunctionBotton() == 0);
	}
	else if (modenumber == 4)
	{
		Show(r1, g1, b1, w1);
		while (ScanFunctionBotton() == 0 && TCNT1 < f1 * 1563);
	}
	else if (modenumber == 1)
	{
		Show(r1, g1, b1, w1);
		TCNT1 = 0X00;
		while (ScanFunctionBotton() == 0 && TCNT1 < f1 * 1563);
		Show(0, 0, 0, 0);
		TCNT1 = 0X00;
		while (ScanFunctionBotton() == 0 && TCNT1 < f1 * 1563);
	}
	else if (modenumber == 2)
	{
		int level = 0;
		int timelevel = (f2 * 500 / 512)*15.625;
		while (level != 255 && ScanFunctionBotton() == 0)
		{
			Show((r1*level) / 255, (g1*level) / 255, (b1*level) / 255, (w1*level) / 255);
			++level;
			TCNT1 = 0X00;
			while (ScanFunctionBotton() == 0 && TCNT1 < timelevel);
		}
		while (level != 0 && ScanFunctionBotton() == 0)
		{
			Show((r1*level) / 255, (g1*level) / 255, (b1*level) / 255, (w1*level) / 255);
			--level;
			TCNT1 = 0X00;
			while (ScanFunctionBotton() == 0 && TCNT1 < timelevel);
		}
	}
}
//=======================================================//
//=======================================================//
void loop()
{
	int m1 = ScanFunction(1, 1, 1, 0, 0, 0, NULL, NULL, NULL);
	if (digitalRead(bluetoothin) == 0 && ledOpen == 0 && m1 == 2)//蓝牙
		bluetoothfunction();
	else if ((m1 == 1 || m1 == 3) && ledOpen == 0)
	{
		remember();
		led_on(m1);
		led_close();
	}
}
void remember()
{
	eepromwrite(1, colorDirection);//关机时颜色
	eepromwrite(2, mode1);//关机时模式
	eepromwrite(7, 1);//有记录
}
//--------------------////--------------------////--------------------//
//--------------------//         LED相关        //--------------------//
//--------------------////--------------------////--------------------//
void prepare()//存储待显示颜色
{
	int t = 0;
	for (t = 0; t != 15; ++t)
	{
		r[t] = EEPROM.read(891 + 4 * t);
		g[t] = EEPROM.read(891 + 4 * t + 1);
		b[t] = EEPROM.read(891 + 4 * t + 2);
		w[t] = EEPROM.read(891 + 4 * t + 3);
	}
}
void led_on(int m0)
{
	//if(voltageenough()==1)
   //{
	prepare();
	ledOpen = 1;
	//colorDirection=EEPROM.read(15);
	if (m0 == 3 && EEPROM.read(7) == 1)
	{
		colorDirection = EEPROM.read(1);
		mode1 = EEPROM.read(2);
	}
	else
	{
		colorDirection = 0;
		mode1 = 0;
	}
	groupMax = EEPROM.read(870 + EEPROM.read(3));
	eepromwrite(7, 0);
	ConstantMode(r[colorDirection], g[colorDirection], b[colorDirection], w[colorDirection]);
	led_adjust();
	//}
	/*else
	{
	  OCR0A=255;
	  int m7=ScanFunction(0,0,0,0,1,0);
	  led_close();
	}*/
}
//--------------------//
void led_adjust()
{
	bool flagadj = true;
	do {
		switch (ScanFunction(1, 1, 1, 0, 1, 0, NULL, 1500, NULL))
		{
		case 1:
			led_move(0); led_mode(); 
			break;
		case 3:
			led_move(1); led_mode(); 
			break;
		case 2:
			if (mode1 == 2)
				mode1 = 0;
			else
				++mode1;
			led_mode();
			break;
		case 11:
			led_modechoose(); 
			break;
		case 12:
			return; 
			break;
		default:
			break;
		}
		if (straightTurnOff)
			return;
	} while (flagadj);
}
//--------------------//
void led_modechoose()
{
	bool flagmode = true;
	Show(255, 0, 0, 0);
	wait(500);
	do {
		switch (ScanFunction(1, 1, 1, 1, 0, 1, 1500, NULL, 1500))
		{
		case 1:
			led_jump(); 
			break;
		case 3:
			led_SpecialLightMode();
			break;
		case 11:
			break;
		case 13:
			break;
		case 2:
			return;
		case 22:
			straightTurnOff = 1; 
			return;
		default:
			return;
		}
		if (straightTurnOff)
			return;
	} while (flagmode);
}
//--------------------//
void led_mode()
{
	int f1 = EEPROM.read(741 + colorDirection * 2);
	int f2 = EEPROM.read(741 + colorDirection * 2 + 1);
	int r1 = r[colorDirection];
	int g1 = g[colorDirection];
	int b1 = b[colorDirection];
	int w1 = w[colorDirection];
	switch (mode1)
	{
	case 0:
		ConstantMode(r1, g1, b1, w1); 
		break;
	case 1:
		FlashMode(r1, g1, b1, w1, f1); 
		break;
	case 2:
		BreathMode(r1, g1, b1, w1, f2); 
		break;
	default:
		return;
	}
	return;
}
//--------------------//
void led_move(bool towards)
{
	if (towards == 0)
	{
		if (colorDirection < groupMax - 1)
			++colorDirection;
		else if (colorDirection == groupMax - 1)
			colorDirection = 0;
	}
	else if (towards == 1);
	{
		if (colorDirection == 0)
			colorDirection = groupMax - 1;
		else if (colorDirection > 0)
			colorDirection--;
	}
	return;
}
//--------------------//
void led_jump()
{
	Serial.println("jump;");
	digitalWrite(LED_S, LOW);
	int m3 = ScanFunction(1, 1, 1, 0, 1, 0, NULL, 1500, NULL);
	while (m3 != 2)
	{
		m3 = ScanFunction(1, 1, 1, 0, 0, 0, NULL, NULL, NULL);
		if (m3 == 1)
			led_move(0);
		else if (m3 == 3)
			led_move(1);
	}
	if (m3 == 2)
		led_mode();
	else if (m3 == 12)
	{

	}
	return;
}
//--------------------//
void led_SpecialLightMode()
{
	bool flagspe = true;
	do {
		switch (ScanFunction(1, 1, 1, 1, 0, 1, 1000, NULL, 1000))
		{
		case 1:
			SpecialLightMode(0); 
			break;
		case 11:
			SpecialLightMode(1); 
			break;
		case 3:
			SpecialLightMode(2);
			break;
		case 13:
			SpecialLightMode(3); 
			break;
		case 2:
			return; 
			break;
		default:
			break;
		}
		if (straightTurnOff)
			return;
	} while (flagspe);
}
//--------------------//
void led_close()
{
	ledOpen = 0;
	straightTurnOff = 0;
	Show(0, 0, 0, 0);
}
//--------------------////--------------------//
//发光模式
//--------------------////--------------------//
void ConstantMode(int r1, int g1, int b1, int w1)
{
	Serial.print(";colorname="); 
	Serial.print(colorDirection); 
	Serial.println(";");
	if (EEPROM.read(3) != 0)
		LightMode(r1, g1, b1, w1, NULL, NULL, 0);
	return;
}
//---------------------//
void FlashMode(int r1, int g1, int b1, int w1, int f1)
{
	if (EEPROM.read(3) != 0)
		while (ScanFunctionBotton() == 0)
			LightMode(r1, g1, b1, w1, f1, NULL, 1);
	return;
}
//---------------------//
void BreathMode(int r1, int g1, int b1, int w1, int f2)
{
	if (EEPROM.read(3) != 0)
		while (ScanFunctionBotton() == 0)
			LightMode(r1, g1, b1, w1, NULL, f2, 2);
	return;
}
//---------------------//
void SpecialLightMode(int m)
{
	int* function = new int[100];
	byte length = EEPROM.read(941 + m);
	//-----//
	for (int t = 0; t != (length - 1) / 4 + 1; ++t)
	{
		int i = EEPROM.read(771 + t + 25 * m);
		int a = i / 64;
		int b = i / 16 - (i / 64 * 4);
		int c = i / 4 - (i / 16 * 4);
		int d = i - (i / 4 * 4);
		function[4 * t] = a;
		function[4 * t + 1] = b;
		function[4 * t + 2] = c;
		function[4 * t + 3] = d;
	}
	//-----//
	if (EEPROM.read(3) != 0)
	{
		int f3 = EEPROM.read(741 + colorDirection * 2);
		int f4 = EEPROM.read(741 + colorDirection * 2 + 1);
		int r3 = r[colorDirection];
		int g3 = g[colorDirection];
		int b3 = b[colorDirection];
		int w3 = w[colorDirection];
		while (ScanFunctionBotton() == 0)
		{
			for (int i = 0; i != length && ScanFunctionBotton() == 0; ++i)
			{
				if (function[i] == 0)
					LightMode(0, 0, 0, 0, f3, NULL, 4);
				else if (function[i] == 1)
					LightMode(r3, g3, b3, w3, f3, NULL, 4);
				else if (function[i] == 2)
					LightMode(r3, g3, b3, w3, f3, f4, 2);
			}
		}
	}
	delete[]function;
	return;
}
//---------------------////---------------------//
//蓝牙人机交互系统代码
//---------------------////---------------------//
//============================================================//
//调用模块列表
//============================================================//
byte* cutstr(String inputstr, char divide)
{
	int k = 1;
	int figurenumber = inputstr.length();
	for (int i = 0; i != figurenumber; ++i)
		if (inputstr[i] == ',')
			++k;
	byte *divided = new byte[k + 1];
	for (int i = 0; i != k + 1; ++i)
		divided[i] = 0;
	String midway = "";
	k = 1;
	for (int i = 0; i != figurenumber; ++i)
	{
		if (inputstr[i] != divide && i != figurenumber - 1)
			midway += inputstr[i];
		else
		{
			midway += inputstr[i];
			divided[k] = midway.toInt();
			++k;
			midway = "";
		}
	}
	divided[0] = k;
	return divided;
}
//============================================================//
bool checkfigure(String checkstring, int mode)//检测字符是否为数字
{
	int figurenumber1 = checkstring.length();
	if (mode == 1)
	{
		for (int i = 0; i != figurenumber1; ++i)
		{
			char figure1 = checkstring[i];
			if (isdigit(figure1) == 1 || figure1 == ',')
				continue;
			else
				return  false;
		}
		return true;
	}
	else if (mode == 0)
	{
		for (int i = 0; i != figurenumber1; ++i)
		{
			char figure1 = checkstring[i];
			if (isdigit(figure1) == 1)
				continue;
			else
				return  false;
		}
		return true;
	}
}
//============================================================//
bool checknumber(byte receivednumber[], byte minimum1, byte maximum1, byte minimum2,
	byte maximum2, byte minimum3, byte maximum3, byte mode, byte arrlength)//检测数字是否在范围内
{
	for (int k1 = 1; k1 != arrlength; ++k1)
	{
		if (mode == 1)
		{
			if (receivednumber[k1] <= maximum1 && receivednumber[k1] >= minimum1)
				continue;
			else
				return false;
		}
		else if (mode == 2)
		{
			if ((receivednumber[k1] <= maximum1 && receivednumber[k1] >= minimum1 && k1%mode == 0) ||
				(receivednumber[k1] <= maximum2 && receivednumber[k1] >= minimum2 && k1%mode == 1))
				continue;
			else
				return false;
		}
		else if (mode == 3)
		{
			if ((receivednumber[k1] <= maximum1 && receivednumber[k1] >= minimum1 && k1%mode == 0) ||
				(receivednumber[k1] <= maximum2 && receivednumber[k1] >= minimum2 && k1%mode == 1) ||
				(receivednumber[k1] <= maximum3 && receivednumber[k1] >= minimum3 && k1%mode == 2))
				continue;
			else
				return false;
		}
	}
	return true;
}
//============================================================//
byte SpecialLightModetoint(String input)
{
	byte k = 1, figure = input.length();
	byte k_1 = pow(10, figure - 1);
	for (byte i = 0; i != figure; ++i, k /= 10)
		k += (input[i] - 48) * k_1;
	return k;
}
//============================================================//
void bluetoothfunction()
{
	Serial.println("Press a key you like");
	String examine = "";
	while (Serial.available() == 0);
	while (Serial.available() != 0)
	{ 
		delay(2); 
		char x = Serial.read(); 
		examine += x; 
	}
	Serial.println("start");
	choosefunction();
}
//============================================================//
void choosefunction()
{
	String receivedstring;
	while (!receivedstring.equals("out"))
	{
		Serial.println("Please input the item you want");
		receivedstring = "";
		while (Serial.available() == 0);
		while (Serial.available() != 0)
		{ 
			delay(2); 
			char x = Serial.read(); 
			receivedstring += x; 
		}
		Serial.println(receivedstring);
		delay(5);
		if (receivedstring.equals("changecolorgroup"))
		{//一期测试完成
			Serial.println("function_1"); function_1();
		}
		else if (receivedstring.equals("freelycombinecolor"))
		{//一期测试完成
			Serial.println("function_2"); function_2();
		}
		else if (receivedstring.equals("changeinsidecolor"))
		{//一期测试完成
			Serial.println("function_3"); function_3();
		}
		else if (receivedstring.equals("changefrequency")) 
		{//一期测试完成
			Serial.println("function_4"); function_4();
		}
		else if (receivedstring.equals("SpecialLightMode"))
		{
			Serial.println("function_5"); function_5();
		}
		else if (receivedstring.equals("out"));
		else
			Serial.println("There is not such a function");
	}
	Serial.println("bluetooth end");
}
//============================================================//
void function_1()
{
	Serial.println("The mode is to change the color group");
	Serial.println("Please input the correct data");
	String receivedstring1 = "";
	while (Serial.available() == 0);
	while (Serial.available() != 0) 
	{ 
		delay(2); 
		char x1 = Serial.read(); 
		receivedstring1 += x1; 
	}
	if (checkfigure(receivedstring1, 0))
	{
		byte *receivednumber1 = cutstr(receivedstring1, ',');
		byte arraylength1 = receivednumber1[0];
		if (checknumber(receivednumber1, 1, 12, NULL, NULL, NULL, NULL, 1, arraylength1 + 1))
		{
			byte group1 = receivednumber1[1] - 1;
			for (byte func_11 = 0; func_11 != 15; ++func_11)
				for (byte func_12 = 0; func_12 != 4; ++func_12)
					eepromwrite(891 + 4 * func_11 + func_12, EEPROM.read(21 + group1 * 60 + 4 * func_11 + func_12));
			eepromwrite(3, 871 + group1);
			Serial.println("OK");
		}
		else
			Serial.println("The data are beyond the limit");
		delete[]receivednumber1;
	}
	else
		Serial.println("The data contain alpha(s)");
}
//============================================================//
void function_2()
{
	Serial.println("The mode is to combine a new group what you want");
	Serial.println("Please input the correct data");
	String receivedstring2 = "";
	while (Serial.available() == 0);
	while (Serial.available() != 0) 
	{ 
		delay(2); 
		char x2 = Serial.read(); 
		receivedstring2 += x2; 
	}
	if (checkfigure(receivedstring2, 1))
	{
		byte *receivednumber2 = cutstr(receivedstring2, ',');
		byte arraylength2 = receivednumber2[0];
		if (checknumber(receivednumber2, 1, 12, 1, 15, NULL, NULL, 2, arraylength2 + 1) && arraylength2 % 2 == 0)
		{
			byte func_21 = arraylength2 / 2;
			for (byte i = 0; i != func_21; ++i)
				for (int func_22 = 0; func_22 != 4; ++func_22)
					eepromwrite(891 + 4 * i + func_22,
						EEPROM.read(21 + 60 * (receivednumber2[i * 2 + 1] - 1) + 4 * (receivednumber2[i * 2 + 2] - 1) + func_22));
			eepromwrite(886, func_21);
			eepromwrite(3, 13);
			Serial.println("OK");
		}
		else
		{
			if (arraylength2 % 2 != 0)
				Serial.println("The number of the data is not enough or too many");
			else
				Serial.println("The data are beyond the limit");;
		}
		delete[]receivednumber2;
	}
	else
		Serial.println("The data contain alpha(s)");
}
//============================================================//
void function_3()
{
	Serial.println("The mode is to change the colors contained");
	Serial.println("Please input your data of the address");
	String receivedstring31 = "";
	while (Serial.available() == 0)
	while (Serial.available() != 0) 
	{ 
		delay(2); 
		char x31 = Serial.read(); 
		receivedstring31 += x31; 
	}
	if (checkfigure(receivedstring31, 1))
	{
		byte *receivednumber31 = cutstr(receivedstring31, ',');
		byte arraylength31 = receivednumber31[0];
		if (checknumber(receivednumber31, 1, 12, 1, 15, NULL, NULL, 2, arraylength31 + 1)
			&& arraylength31 % 2 == 0)//输入完需要改变的颜色的地址
		{
			Serial.println("Please input your data of the color values");
			receivedstring31 = "";
			while (Serial.available() == 0);
			while (Serial.available() != 0) 
			{ 
				delay(2); 
				char x32 = Serial.read(); 
				receivedstring31 += x32; 
			}
			if (checkfigure(receivedstring31, 1))
			{
				byte *receivednumber32 = cutstr(receivedstring31, ',');
				byte arraylength32 = receivednumber32[0];
				if (checknumber(receivednumber32, 0, 255, NULL, NULL, NULL, NULL, 1, arraylength32 + 1)
					&& arraylength32 % 4 == 0 && arraylength32 == arraylength31 * 2)
				{
					for (byte func_32 = 0; func_32 != arraylength31 / 2; ++func_32)
						for (byte func_33 = 0; func_33 != 4; ++func_33)
							eepromwrite(21 + (receivednumber31[func_32 * 2 + 1] - 1) * 60 + (receivednumber31[func_32 * 2 + 2] - 1) * 4 + func_33,
								receivednumber32[func_32 * 4 + func_33 + 1]);
					Serial.println("OK");
				}
				else
				{
					if (arraylength32 % 4 != 0)
						Serial.println("The number of the color values is not enough or too many");
					else if (arraylength32 != arraylength31 * 2)
						Serial.println("The number of address is not equal to color values");
					else
						Serial.println("The data of color values are beyond the limit");
				}
				delete[]receivednumber32;
			}
			else
				Serial.println("The data of color values contain alpha(s)");
		}
		else
		{
			if (arraylength31 % 2 != 0)
				Serial.println("The number of the address is not enough or too many");
			else
				Serial.println("The data of address are beyond the limit");
		}
		delete[]receivednumber31;
	}
	else
		Serial.println("The data of address contain alpha(s)");
}
//============================================================//
void function_4()
{
	Serial.println("The mode is to change the frequencys");
	Serial.println("Please input your data of the address");
	String receivedstring41 = "";
	while (Serial.available() == 0);
	while (Serial.available() != 0) 
	{ 
		delay(2); 
		char x41 = Serial.read(); 
		receivedstring41 += x41; 
	}
	if (checkfigure(receivedstring41, 1))
	{
		byte *receivednumber41 = cutstr(receivedstring41, ',');
		byte arraylength41 = receivednumber41[0];
		if (checknumber(receivednumber41, 1, 15, NULL, NULL, NULL, NULL, 1, arraylength41 + 1))
			//输入完需要改变的颜色的地址
		{
			Serial.println("Please input your data of the color values");
			receivedstring41 = "";
			while (Serial.available() == 0);
			while (Serial.available() != 0) 
			{ 
				delay(2); 
				char x42 = Serial.read(); 
				receivedstring41 += x42; 
			}
			if (checkfigure(receivedstring41, 1))
			{
				byte *receivednumber42 = cutstr(receivedstring41, ',');
				byte arraylength42 = receivednumber42[0];
				if (checknumber(receivednumber42, 0, 255, NULL, NULL, NULL, NULL, 1, arraylength42 + 1)
					&& arraylength42 % 2 == 0 && arraylength42 == arraylength41 * 2)
				{
					for (byte func_42 = 0; func_42 != arraylength41; ++func_42)
						for (byte func_43 = 0; func_43 != 2; ++func_43)
							eepromwrite(741 + (receivednumber41[func_42 * 2 + 1] - 1) * 2 + func_43,
								receivednumber42[func_42 * 2 + func_43 + 1]);
					Serial.println("OK");
				}
				else
				{
					if (arraylength42 % 2 != 0)
						Serial.println("The number of the color values is not enough or too many");
					else if (arraylength42 != arraylength41 * 2)
						Serial.println("The number of address is not equal to color values");
					else
						Serial.println("The data of color values are beyond the limit");
				}
				delete[]receivednumber42;
			}
			else
				Serial.println("The data of color values contain alpha(s)");
		}
		else
			Serial.println("The data of address are beyond the limit");
		delete[]receivednumber41;
	}
	else
		Serial.println("The data of address contain alpha(s)");
}
//============================================================//
void function_5()
{
	Serial.println("The mode is to input the SpecialLightMode design");
	Serial.println("Please input the address(1 to 4)");
	String receivedstring51 = "";
	while (Serial.available() == 0);
	while (Serial.available() != 0) 
	{ 
		delay(2); 
		char x51 = Serial.read(); 
		receivedstring51 += x51; 
	}
	if (receivedstring51[0] < '5'&&receivedstring51[0] > '0')
	{
		byte address_5 = receivedstring51[0] - '0';
		receivedstring51 = "";
		Serial.println("Please input your data");
		while (Serial.available() == 0) {}
		while (Serial.available() != 0) { delay(2); char x51 = Serial.read(); receivedstring51 += x51; }
		int length51 = receivedstring51.length();
		if (checkfigure(receivedstring51, 0))
		{
			bool flag51 = false;
			for (int func_51 = 0; func_51 != length51; ++func_51)
				if (receivedstring51[func_51] - '0' > 2)
					flag51 = true;
			byte func_52 = 0, times_5 = 64;
			if (flag51)
				Serial.println("The data are beyond the limit");
			else
			{
				for (int func_51 = 0; func_51 != length51 && func_51 != 100; ++func_51)
				{
					byte unit = 0;
					if (func_52 != 4)
					{
						unit += (receivedstring51[func_51] - '0')*times_5;
						++func_52;
						times_5 /= 4;
					}
					else
					{
						eepromwrite(771 + 25 * (address_5 - 1) + func_51 / 4, unit);
						func_52 = unit = 0;
						times_5 = 64;
					}
				}
				eepromwrite(940 + address_5, length51);
				Serial.println("OK");
			}
		}
		else
		{
			if (length51 > 100)
				Serial.println("The length of data is beyond limit");
			else
				Serial.println("The data contain alpha(s)");
		}
	}
}