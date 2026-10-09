#include "timebase.h"
#include "stm32f4xx.h"

#define CTRL_ENABLE (1U<<0)
#define CTRL_TICKINT (1U<<1)
#define CTRL_CLCKSRC (1U<<2)
#define CTRL_COUNTFLAG (1U<<16)

//frecventa sistemului nostru este 16MHz => 16 milioane de cicluri de ceas intr o secunda
#define ONE_SEC_LOAD 16000000

//cand avem un interrupt, vrem sa adaugam 1 la nr total de tick uri
#define TICK_FREQ 1

#define MAX_DELAY 0xFFFFFFFF

volatile uint32_t g_curr_tick;
volatile uint32_t g_curr_tick_p;

//systick genereaza un interrupt in fiecare secunda
void timebase_init(void) {
	//dam disable la interrupt ul global -> nu vrem sa avem intreruperi in timpul configurarii
	__disable_irq();

	//decidem unitatea de masura -> vrem secunde
	//incarcam timerul cu nr de cicluri de clock per secunda
	SysTick->LOAD = ONE_SEC_LOAD - 1; //pt ca incepem cu 0
	//dam clear la registru pt valoarea curenta systick
	SysTick->VAL = 0;
	//selectam sursa interna de ceas
	SysTick->CTRL = CTRL_CLCKSRC;
	//dam enable la interrupt
	SysTick->CTRL |= CTRL_TICKINT;
	//dam enable la systick
	SysTick->CTRL |= CTRL_ENABLE;

	//dam enable la interrupt ul global
	__enable_irq();

}

void tick_increment(void) {
	g_curr_tick += TICK_FREQ;
}

//handlerul pt systick da increment la tick uri cu fiecare interrupt
void SysTick_Handler(void) {
	tick_increment();
}

//vrem operatia sa fie atomica(nu vrem interrupt uri in timp ce se executa)
uint32_t get_tick(void) {
	__disable_irq();
	g_curr_tick_p = g_curr_tick;
	__enable_irq();
	return g_curr_tick_p;
}

//delay in secunde
void delay(uint32_t delay) {
	uint32_t tickstart = get_tick();
	uint32_t wait = delay;
	//nr max de delay uri
	if(wait < MAX_DELAY) {
		wait += (uint32_t)TICK_FREQ;
	}
	while((get_tick() - tickstart) < wait) {
		//delay
	}
}
