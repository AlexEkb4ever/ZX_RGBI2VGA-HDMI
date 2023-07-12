# -*- coding: utf-8 -*-


import PySimpleGUI as sg
#import serial.tools.list_ports
import time

import re
import sys
import glob
import serial


def write_ser(ser,key,value):
    return True

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
#тема по умолчанию 
sg.change_look_and_feel('SystemDefaultForReal') #GreenTan 

#получения списка портов
#dev_TTY=[comport.device for comport in serial.tools.list_ports.comports()]#v1
dev_TTY=serial_ports()#v2
if len(dev_TTY)==0:
    dev_TTY=[""]
    sg.PopupNoButtons("не найдены последовательные порты",auto_close=True)
    #exit(0)
    sys.exit()
    
    

#print([comport.device for comport in serial.tools.list_ports.comports()])

#Draw the button
namesMode=('самосинхронизация', 'внешняя частота','c 6 ноги z80');


comboBoxMode=sg.Combo(namesMode,default_value=namesMode[0], size=(20, 1),key='modeName',readonly=True)
comboBoxPorts=sg.InputCombo(dev_TTY,default_value=dev_TTY[0], size=(40, 1),key='portName',readonly=True)
sliderX=sg.Frame(layout=[[
     sg.Slider((200, 0), orientation='h', enable_events=True, key='SLIDERX')          
    ]],title=" смещение X",title_location=sg.TITLE_LOCATION_TOP,key='settingsBlock')

sliderY=sg.Frame(layout=[[
     sg.Slider((0, 200), orientation='h', enable_events=True, key='SLIDERY')          
    ]],title=" смещение Y",title_location=sg.TITLE_LOCATION_TOP,key='settingsBlock')

sliderF=sg.Frame(layout=[[
     sg.Slider((6000, 9000), orientation='h', enable_events=True, key='-SLIDERF-',size=(250,20))          
    ]],title="внутренняя частота захвата (кГц)",title_location=sg.TITLE_LOCATION_TOP,key='setting_F_Block')

dividerF=sg.Frame(layout=[[
     sg.Slider((1, 5), orientation='h', enable_events=True, key='-SLIDER_f_div-')          
    ]],title=" делитель ",title_location=sg.TITLE_LOCATION_TOP,key='dividerFblock')

slider_VS_LEN=sg.Frame(layout=[[
     sg.Slider((50, 500), orientation='h', enable_events=True, key='-SLIDER_VS_LEN-')          
    ]],title=" Длина КСИ ",title_location=sg.TITLE_LOCATION_TOP,key='settings_VS_LEN_Block')

invBlock=sg.Frame(layout=[[
     sg.CBox('F', size=(3, 1),key='is_inv_F'),    
     sg.CBox('KSI', size=(3, 1),key='is_inv_VS'),  
     sg.CBox('SSI', size=(3, 1),key='is_inv_HS'),  
     sg.CBox('I', size=(3, 1),key='is_inv_I'),  
     sg.CBox('R', size=(3, 1),key='is_inv_R'),  
     sg.CBox('G', size=(3, 1),key='is_inv_G'),  
     sg.CBox('B', size=(3, 1),key='is_inv_B'),  
    ]],title="инвертирование входных сигналов",title_location=sg.TITLE_LOCATION_TOP,key='setting_inv_Block')

sliderDelay0= sg.Slider((0, 31), orientation='h', enable_events=True, key='-SLIDER_DELAY_0-')          
   

sliderDelay1=sg.Frame(layout=[[
     sg.Slider((0, 31), orientation='h', enable_events=True, key='-SLIDER_DELAY_1-')          
    ]],title=" по фронту",title_location=sg.TITLE_LOCATION_TOP,key='-SLIDER_DELAY_BLOCK1-')

sliderDelay2=sg.Frame(layout=[[
     sg.Slider((0, 31), orientation='h', enable_events=True, key='-SLIDER_DELAY_2-')          
    ]],title=" по спаду",title_location=sg.TITLE_LOCATION_TOP,key='-SLIDER_DELAY_BLOCK2-')

syncSelect=sg.Frame(layout=[[sg.Radio('раздельная ', "SYNCMODE", default=True,key="isVS_HS"),
                           sg.Radio('синхросмесь ', "SYNCMODE",key="isVHS")],[sg.Push(),slider_VS_LEN,sg.Push()]],title=" синхронизация ",title_location=sg.TITLE_LOCATION_TOP)


btn1=sg.Button('Применить', size=(10,1),key="btn1Click")

block0=[
         [sg.Frame(layout=[[sg.Radio('VGA', "VIDEOMODE", default=True, size=(5, 1),key="isVGAout",disabled=False),
                           sg.Radio('HDMI ', "VIDEOMODE",key="isHDMIout",disabled=False)],[
                           sg.Radio('RGB', "VIDEOMODE",  size=(5, 1),key="isRGBout",disabled=True),
                           sg.Radio('COMP', "VIDEOMODE",  key="isCOMPout",disabled=True)],
                           [
                               sg.Push(),sg.Frame(layout=[[sg.CBox('широкоэкранный', key='is_widescreen',disabled=True)]], title=" пропорции ",title_location=sg.TITLE_LOCATION_TOP,key="WIDE_MODE_BLOCK"),sg.Push()],[                            
                               
                            sg.Frame(layout=[[sg.Radio(' PAL(50Гц) ', "SYS_COLOR_MODE", default=True,key="isPAL",disabled=False),
                                              sg.Radio(' NTSC(60Гц) ', "SYS_COLOR_MODE",key="isNTSC",disabled=False)]], title=" стандарт ",title_location=sg.TITLE_LOCATION_TOP,key="C_MODE_BLOCK")                               
                               ]], title=" видеовыход ",title_location=sg.TITLE_LOCATION_TOP),
          sg.Frame(layout=[[sg.Radio(' 1X ', "BUFMODE", default=True, size=(5, 1),key="is1X_BUFMODE",disabled=False),
                                     sg.Radio(' 3X ', "BUFMODE",key="is3X_BUFMODE",disabled=False)]], title=" буферизация ",title_location=sg.TITLE_LOCATION_TOP)          
          ],
    
         [sg.Frame(layout=[[comboBoxPorts]],title=" выбор порта",title_location=sg.TITLE_LOCATION_TOP,key='portNameBlock')]]

block1=[[
         syncSelect,
         sg.Frame(layout=[[comboBoxMode],[dividerF]],title=" источник пиксельклока",title_location=sg.TITLE_LOCATION_TOP)],
        
        
        [sliderF]
        ]

block2=[[sg.Frame(layout=[
    
  
    [sg.Push(),sliderX, sliderY,sg.Push()]       
   
    ,
    [sg.Push(),sg.Frame(layout=[[sliderDelay1,sliderDelay2],[sg.Push(),sliderDelay0,sg.Push()]],title=" задержка захвата",title_location=sg.TITLE_LOCATION_TOP,key='delay_cap_block'),sg.Push()]
    ,
    [sg.Push(),invBlock,sg.Push()]
    ,
    
     #[sg.Push(),sg.Column( layout=[[sliderF]] , element_justification='c'),sg.Push()]
    
    ],title="",title_location=sg.TITLE_LOCATION_TOP,key='settingsBlock')]]


blockBTN=[[btn1]]


layout = [
     [sg.Push(),sg.Column( layout=block0 , element_justification='c'),sg.Push()]
     
    ,    
     [sg.Push(),sg.Column( layout=block1 , element_justification='c'),sg.Push()]
        
    ,
     [sg.Push(),sg.Column( layout=block2 , element_justification='c'),sg.Push()]
    ,    
    
     [sg.Push(),sg.Column( layout=blockBTN , element_justification='c'),sg.Push()]
    ,
    
        ]

#Draw the window

window = sg.Window('настройки RGB to VGA & HDMI', layout,font=('',12),margins=(10,0), size=(600,650), default_element_size=(40, 1),scaling=True, grab_anywhere=False)
#window.set_icon(red_icon)
#window.set_icon(r'/home/alex/prj/python/GUI/01.ico')

ser=None
oldSerName=None
itst=0
x_sh='0'
y_sh='0'
l_mode='read'
while True:
    event, values = window.read(timeout=100)
    if event is None:
        break
    
    #проверка подключения последовательного порта
    #print(values)
    
    xnew=int(values['SLIDERX'])
    ynew=int(values['SLIDERY'])    
    
    if event=='__TIMEOUT__':
        #смена последовательного порта
        if (comboBoxPorts.get()!=oldSerName):
            if l_mode!='write':
                l_mode='read'
            ser_name=comboBoxPorts.get()
            if ser==None:
                while ser==None:
                    try:
                        ser=serial.Serial(ser_name,115200, timeout=0.001)
                        oldSerName=ser_name
                        
                        
                    except:
                        ser=None
                        print("create serial "+ser_name+" error")
                        time.sleep(1)
                        
            else:
                ser.close()
                ser=serial.Serial(ser_name,115200, timeout=0.001)
                oldSerName=ser_name            
         
               
       
            
       
    
    
        
    
    #print(event)
    #сохранение всего блока данных
    if event=='btn1Click':
        #print("Saving data")
        sg.PopupNoButtons("перезагрузка платы ждите....",auto_close=True)
                
        l_mode='write'
    #print(btn1.tex)
    #print(values)
    
    #выбор режима
    modeName=window['modeName'].get()
    
    #print(modeName)
    if modeName==namesMode[0]: #самосинхронизация
        window['dividerFblock'].hide_row()        
        window['setting_F_Block'].unhide_row()
        window['-SLIDER_DELAY_0-'].unhide_row() 
        window['-SLIDER_DELAY_BLOCK1-'].hide_row()
        window['-SLIDER_DELAY_BLOCK2-'].hide_row()
        
    if modeName==namesMode[1]: #внешняя частота
        window['dividerFblock'].unhide_row()
        window['setting_F_Block'].hide_row()        
        window['-SLIDER_DELAY_0-'].unhide_row()        
        window['-SLIDER_DELAY_BLOCK1-'].hide_row()
        window['-SLIDER_DELAY_BLOCK2-'].hide_row()
    if modeName==namesMode[2]: #c 6 ноги z80
        window['dividerFblock'].hide_row()        
        window['setting_F_Block'].hide_row()
        window['-SLIDER_DELAY_0-'].hide_row()        
        window['-SLIDER_DELAY_BLOCK1-'].unhide_row()
        window['-SLIDER_DELAY_BLOCK2-'].unhide_row()
    
    if window['isHDMIout'].get():
        window['C_MODE_BLOCK'].hide_row()
        window['WIDE_MODE_BLOCK'].unhide_row()
        
    if window['isVGAout'].get():
        window['C_MODE_BLOCK'].hide_row() 
        window['WIDE_MODE_BLOCK'].unhide_row()
        
    if window['isRGBout'].get():
        window['C_MODE_BLOCK'].unhide_row()
        window['WIDE_MODE_BLOCK'].hide_row()
        
    if window['isCOMPout'].get():
        window['C_MODE_BLOCK'].unhide_row()  
        window['WIDE_MODE_BLOCK'].hide_row()
        
        
        
    if window['isVHS'].get():
        window['settings_VS_LEN_Block'].unhide_row()
    if window['isVS_HS'].get():
        window['settings_VS_LEN_Block'].hide_row()        
   
    #работа с последовательным портом
    if event=='__TIMEOUT__':
        if ser!=None:
            try: 
                #проверка режима железа
                ser.write("mode\n".encode())
                time.sleep(0.1)
                hw_mode=str(ser.readall())
                #print(hw_mode)
                
                #mode 0 - режим записи и считывания значений
                if (hw_mode.find("mode 0")>0):
                    print("hw mode 0")
                    #желаемый режим
                   
                    #сохранение в чип
                    if l_mode=='write':
                        sg.PopupNoButtons("сохранение параметров",auto_close=True)
                       
                                               
                       
                        ser.write(("wcap_sh_x "+str(int(values['SLIDERX']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()
                        
                        ser.write(("wcap_sh_y "+str(int(values['SLIDERY']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()     
                        
                        ser.write(("wcap_ext_f_div "+str(int(values['-SLIDER_f_div-']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()    
                        
                        ser.write(("wcap_int_f "+str(int(values['-SLIDERF-']))+"000\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()      
                        
                        ser.write(("wcap_delay "+str(int(values['-SLIDER_DELAY_0-']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()                         
                        
                        ser.write(("wcap_delay_rise "+str(int(values['-SLIDER_DELAY_1-']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()  
                        
                        ser.write(("wcap_delay_fall "+str(int(values['-SLIDER_DELAY_2-']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()   
                        
                        ser.write(("wcap_len_VS "+str(int(values['-SLIDER_VS_LEN-']))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()
                        
                        iD=0
                        if values['isHDMIout']:
                            iD=1
                        if values['isRGBout']:
                            iD=2
                        if values['isCOMPout']:
                            iD=3
                        ser.write(("wvideo_out "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()
                        
                        
                        iD=0
                        if values['isNTSC']:
                            iD=1
                        
                        ser.write(("wc_mode "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()                        
                        
                        
                        
                        iD=0
                        if values['is3X_BUFMODE']:
                            iD=1
                        ser.write(("wis_3X_bufmode "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()                        
                        
                        iD=0
                        if values['isVHS']:
                            iD=1
                        ser.write(("wcap_sync_mode "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall() 
                        
                        iD=0
                        if values['is_widescreen']:
                            iD=1
                                                        
                        ser.write(("wwide_mode "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()                         
                 
                        iD=0
                        if values['modeName']==namesMode[1]:
                            iD=1
                        if values['modeName']==namesMode[2]:
                            iD=2                              
                          
                        ser.write(("wcap_p_clk_mode "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall() 
                        
                        iD=0
                        if values['is_inv_F']:
                            iD|=1
                        iD<<=1                         
                        if values['is_inv_VS']:
                            iD|=1
                        iD<<=1 
                        if values['is_inv_HS']:
                            iD|=1
                        iD<<=1 
                        if values['is_inv_I']:
                            iD|=1
                        iD<<=1 
                        if values['is_inv_R']:
                            iD|=1
                        iD<<=1 
                        if values['is_inv_G']:
                            iD|=1
                        iD<<=1 
                        if values['is_inv_B']:
                            iD|=1
                        ser.write(("wcap_in_inv_mask "+str(int(iD))+"\n").encode())
                        time.sleep(0.01)                        
                        ser.readall()                         
                        
                        print("saving data")
                        l_mode='user_mode'
                        ser.write("save\n".encode())
                        continue                        
                    #чтение из чипа
                    if l_mode=='read':
                        #показать окно ожидания загрузки 
                        sg.PopupNoButtons("загрузка параметров",auto_close=True)
                        #msg=sg.popup("загрузка параметров",no_titlebar=False,button_type=None,auto_close=True)
                       
                        #установка параметров на форму
                        ser.write("rcap_sh_x\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['SLIDERX'].update(value=i_data)
                        
                        ser.write("rcap_sh_y\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['SLIDERY'].update(value=i_data)  
                        
                        ser.write("rcap_ext_f_div\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDER_f_div-'].update(value=i_data)                                    
                        
                        ser.write("rcap_int_f\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDERF-'].update(value=i_data/1000) 
                        
                        ser.write("rcap_delay\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDER_DELAY_0-'].update(value=i_data)                         
                        

                        ser.write("rcap_delay_rise\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDER_DELAY_1-'].update(value=i_data)                         
                        

                        ser.write("rcap_delay_fall\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDER_DELAY_2-'].update(value=i_data)                         
                         
                        ser.write("rcap_len_VS\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['-SLIDER_VS_LEN-'].update(value=i_data)                             
                        
                        ser.write("rwide_mode\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        if i_data==1:
                            window['is_widescreen'].update(True)    
                        else:
                            window['is_widescreen'].update(False)    

                        
                        
                        ser.write("rvideo_out\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        if i_data==0:
                            window['isVGAout'].update(True)
                        if i_data==1:
                            window['isHDMIout'].update(True)        
                        if i_data==2:
                            window['isRGBout'].update(True)    
                        if i_data==3:
                            window['isCOMPout'].update(True) 
                        
                        ser.write("rc_mode\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        if i_data==0:
                            window['isPAL'].update(True)
                        if i_data==1:
                            window['isNTSC'].update(True)                          
                        
                        
                        ser.write("ris_3X_bufmode\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        print("s="+s1+" buf mode="+str(i_data))
                        if i_data==30:
                            window['is1X_BUFMODE'].update(True)
                        if i_data==31:
                            window['is3X_BUFMODE'].update(True)                              
                        
                        
                        ser.write("rcap_sync_mode\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        if i_data==0:
                            window['isVS_HS'].update(True)
                        if i_data==1:
                            window['isVHS'].update(True)                             
                        
                        
                        ser.write("rcap_p_clk_mode\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['modeName'].update(namesMode[i_data])
                        
                        ser.write("rcap_in_inv_mask\n".encode())
                        time.sleep(0.01)
                        s1=str(ser.readall())
                        i_data = int(''.join(x for x in s1 if x.isdigit()))
                        window['is_inv_B'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_G'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_R'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_I'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_HS'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_VS'].update(i_data&1)                        
                        i_data>>=1
                        window['is_inv_F'].update(i_data&1)                        
                                                                
                                                       
                        
                        l_mode='user_mode'
                        ser.write("exit\n".encode())
                        continue
                       
                        
                #mode 1 - можно менять лишь смещение        
                if (hw_mode.find("mode 1")>0):
                    print("hw mode 1")  
                     
                    
                    if l_mode!='user_mode':
                        #если находимся не в том режиме - рестартуем
                        print("restart hw")
                        ser.write("reset\n".encode())
                        time.sleep(0.1)                    
                        continue
                    if x_sh!=xnew:
                        st1="wcap_sh_x "+str(xnew)
                        #print(st1)
                        ser.write(st1.encode())  
                        x_sh=xnew
                        time.sleep(0.1)
                        print(str(ser.readall()))
                        #print(" test "+st1)
                    
                    if y_sh!=ynew:
                   
                   
                        st1="wcap_sh_y "+str(ynew)
                        ser.write(st1.encode())  
                        y_sh=ynew
                        time.sleep(0.1)                
                        print(str(ser.readall()))
                    #print(" test "+st1)
               
            except:
                print("error serial")
                ser.close()
                time.sleep(0.1)
                ser=None
                oldSerName=None
            
      
        
        
