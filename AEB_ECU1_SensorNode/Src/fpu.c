#include "stm32f4xx.h"


void fpu_enable(void) {
	//dam enable la floating point unit (enable CP10
	//si CP11 full access)
	//SCB = system control block
	//daca nu am pune pipe(|) am avea 1 pe poz 20 si 0 in rest
	//noi vrem doar sa actualizam bitul 20, si restul sa ramana la fel
	SCB->CPACR |=(1U<<20);
	SCB->CPACR |=(1U<<21);
	SCB->CPACR |=(1U<<22);
	SCB->CPACR |=(1U<<23);
}
