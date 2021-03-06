
#include "torreta.h"

int disparos; //Variable que almacena numero de balas restantes
int debounceTime = DEBOUNCE_TIME;
//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------
void InicializaTorreta (TipoTorreta *p_torreta) {
	//Inicializamos pin impacto como entrada de interrupcion(flanco subido)
	pinMode (PIN_IMPACTO, INPUT);
	pullUpDnControl(PIN_IMPACTO, PUD_DOWN);
	wiringPiISR (PIN_IMPACTO, INT_EDGE_RISING, impacto_recibido_isr);
	//Inicializamos pin diparo como salida, y la ponemos a low
	pinMode (PIN_DISPARO, OUTPUT);
	digitalWrite (PIN_DISPARO, LOW);


	//Inicializamos servo horizontal(x)
	p_torreta->servo_x.incremento = SERVO_INCREMENTO;
	p_torreta->servo_x.minimo 	= SERVO_MINIMO;
	p_torreta->servo_x.maximo 	= SERVO_MAXIMO;

	//Lo orientamos hacia su posicion central inicialmente
	p_torreta->servo_x.inicio 	= SERVO_MINIMO + (SERVO_MAXIMO - SERVO_MINIMO)/2;
	p_torreta->servo_x.posicion 	= p_torreta->servo_x.inicio;
	//Si su posicion excede el maximo lo ponemos al maximo
	if(p_torreta->servo_x.posicion > p_torreta->servo_x.maximo)
		p_torreta->servo_x.posicion = p_torreta->servo_x.maximo;
	//Si excede el minimo lo ponemos al minimo
	if(p_torreta->servo_x.posicion < p_torreta->servo_x.minimo)
		p_torreta->servo_x.posicion = p_torreta->servo_x.minimo;
	//Escribimos señal pwm software en su pin
	softPwmCreate (SERVOX_PIN, p_torreta->servo_x.inicio, SERVO_PWM_RANGE); // Internamente ya hace: piHiPri (90) ;
	softPwmWrite(SERVOX_PIN, p_torreta->servo_x.posicion);

	//Inicializamos servo vertical(y)
	p_torreta->servo_y.incremento = SERVO_INCREMENTO;
	p_torreta->servo_y.minimo 	= SERVO_MINIMO;
	p_torreta->servo_y.maximo 	= SERVO_MAXIMO;

	//Lo orientamos hacia su posicion central inicialmente
	p_torreta->servo_y.inicio 	= SERVO_MINIMO + (SERVO_MAXIMO - SERVO_MINIMO)/2;
	p_torreta->servo_y.posicion 	= p_torreta->servo_y.inicio;
	//Si su posicion excede el maximo lo ponemos al maximo
	if(p_torreta->servo_y.posicion > p_torreta->servo_y.maximo)
		p_torreta->servo_y.posicion = p_torreta->servo_y.maximo;
	//Si excede el minimo lo ponemos al minimo
	if(p_torreta->servo_y.posicion < p_torreta->servo_y.minimo)
		p_torreta->servo_y.posicion = p_torreta->servo_y.minimo;
	//Escribimos señal pwm software en su pin
	softPwmCreate (SERVOY_PIN, p_torreta->servo_y.inicio, SERVO_PWM_RANGE); // Internamente ya hace: piHiPri (90) ;
	softPwmWrite(SERVOY_PIN, p_torreta->servo_y.posicion);

	//Incializamos balas iniciales a 10, y impactos logrados a 0
	disparos = 10;
	p_torreta->impactos = 0;
	//Declaramos temporizador del disparo
	p_torreta->p_timer = tmr_new(timer_disparo_isr);
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaComienzo (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_SYSTEM_START);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int VolverMove (fsm_t* this) {
	return 1;
}

int CompruebaJoystickUp (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_JOYSTICK_UP);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaJoystickDown (fsm_t* fsm_player){
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_JOYSTICK_DOWN);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaJoystickLeft (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_JOYSTICK_LEFT);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaJoystickRight (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_JOYSTICK_RIGHT);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaTimeoutDisparo (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_SHOOT_TIMEOUT);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaImpacto (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_TARGET_DONE);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaTriggerButton (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_TRIGGER_BUTTON);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

int CompruebaFinalJuego (fsm_t* this) {
	int result = 0;
	piLock (SYSTEM_FLAGS_KEY);
	result = (flags_juego & FLAG_SYSTEM_END);
	piUnlock (SYSTEM_FLAGS_KEY);
	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void ComienzaSistema (fsm_t* this) {
	piLock(STD_IO_BUFFER_KEY);
	printf("GAME STARTED! GOOD LUCK\n");
	fflush(stdout);
	piUnlock(STD_IO_BUFFER_KEY);
}

void MueveTorretaArriba (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= (~FLAG_JOYSTICK_UP);
	piUnlock (SYSTEM_FLAGS_KEY);

	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	if(p_torreta->servo_y.posicion + p_torreta->servo_y.incremento <= p_torreta->servo_y.maximo) {
		p_torreta->servo_y.posicion = p_torreta->servo_y.posicion + p_torreta->servo_y.incremento;

		softPwmWrite(SERVOY_PIN, p_torreta->servo_y.posicion);
	}
}

void MueveTorretaAbajo (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= (~FLAG_JOYSTICK_DOWN);
	piUnlock (SYSTEM_FLAGS_KEY);

	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	if(p_torreta->servo_y.posicion - p_torreta->servo_y.incremento >= p_torreta->servo_y.minimo) {
		p_torreta->servo_y.posicion = p_torreta->servo_y.posicion - p_torreta->servo_y.incremento;

		softPwmWrite(SERVOY_PIN, p_torreta->servo_y.posicion);
	}
}

void MueveTorretaIzquierda (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= (~FLAG_JOYSTICK_LEFT);
	piUnlock (SYSTEM_FLAGS_KEY);

	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	if(p_torreta->servo_x.posicion + p_torreta->servo_x.incremento <= p_torreta->servo_x.maximo) {
			p_torreta->servo_x.posicion = p_torreta->servo_x.posicion + p_torreta->servo_x.incremento;

		softPwmWrite(SERVOX_PIN, p_torreta->servo_x.posicion);
	}
}

void MueveTorretaDerecha (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= (~FLAG_JOYSTICK_RIGHT);
	piUnlock (SYSTEM_FLAGS_KEY);

	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);

	if(p_torreta->servo_x.posicion - p_torreta->servo_x.incremento >= p_torreta->servo_x.minimo) {
			p_torreta->servo_x.posicion = p_torreta->servo_x.posicion - p_torreta->servo_x.incremento;


		softPwmWrite(SERVOX_PIN, p_torreta->servo_x.posicion);
	}
}

void DisparoIR (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= ~FLAG_TRIGGER_BUTTON;
	piUnlock (SYSTEM_FLAGS_KEY);

	piLock (PLAYER_FLAGS_KEY);
	flags_player |= FLAG_START_DISPARO;
    piUnlock (PLAYER_FLAGS_KEY);

    TipoTorreta *p_torreta;
    p_torreta = (TipoTorreta*)(this->user_data);
    disparos--;

    digitalWrite (PIN_DISPARO, HIGH);
	tmr_startms(p_torreta->p_timer,SHOOT_TIMEOUT);

}

void FinalDisparoIR (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= ~FLAG_SHOOT_TIMEOUT;
	piUnlock (SYSTEM_FLAGS_KEY);

    digitalWrite (PIN_DISPARO, LOW);


}

void ImpactoDetectado (fsm_t* this) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego &= ~FLAG_TARGET_DONE;
	piUnlock (SYSTEM_FLAGS_KEY);

	digitalWrite (PIN_DISPARO, LOW);
	piLock (STD_IO_BUFFER_KEY);
	printf("AU! IMPACTO RECIBIDO!\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	TipoTorreta *p_torreta;
	p_torreta = (TipoTorreta*)(this->user_data);
	p_torreta->impactos++;

	piLock (PLAYER_FLAGS_KEY);
	flags_player |= FLAG_START_IMPACTO;
	piUnlock (PLAYER_FLAGS_KEY);

}

void FinalizaJuego (fsm_t* this) {
	piLock (STD_IO_BUFFER_KEY);
	printf("GAME ENDED! HOPE YOU PLAY AGAIN SOON!\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	//Restablecemos pines pwm a estado normal
	pwmWrite(18,0);
	pwmWrite(19,0);
	exit(0);
}

//TEMPORIZADOR DISPARO
void timer_disparo_isr (union sigval value) {
	piLock (SYSTEM_FLAGS_KEY);
	flags_juego |= FLAG_SHOOT_TIMEOUT; //Activa el flag de final de nota
	piUnlock (SYSTEM_FLAGS_KEY);
}

//FUNCION INTERRUPCION IMPACTO
void impacto_recibido_isr (void) {
	//Prevencion de rebotes
	if (millis () < debounceTime) {
		debounceTime = millis () + DEBOUNCE_TIME;
		return;
	}

	piLock (SYSTEM_FLAGS_KEY);
	flags_juego |= FLAG_TARGET_DONE;
	piUnlock (SYSTEM_FLAGS_KEY);

	//Actualizamos valor del debounceTime
	debounceTime = millis () + DEBOUNCE_TIME;
}
