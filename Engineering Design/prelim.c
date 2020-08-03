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
#include "project.h"
#define SPEED 35
#define DEGREES_L 56
#define DEGREES_R 56.1
#define DISTANCE 300
#define COLLISION_DISTANCE 7
#define EVASIVE_HORIZONTAL_DISTANCE 35
#define EVASIVE_VERTICAL_DISTANCE 50
#define PUCK_DISTANCE 4
#define MAX_FREQ_READINGS 20
#define PUCK_OFFSET 150
#define AFTER_DETECTION 17
#define SYNC_RANGE 10
#define CORRECTION 5

//global variables
int speedR = SPEED;
int speedL = SPEED;
int16 countR;
int16 countR_Store;
int16 countL;
int16 countL_Store;
int turnLeft = 0;
int turnRight = 0;
int moveForward = 0;
int returning=0;

void MotorCmd(char inst);
int TurnRight(int16 cR, int16 cL, int degrees);
int TurnLeft(int32 cR,int32 cL, int degrees);
int MoveForward(int32 countR, int32 countL, int distance);
void countReset();
void sync();

int obstacle = 0;

int16 horizontal_distance = 0;
int16 vertical_distance = 0;

//Collision Detection Interrupt Handlers
uint16 count1 = 0;
int distance_measured1 = 0;
CY_ISR(Timer_1_Handler){
    Timer_1_ReadStatusRegister();
    count1 = Timer_1_ReadCounter();
    distance_measured1 = (65535-count1)/58;
    if(distance_measured1 < COLLISION_DISTANCE ){
        if(returning!=1){
            obstacle = 1;
        }
    }
    else if(distance_measured1 >= COLLISION_DISTANCE){
        
    }
}

uint16 count2 = 0;
int distance_measured2 = 0;
CY_ISR(Timer_2_Handler){
    Timer_2_ReadStatusRegister();
    count2 = Timer_2_ReadCounter();
    distance_measured2 = (65535-count2)/58;
    if(distance_measured2 < COLLISION_DISTANCE ){
        if(returning!=1){
            obstacle = 1;
        }
    }
    else if(distance_measured2 >= COLLISION_DISTANCE){
        
    }
}

//Puck Detection Interrupt Handlers
uint16 count3 = 0;
int distance_measured3 = 0;
int puck_detected = 0;
CY_ISR(Timer_3_Handler){
    Timer_3_ReadStatusRegister();
    count3 = Timer_3_ReadCounter();
    distance_measured3 = (65535-count3)/58;
    if(distance_measured3 < PUCK_DISTANCE ){
        if(puck_detected != 2&&returning!=1){
            puck_detected=1;
        }
    }
    else if(distance_measured2 >= PUCK_DISTANCE){
        
    }
}

//Color Detection Interrupt Handlers
int color_compare = 0;
CY_ISR(Color_Handler){
    PWM_Window_ReadStatusRegister();
    color_compare=1;
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    //Locomotion System Initialization
    PWM_R_Start();
    PWM_L_Start();
    QuadDec_R_Start();
    QuadDec_L_Start();
    int command = 0;
    int command1 = 0;
    int command2 = 0;
    int E_State = 0;
    int N_State = 0;
    int avoided=0;
    
    //Collision Detection System Initialization
    Timer_1_Start();
    isr_1_StartEx(Timer_1_Handler);
    Timer_2_Start();
    isr_2_StartEx(Timer_2_Handler);
    S2_Write(0);
    S3_Write(0);
    int16 count;
    int reading = 0;
    int freq[MAX_FREQ_READINGS];
    int mean[3];
    int meanCount=0;
    
    //Puck Detection System Initialization
    Timer_3_Start();
    isr_3_StartEx(Timer_3_Handler);
    
    //Color Detection System Initialization
    PWM_Window_Start();
    Counter_Start();
    ISR_Compare_StartEx(Color_Handler);
    
    for(;;)
    {
        countR = QuadDec_R_GetCounter();
        countL = QuadDec_L_GetCounter();
        
        if(command==0){
            command=TurnRight(countR,countL,90);
        }
        else{
            CyDelay(1000);
            command=0;
        }
        
        //Prelim Normal operation code
        /*
        if(obstacle==1&&N_State<8&&avoided==0){
            switch(E_State){
                case 0:
                    MotorCmd('S');
                    CyDelay(1000);
                    E_State++;
                    command=0;
                break;
                case 1:
                    if(command==0){
                        command = TurnLeft(countR,countL,90);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 2:
                    if(command==0){
                        command = MoveForward(countR,countL,EVASIVE_HORIZONTAL_DISTANCE);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 3:
                    if(command==0){
                        command = TurnRight(countR,countL,90);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 4:
                    if(command==0){
                        command = MoveForward(countR,countL,EVASIVE_VERTICAL_DISTANCE);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 5:
                    if(command==0){
                        command = TurnRight(countR,countL,90);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 6:
                    if(command==0){
                        command = MoveForward(countR,countL,EVASIVE_HORIZONTAL_DISTANCE-4);
                    }
                    else{
                        E_State++;
                        command=0;
                    }
                break;
                case 7:
                    if(command==0){
                        command = TurnLeft(countR,countL,90);
                    }
                    else{
                        E_State=0;
                        command=0;
                        obstacle=0;
                        avoided=1;
                    }
                break;
            }
        }
        else{
            if(puck_detected==0){
                switch(N_State){
                case 0:
                    MotorCmd('F');
                break;
                case 1:
                    if(command1==0){
                        command1=TurnLeft(countR,countL,90);
                    }
                    else{
                        command1=0;
                        N_State++;
                    }
                break;
                case 2:
                    MotorCmd('F');
                break;
                case 3:
                    if(command1==0){
                        command1=TurnRight(countR,countL,90);
                    }
                    else{
                        command1=0;
                        N_State++;
                    }
                break;
                case 4:
                    if(avoided==1&&obstacle==1){
                        //go to return state
                        N_State++;
                        command1=0;
                    }
                    else{
                        MotorCmd('F');
                    }
                break;
                case 5:
                    if(command1==0){
                        command1=TurnRight(countR,countL,90);
                    }
                    else{
                        command1=0;
                        N_State++;
                        countReset();
                        returning=1;
                        obstacle=0;
                    }
                break;
                case 6:
                    if(command1==0){
                        command1 = MoveForward(countR,countL,((horizontal_distance)/DISTANCE)+AFTER_DETECTION);
                    }
                    else{
                        N_State++;
                        command1=0;
                    }
                break;
                case 7:
                    if(command1==0){
                        command1 = TurnRight(countR,countL,90);
                    }
                    else{
                        command1=0;
                        N_State++;
                        obstacle=0;
                        
                    }
                break;
                case 8:
                    if(obstacle==0){
                        MotorCmd('F');
                    }
                    else{
                        N_State++;
                        countReset();
                    }
                break;
                default:
                    MotorCmd('S');
                break;
                }
            }
            else if(puck_detected==1){
                CyDelay(PUCK_OFFSET);
                MotorCmd('S');
                if(N_State==0||N_State==4){
                    vertical_distance += countL;
                    countReset();
                }
                else if(N_State==2){
                    horizontal_distance += countL;
                    countReset();
                }
            }
            else if(puck_detected==2){
                if(command2==0){
                    command2 = MoveForward(countR,countL,AFTER_DETECTION);
                }
                else{
                    command2=0;
                    puck_detected=0;
                    N_State++;
                }
            }
        }
        
        //Color Detection Code
        if(color_compare==1&&puck_detected==1){
            count = 100*Counter_ReadCounter();
            if(reading<MAX_FREQ_READINGS){
                freq[reading] = count;
                reading++;
            }
            else{
                int sum=0;
                for(int i = 0;i<MAX_FREQ_READINGS;i++){
                    sum+=freq[i];
                }
                mean[meanCount] = sum/MAX_FREQ_READINGS;
                if(meanCount<2){
                    switch(meanCount){
                        case 0://If the current value that's being read is RED
                            S2_Write(1);    //Change it to read GREEN
                            S3_Write(1);
                        break;
                        case 1://If the current value that's being read is GREEN
                            S2_Write(0);    //Change it to read Blue
                            S3_Write(1);
                        break;
                    }
                    meanCount++;
                }
                else{
                    //N.B The color code for mean is
                    //mean[0] = RED
                    //mean[1] = GREEN
                    //mean[2] = BLUE
                    int color;
                    //If RED is greaeter than GREEN
                    if(mean[0]>mean[1]){
                        //If RED is greater than BLUE
                        if(mean[0]>mean[2]){
                            //RED is the most dominant color.
                            color = 0;
                        }
                        else{
                            //BLUE is the most dominant color.
                            color = 2;
                        }
                    }
                    //Else GREEN is greater than RED
                    else{
                        //If GREEN is greater than BLUE
                        if(mean[1]>mean[2]){
                            //GREEN is the most dominant color
                            color = 1;
                        }
                        else{
                            //BLUE is the most dominant color
                            color = 2;
                        }
                    }
                    switch(color){
                        case 0:
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        break;
                        case 1:
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        CyDelay(100);
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        break;
                        case 2:
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        CyDelay(100);
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        CyDelay(100);
                        LED_Write(1);
                        CyDelay(100);
                        LED_Write(0);
                        break;
                        default:
                        LED_Write(0);
                        break;
                    }
                    //Reset the values
                    meanCount=0;
                    color=0;
                    S2_Write(0);
                    S3_Write(0);
                    puck_detected=2;
                }
                reading=0;
            }
            color_compare=0;
        }*/
        
        //Collision Detection While Loops
        /*
        while(Sensor1_Echo_Read()==0){
            Sensor1_Trigger_Write(1);
            CyDelayUs(10);
            Sensor1_Trigger_Write(0);
        }
        CyDelay(10);
        
        while(Sensor2_Echo_Read()==0){
            Sensor2_Trigger_Write(1);
            CyDelayUs(10);
            Sensor2_Trigger_Write(0);
        }
        CyDelay(10);
        
        //Puck Detection While Loops
        while(Puck_Echo_Read()==0){
            Puck_Trigger_Write(1);
            CyDelayUs(10);
            Puck_Trigger_Write(0);
        }
        CyDelay(10);
        */
    }
}

void MotorCmd(char inst){
    {
    switch(inst){
        case 'F':
        sync();
        PWM_R_WriteCompare1(0);
        PWM_R_WriteCompare2(speedR);
        PWM_L_WriteCompare1(0);
        PWM_L_WriteCompare2(speedL);
        break;
        case 'B':
        PWM_R_WriteCompare1(speedR);
        PWM_R_WriteCompare2(0);
        PWM_L_WriteCompare1(speedL);
        PWM_L_WriteCompare2(0);
        break;
        case 'S':
        PWM_R_WriteCompare1(0);
        PWM_R_WriteCompare2(0);
        PWM_L_WriteCompare1(0);
        PWM_L_WriteCompare2(0);
        break;
        case 'R':
        speedL=SPEED+3;
        speedR=SPEED;
        PWM_R_WriteCompare1(speedR);
        PWM_R_WriteCompare2(0);
        PWM_L_WriteCompare1(0);
        PWM_L_WriteCompare2(speedL);
        break;
        case 'L':
        speedL=SPEED+3;
        speedR=SPEED;
        PWM_R_WriteCompare1(0);
        PWM_R_WriteCompare2(speedR);
        PWM_L_WriteCompare1(speedL);
        PWM_L_WriteCompare2(0);
        break;
    }
}
}
int TurnRight(int16 cR,int16 cL, int degrees){
    if(turnRight==0){
        //This checks whether the function is running for the first time.
        countR_Store = cR;
        countL_Store = cL;
        turnRight = 1;
        return 0;
    }
    else{
        //Otherwise, run the program as normal.
        if((countL-countL_Store)<=(DEGREES_R*degrees)){
            //This checks whether the motor still didn't make a 90 turn.
            //If not, continue to turn right.
            MotorCmd('R');
            return 0;
        }
        else{
            //Once we have made a 90 degree turn, we need to clear the storage variables
            //and set action to 0 again.
            QuadDec_R_SetCounter(0);
            countR_Store = 0;
            countR=0;
            QuadDec_L_SetCounter(0);
            countL_Store = 0;
            countL=0;
            turnRight = 0;
            MotorCmd('S');
            return 1;   //Returning one tells whichever part of the program that it is calling it
                        //that the right turn is complete.
        }
    }
}
int TurnLeft(int32 cL, int32 cR, int degrees){
    if(turnLeft==0){
        //This checks whether the function is running for the first time.
        countL_Store = cL;
        countR_Store = cR;
        turnLeft = 1;
        return 0;
    }
    else{
        //Otherwise, run the program as normal.
        if((countR-countR_Store)<=(DEGREES_L*degrees)){
            //This checks whether the motor still didn't make a 90 turn.
            //If not, continue to turn right.
            MotorCmd('L');
            return 0;
        }
        else{
            //Once we have made a 90 degree turn, we need to clear the storage variables
            //and set action to 0 again.
            QuadDec_R_SetCounter(0);
            countR_Store = 0;
            countR=0;
            QuadDec_L_SetCounter(0);
            countL_Store = 0;
            countL=0;
            turnLeft=0;
            MotorCmd('S');
            return 1;   //Returning one tells whichever part of the program that it is calling it
                        //that the right turn is complete.
        }
    }
}
int MoveForward(int32 countR, int32 countL, int distance){
    if(moveForward==0){
        //This checks whether the function is running for the first time.
        countR_Store = countR;
        countL_Store = countL;
        moveForward = 1;
        return 0;
    }
    else{
        //Otherwise, run the program as normal.
        if(((countL-countL_Store)<=(DISTANCE*distance))||((countR-countR_Store)<=(DISTANCE*distance))){
            //This checks whether the motor still didn't move forward
            //If not, continue to move forward.
            MotorCmd('F');
            return 0;
        }
        else{
            //Once we have moved the required distance, we need to clear the storage variables
            //and set action to 0 again.
            QuadDec_R_SetCounter(0);
            countR_Store = 0;
            countR=0;
            QuadDec_L_SetCounter(0);
            countL_Store = 0;
            countL=0;
            moveForward = 0;
            MotorCmd('S');
            return 1;   //Returning one tells whichever part of the program that it is calling it
                        //that the right turn is complete.
        }
        
    }
}
void countReset(){
    countL = 0;
    QuadDec_L_SetCounter(0);
    countR = 0;
    QuadDec_R_SetCounter(0);
}
void sync(){
    if(countR-countL>SYNC_RANGE){
        speedL=SPEED+CORRECTION;
        speedR=SPEED-CORRECTION;
    }
    else if(countL-countR>SYNC_RANGE){
        speedR=SPEED+CORRECTION;
        speedL=SPEED-CORRECTION;
    }
}

/* [] END OF FILE */
