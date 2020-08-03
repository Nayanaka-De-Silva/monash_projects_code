/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <stdio.h>
#include <stdarg.h>
#include <project.h>

int All;
int Tank_A_sw;  
int Tank_B_sw;
int Tank_C_sw;
int flag_A =0;
int flag_B = 0;
int flag_C = 0;
int j = 0;
int t = 0;
int pump = 0;
int flag_end = 0;
int flag_start = 0;
int endpoint_flag =0;
int timer_flag = 0;
int timer_endpoint_flag = 0;
uint8 buff[64];
int initial_flag =1 ;
uint16 IR;
int A;
int etch_devel_flag = 0;
int Time;
uint8 i;
uint8 out_buff[8];
char c[5];

// For testing with led
void test()
{
    led_Write(1); 
    CyDelay(1000);
    led_Write(0);
    CyDelay(1000);
}

// LCD printing
void printlcd(const char *fmt, ...)
{
    char buffer[32];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    LCD_Position(0, 0);
    LCD_PrintString(buffer);

}

// Move the arm in Y axis up
 void forward_X()
{
    Direc_X_Write(0);
    PWM_1_Start();
    PWM_1_WriteCompare(12);
}

// Move the arm horizontaly up
void forward_Y()
{
    Direc_Y_Write(0);
    PWM_2_Start();
    PWM_2_WriteCompare(12);     
}

// Move the arm horizontaly down
void backward_Y()
{
    Direc_Y_Write(1);
    PWM_2_Start();
    PWM_2_WriteCompare(12);
}

// Move the arm vertically down
void backward_X()
{
    Direc_X_Write(1);
    PWM_1_Start();
    PWM_1_WriteCompare(12);
}

// hold position vertically
void hold_x()
{
    PWM_1_Start();
    PWM_1_WriteCompare(0);
}

// Hold position horizontally
void hold_y()
{
    PWM_2_Start();
    PWM_2_WriteCompare(0);
}

// Open the arm gripper
void gripper_open ()
{
    Gripper_A_Write(1);
    CyDelay(3000);
    Gripper_A_Write(0);
}

// Close the arm gripper
void gripper_close()
{
    Gripper_B_Write(1);
    CyDelay(5000);
    Gripper_B_Write(0);
}

    

void Tank_A()
{
    if ( All && !flag_A)            // All process and the arm triggers IRA for the first time
    { 
        flag_end = 0;
        initial_flag=0;             // Enables the Microswitch down to work
        etch_devel_flag =1;         // Etching or developing takes place in these tanks
        hold_y();
        backward_X();
        CyDelay(6300);
        PWM_1_Stop();
        flag_A = 1;                 // Prevent the arm entering the same tank again 
        
    }
    
    else if (All && flag_A)         // All process and the the arm triggers IRA for the second time
    {
        etch_devel_flag= 0;
        PUMP_4_Write(1);            // turn on the output pump for tank A
        PUMP_5_Write(1);            // turn on the output pump for tank B
        PUMP_6_Write(1);            // turn on the output pump for tank C
        
        CyDelay(300);               // delay to position the IR back to correct position
        flag_A = 1;                 // Process in Tank A completed
        flag_B = 1;                 // Process in Tank B completed
        flag_C = 1;                 // Process in Tank C completed
        
        hold_x();
        backward_Y();
    }
    
    else if (Tank_A_sw && !flag_A)  // Tank A only process and IRA triggers for the first time
    {
        etch_devel_flag =1;         // Etching or devlopment takes place in these tanks
        hold_y();
        backward_X();
        CyDelay(6300);
        PWM_1_Stop();
        flag_A = 1;                 // Process in Tank A completed
        initial_flag = 0;           // Enables the Micro Switch down
        
        PUMP_1_Write(0);            // turn off the input pump for Tank A
        pump=1;                     // Prevent the pump from turning on again
    }
    
    else if (Tank_A_sw && flag_A)   // Tank A only process and IRA triggers for the second time
    {
        etch_devel_flag= 0;         // Etching or development already took place in these tanks
        PUMP_6_Write(1);            // Turn off the output pump for tank A
        pump = 1;                   // Prevent the pump from turning on again
        
        CyDelay(300);
        hold_x();
        backward_Y();
        CyDelay(3500);
        PWM_2_Stop();
        
        flag_B = 1;                 // Process in Tank B completed
        flag_C = 1;                 // Process in Tank C completed
    }
    isr_tank_A_ClearPending(); 
}

void Tank_B()
{
    if(All && !flag_B)              // All process and IRB triggers for the first time
    {
        hold_y();
        backward_X();
        CyDelay(6300);
        PWM_1_Stop();
        flag_B = 1;                 // Process in Tank B completed
        initial_flag = 0;           // Enables the Micro Switch down
        
    }
    
    else if (Tank_B_sw && !flag_B)  // Tank B only process and IRB triggers for the first time
    {
        hold_y();
        backward_X();
        CyDelay(8000);
        PWM_1_Stop();
        flag_B = 1;                 // Process in Tank B completed
        initial_flag = 0;           // Enables the Micro Switch down
        
        PUMP_2_Write(0);            // Turn off the input pump for tank B
        pump=1;                     // Prevent the pump from turning on again
    
    }
    
    else if (Tank_B_sw && flag_B)   // Tank B only process and IRB triggers for the second time
    {
        PUMP_5_Write(1);            // Turn off the output pump for tank B
        pump =1;                    // Prevent the pump from turning on again
        
        flag_B = 1;                 // Process in Tank B completed
        flag_C = 1;                 // Process in Tank C completed
        
        hold_x();
        CyDelay(1000);
        backward_Y();      
    }
    
    isr_tank_B_ClearPending();
}

void Tank_C()
{
    if (All && !flag_C)             // All process and IRC triggers for the first time
    {
        etch_devel_flag = 1;        // Etching or developing takes place in these tanks
        
        if (!flag_start)            // IRC triggering for the first time at the start
        {
            initial_flag = 0;       // Enables the Micro Switch down
            hold_x();
            hold_y();
            CyDelay(5000);
            gripper_open();
            CyDelay(4000);
            gripper_close();
            CyDelay(4000);
        }
          
        hold_y();
        
        backward_X();
        CyDelay(5200);
        PWM_1_Stop();
        flag_C = 1;                 // Process in Tank C completed 
        
        PUMP_3_Write(0);            // Turn off the input pump for tank C
        PUMP_2_Write(0);            // Turn off the input pump for tank B
        PUMP_1_Write(0);            // Turn off the input pump for tank B
        pump=1;                     // Prevent the pump from turning on again
    }
    
    
    
    else if ((All||Tank_A||Tank_B||Tank_C) && flag_C && flag_B)  // All process or only Tank A or only Tank B or only
    {                                                            // Tank C and process in Tank B and Tank C completed
        PWM_2_Stop();
        etch_devel_flag= 0;         // Etching or developing already took place
        
        PUMP_6_Write(0);            // Turn off the output pump for tank C
        PUMP_5_Write(0);            // Turn off the output pump for tank B
        PUMP_4_Write(0);            // Turn off the output pump for tank A
        hold_y();
        CyDelay(10000);
        pump =1 ;                   // Prevent the pump from turning on again
    }
    
    else if(All && flag_C)
    {
        etch_devel_flag= 0;         // Etching or developing already took place
    }
    
    else if (Tank_C_sw && !flag_C)  // Tank C only process and IRC triggers for the first time
    {
        etch_devel_flag = 1;        // Etching or developing takes place in these tanks
        hold_y();
        backward_X();
        CyDelay(8000);
        PWM_1_Stop();
        flag_C = 1;                 // Process in Tank C completed 
        initial_flag = 0;           // Enables the Micro Switch down
        
        PUMP_3_Write(0);            // Turn off the input pump for tank C
        pump =1 ;                   // Prevent the pump from turning on again 
    }
    
    else if (Tank_C_sw && flag_C)   // Tank C only process and IRC triggers for the second time
    {
        etch_devel_flag=0;          // Etching or developing already took place
        PUMP_4_Write(1);            // turn off the output pump for tank C
        //PUMP_5_Write(0);          // turn off the output pump for tank B
        //PUMP_4_Write(0);          // turn off the output pump for tank A
        pump =1 ;
        hold_x();
        hold_y();
        CyDelay(5000);
        PUMP_6_Write(0);
        CyDelay(10000);
    }
    
    isr_tank_C_ClearPending();
}

 void horizontal()
{
    backward_X();
    CyDelay(250);
    PWM_1_Stop();
    hold_x();
    CyDelay(1000);
    
    if ((All && flag_A && flag_B && flag_C )||(Tank_A_sw && flag_A)||(Tank_B_sw && flag_B )||(Tank_C_sw && flag_C) )
    {
        backward_Y();
    }
    
    else
    {
        forward_Y();
    }

    Micro_up_isr_ClearPending();
}


void endpointtime()
{
    timer_endpoint_flag = 1;
    forward_X();
    
}
void rinse()
{
    timer_isr_etch_SetPriority(0);
    if(initial_flag)
    {
        
    }
    
    else if (etch_devel_flag)
    {
        if(endpoint_flag)
        {
            while(!flag_end)
            {
                forward_X();
            CyDelay(3800);
            PWM_1_Stop();
            ADC_Start();
            IR = ADC_Read16();
            A =ADC_CountsTo_mVolts(IR);
            sprintf(c , "%i" , A);
            //uint16 iii;
           
       //for(iii=0;iii<sizeof c;iii++)
		//{
			//while(!USB_CDCIsReady()); //wait until USB is ready
			//USB_PutData((uint8*)c,5); //send next 64 byte packet (maximum size)
		//}
            printlcd("%s",c);
            CyDelayUs(10);
            if (A >= 4150000)
            {
            flag_end = 1 ;
                forward_X();
                
            }
            else {
            
            backward_X();
            CyDelay(3800);
            PWM_1_Stop();
            }
            }
        
    }
    
            
            
    
    else if (timer_flag)
    {
         
        Timer_etch_Start();
        Timer_etch_WriteCounter(Time);
        
        while(!timer_endpoint_flag)
        {
           
            forward_X();
            CyDelay(3500);
            PWM_1_Stop();
            backward_X();
            CyDelay(3500);
            PWM_1_Stop();
            int b= Timer_etch_ReadCounter();
            printlcd("%i",b);
            CyDelayUs(10);
            
        }
        
        forward_X();
    }
    }
    
    else
    {
        forward_X();
    }
}

void display(const char *x) 
{
    while(USB_CDCIsReady() == 0u);
    USB_PutString(x);
    LCD_ClearDisplay();
    printlcd(x);
}
    
        
    
    



int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

  CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
  USB_Start(0,USB_DWR_VDDD_OPERATION);
  while (!USB_GetConfiguration());
  USB_CDC_Init();
  LCD_Start();
  ADC_Start();

        
  isr_tank_A_StartEx(Tank_A);
  isr_tank_B_StartEx(Tank_B);
  isr_tank_C_StartEx(Tank_C);
  Micro_up_isr_StartEx(horizontal);
  Micro_down_isr_StartEx(rinse);

  timer_isr_etch_StartEx(endpointtime);




    for(;;)
    {
        
        
      if(USB_DataIsReady() != 0u)
        {
            uint8 tempBuff[16];
            uint8 count = USB_GetCount();
            USB_GetData(tempBuff,count);
            
            for (j=0; j<count; j++)
            {
                buff[j] = tempBuff[j];
            }
             //display("Ready!");
            switch (buff[0]) 
            {
                case '1': 
                PUMP_1_Write(1);
                PUMP_2_Write(1);
                PUMP_3_Write(1);
                break;
                
                case '2': 
                All=1;
                break;
                
                case '3': 
                Tank_A_sw = 1;
                break;
                
                case '4': 
                Tank_B_sw = 1;
                break;
                
                case '5': 
                Tank_C_sw = 1;
                break;
            
                case '6': 
                led_Write(0);
                CyDelay(2000);
                break;
                
                case '7': 
                gripper_open();
                break;
                
                case '8': 
                gripper_close();
                break;
                
                case '9': 
                PUMP_5_Write(1);
                PUMP_4_Write(1);
                PUMP_6_Write(1);
                break;
                
                case 'a': 
                PUMP_5_Write(0);
                PUMP_4_Write(0);
                PUMP_6_Write(0);
                break;
                
                case 'b': 
                PUMP_1_Write(0);
                PUMP_2_Write(0);
                PUMP_3_Write(0);
                break;  
            
                case 'c': 
                forward_X();
                break;
                
                case 'd': 
                hold_x();
                hold_y();
                break; 
                
                case 'e': 
                hold_x();
                hold_y();
                break;
                
                case 'f': 
                endpoint_flag = 1;
                break;
                
                case 'g': 
                t =(buff[1]-48)*10+(buff[2]-48);
                Time = t*100;
                printlcd("%i",Time);
                CyDelayUs(10);
                timer_flag =1;
                break; 
            }
        }
        
        /*All = Sequen_pin_Read();
        Tank_A_sw = Tank_A_pin_Read();
        Tank_B_sw = Tank_B_pin_Read();
        Tank_C_sw = Tank_C_pin_Read();*/
        
        if(!All&&!pump)
        {
            if (Tank_A_sw)
            {
                PUMP_1_Write(1);
            }
            
            else if(Tank_B_sw)
            {
                PUMP_2_Write(1);
            }
            
            else if (Tank_C_sw)
            {
                PUMP_3_Write(1);
            }
        }
        
        else if (!pump)
        {
            PUMP_1_Write(1);
            PUMP_2_Write(1);
            PUMP_3_Write(1);
        }
    }
}
