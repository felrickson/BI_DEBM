const int sensorPin = A2;
int gotas = 0;
bool gotaDetectada = false;
float limiar = 1.0; // Limiar para condição de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {
   // Lê o valor analógico do sensor e converte para tensão
   float sensorTensao = analogRead(sensorPin) *100 / 1023.0;
   Serial.println(sensorTensao);

   // Verifica se a tensão está acima do limiar para detectar uma gota
   if (sensorTensao > limiar && !gotaDetectada) {
      gotas++;
      gotaDetectada = true;
      Serial.print("Gotas contadas: ");
      Serial.println(gotas);
   } else if (sensorTensao < limiar) {
      gotaDetectada = false;
   }

   delay(100);
}
