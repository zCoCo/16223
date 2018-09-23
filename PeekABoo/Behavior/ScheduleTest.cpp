#include <iostream>
#include "Schedule.h"

Schedule* sch = new Schedule();

// Test:
int main()
{
    std::cout << "Hello world!" << std::endl;

    sch->EVERY(500)->DO( std::cout << "Tic - " << millis() << std::endl );
    sch->EVERY(500)->DO( std::cout << "Tok - " << millis() << std::endl );
    //sch->IN(500)->DO( sch->EVERY(500)->DO( std::cout << "Tok - " << millis() << std::endl ) );

    sch->WHEN(millis() > 550 && millis() < 720 || millis() > 1123)->DO( std::cout << "Chirp - " << millis() << std::endl );
    sch->WHEN(millis() < 500 || millis() > 710)->DO( std::cout << "Chop - " << millis() << std::endl );

    sch->WHILE(millis() > 1023 && millis() < 1025)->DO(std::cout << "BEEP - " << millis() << std::endl);

    //sch->EVERY(2)->WHILE(millis() > 1105 && millis() < 1142)->DO(std::cout << "CLIP - " << millis() << std::endl);

    while(1){
      sch->loop();
    }
}
