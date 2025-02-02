#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Arduino_LED_Matrix.h"

RF24 radio(9, 10); // CE, CSN
ArduinoLEDMatrix matrix;
const byte address[6] = "00001";

uint8_t frame[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,1,0,0,0,0,1,1,0,0},
  {0,1,0,0,1,0,0,1,0,0,1,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

struct RSAKey {
  long exp;    
  long mod;   
};

RSAKey publicKey, privateKey; 
RSAKey receveirKey;

// Prototypes
long modPow(long base, long exponent, long mod);
long modInverse(long a, long m);
void generateKeys();
bool receveid = false;
void setup() {
  Serial.begin(9600);
  matrix.begin();
  
  // Generate keys ONCE (not in loop)
  generateKeys();
  
  radio.begin();
  radio.openReadingPipe(0,address); // Receiver
  radio.setPALevel(RF24_PA_LOW);
  //radio.stopListening();
    //radio.openReadingPipe(0, address); // Receiver
    //radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {
  matrix.renderBitmap(frame, 8, 12);
  if(receveid){
    Serial.print("nonva");
    String original = "BREAD";
    String encrypted = encrypt(original, receveirKey);
    //String decrypted = decrypt(encrypted, privateKey);

    //Serial.print(publicKey.exp);
    //String toSend = String(publicKey.exp) + "." + String(publicKey.mod);
    //char converted[toSend.length()+1];
    //toSend.toCharArray(converted,sizeof(converted));
   // radio.write(converted,sizeof(converted));
    Serial.print("Original: ");
    Serial.println(original);
    //Serial.print("Encrypted: ");
    //Serial.println(encrypted);
    //Serial.print("Decrypted: ");
    //Serial.println(decrypted);
    char encryptedToSend[encrypted.length()+1];
    encrypted.toCharArray(encryptedToSend,encrypted.length()+1);
    radio.write(encryptedToSend,sizeof(encryptedToSend));
  } else{
    if(radio.available()){
        //Serial.print("nonva");
        char text[32] = "";  // Buffer for received data
        radio.read(&text, sizeof(text));
        String message = String(text);
        Serial.println(message);
        int point = -1;
        for(int i = 0; i < message.length(); i++){
          if(message[i] == '.'){
            point = i;
            break;
          }
        }
        if(point != -1){
          String e =  message.substring(0,point);
          String mod = message.substring(point+1);
          receveirKey = {long(e.toInt()),long(mod.toInt())};
          receveid = true;
          radio.openWritingPipe(address);
          radio.stopListening();
        }
    }
  }

  //radio.write(encryptedToSend,sizeof(encryptedToSend));
  delay(1000);
}

// Generate keys once (fixed primes)
void generateKeys() {
  long p = 61;
  long q = 67;
  long n = p * q;
  long phi = (p-1)*(q-1);
  long e = 17;
  
  publicKey.exp = e;
  publicKey.mod = n;
  privateKey.exp = modInverse(e, phi);
  privateKey.mod = n;
}

long modInverse(long a, long m) {
  long m0 = m, t, q;
  long x0 = 0, x1 = 1;
  if (m == 1) return 0;
  while (a > 1) {
    q = a / m;
    t = m;
    m = a % m;
    a = t;
    t = x0;
    x0 = x1 - q * x0;
    x1 = t;
  }
  return x1 < 0 ? x1 + m0 : x1;
}

long modPow(long base, long exponent, long mod) {
  long result = 1;
  base = base % mod;
  while (exponent > 0) {
    if (exponent % 2 == 1)
      result = (result * base) % mod;
    exponent >>= 1;
    base = (base * base) % mod;
  }
  return result;
}

String encrypt(String message, RSAKey key) {
  String encrypted;
  for (int i = 0; i < message.length(); i++) {
    long m = message[i]; // Get ASCII value
    long c = modPow(m, key.exp, key.mod);
    encrypted += String(c);
    if (i < message.length()-1) encrypted += ",";
  }
  return encrypted;
}

String decrypt(String encrypted, RSAKey key) {
  String decrypted;
  int startIndex = 0;
  while (true) {
    int commaIndex = encrypted.indexOf(',', startIndex);
    if (commaIndex == -1) commaIndex = encrypted.length();
    
    String numStr = encrypted.substring(startIndex, commaIndex);
    if (numStr.length() > 0) {
      long c = numStr.toInt(); 
      long m = modPow(c, key.exp, key.mod);
      decrypted += (char)m;
    }
    startIndex = commaIndex + 1;
    if (startIndex > encrypted.length()) break;
  }
  return decrypted;
}