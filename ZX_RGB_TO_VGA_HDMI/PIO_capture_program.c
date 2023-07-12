#include "PIO_capture_program.h"
#include "inttypes.h"

#include "hardware/pio.h"
#include "g_config.h"

//программа захвата с делением входной частоты

uint16_t pio_program1_instructions[] = {
    
    0xa042, //  0: nop                               
    0xe020, //  1: set    x, 0                       
    0x2000|F_CAP_PIN, //  2: wait   1 gpio, 0                  
    0x4008, //  3: in     pins, 8                    
    0x2080|F_CAP_PIN, //  4: wait   0 gpio, 0                  
    0x0042, //  5: jmp    x--, 2    
   // 0xa042, //  6: nop              //вставка - заменить задержкой прошлой команды         
                 
    0x8020, //  6: push   block                      
    0x00c1, //  7: jmp    pin, 1                     
    0xe020, //  8: set    x, 0                       
    0x2000|F_CAP_PIN, //  9: wait   1 gpio, 0                  
    0x00c0, // 10: jmp    pin, 0  
    0x4008, //  11: in     pins, 8                      
    0x2080|F_CAP_PIN, // 12: wait   0 gpio, 1                  
    0x0049, // 13: jmp    x--, 9                     
    //0xa042, //  14: nop              //вставка - заменить задержкой прошлой команды         
    0x8020, // 14: push   block                      
    0x0008, // 15: jmp    8                   

};


const struct pio_program pio_program1_CAP = {
    .instructions = pio_program1_instructions,
    .length = 16,
    .origin = -1,
};

//программа захвата с самосинхронизацией
uint16_t pio_program0_instructions[] = {
    
               //     .wrap_target
    0xa042, //  0: nop               //команда для формирования задержки захвата               
    0x4008, //  1: in     pins, 8                    
 //   0xa042, //  2: nop              //вставка - заменить задержкой прошлой команды         
    0x8020, //  2: push   block                      
    0xa842, //  3: nop                           [8] 
    0x00c1, //  4: jmp    pin, 1 
    0x4008, //  5: in     pins, 8       //подсинхронизация по строчной синхре     
           
    0x8020, //  6: push   block                      
  //  0x00c0, //  6: jmp    pin, 0                     
    0x00c0, //  7: jmp    pin, 0                     
    0x00c0, //  8: jmp    pin, 0                     
    0x00c0, //  9: jmp    pin, 0                     
    0x00c0, //  10: jmp    pin, 0                     
    0x00c0, //  11: jmp    pin, 0                     
    0x00c0, //  12: jmp    pin, 0                     
    0x00c0, //  13: jmp    pin, 0                     
    0x00c0, //  14: jmp    pin, 0 
    //                 
    0x00c0, //  15: jmp    pin, 0                     
   
    0x0005, // 16: jmp    5                          
            //     .wrap



};


const struct pio_program pio_program0_CAP = {
    .instructions = pio_program0_instructions,
    .length = 17,
    .origin = -1,
};


//программа захвата с захватом по фронту и спаду

uint16_t pio_program2_instructions[] = {
  
               
    0x2080|F_CAP_PIN, //  0: wait   1 gpio, 0    
    0xa042, //  1: nop                     //команда для формирования задержки захвата                   
    0x4008, //  2: in     pins, 8  
    //0xa042, //  3: nop              //вставка - заменить задержкой прошлой команды         
 
    0x8020, //  3: push   block                    
    0x2000|F_CAP_PIN, //  4: wait   0 gpio, 0   
    0xa042, //  5: nop                    //команда для формирования задержки захвата                      
    0x4008, //  6: in     pins, 8   
    //0xa042, //  7: nop              //вставка - заменить задержкой прошлой команды         

    0x8020, //  7: push   block     
    
                    
  
};



const struct pio_program pio_program2_CAP = {
    .instructions = pio_program2_instructions,
    .length = 8,
    .origin = -1,
};


//l0: wait 0 gpio,0
//jmp x--, l1 
//set x [0] //при нечётном(целом) делителе здесь оставить 0 в x 
//in pins,8
//l1: wait 1 gpio,0
//jmp x--, l0 
//set x [0]
//in pins,8
