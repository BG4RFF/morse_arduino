
#include "Wire.h"
#include "morseduino.h"

  //Configuration for the LED Kaleidoscope
  //Control Array
  //{pin, intensity, direction, increment}
int LED[3][4] = {{6, 255, 0, 3}, {5, 255, 0, 2}, {3, 255, 0, 1}};

  //Keys - Color
#define RED 2
#define GREEN 1
#define BLUE 0

  // Keys - Control
#define LED_PIN 0
#define LED_INTENSITY 1
#define LED_DIRECTION 2
#define LED_INCREMENT 3

  // Keys - Direction
#define INCREASE 1
#define DECREASE 0

  // MODE CONFIG ------------------
int mode_new = false;

int mode_delay = 4000;

  // MODE CONFIG ------------------



  // SPECTRUM MODE CONFIG ---------

  // Max Brightness
int MAX = 255;

  // Min Brightness
int MIN = 0;

  // Spectrum step interval.
unsigned long spectrum_step_interval = 100;
unsigned long spectrum_step_last = false;

  // ------------------------------



  // MORSE MODE CONFIG ------------

  // Morse step interval.
unsigned long morse_step_interval = 400;
unsigned long morse_step_last = false;

  // Next 
unsigned long step_last = 0;
  // END Kaleidoscope Configuration

unsigned int device_mode = DEVICE_MODE_SPECTRUM; // Options: DEVICE_MODE_MORSE, DEVICE_MODE_SPECTRUM


  //Morse configuration
MORSE_CHAR this_char = {0, 31};
MORSE_CHAR next_char = {0, 31};
char char_new = false;
bool char_done = false;
bool char_next_delay = false;
bool morse_stop = false;
unsigned int last_command = false;

  // -----------------------------


unsigned int i = 0;

void setup()
{
        #if VERBOSE
        Serial.begin(BAUD);
        #endif

	Wire.begin(ADDRESS);
	Wire.onReceive(receiveEvent);
	Wire.onRequest(requestEvent);

          // Configure all LED pins to output mode.
          // Set everything to off.
	  // Assign random starting values.
        for(i = 0; i < 3; i++){
                pinMode(LED[i][LED_PIN], OUTPUT);
                analogWrite(LED[i][LED_PIN], LOW);
		LED[i][LED_INTENSITY] = random(0, 255);
		LED[i][LED_DIRECTION] = random(0, 1);
		LED[i][LED_INCREMENT] = random(1, 4);
        }

	pinMode(ITTERATION_PIN, OUTPUT);

	pinMode(SIGNAL_PIN, OUTPUT);

	digitalWrite(ITTERATION_PIN, LOW);

	  // We start with nothing queued, so let us signal that we are ready to start receiving input.
	digitalWrite(SIGNAL_PIN, HIGH);

	  // Initialize stepper timer.
        step_last = millis();

}



void loop(){
	if(mode_new){
		set_mode();
	}

	unsigned long now = millis();

	if(device_mode == DEVICE_MODE_SPECTRUM){
		if(now - step_last >= spectrum_step_interval && now - morse_step_last >= mode_delay){
			#if VERBOSE
				//Serial.println("Stepping.");
			#endif

			increment_color();
			step_last = millis();
	
			#if VERBOSE
			step_signal();
			#endif
		}
	}else if(device_mode == DEVICE_MODE_MORSE){
		  // If the time that has passed since the last non-zero morse transmission is greater than the mode_delay, change the mode, because, when we're idle, we should be showing some pretty lights.
		if(now - morse_step_last >= mode_delay && morse_step_last != false){
			#if VERBOSE
			Serial.println("Idle device, switching to spectrum mode.");
			#endif
			device_mode = DEVICE_MODE_SPECTRUM;
			return;
		}

		if(morse_stop){
			morse_clear_queue();
		}
	
		if(char_new){
			char_translate();
		}
	
		if(char_done){
			if(char_next_delay){
				this_char.encoding = MORSE_CHAR_GAP_ENCODING;
				this_char.index = MORSE_CHAR_GAP_INDEX;
				char_next_delay = false;
			}else{
				if(next_char.encoding == MORSE_CHAR_SPACE_ENCODING || next_char.encoding == false){
					char_next_delay = false;
				}else{
					char_next_delay = true;
				}
				this_char = next_char;
				morse_char_reset(next_char);
				morse_request_input(true);
			}
			char_done = false;
		}
	
		// Using a check of time instead of a dealy function to control timing of the script.
		if(now - step_last >= morse_step_interval && now - spectrum_step_last >= mode_delay){
			#if VERBOSE
			//Serial.println("Stepping.");
			#endif
			morse_write();
			step_last = millis();
			  // All morse characters have a non-zero value.  Including the spaces/delays.  When a non-zero character is being written, update the timestamp for last write.
	
			#if VERBOSE
			step_signal();
			#endif
		}
	}
}

void set_mode(){
	#if VERBOSE
	Serial.println("Set Mode.");
	#endif

	switch(mode_new){
		case DEVICE_MODE_MORSE:
			write_off();
			device_mode = DEVICE_MODE_MORSE;
			break;
		case DEVICE_MODE_SPECTRUM:
			if(device_mode == DEVICE_MODE_MORSE){
				morse_clear_queue();
			}
			device_mode = DEVICE_MODE_SPECTRUM;
			break;
		default:
			break;
	}

	mode_new = false;
}

/*

*/

  // Clear the queue, stop MORSE transmissions.
void morse_clear_queue(){
	morse_char_reset(next_char);
	morse_char_reset(this_char);
	char_done = true;
	char_next_delay = false;

	morse_stop = false;
}

  // Empty the MORSE_CHAR variable.
void morse_char_reset(MORSE_CHAR &reset){
	reset.encoding = false;
	reset.index = 31;
}

  // Translate the ascii character to the MORSE/Binary encoded long value.
void char_translate(){
	#if VERBOSE
		Serial.print("New Character: ");
		Serial.println(char_new);
	#endif

	  // If there is still character data queued, we aren't expecting any new characters.
	  // There is clearly some sort of misunderstanding between the master and the slave, however, we're going to favor the slave in this case, and ignore the new commands sent by the master.
	  // If the master wants to override a previous command, it needs to send a COMMAND_Stop to clear out the queue.
	if(next_char.encoding){
		return;
	}

	switch(char_new){
		case 'A':
		case 'a':
			next_char.encoding = MORSE_CHAR_A_ENCODING;
			next_char.index = MORSE_CHAR_A_INDEX;
			break;
		case 'B':
		case 'b':
			next_char.encoding = MORSE_CHAR_B_ENCODING;
			next_char.index = MORSE_CHAR_B_INDEX;
			break;
		case 'C':
		case 'c':
			next_char.encoding = MORSE_CHAR_C_ENCODING;
			next_char.index = MORSE_CHAR_C_INDEX;
			break;
		case 'D':
		case 'd':
			next_char.encoding = MORSE_CHAR_D_ENCODING;
			next_char.index = MORSE_CHAR_D_INDEX;
			break;
		case 'E':
		case 'e':
			next_char.encoding = MORSE_CHAR_E_ENCODING;
			next_char.index = MORSE_CHAR_E_INDEX;
			break;
		case 'F':
		case 'f':
			next_char.encoding = MORSE_CHAR_F_ENCODING;
			next_char.index = MORSE_CHAR_F_INDEX;
			break;
		case 'G':
		case 'g':
			next_char.encoding = MORSE_CHAR_G_ENCODING;
			next_char.index = MORSE_CHAR_G_INDEX;
			break;
		case 'H':
		case 'h':
			next_char.encoding = MORSE_CHAR_H_ENCODING;
			next_char.index = MORSE_CHAR_H_INDEX;
			break;
		case 'I':
		case 'i':
			next_char.encoding = MORSE_CHAR_I_ENCODING;
			next_char.index = MORSE_CHAR_I_INDEX;
			break;
		case 'J':
		case 'j':
			next_char.encoding = MORSE_CHAR_J_ENCODING;
			next_char.index = MORSE_CHAR_J_INDEX;
			break;
		case 'K':
		case 'k':
			next_char.encoding = MORSE_CHAR_K_ENCODING;
			next_char.index = MORSE_CHAR_K_INDEX;
			break;
		case 'L':
		case 'l':
			next_char.encoding = MORSE_CHAR_L_ENCODING;
			next_char.index = MORSE_CHAR_L_INDEX;
			break;
		case 'M':
		case 'm':
			next_char.encoding = MORSE_CHAR_M_ENCODING;
			next_char.index = MORSE_CHAR_M_INDEX;
			break;
		case 'N':
		case 'n':
			next_char.encoding = MORSE_CHAR_N_ENCODING;
			next_char.index = MORSE_CHAR_N_INDEX;
			break;
		case 'O':
		case 'o':
			next_char.encoding = MORSE_CHAR_O_ENCODING;
			next_char.index = MORSE_CHAR_O_INDEX;
			break;
		case 'P':
		case 'p':
			next_char.encoding = MORSE_CHAR_P_ENCODING;
			next_char.index = MORSE_CHAR_P_INDEX;
			break;
		case 'Q':
		case 'q':
			next_char.encoding = MORSE_CHAR_Q_ENCODING;
			next_char.index = MORSE_CHAR_Q_INDEX;
			break;
		case 'R':
		case 'r':
			next_char.encoding = MORSE_CHAR_R_ENCODING;
			next_char.index = MORSE_CHAR_R_INDEX;
			break;
		case 'S':
		case 's':
			next_char.encoding = MORSE_CHAR_S_ENCODING;
			next_char.index = MORSE_CHAR_S_INDEX;
			break;
		case 'T':
		case 't':
			next_char.encoding = MORSE_CHAR_T_ENCODING;
			next_char.index = MORSE_CHAR_T_INDEX;
			break;
		case 'U':
		case 'u':
			next_char.encoding = MORSE_CHAR_U_ENCODING;
			next_char.index = MORSE_CHAR_U_INDEX;
			break;
		case 'V':
		case 'v':
			next_char.encoding = MORSE_CHAR_V_ENCODING;
			next_char.index = MORSE_CHAR_V_INDEX;
			break;
		case 'W':
		case 'w':
			next_char.encoding = MORSE_CHAR_W_ENCODING;
			next_char.index = MORSE_CHAR_W_INDEX;
			break;
		case 'X':
		case 'x':
			next_char.encoding = MORSE_CHAR_X_ENCODING;
			next_char.index = MORSE_CHAR_X_INDEX;
			break;
		case 'Y':
		case 'y':
			next_char.encoding = MORSE_CHAR_Y_ENCODING;
			next_char.index = MORSE_CHAR_Y_INDEX;
			break;
		case 'Z':
		case 'z':
			next_char.encoding = MORSE_CHAR_Z_ENCODING;
			next_char.index = MORSE_CHAR_Z_INDEX;
			break;
		case '0':
			next_char.encoding = MORSE_CHAR_ZERO_ENCODING;
			next_char.index = MORSE_CHAR_ZERO_INDEX;
			break;
		case '1':
			next_char.encoding = MORSE_CHAR_ONE_ENCODING;
			next_char.index = MORSE_CHAR_ONE_INDEX;
			break;
		case '2':
			next_char.encoding = MORSE_CHAR_TWO_ENCODING;
			next_char.index = MORSE_CHAR_TWO_INDEX;
			break;
		case '3':
			next_char.encoding = MORSE_CHAR_THREE_ENCODING;
			next_char.index = MORSE_CHAR_THREE_INDEX;
			break;
		case '4':
			next_char.encoding = MORSE_CHAR_FOUR_ENCODING;
			next_char.index = MORSE_CHAR_FOUR_INDEX;
			break;
		case '5':
			next_char.encoding = MORSE_CHAR_FIVE_ENCODING;
			next_char.index = MORSE_CHAR_FIVE_INDEX;
			break;
		case '6':
			next_char.encoding = MORSE_CHAR_SIX_ENCODING;
			next_char.index = MORSE_CHAR_SIX_INDEX;
			break;
		case '7':
			next_char.encoding = MORSE_CHAR_SEVEN_ENCODING;
			next_char.index = MORSE_CHAR_SEVEN_INDEX;
			break;
		case '8':
			next_char.encoding = MORSE_CHAR_EIGHT_ENCODING;
			next_char.index = MORSE_CHAR_EIGHT_INDEX;
			break;
		case '9':
			next_char.encoding = MORSE_CHAR_NINE_ENCODING;
			next_char.index = MORSE_CHAR_NINE_INDEX;
			break;
		case ',':
			next_char.encoding = MORSE_CHAR_COMMA_ENCODING;
			next_char.index = MORSE_CHAR_COMMA_INDEX;
			break;
		case '.':
			next_char.encoding = MORSE_CHAR_PERIOD_ENCODING;
			next_char.index = MORSE_CHAR_PERIOD_INDEX;
			break;
		case '/':
			next_char.encoding = MORSE_CHAR_SLASH_ENCODING;
			next_char.index = MORSE_CHAR_SLASH_INDEX;
			break;
		case '?':
			next_char.encoding = MORSE_CHAR_QUESTION_ENCODING;
			next_char.index = MORSE_CHAR_QUESTION_INDEX;
			break;
		case '@':
			next_char.encoding = MORSE_CHAR_AT_ENCODING;
			next_char.index = MORSE_CHAR_AT_INDEX;
			break;
		case ' ':
			next_char.encoding = MORSE_CHAR_SPACE_ENCODING;
			next_char.index = MORSE_CHAR_SPACE_INDEX;
			break;
		default:
			break;
	}

	  // If there is no encoded value in next_char.encoding, the character supplied by the master was not a valid MORSE character, and we are going to ignore it.
	  // If no valid character was received, we are going to request the next.
	if(next_char.encoding){

	}else{
		morse_request_input(true);
	}

	char_new = false;
}

#if VERBOSE
  // Blinks an LED at the end of every MORSE interval to provide some indication of progress/tempo.
void step_signal(){
	digitalWrite(ITTERATION_PIN, HIGH);
	delay(50);
	digitalWrite(ITTERATION_PIN, LOW);
}
#endif

  // Given an encoded character, and an index for the progress we've made in transmitting the character, write the next value.
void morse_write(){
	if(this_char.encoding){
		#if VERBOSE
		Serial.println("Updating last step.");
		#endif
		morse_step_last = millis();
	}

	unsigned long mask = bit(31 - this_char.index);
	unsigned long output = this_char.encoding & mask;

	#if VERBOSE
		Serial.print(mask);
		Serial.print("|");
		Serial.print(this_char.encoding);
		Serial.print("|");
		Serial.println(output);
	#endif

	if(output){
		write_on();
	}else{
		write_off();
	}

	this_char.index++;
	index_check();
}

  // Check to see if we've transmitted the entire encoded character.
void index_check(){
	if(this_char.index >= 32){
		morse_char_reset(this_char);
		char_done = true;
	}
}

  // Signal the master that we're ready to receive more input.
void morse_request_input(bool value){
	if(value){
		digitalWrite(SIGNAL_PIN, HIGH);
	}else{
		digitalWrite(SIGNAL_PIN, LOW);
	}
}

void increment_color(){
	spectrum_step_last = millis();

        for(i = 0; i < 3; i++){
                if(LED[i][LED_DIRECTION] == INCREASE){
                        LED[i][LED_INTENSITY] = LED[i][LED_INTENSITY] + LED[i][LED_INCREMENT];
                }else if(LED[i][LED_DIRECTION] == DECREASE){
                        LED[i][LED_INTENSITY] = LED[i][LED_INTENSITY] - LED[i][LED_INCREMENT];
                }else{

                }

                if(LED[i][LED_INTENSITY] >= MAX){
                        LED[i][LED_DIRECTION] = DECREASE;
                        LED[i][LED_INTENSITY] = MAX;
			LED[i][LED_INCREMENT] = random(1, 4);
                }else if(LED[i][LED_INTENSITY] <= MIN){
                        LED[i][LED_DIRECTION] = INCREASE;
                        LED[i][LED_INTENSITY] = MIN;
			LED[i][LED_INCREMENT] = random(1,4);
                }else{

                }

        #if VERBOSE
        //Serial.println(LED[i][LED_INTENSITY]);
        #endif

        }

        write_color();
}

void write_color(){
        for(i = 0; i < 3; i++){
                analogWrite(LED[i][LED_PIN], LED[i][LED_INTENSITY]);
        }
}

void write_on(){
        for(i = 0; i < 3; i++){
                digitalWrite(LED[i][LED_PIN], HIGH);
        }
}

void write_off(){
        for(i = 0; i < 3; i++){
                analogWrite(LED[i][LED_PIN], LOW);
        }
}

  // Process input from the master.
void receiveEvent(int bytes) {
	unsigned int operation = Wire.read();

	switch(operation){
		case COMMAND_SetChar:
			char_new = Wire.read();
			morse_request_input(false);
			if(device_mode == DEVICE_MODE_SPECTRUM){
				mode_new = DEVICE_MODE_MORSE;
				morse_step_last = false;
			}
			last_command = COMMAND_SetChar;
		#if VERBOSE
			Serial.println("COMMAND_SetChar");
			Serial.print("Char: ");
			Serial.println(char_new);
		#endif
			break;
		case COMMAND_SetColor:
			LED[RED][LED_INTENSITY] = Wire.read();
			LED[GREEN][LED_INTENSITY] = Wire.read();
			LED[BLUE][LED_INTENSITY] = Wire.read();
			last_command = COMMAND_SetChar;
		#if VERBOSE
			Serial.println("COMMAND_SetColor");
		#endif
			break;
		case COMMAND_SetIncrement:
			LED[RED][LED_INCREMENT] = Wire.read();
			LED[GREEN][LED_INCREMENT] = Wire.read();
			LED[BLUE][LED_INCREMENT] = Wire.read();
			last_command = COMMAND_SetIncrement;
		#if VERBOSE
			Serial.println("COMMAND_SetIncrement");
		#endif
			break;
		case COMMAND_SetMode:
			mode_new = Wire.read();
			last_command = COMMAND_SetMode;
		#if VERBOSE
			Serial.println("COMMAND_SetMode");
		#endif
			break;
		case COMMAND_Stop:
			morse_stop = true;
			last_command = COMMAND_SetChar;
		#if VERBOSE
			Serial.println("COMMAND_Stop");
		#endif
			break;
		default:
			break;
	}

	// Consume any remainder bytes...
	while(Wire.available()){
		Wire.read();
	}

	#if VERBOSE
	Serial.println("Receive Event.");
	#endif
}

  // Respond to the master upon a read request.
void requestEvent() {
	// Both the Arduino and RPi are little-endian, no conversion needed...
	Wire.write(true);

	#if VERBOSE
	Serial.println("Request Event.");
	#endif
}
