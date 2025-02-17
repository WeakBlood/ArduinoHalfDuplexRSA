#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

struct RSAKey {
  long exp;    
  long mod;   
};

RSAKey publicKey, privateKey; 
bool keySent = false;

void setup() {
  Serial.begin(9600);
  radio.begin();
  generateKeys();
  
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address); // Initial pipe for sending keys
  radio.stopListening(); // Start in transmit mode
  Serial.println("Receiver Ready");
}

void loop() {
  if (!keySent) {
    // Phase 1: Send public key repeatedly
    String toSend = String(publicKey.exp) + "." + String(publicKey.mod);
    char converted[toSend.length() + 1];
    toSend.toCharArray(converted, sizeof(converted));
    
    if (radio.write(converted, sizeof(converted))) {
      Serial.println("Sent public key: " + toSend);
      radio.openReadingPipe(0,address); // Receiver
      radio.startListening();
      keySent = true;
    }
  } else {
    // Phase 2: Switch to receive mode
    //Serial.print("heys");
    // Handle incoming messages
    if (radio.available()) {
      char text[64] = "";
      radio.read(text, sizeof(text));
      Serial.print("Before decryption: ");
      Serial.print(text);
      Serial.print('\n');
      String dec = decrypt(String(text), privateKey);
      Serial.print("Decrypted: ");
      Serial.println(dec);
    }
  }
  delay(1000);
}
void generateKeys() {
  long p = 61;
  long q = 53;
  long n = p * q;
  long phi = (p-1)*(q-1);
  long e = 17;
  // REALLY UNSAFE RSA KEY GENERATION.
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
