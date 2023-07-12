#include "VGA.h"
#include "hardware/clocks.h"

#include "hardware/structs/pll.h"
#include "hardware/structs/systick.h"

#include "hardware/dma.h"
#include "hardware/irq.h"
#include <string.h> 
#include "g_config.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "stdlib.h"
#include "v_buf.h"


uint16_t pio_program_VGA_instructions[] = {
                       
        //     .wrap_target

               //     .wrap_target
    0x6008, //  0: out    pins, 8                    
            //     .wrap                
            //     .wrap
};

const struct pio_program pio_program_VGA = {
    .instructions = pio_program_VGA_instructions,
    .length = 1,
    .origin = -1,
};




//базовые уровни сигналов

#define TMPL_VS   (0b01000000)
#define TMPL_VHS  (0b00000000)

#define TMPL_HS (0b10000000)
#define TMPL_NORM (0b11000000)

bool is_wide=false;




static uint32_t* lines_pattern[8];

//пока выделим память для шаблонов строк с запасом по 3кб на линию

#define L_STEP (3072)
//static uint32_t* lines_data=RAM_block;//[8*(L_STEP/4)];

static uint8_t spec_colors[16];



void  __not_in_flash_func(memset32)(uint32_t* dst,const uint32_t data, uint32_t size);


static int dma_chan_ctrl;
void __not_in_flash_func(dma_handler_VGA)() {

    dma_hw->ints0 = 1u << dma_chan_ctrl;   
    static uint32_t frame_i=0;
    static uint32_t line_active=0;
    static uint8_t* vbuf=NULL;
    line_active++;
    if (line_active==525) {line_active=0;frame_i++;vbuf=v_buf_get_out();}
    switch (line_active)
            {
                case 480 ... 490:
                        //VS front
                        dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[0], false);
                       return;
                case 491:
                case 492:

                        //VS SYNC
                        dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[1], false);
                        return;
                case 493 ... 524:

                         dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[0], false);
                       return;
               


                default:
                    // dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[6], false);return;
                    break;
            }

   


    if (!(vbuf)) {dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[2], false) ;return;}
    //зона прорисовки изображения
    int line=line_active/2;
    uint8_t* vbuf8=vbuf+(line)*V_BUF_W/2; 

    uint32_t** ptr_vbuf_OUT=&lines_pattern[2];
    switch (line_active%4)
    {
    case 0:
        ptr_vbuf_OUT=&lines_pattern[2];
        break;
    case 1:
         dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[2], false) ; return;
    
    case 2:
        ptr_vbuf_OUT=&lines_pattern[3];
        break;
    case 3:
         dma_channel_set_read_addr(dma_chan_ctrl,&lines_pattern[3], false) ; return;
    default:
        break;
    }

     
  
    
    uint8_t* vbuf_OUT=(uint8_t*)(*ptr_vbuf_OUT);
  
    if (is_wide) 
    for(int i=20;i--;)
    {
        *vbuf_OUT++=spec_colors[*vbuf8&0xf];
        *vbuf_OUT++=spec_colors[*vbuf8>>4];
    }

    for(int i=V_BUF_W/2;i--;)
    {
             *vbuf_OUT++=spec_colors[*vbuf8&0xf];
             *vbuf_OUT++=spec_colors[*vbuf8++>>4];
    }
    vbuf8--;
    if (is_wide) 
    for(int i=20;i--;)
    {
        *vbuf_OUT++=spec_colors[*vbuf8&0xf];
        *vbuf_OUT++=spec_colors[*vbuf8>>4];
    }
        
    dma_channel_set_read_addr(dma_chan_ctrl,ptr_vbuf_OUT, false) ;



    
  
}

void setVGAWideMode(bool w_mode){is_wide=w_mode;};






void startVGA()
{
    //инициализация палитры 
    for(int i=0;i<16;i++)
    {
        uint8_t c=pallete8[i]|0xc0;
        spec_colors[i]=(c<<0);
    }

    set_sys_clock_khz(252000, true);
    

    int line_size;
    double fdiv=100;
    int HS_SIZE=4;
    int HS_SHIFT=100;
    if (is_wide)
    {
        fdiv=clock_get_hz(clk_sys)/(31468750.0/2);//частота VGA для широкоэкранного режима
        line_size=500;
        HS_SHIFT=410;
        HS_SIZE=60;

    }
    else
    {
        fdiv=clock_get_hz(clk_sys)/(25175000.0/2);//частота VGA по умолчанию
        line_size=400;
        HS_SHIFT=328;
        HS_SIZE=48;

    }
   
    
    
    // uint8_t* base_ptr=lines_data;
    uint8_t* base_ptr=calloc(line_size*9/4,sizeof(uint32_t));;
    lines_pattern[0]=(uint32_t*)base_ptr;
    //пустая строка
    memset(base_ptr,TMPL_NORM,line_size);
    memset(base_ptr+HS_SHIFT,TMPL_HS,HS_SIZE);

    base_ptr+=L_STEP;
    lines_pattern[1]=(uint32_t*)base_ptr;
    // кадровая синхра
    memset(base_ptr,TMPL_VS,line_size);
    memset(base_ptr+HS_SHIFT,TMPL_VHS,HS_SIZE);

    base_ptr+=L_STEP;
    lines_pattern[2]=(uint32_t*)base_ptr;
    memcpy(base_ptr,lines_pattern[0],line_size);

    base_ptr+=L_STEP;
    lines_pattern[3]=(uint32_t*)base_ptr;
    memcpy(base_ptr,lines_pattern[0],line_size);

   



    //инициализация PIO
    //загрузка программы в один из PIO
    uint offset = pio_add_program(PIO_VGA, &pio_program_VGA);
    uint sm=SM_VGA;

    for(int i=0;i<8;i++){gpio_init(beginVGA_PIN+i);
    gpio_set_dir(beginVGA_PIN+i,GPIO_OUT);pio_gpio_init(PIO_VGA, beginVGA_PIN+i);};//резервируем под выход PIO
  
    pio_sm_set_consecutive_pindirs(PIO_VGA, sm, beginVGA_PIN, 8, true);//конфигурация пинов на выход
    //pio_sm_config c = pio_vga_program_get_default_config(offset); 

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + 0, offset + (pio_program_VGA.length-1));


    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);//увеличение буфера TX за счёт RX до 8-ми 
    sm_config_set_out_shift(&c, true, true, 32);
    sm_config_set_out_pins(&c, beginVGA_PIN, 8);
    pio_sm_init(PIO_VGA, sm, offset, &c);

    pio_sm_set_enabled(PIO_VGA, sm, true);


   
   



    // set_sys_clock_khz(256000, true);
    // sleep_ms(10); 
    // fdiv=8;


    uint32_t div32=(uint32_t) (fdiv * (1 << 16)+0.5);
    PIO_VGA->sm[sm].clkdiv=div32&0xfffff000; //делитель для конкретной sm 
    //инициализация DMA

    
    dma_chan_ctrl = dma_claim_unused_channel(true);
    int dma_chan = dma_claim_unused_channel(true);
    //основной ДМА канал для данных
    dma_channel_config c0 = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);

    channel_config_set_read_increment(&c0, true);
    channel_config_set_write_increment(&c0, false);

    uint dreq=DREQ_PIO1_TX0+sm;
    if (PIO_VGA==pio0) dreq=DREQ_PIO0_TX0+sm;

    channel_config_set_dreq(&c0, dreq);
    channel_config_set_chain_to(&c0, dma_chan_ctrl);                        // chain to other channel
   
    dma_channel_configure(
        dma_chan,
        &c0,
        &PIO_VGA->txf[sm], // Write address 
        lines_pattern[0],             // read address 
        line_size/4, //
        false             // Don't start yet
    );
    //канал DMA для контроля основного канала
    dma_channel_config c1 = dma_channel_get_default_config(dma_chan_ctrl);
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);
    
    channel_config_set_read_increment(&c1, false);
    channel_config_set_write_increment(&c1, false);
    channel_config_set_chain_to(&c1, dma_chan);                         // chain to other channel
    //channel_config_set_dreq(&c1, DREQ_PIO0_TX0);



    dma_channel_configure(
        dma_chan_ctrl,
        &c1,
        &dma_hw->ch[dma_chan].read_addr, // Write address 
        &lines_pattern[0],             // read address
        1, // 
        false             // Don't start yet
    );
    //dma_channel_set_read_addr(dma_chan, &DMA_BUF_ADDR[0], false);

    dma_channel_set_irq0_enabled(dma_chan_ctrl, true);
    
 

  

    
     // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    
    

    
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler_VGA);

    irq_set_enabled(DMA_IRQ_0, true);
    dma_start_channel_mask((1u << dma_chan)) ;


    G_PRINTF_INFO("init VGA video out\n");
};