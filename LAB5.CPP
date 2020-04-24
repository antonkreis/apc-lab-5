#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>

unsigned short counter;
int alarm_flag;

void interrupt(*int_delay_old)(...);

void interrupt(*int_alarm_old)(...);

void interrupt int_delay_new(...) {
	counter++;
	int_delay_old();
}

void interrupt int_alarm_new(...) {
	alarm_flag = 1;
	int_alarm_old();
}

unsigned int fromBCD(unsigned char bcd) {
	unsigned char low = bcd;
	unsigned char high = bcd;
	high = low;
	low = low << 4;
	low = low >> 4;
	high = high >> 4;
	high *= 10;
	return (unsigned int)(high + low);
}

unsigned char toBCD(unsigned int num) {
	unsigned char high = (num / 10);
	high = high << 4;
	unsigned char low = (num % 10);
	return high | low;
}

void getTime() {
	printf("=============Get Time=============\n");
	outp(0x70, 0x00); //second
	int second = fromBCD(inp(0x71));
	outp(0x70, 0x02); //minute
	int minute = fromBCD(inp(0x71));
	outp(0x70, 0x04); //hour	
	int hour = fromBCD(inp(0x71));
	outp(0x70, 0x06); //day of week 
	int week = fromBCD(inp(0x71));
	outp(0x70, 0x07); //day of month
	int day = fromBCD(inp(0x71));
	outp(0x70, 0x08); //month
	int month = fromBCD(inp(0x71));
	outp(0x70, 0x09); //year
	int year = fromBCD(inp(0x71));
	printf("The current date and time:\n");
	printf("Year: %d\n", year);
	printf("Month: %d\n", month);
	printf("Week: %d\n", week);
	printf("Day: %d\n", day);
	printf("Hour: %d\n", hour);
	printf("Minute: %d\n", minute);
	printf("Second: %d\n", second);
}

void setTime() {
	printf("=============Set Time=============\n");
	unsigned char state;
	unsigned int year, month, week, day, hour, minute, second;
	printf("Enter the new time and date:\n");
	do {
		rewind(stdin);
		printf("Year (0-99): ");
	} while (!scanf("%u", &year) || year < 0 || year > 99);

	do {
		rewind(stdin);
		printf("Month (1-12): ");
	} while (!scanf("%u", &month) || month < 1 || month > 12);
	
	do {
		rewind(stdin);
		printf("Week (1-7, Sun - 1): ");
	} while (!scanf("%u", &week) || week < 1 || week > 7);
	
	do {
		rewind(stdin);
		printf("Day (1-31): ");
	} while (!scanf("%u", &day) || day < 1 || day > 31);
	
	do {
		rewind(stdin);
		printf("Hour (0-23): ");
	} while (!scanf("%u", &hour) || hour < 0 || hour > 23);

	do {
		rewind(stdin);
		printf("Minute (0-59): ");
	} while (!scanf("%u", &minute) || minute < 0 || minute > 59);

	do {
		rewind(stdin);
		printf("Second (0-59): ");
	} while (!scanf("%u", &second) || second < 0 || second > 59);
	
	do {
		outp(0x70, 0xA);
		state = inp(0x71) & 0x80;
	} while (state);
	outp(0x70, 0xB);
	outp(0x71, inp(0x71) | 0x80);

	outp(0x70, 0x00);
	outp(0x71, toBCD(second));

	outp(0x70, 0x02);
	outp(0x71, toBCD(minute));

	outp(0x70, 0x04);
	outp(0x71, toBCD(hour));

	outp(0x70, 0x06);
	outp(0x71, toBCD(week));

	outp(0x70, 0x07);
	outp(0x71, toBCD(day));

	outp(0x70, 0x08);
	outp(0x71, toBCD(month));

	outp(0x70, 0x09);
	outp(0x71, toBCD(year));


	outp(0x70, 0xB);
	outp(0x71, inp(0x71) & 0x7F);
}

void delayMS() {
	printf("=============Delay=============\n");
	counter = 0;
	unsigned short delay;
	do {
		rewind(stdin);
		printf("Enter the delay in ms (1 - 65535): ");
	} while (!scanf("%hu", &delay) || delay < 1 || delay > 65535);


	int_delay_old = getvect(0x70); 

	_disable();
	setvect(0x70, int_delay_new);
	_enable();

	outp(0xA1, inp(0xA1) & 0xFE); // enable interrupts from RTC


	outp(0x70, 0xB);
	outp(0x71, inp(0x71) | 0x40);

	printf("Start of delay.\n");
	while (1) {
		if (counter == delay) {
			printf("End of delay.\n");
			break;
		}
	}

	outp(0x70, 0xB);
	outp(0x71, inp(0x71) & 0xBF); //10111111

	_disable();
	setvect(0x70, int_delay_old);
	_enable();
}

void alarmClock() {
	printf("=============Alarm clock=============\n");
	outp(0x70, 0xB);
	outp(0x71, inp(0x71) & 0xDF); //11011111
	unsigned int hour, minute, second;
	alarm_flag = 0;
	printf("Enter time:\n");

	do {
		rewind(stdin);
		printf("Hour (0-23): ");
	} while (!scanf("%u", &hour) || hour < 0 || hour > 23);

	do {
		rewind(stdin);
		printf("Minute (0-59): ");
	} while (!scanf("%u", &minute) || minute < 0 || minute > 59);

	do {
		rewind(stdin);
		printf("Second (0-59): ");
	} while (!scanf("%u", &second) || second < 0 || second > 59);


	outp(0xA1, inp(0xA1) & 0xFE); // enable interrupts from RTC

	outp(0x70, 0x01);
	outp(0x71, toBCD(second));

	outp(0x70, 0x03);
	outp(0x71, toBCD(minute));

	outp(0x70, 0x05);
	outp(0x71, toBCD(hour));

	int_alarm_old = getvect(0x4A);
	_disable();
	setvect(0x4A, int_alarm_new);
	_enable();

	outp(0x70, 0xB);
	outp(0x71, inp(0x71) | 0x20);

	while (1) {
		if (alarm_flag) {
			printf("Alarm!!!\n");
			break;
		}
	}

	outp(0x70, 0xB);
	outp(0x71, inp(0x71) & 0xDF); //11011111
	_disable();
	setvect(0x4A, int_alarm_old);
	_enable();
}


int main() {
	char option;
	while (1)
	{
		system("cls");
		printf("=============Lab5=============\n");
		printf("1. Get time\n");
		printf("2. Set time\n");
		printf("3. Delay in ms\n");
		printf("4. Alarm clock\n");
		printf("5. Exit\n");
		printf("Choose an option ");
		option = getch();
		switch (option) {
		case '1': 
			system("cls");
			getTime();
			getch();
			break;
		case '2':
			system("cls");
			setTime();
			getch();
			break;
		case '3': 
			system("cls");
			delayMS();
			getch();
			break;
		case '4':
			system("cls");
			alarmClock();
			getch();
			break;
		case '5':
			return 0;
		default:;
		}
	}
}