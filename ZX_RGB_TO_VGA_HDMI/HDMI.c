#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "stdlib.h"

#include "hardware/structs/pll.h"
#include "hardware/structs/systick.h"

#include "hardware/dma.h"
#include "hardware/irq.h"
//#include <string.h> 
//#include "pico/mem_ops.h"
#include "HDMI.h"
#include "hardware/vreg.h"
#include "v_buf.h"

static const uint16_t instructions_PIO_HDMI[] = {
    
    0x7006, //  0: out    pins, 6         side 2     
    0x7006, //  1: out    pins, 6         side 2     
    0x7006, //  2: out    pins, 6         side 2     
    0x7006, //  3: out    pins, 6         side 2     
    0x7006, //  4: out    pins, 6         side 2     
    0x6806, //  5: out    pins, 6         side 1     
    0x6806, //  6: out    pins, 6         side 1     
    0x6806, //  7: out    pins, 6         side 1     
    0x6806, //  8: out    pins, 6         side 1     
    0x6806, //  9: out    pins, 6         side 1  

      
};

static const struct pio_program program_PIO_HDMI = {
    .instructions = instructions_PIO_HDMI,
    .length = 10,
    .origin = -1,
};

static void  __not_in_flash_func(memset64)(uint64_t* dst,const uint64_t data, uint32_t size)
{
        
        dst[0]=data;
        for(int i=1;i<size;i++){*++dst=*(dst-1);};

        // dst[0]=data;
        // for(int i=size;--i;){*++dst=*(dst-1);};

}

static void  __not_in_flash_func(memset64_inv)(uint64_t* dst,const uint64_t data, uint32_t size)
{
        uint64_t ndata=data^0x0003ffffffffffffl;
        for(int i=0;i<size/2;i++){*dst++=data;*dst++=ndata;};

       

}

static uint64_t get_ser_diff_data(uint16_t dataR,uint16_t dataG,uint16_t dataB)
{
    uint64_t out64;
    for(int i=0;i<10;i++)
    {
        out64<<=6;
        if(i==5) out64<<=2;
        uint8_t bR=(dataR>>(9-i))&1;
        uint8_t bG=(dataG>>(9-i))&1;
        uint8_t bB=(dataB>>(9-i))&1;
        
        bR|=(bR^1)<<1;
        bG|=(bG^1)<<1;
        bB|=(bB^1)<<1;
        
        if (HDMI_PIN_invert_diffpairs)
        {
            bR^=0b11;
            bG^=0b11;
            bB^=0b11;
        };
        uint8_t d6;
        if (HDMI_PIN_RGB_notBGR)
        {
            d6=(bR<<4)|(bG<<2)|(bB<<0);
        }
        else
        {
            d6=(bB<<4)|(bG<<2)|(bR<<0);
        };


        out64|=d6;



    }
    return out64;
};

#define scr_W (320)
#define scr_H (240)
static uint8_t* screen_buf;

static int dma_chan_ctrl;

#define SIZE_DMA_BUF (800*2)
//количество буферов ДМА можно увеличить, если будет подёргивание

static uint32_t* DMA_BUF[2];
static uint32_t* DMA_BUF_ADDR[2];
static uint64_t sync_data[4];
static uint64_t R64,G64,B64,Y64;

static uint64_t palette[32];


//конвертор TMDS
static uint tmds_encoder(uint8_t d8)
{
    int s1=0;
    for(int i=0;i<8;i++) s1+=(d8&(1<<i))?1:0;
    bool is_xnor=false;
    if ((s1>4)||((s1==4)&&((d8&1)==0))) is_xnor=true;
    uint16_t d_out=d8&1;
    uint16_t qi=d_out;
    for(int i=1;i<8;i++)
    {
        d_out|=((qi<<1)^(d8&(1<<i)))^(is_xnor<<i);
        qi=d_out&(1<<i);
    }
    
    if(is_xnor) d_out|=1<<9; 
    else d_out|=1<<8; 

    return d_out;

}

static void __not_in_flash_func(dma_handler_HDMI)() {
    static uint32_t inx_buf_dma;  
    static uint y;
    dma_hw->ints0 = 1u << dma_chan_ctrl;
    dma_channel_set_read_addr(dma_chan_ctrl,&DMA_BUF_ADDR[inx_buf_dma&1], false);
    y=(y==524)?0:(y+1);
    if (y==0) screen_buf=v_buf_get_out();
     if ((y&1)) return;

    inx_buf_dma++;


    uint64_t* activ_buf=(uint64_t*)(DMA_BUF[inx_buf_dma&1]);
    uint64_t* not_activ_buf=(uint64_t*)(DMA_BUF[(inx_buf_dma+1)&1]);
    if (screen_buf==NULL) return;  

    if (y<480)
    {
        //область изображения
     
        {
            uint8_t* scr_buf=&screen_buf[(y/2)*V_BUF_W/2];
            uint64_t* activ_buf_l=activ_buf;

        for(int i=scr_W/2;i--;)
        {
            uint8_t c2=*scr_buf++;
            uint64_t* c64=&palette[(c2&0xf)*2];
                *activ_buf_l++=*c64++;
                *activ_buf_l++=*c64;
            c2>>=4;
            c64=&palette[(c2&0xf)*2];
                *activ_buf_l++=*c64++;
                *activ_buf_l++=*c64;
        } 

   

        //ССИ
          memset64(activ_buf+640,sync_data[0b00],16);
          memset64(activ_buf+656,sync_data[0b01],96);
          memset64(activ_buf+752,sync_data[0b00],48);

        }
      
       

    }
    else
    {
        if ((y>=490) && (y<492))
        {
            //кадровый синхроимпульс
            memset64(activ_buf,sync_data[0b10],656);
            memset64(activ_buf+656,sync_data[0b11],96);
            memset64(activ_buf+752,sync_data[0b10],48);
            

        }else
        {
            memset64(activ_buf,sync_data[0b00],656);
            memset64(activ_buf+656,sync_data[0b01],96);
            memset64(activ_buf+752,sync_data[0b00],48);

        };

    }
    


   
    
}

void startHDMI()
{
    //screen_buf=scr_buf;
    vreg_set_voltage(VREG_VOLTAGE_1_25);
    sleep_ms(10);
    set_sys_clock_khz(252000, false);
    sleep_ms(10);
    //инициализация констант
    uint16_t b0=0b1101010100;
    uint16_t b1=0b0010101011;
    uint16_t b2=0b0101010100;
    uint16_t b3=0b1010101011;


    sync_data[0b00]=get_ser_diff_data(b0,b0,b3);
    sync_data[0b01]=get_ser_diff_data(b0,b0,b2);
    sync_data[0b10]=get_ser_diff_data(b0,b0,b1);
    sync_data[0b11]=get_ser_diff_data(b0,b0,b0);



    R64=get_ser_diff_data(tmds_encoder(255),tmds_encoder(0),tmds_encoder(0));
    G64=get_ser_diff_data(tmds_encoder(0),tmds_encoder(255),tmds_encoder(0));
    B64=get_ser_diff_data(tmds_encoder(0),tmds_encoder(0),tmds_encoder(255));
    Y64=get_ser_diff_data(tmds_encoder(255),tmds_encoder(255),tmds_encoder(0));

    //инициализация палитры

    for(int c=0;c<16;c++)
    {
        uint8_t GL=170;
        uint8_t Y=(c>>3)&1;
        uint8_t R=((c>>2)&1)?(Y?255:170):0;
        uint8_t G=((c>>1)&1)?(Y?255:170):0;
        uint8_t B=((c>>0)&1)?(Y?255:170):0;
        palette[c*2]=get_ser_diff_data(tmds_encoder(R),tmds_encoder(G),tmds_encoder(B));
        palette[c*2+1]=palette[c*2]^0x0003ffffffffffffl;
        // n_palette[c]=palette[c]^0x0003ffffffffffffl;

    }

    DMA_BUF[0]=calloc(SIZE_DMA_BUF,sizeof(uint32_t));
    DMA_BUF[1]=calloc(SIZE_DMA_BUF,sizeof(uint32_t));


    //инициализация PIO
    uint offset = pio_add_program(pio_HDMI0, &program_PIO_HDMI);
    pio_sm_config c = pio_get_default_sm_config();


    sm_config_set_wrap(&c, offset, offset + (program_PIO_HDMI.length-1)); 
         
    //настройка side set
    sm_config_set_sideset_pins(&c,beginHDMI_PIN_clk);
    sm_config_set_sideset(&c,2,false,false);
    for(int i=0;i<2;i++)
        {           
            pio_gpio_init(pio_HDMI0, beginHDMI_PIN_clk+i);
            gpio_set_drive_strength(beginHDMI_PIN_clk+i,GPIO_DRIVE_STRENGTH_12MA);
            gpio_set_slew_rate(beginHDMI_PIN_clk+i,GPIO_SLEW_RATE_FAST);
        }
    
    pio_sm_set_pins_with_mask(pio_HDMI0, sm_HDMI0, 3u<<beginHDMI_PIN_clk, 3u<<beginHDMI_PIN_clk);
    pio_sm_set_pindirs_with_mask(pio_HDMI0, sm_HDMI0,  3u<<beginHDMI_PIN_clk,  3u<<beginHDMI_PIN_clk);
    //пины

    for(int i=0;i<6;i++)
    {
        gpio_set_slew_rate(beginHDMI_PIN_data+i,GPIO_SLEW_RATE_FAST);
        pio_gpio_init(pio_HDMI0, beginHDMI_PIN_data+i);
        gpio_set_drive_strength(beginHDMI_PIN_data+i,GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_slew_rate(beginHDMI_PIN_data+i,GPIO_SLEW_RATE_FAST);
        // gpio_is_input_hysteresis_enabled(beginHDMI_PIN_data+i);
    }
   


    
  
    pio_sm_set_consecutive_pindirs(pio_HDMI0, sm_HDMI0, beginHDMI_PIN_data, 6, true);//конфигурация пинов на выход

    sm_config_set_out_shift(&c, true, true, 30);

    sm_config_set_out_pins(&c, beginHDMI_PIN_data, 6);

    sm_config_set_fifo_join(&c,PIO_FIFO_JOIN_TX);
    




    pio_sm_init(pio_HDMI0, sm_HDMI0, offset, &c);
    pio_sm_set_enabled(pio_HDMI0, sm_HDMI0, true);


    //инициализация DMA 
    dma_chan_ctrl = dma_claim_unused_channel(true);
    int dma_chan = dma_claim_unused_channel(true);

    //основной ДМА канал для данных
    dma_channel_config c0 = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_32);

    channel_config_set_read_increment(&c0, true);
    channel_config_set_write_increment(&c0, false);

    uint dreq=DREQ_PIO1_TX0+sm_HDMI0;
    if (pio_HDMI0==pio0) dreq=DREQ_PIO0_TX0+sm_HDMI0;

    channel_config_set_dreq(&c0, dreq);
    channel_config_set_chain_to(&c0, dma_chan_ctrl);                        // chain to other channel
   
    dma_channel_configure(
        dma_chan,
        &c0,
        &pio_HDMI0->txf[sm_HDMI0], // Write address 
        &DMA_BUF[0][0],             // read address 
        SIZE_DMA_BUF, //
        false             // Don't start yet
    );
    
    //канал DMA для контроля основного канала
    dma_channel_config c1 = dma_channel_get_default_config(dma_chan_ctrl);
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);
    
    channel_config_set_read_increment(&c1, false);
    channel_config_set_write_increment(&c1, false);
    channel_config_set_chain_to(&c1, dma_chan);                         // chain to other channel
    //channel_config_set_dreq(&c1, DREQ_PIO0_TX0);

   


    DMA_BUF_ADDR[0]=&DMA_BUF[0][0];
    DMA_BUF_ADDR[1]=&DMA_BUF[1][0];
    

    dma_channel_configure(
        dma_chan_ctrl,
        &c1,
        &dma_hw->ch[dma_chan].read_addr, // Write address 
        &DMA_BUF_ADDR[0],             // read address
        1, // 
        false             // Don't start yet
    );
    // dma_channel_set_read_addr(dma_chan, &DMA_BUF_ADDR[0], false);



    dma_channel_set_irq0_enabled(dma_chan_ctrl, true);

  

    

     // Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler_HDMI);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_start_channel_mask((1u << dma_chan_ctrl)) ;











};