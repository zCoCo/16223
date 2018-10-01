#ifdef _CFCT_ // Compiling for C Testing
#include <iostream>
#include <time.h>
unsigned long millis(){ return 1000 * clock() / CLOCKS_PER_SEC; }
#include "Schedule.h"

Schedule* sch = new Schedule();

// Test:
int main()
{
    std::cout << "Hello world!" << std::endl;
    sch->EVERY(1000)->DO( std::cout << "Tic - " << millis() << std::endl );
    //sch->EVERY(500)->DO( std::cout << "Tok - " << millis() << std::endl );
    sch->IN(500)->DO( sch->EVERY(1000)->DO( std::cout << "Toc - " << millis() << std::endl ); );

    sch->WHEN(millis() > 550 && millis() < 720 || millis() > 1123)->DO( std::cout << "Chirp - " << millis() << std::endl );
    sch->WHEN(millis() < 500 || millis() > 710)->DO( std::cout << "Chop - " << millis() << std::endl );

    sch->WHILE(millis() > 1023 && millis() < 1025)->DO(std::cout << "BEEP - " << millis() << std::endl);

    sch->EVERY_WHILE(2, millis() >= 1105 && millis() <= 1142 || millis() >= 2100 && millis() <= 2200)->DO(std::cout << "CLIP - " << millis() << std::endl);

    while(1){
      sch->loop();
    }
}
#endif
