#define EN        8
#define INPUT_SIZE 15

//Direction pin
#define X_DIR     5 
#define Y_DIR     6
#define Z_DIR     7

//Step pin
#define X_STP     2
#define Y_STP     3 
#define Z_STP     4 


//lista de programas
//1) Mover disipadores solamente.
//2) Mover prensa solamente.
//3) Hacer corte manual.
//4) Prender/Apagar Sierra.
#define DISIP_LIM_IZQ 9
#define DISIP_LIM_DER 10
#define SIERRA    11
#define SIERRA_SUP_LIM     12
#define SIERRA_INF_LIM     A3

int delayTime_Y=90;//STEP 1/8
int delayTime_X=30;//STEP 1/16
int delayTime_Z_down=250;//STEP 1/8
int delayTime_Z_up=50;//STEP 1/8
char input[INPUT_SIZE + 1];
char *valores [3];
int contador = 0;
boolean estado_sierra = false;
int contador_cortes = 0;
int num_cortes = 0;



void AvanzaMotor(byte stepPin,int time_delay){
  
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(time_delay); 
  digitalWrite(stepPin, LOW);
  delayMicroseconds(time_delay);
}

void ToggleSierra(){

  if(estado_sierra == false){
    digitalWrite(SIERRA, HIGH);
    estado_sierra = true;
  }
  else {
    digitalWrite(SIERRA, LOW);
    estado_sierra = false;
  }
}

void SierraArriba(){
  digitalWrite(Z_DIR, HIGH);
  int estado_sensor_sup = digitalRead(SIERRA_SUP_LIM);
    
  while(estado_sensor_sup == 0){
    AvanzaMotor(Z_STP,delayTime_Z_up);
    estado_sensor_sup = digitalRead(SIERRA_SUP_LIM);
  }
}

boolean SierraAbajo(int estado_sensor_inf){
  
  boolean paro_corte = false;
  
  digitalWrite(Z_DIR, LOW);
  delay(1000);
  while(estado_sensor_inf == 0){
    AvanzaMotor(Z_STP,delayTime_Z_down);
    estado_sensor_inf = digitalRead(SIERRA_INF_LIM);
    if(Serial.available()){
      paro_corte = true;
      Serial.readBytes(input,INPUT_SIZE);
      break;
    }
  }
  return paro_corte;
}

boolean EstudiaSensor(int sensor_por_checar){

  int valor_sensor;
  boolean parar = false;
  
  if(sensor_por_checar != 0){
    valor_sensor = digitalRead(sensor_por_checar);
    if(valor_sensor == 1){
      parar = true;
      return parar;
    }
  }
  return parar;
  
}

boolean ChecarSiParar(byte pinDireccion, boolean dir){

  int sensor_a_checar = 0;
  boolean parar = false;
  
  if(pinDireccion == X_DIR && dir == 1){
    sensor_a_checar = DISIP_LIM_DER;
  }
  else if(pinDireccion == X_DIR && dir ==0){
    sensor_a_checar = DISIP_LIM_IZQ;
  }
  else if(pinDireccion == Z_DIR && dir == 1){
    sensor_a_checar = SIERRA_SUP_LIM;
  }
  else if(pinDireccion == Z_DIR && dir == 0){
    sensor_a_checar = SIERRA_INF_LIM;
  }

  parar = EstudiaSensor(sensor_a_checar);
  if(parar){
    return parar;
  }
  return parar;
}

boolean MueveEjeXoYoZ(boolean direccion, byte pinDir, byte pinStep,long pasos){
  
  int sensor_disip_der;
  int time_delay;
  boolean parar = false;
  
  if(pinDir == X_DIR){
    time_delay = delayTime_X;
  }
  else if(pinDir == Y_DIR){
    time_delay = delayTime_Y;
  }
  else if(pinDir == Z_DIR && direccion == 0){
    time_delay = delayTime_Z_down;
  }
  else if(pinDir == Z_DIR && direccion == 1){
    time_delay = delayTime_Z_up;
  }
  
  digitalWrite(pinDir, direccion);
  for (int i = 0; i < pasos; i++) {
    parar = ChecarSiParar(pinDir,direccion);
    if(parar){
      return parar;
    }
    AvanzaMotor(pinStep,time_delay);
    if(Serial.available()){
      Serial.readBytes(input,INPUT_SIZE);
      parar = true;
      return parar;
    }
  }
  return parar;
}

boolean step(boolean dir, byte dirPin, byte stepperPin, long steps) {
  
  boolean parar = false;
  
  if(steps < 0){
    
    int estado_inf_sierra = digitalRead(SIERRA_INF_LIM);
    int estado_sup_sierra = digitalRead(SIERRA_SUP_LIM);
    boolean parar_corte = false;
    
    if(estado_sup_sierra == 1 && estado_inf_sierra == 0){
      
      if(num_cortes == 0 || contador_cortes == 0) {
        ToggleSierra();
      }
      if(num_cortes != 0){
         contador_cortes++;
      }
    
      parar_corte = SierraAbajo(estado_inf_sierra);
      
      if(parar_corte){
        ToggleSierra();
        SierraArriba();
        parar = true;
        return parar;
      }
      delay(300);
      SierraArriba();
      if(num_cortes == 0 || contador_cortes == (num_cortes + 1)){
         ToggleSierra();
      }
    }
    return parar;
  }
  else {
    parar = MueveEjeXoYoZ(dir,dirPin,stepperPin,steps);
    return parar;
  }
}

boolean CorteSierra(){
  
  int estado_sup_sierra = digitalRead(SIERRA_SUP_LIM);
  boolean parar = false;
  
  if(estado_sup_sierra = 1){
    parar = step(0,Z_DIR,Z_STP,-1);
    return parar;
  }
}

void PruebaSistema(){
  int stps_y = 1000;
  boolean parar = false;
  long stps_x = 640000;
  
  parar = step(0,Y_DIR, Y_STP, stps_y);
  if(parar){
    return;
  }
  delay(500);
  parar = step(0,Y_DIR, Y_STP, stps_y);
   if(parar){
    return;
  }
  delay(500);
  parar = step(1,Y_DIR, Y_STP, stps_y);
   if(parar){
    return;
  }
  delay(500);
  parar = step(1,Y_DIR, Y_STP, stps_y);
   if(parar){
    return;
  }
  delay(500);
  parar = step(0,X_DIR, X_STP, stps_x);
  if(parar){
    delay(500);
    parar = step(1,X_DIR, X_STP, stps_x);
    if(parar){
      delay(500);
      int estado_inf_sierra = digitalRead(SIERRA_INF_LIM);
      parar = SierraAbajo(estado_inf_sierra);
      if(parar){
        return;
      }
      delay(500);
      SierraArriba();
      delay(500);
      ToggleSierra();
      delay(2000);
      ToggleSierra();
    }
    else{
      return;
    }
  }
  else {
    return;
  }
}

void setup(){

  pinMode(X_DIR, OUTPUT); 
  pinMode(X_STP, OUTPUT);

  pinMode(Y_DIR, OUTPUT); 
  pinMode(Y_STP, OUTPUT);

  pinMode(Z_DIR, OUTPUT); 
  pinMode(Z_STP, OUTPUT);

  pinMode(EN, OUTPUT);
  pinMode(SIERRA, OUTPUT);
  
  pinMode(SIERRA_SUP_LIM, INPUT);
  pinMode(SIERRA_INF_LIM, INPUT);
  pinMode(DISIP_LIM_DER, INPUT);
  pinMode(DISIP_LIM_IZQ, INPUT);

  digitalWrite(X_DIR,LOW);
  digitalWrite(X_STP,LOW);
  digitalWrite(Y_DIR,LOW);
  digitalWrite(Y_STP,LOW);
  digitalWrite(Z_DIR,LOW);
  digitalWrite(Z_STP,LOW);
  digitalWrite(EN, LOW);
  digitalWrite(SIERRA, LOW);
  Serial.begin(9600);
}

void loop(){

  /*int estado_inf_sierra = digitalRead(SIERRA_INF_LIM);
  int estado_sup_sierra = digitalRead(SIERRA_SUP_LIM);
  int estado_der = digitalRead(DISIP_LIM_DER);
  int estado_izq = digitalRead(DISIP_LIM_IZQ);
  Serial.println("Inferior Sierra");
  Serial.println(estado_inf_sierra);
  Serial.println("Superior Sierra");
  Serial.println(estado_sup_sierra);
  Serial.println("Derecha");
  Serial.println(estado_der);
  Serial.println("Izquierda");
  Serial.println(estado_izq);
  delay(1000);*/
  
  if(Serial.available()) {
    
    for(int i=0;i<=INPUT_SIZE;i++){
      input[i] = 0;
    }

    byte size = Serial.readBytes(input, INPUT_SIZE);
    
    //Serial.println("Nuevo mensaje");
    //Serial.println(input);
    char* command = strtok(input,":");
    while (command != NULL) {
      valores[contador] = command;
      command = strtok(NULL,":");
      contador++;
      if(contador == 3){
        break;
      }
    }
    contador = 0;
    int codigo_programa = atoi(valores[0]);
    float distancia = atof(valores[1]);
    int direccion = atoi(valores[2]);
    boolean dir = false;

    if (direccion == 1) {
      dir = true;
    }
    /*for(int i=0; i<=2;i++){
      Serial.println(valores[i]);
    }*/

    if (codigo_programa == 1 || codigo_programa == 2 || codigo_programa == 3){

      if(codigo_programa == 1){
        long stps = distancia*3200/0.8;
        step(dir, X_DIR, X_STP, stps);
      }
      else if (codigo_programa == 2){
        long stps = distancia*1600/0.8;
        step(dir, Y_DIR, Y_STP, stps);
      }
      else if (codigo_programa == 3){
        long stps = distancia*1600/0.8;
        step(dir, Z_DIR, Z_STP, stps);
      }
    } 

    else if (codigo_programa == 4) {
      
      int stps = 1000;
      boolean parar = false;

      step(0,Y_DIR, Y_STP, stps);
      delay(500);
      parar = CorteSierra();
      if(!parar){
        step(1,Y_DIR, Y_STP, stps);
      }
    }
      
    else if (codigo_programa == 5){
      
      int stps_y = 1000;
      long stps_x = (distancia*3200/0.8) + 1120;
      boolean parar = false;
      num_cortes = direccion;
      
      step(0,Y_DIR, Y_STP, stps_y);
      delay(500);
      parar = CorteSierra();
      if(!parar){
        step(1,Y_DIR, Y_STP, stps_y);
      }
      
      if(!parar){
        for(int num_piezas = 0; num_piezas < direccion; num_piezas++){
          parar = step(0,X_DIR, X_STP, stps_x);
          if(parar){
            contador_cortes=0;
            break;
          }
          delay(500);
          parar = step(0,Y_DIR, Y_STP, stps_y);
          if(parar){
            contador_cortes=0;
            break;
          }
          delay(500);
          parar = CorteSierra();
          if(parar){
            contador_cortes=0;
            break;
          }
          parar = step(1,Y_DIR, Y_STP, stps_y);
          if(parar){
            contador_cortes=0;
            break;
          }
          delay(500);
        }
        contador_cortes = 0;
        num_cortes = 0;
      }
    }

    else if (codigo_programa == 6) {
      ToggleSierra();
    }
    else if (codigo_programa == 7) {
      delayTime_Z_down=distancia;
    }
    else if (codigo_programa == 8) {
      PruebaSistema();
    }
  } 
}


