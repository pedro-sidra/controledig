// Timer frequency = 10 kHz
// Timer period = 1.0e-4 s = 100 us

// dominant pole: s = -355.196886
// ts = 0.01126136

// pwm frequency = 31372.55 Hz 
// pwm period = 31.875 us

const int BATCH_SIZE = 200;
int y[BATCH_SIZE] = {};
float u[BATCH_SIZE] = {};
float utemp =0;

float perturb = 0;

const bool DEBUG = false;

char timer_count = 0;
bool send_serial = false;
bool on = false;
uint16_t sample_count = 0;

float ref =2.5, e=0, eant=0, i=0, d=0;

#define KP 1.143677268970767
#define KI 0.4913055708142872 
#define KD 0.0964895917972739

/* #define KD -0.34313250699481657 */
/* #define KI 1.2135917662606848 */ 
/* #define KP 1.7358097748380037 */ 


/* #define KD -0.13315503221689973 */
/* #define KI 0.8123151004689042 */ 
/* #define KP 1.4963652401153276 */ 


/* #define KD -0.13315503221689973 */
/* #define KI 0.8123151004689042 */ 
/* #define KP 1.4963652401153276 */ 

/* #define KP 0.677745861404204 */ 
/* #define KI 0.25056317729683636 */ 
/* #define KD 0.3458013650477928 */

char received_from_serial = '0';

void send_samples()
{
    int i=0;
    for (i=0; i < BATCH_SIZE; i++){
        Serial.print(5.0*y[i]/1023.0);
        Serial.print(" ");
        Serial.println(5.0*u[i]/255.0);
    }
}

ISR(TIMER2_COMPA_vect)
{
    timer_count++;
    if (timer_count>10)
    {
        timer_count=0;
        eant = e;
        y[sample_count] = analogRead(A0);
        e = ref - (float)y[sample_count]*5.0/1023.0;
        i = e + i;
        d = e - eant;
        // 51 = 255/5
        u[sample_count] = 51*((KP*e + KI*i + KD*d));
        utemp = (51*perturb + u[sample_count]) ;
        if (utemp>255)
            utemp = 255;
        if (utemp<0)
            utemp = 0;
        analogWrite(9, utemp);
        sample_count ++;
        if ((sample_count % 100) == 0)
        {
            if (perturb == 0) perturb=-0.5;
            perturb *= -1;
        }
        if (sample_count == BATCH_SIZE)
        {
            sample_count=0;
            send_serial = true;
        }
    }
}

void setup() {
    Serial.begin(115200);
    // Timer secrets: https://playground.arduino.cc/Main/TimerPWMCheatsheet
    // https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
    TCCR1B = (TCCR1B & 0b11111000) | 0x01;

    //set timer2 interrupt at 8kHz
    TCCR2A = 0; TCCR2B = 0; TCNT2  = 0;
    // set compare match register for 10khz increments
    OCR2A = 199;// = (16*10^6) / (10e3*8) - 1 (must <256)
    // turn on CTC mode, Set CS21 bit for 8 prescaler
    TCCR2A |= (1 << WGM21); TCCR2B |= (1 << CS21);
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);

    analogWrite(9, 0);
}
void loop() {

    if (send_serial == true)
    {
        send_serial = false;
        send_samples();
    }
    delay(500);

    /* Serial.print(5.0*y[sample_count]/1023.0); */
    /* Serial.print(" "); */
    /* Serial.println(5.0*u[sample_count]/255.0); */
    /* delay(1); */
    /* if (Serial.available()){ */
    /*     received_from_serial = Serial.read(); */
    /*     switch (received_from_serial){ */
    /*         case 'b': */
    /*             on = true; */
    /*             break; */
    /*         case 'q': */
    /*             on = false; */
    /*             analogWrite(9, 0); */
    /*             break; */
    /*     } */
    /*     received_from_serial = '0'; */
    /* } */
}
