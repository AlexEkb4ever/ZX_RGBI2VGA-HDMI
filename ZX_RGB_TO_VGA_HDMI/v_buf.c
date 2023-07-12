#include "v_buf.h"
#include "g_config.h"

uint8_t* v_bufs[3]={g_gbuf,g_gbuf+V_BUF_SZ,g_gbuf+2*V_BUF_SZ};

bool is_show_vbuf[]={false,false,false};

uint64_t inx_frame=0;

uint64_t i_frame_vbuf[3]={0,0,0};

int inxVbufIn=0;
int inxVbufOut=0;
bool is_3x_bufmode=false;
bool is_first_image=true;


void* __not_in_flash_func(v_buf_get_out)()
    {
        if (!is_3x_bufmode|is_first_image) return v_bufs[0];

        if(!is_show_vbuf[(inxVbufOut+1)%3]) 
        {
            is_show_vbuf[inxVbufOut]=true;
            inxVbufOut=(inxVbufOut+1)%3;
            return v_bufs[inxVbufOut];
        }

        if(!is_show_vbuf[(inxVbufOut+2)%3]) 
        {
            is_show_vbuf[inxVbufOut]=true;
            inxVbufOut=(inxVbufOut+2)%3;
            return v_bufs[inxVbufOut];
        }

        return v_bufs[inxVbufOut];

    };
void* __not_in_flash_func(v_buf_get_in)()
    {
        if (!is_3x_bufmode) return v_bufs[0];

        

        if (inxVbufIn>=0)
        {
            inx_frame++;
            if ((inx_frame)>1) is_first_image=false;
            i_frame_vbuf[inxVbufIn]=inx_frame;
            is_show_vbuf[inxVbufIn]=false;
        }
        
        if(is_show_vbuf[(inxVbufIn+1)%3]) 
        {
            inxVbufIn=(inxVbufIn+1)%3;
            return v_bufs[inxVbufIn];
        }

        if(is_show_vbuf[(inxVbufIn+2)%3]) 
        {
            inxVbufIn=(inxVbufIn+2)%3;
            return v_bufs[inxVbufIn];
        }


        return NULL;


    };

void enable_3x_bufmode(){is_3x_bufmode=true;};
void disable_3x_bufmode(){is_3x_bufmode=false;};