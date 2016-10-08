*****************************************************
This program was produced by the
CodeWizardAVR V1.25.9 Professional
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : PWM Headlamd 
Version : 1.2
Date    : 06.03.2011
Author  : Mitya                           
Company : NSS                             
Comments: 
Условие розжига: Масло = 1 Ручник = 1 Габариты = 0
Каждое нажатие кнопки увеличивает значение розжига на 20% 
По достижении 100% скидывает на минимум: 20%
//Значение запоминается в EEPROM. Не факт                       !!!!!!!!!!!!!!!!!
Время розжига ~ 1 сек.
Подключение:
5 PB0 Выход ШИМ или 0
6 PB1 Выход 1 когда есть ШИМ, 0 когда нет.
7 PB2 Вход 1 блокировка ШИМ
2 PB3 Вход 0 блокировка ШИМ
3 PB4 Вход кнопка на общий
1 PB5 не используется


Chip type           : ATtiny13
Clock frequency     : 4,800000 MHz
Memory model        : Tiny
External SRAM size  : 0
Data Stack size     : 16
*****************************************************/

#include <tiny13.h>
#include <delay.h>
#include <button.h>
	
u08 bt1,bt2,bt3,bt4,bt5,bt_l,bt_l2,bt_al;	// Переменные - флажки нажатых кнопок. 
						// Эффективней их сделать битовыми полями
				// Но мне было лень. Оптимизируйте сами :)
u16 bt_mode_time = 2000;	// Длительность последовательности анализа
				// Сделано переменной, а не константой
				// чтобы можно было менять на лету. Снижая
				// там, где не надо ждать длительных комбинаций
				// что сильно повышает отзывчивость интерфейса
 
u08 bt_cnt;		// Флаг сигнализирующий, что идет анализ последовательности		
u08 bt_cnt_s;		// Счетчик коротких нажатий		
u08 bt_cnt_l;		// Счетчик длинных нажатий
	
#define starttime 30 // Время розжига
          
unsigned char GetData (void)
{
unsigned char i, j;
for(i=0;i<30;i++)  
  { 
    //В течение 30 секунд делаем 30 отборов и проверок значений блокировок я так понимаю что это для надежности               
    delay_us(1000);
    j = (PINB & 0x1E);  // pb1 - pb4
    delay_us(1000);
    if(j != (PINB & 0x1E)) i=0;  //1Е = 11110
  }
return j;
}
 
eeprom int pwm_Need_level;    //Значение ШИМ сохраненное в ЕЕPROM и к которому будет стремится контроллер
int pwm_current = 0;          //Текущее значение ШИМ, которое будет меняться в зависимости от значения pwm_Need_Level
int pwm_step    = 51;         //Шаг ШИМ всегда равно 51 так как 20% от 255 это 51
unsigned char laststate   = 0;//Если это последнее состояние то чего? !!!!!!!!!!!!!!!!!!!
unsigned char oldgab      = 0;//Габариты
unsigned char oldbrk      = 0;//Масло и ручник 

void main(void)
{
//В эту переменную записываем то, что считываем по блокировкам
unsigned char dt   = 0;

// Crystal Oscillator division factor: 1
//Делитель 
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
	
//регистр данных порта В
// 		Func5=In 	Func4=In 	Func3=In 	Func2=In 	Func1=Out 	Func0=Out 
DDRB=0x03; //	5-0 		4-0 		3-0 		2-0 		1-1 		0-1 
// 		State5=P 	State4=P 	State3=P 	State2=T 	State1=0 	State0=0 
PORTB=0x38;//	5-1 		4-1 		3-1 		2-0 		1-0 		0-0

//				DDRx.n		PORTx.n
//(По умолчанию)		0		0
//Высокоимпендансный вход
//Вход с pull up резистором	0		1
//Вызод соединен с землей	1		0
//Выход соединен с VCC(+5 В)	1		1


// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 18,750 kHz
// Mode: Fast PWM top=FFh
// OC0A output: Non-Inverted PWM
// OC0B output: Disconnected
TCCR0A=0x83;//Разобрать каждый из регистров
TCCR0B=0x04;
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// External Interrupt(s) initialization
// INT0: Off
// Interrupt on any change on pins PCINT0-5: Off
GIMSK=0x00;
MCUCR=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
ACSR=0x80;
ADCSRB=0x00;

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/1024k
// Watchdog Timer interrupt: Off
#pragma optsize-
WDTCR=0x39;
WDTCR=0x29;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

//Проверка на отсутствие или некорректность данных в памяти EEPROM, если слетает память EEPROM
if ((pwm_Need_Level < 0) && (pwm_Need_Level > 255) {
	//Записать в EEPROM 20% pwm_level
	pwm_current = 51;
}
oldgab = ((dt & 0x04) != 0); // начальные значения   PB2
oldbrk = ((dt & 0x08) != 0); // начальные значения   PB3

PORTB.1 = 0x00; //Включаем PB1 в HIGH, то есть индикацию ШИМ

while (1)
   {
      #asm("wdr")  // сброс WDT

      dt = GetData(); // считали и отфильтровали ввод со входов

      delay_ms(100); 

	      
	      
	      
	      
	      
	      
	      
	      
	      
	      
	      
        if (laststate == 0x00) {
		//Мигаем 2 раза ШИМ
		
		
		
		
		
		
            laststate = 0x01;
            //255 - 100% 
            //51 - 20%
            pwm_level += 51;
            if (pwm_level > 255) {
                pwm_level = 15; 
                pwm_current = 0;
            }
            pwm_step = pwm_level / starttime + 1; // шаг розжига.               
        }
      } else {
       laststate = 0x00;
       if ((dt & 0x0C) == 0x08) {  //0х0С = 1100  0х08 = 1000  
                                   //PB3 Вход 0 блокировка ШИМ  
                                   //ТУТ 1 Значит не работает блокировка
         // разжигаем
         if (pwm_current < pwm_level) {
              pwm_current += pwm_step;
              if (pwm_current > pwm_level) {
                 pwm_current = pwm_level;
              }
         }
       } else {
         // гасим
         if (pwm_current > 0) {
              pwm_current = 0;

//              pwm_current -= pwm_step;
              if (pwm_current < 0) {
                 pwm_current = 0;
              }
         }
       }
      }
      
      //if (pwm_current == 0x00) {                
      //   OCR0A  = 0x00; 
      //  TCCR0A = 0x00;      
      //} else {
        OCR0A  = 150;
        TCCR0A = 0x83;
      //}      
      
//      (dt &  0x04) = 1 габариты включены
//      (dt &  0x08) = 0 ручник включен
      /*
      if ((((dt & 0x04) != 0) && (oldgab == 0)) 
       || (((dt & 0x08) != 0) && (oldbrk == 0)) && ((dt & 0x04) != 0))
      {
      */
        PORTB.1 = 0x01;
      /*} else 
      if ((((dt & 0x04) == 0) && (oldgab == 1)) 
       || (((dt & 0x08) == 0) && (oldbrk == 1)))
      {
        PORTB.1 = 0x00;
      }                     
      oldgab = ((dt & 0x04) != 0);
      oldbrk = ((dt & 0x08) != 0);
      */
   }
}

void bt_scan(void)// Эта функция сканер. Она должна вызываться раз в 20мс
{
	
#define up 	0	// Дефайны состояний автомата. 0 - по дефолту.
#define dn 	1
#define al 	3	
static u08 bt_state=0;//Переменная состояния автомата
static u08 bt_time=0; //Переменная времени работы автомата	
switch(bt_state) //Сам автомат
{
	case up:
		{
			//0x10 = PB4 == 0 То есть нажатие на кнопку
      			if ((dt & 0x10) == 0x00) // Если нажата кнопка
			{
				bt_state = dn;//Стадию в Dn
				bt_time = 0;//Обнуляем счетчик времени
				
				if (bt_cnt==0)//Если первый раз в комбинации
					{
						SetTimeTask(bt_ok, bt_mode_time); //Запускаем процедуру анализа
						bt_cnt = 1;
					}
			}
			break;
		}
	case dn:
		{
			//0x10 = PB4 == 0 То есть нажатие на кнопку
      			if ((dt & 0x10) == 0x00) // Все еще нажато? 
			{
				if (20>bt_time)//Нажато меньше чем 20*20мс?
				{//да
					bt_time++; //Увеличиваем счсетчик итераций
				}
				else //нет
				{
					bt_state = al; //Нет, уже больше! Да у нас длинное нажатие! Перемодим в ал
					bt_time = 0;//Сбрасываем время замера нажатия
					bt_cnt_l++; //Засчитываем одно длинное нажатие
				}
			}
			else //Уже не нажато
			{
				bt_state = up; //Кнопка опущена?
				bt_time = 0;//Время замера в 0
				bt_cnt_s ++; //Счетчик коротких нажатий
			}
			break;
		}
	case al:
		{
			if ((dt & 0x10) == 0x01) // Отпустили?
			{
				bt_state = up; //  В стадию Up
				bt_time = 0; //время в 0
				bt_al = 1; //Зафиксировали событие "Отпускание после длинного "
			}
			break;
		}
}
SetTimerTask(bt_scan,20);
   
}
    
//А это функция, которая через 2 секунды сработает и подберет все результаты подсчета
void bt_ok(void)
{
	switch(bt_cnt_s)
	{
		case 1: bt1 = 1; break;
		case 2: bt2 = 1; break;
		case 3: bt3 = 1; break;
		case 4: bt4 = 1; break;
		case 5: bt5 = 1; break;
		default: break;
	}
	switch(bt_cnt_l)
	{
		case 1: bt_l = 1; break;
		case 2: bt_l2 = 1; break;
		default: break;
	}
	bt_cnt = 0;
	bt_cnt_s = 0;
	bt_cnt_l = 0;
}
