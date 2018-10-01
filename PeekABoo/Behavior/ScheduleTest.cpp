#ifdef _CFCT_ // Compiling for C Testing
#include <iostream>
#include <time.h>
unsigned long millis(){ return 1000 * clock() / CLOCKS_PER_SEC; }
#include "Schedule.h"

Schedule* sch = new Schedule();

#define pl(x) std::cout << x << std::endl
#define plt(x) std::cout << x << " - " << millis() << std::endl

bool** beepboopd = new bool*(new bool(false));

// Test:
int main()
{
    std::cout << "Hello world!" << std::endl;
    sch->EVERY(1000)->DO( plt("Tic") );
    //sch->EVERY(500)->DO( std::cout << "Tok - " << millis() << std::endl );
    //sch->IN(500)->DO( sch->EVERY(1000)->DO( plt("Toc"); ); );

    //sch->WHEN(millis() > 550 && millis() < 720 || millis() > 1123)->DO( plt("Chirp"); );
    //sch->WHEN(millis() < 500 || millis() > 710)->DO( plt("Chop"); );

    //sch->WHILE(millis() > 1023 && millis() < 1025)->DO( plt("BEEP"); );

    //sch->EVERY_WHILE(2, millis() >= 1105 && millis() <= 1142 || millis() >= 2100 && millis() <= 2200)->DO( plt("CLIP"); );

    beepboopd = sch->IN(3100)->DO_LONG( *(sch->IN(1000)->DO( plt("***BEEP***BOOP***"); )); );
    sch->WHEN(**beepboopd)->DO( plt("## BOP ##"); );

    while(1){
        sch->loop();
    }
}
#endif
