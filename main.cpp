#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Sprite.h"


#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define buzzer 17
#define botonRight 18
#define botonLeft 19
#define botonStart 20
#define botonDisparo 21

#define TONO_DIS_ENEMIGO 400 // Hz
#define TONO_DIS_JUGADOR 350 // Hz
#define TONO_COLISION 800 // Hz

// Duración de los tonos 
#define DURACION_DIS_ENEMIGO 200
#define DURACION_DIS_JUGADOR 200
#define DURACION_COLISION 300


enum Sonidos {
    disEnemigo,
    disJugador,
    colision
};

int count = 0;

const uint8_t Start = 0;
const uint8_t Disparo = 1;
const uint8_t RIGHT = 2;
const uint8_t LEFT = 3;

int enemigosDerrotados = 0;

//maximos del screen
const int XMAX = 240;
const int YMAX = 320;
//iniciar juego
bool juegoIniciado = false;
unsigned long tiempo = 0;
unsigned long prevmillis = 0;
bool juegoTerminado = false;
//jugador movimiento
int x = 0;
int y = 0;
int prev_x = 0;
int prev_y = 0;
//enemigo movimiento
int xE = 10;
int yE = 10; 
int prev_xE = 0;
int prev_yE = 0;
unsigned long prevEnemyMillis = 0; 
const int enemySpeed = 500; 
int enemyDirection = 1;
//disparo jugador
int disparoX = 0;
int disparoY = 0;
int prevDisparoX = 0;
int prevDisparoY = 0;
bool disparoActivo = false;
const int disparoAncho = 2;
const int disparoAlto = 10;
//disparo enemigo
int disparoEnemigoX = 0;
int disparoEnemigoY = 0;
int prevDisparoEnemigoX = 0;
int prevDisparoEnemigoY = 0;
bool disparoEnemigoActivo = false;
const int disparoEnemigoAncho = 2;
const int disparoEnemigoAlto = 10;
unsigned long prevEnemyShotMillis = 0;
const int enemyShotSpeed = 700; 
//asteroide izquierda
int xAsteroide, yAsteroide; // Coordenadas del asteroide
int velXAsteroide = 2;      // Velocidad horizontal del asteroide
int velYAsteroide = 2;      // Velocidad vertical del asteroide
bool asteroideActivo = false;
//asteroide derecha
int xAsteroide1, yAsteroide1; // Coordenadas del asteroide
int velXAsteroide1 = 2;      // Velocidad horizontal del asteroide
int velYAsteroide1 = 2;      // Velocidad vertical del asteroide
bool asteroideActivo1 = false;
//sonido
volatile bool sonidoEnProgreso = false;
int sonidoActivo = -1;
unsigned long tiempoInicioSonido = 0;
unsigned long duracionSonido = 0;

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
GFXcanvas1 Jugadorcanvas(36, 36);
GFXcanvas1 tiempocanvas(128, 16);
GFXcanvas1 Enemigocanvas(36, 36);
GFXcanvas1 AsteroideIzquierdacanvas(45, 45);
GFXcanvas1 AsteroideDerechacanvas(45, 45);

// Declaración de funciones
void setPlayerPosition(int x, int y);
void setEnemigoPosition(int xE, int yE);
void animatePlayer(void);
void moverPlayer(uint8_t direccion);
void moverPlayerDerecha(void);
void moverPlayerIzquierda(void);
void disparoPlayer(void);
void iniciarJuego(void);
void Conteotiempo(void);
void animateEnemigo(void);
void mostrarPantallaInicio(void);
void DisparoEnemigo(void);
void mostrarGameOver(void);
void ColisionDisparoJugador(void);
void ColisionEnemigoJugador(void);
void moverAsteroide(void);
void inicializarAsteroide1(void);
void inicializarAsteroide(void);
void ColisionAsteroideJugador(void);
void reproducirSonido(int sonido);


void setup() {
    Serial.begin(9600);
    Serial.println("Serial inicializado");

    attachInterrupt(digitalPinToInterrupt(botonRight), moverPlayerDerecha, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonLeft), moverPlayerIzquierda, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonDisparo), disparoPlayer, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonStart), iniciarJuego, HIGH);

    screen.begin();
    screen.fillScreen(ILI9341_BLACK);

    mostrarPantallaInicio();

    setPlayerPosition(0, YMAX-32);
    setEnemigoPosition(0, 50);
    
    sei();
}

void loop() {
    if(!juegoIniciado){
        delay(100);
        return;
    }

    animatePlayer();
    animateEnemigo();
    DisparoEnemigo();
    ColisionDisparoJugador();
    ColisionEnemigoJugador();
    moverAsteroide(); 
    ColisionAsteroideJugador();

    unsigned long currentmillis = millis();
    if (currentmillis - prevmillis >= 1000) {
        tiempo++;
        prevmillis = currentmillis;
        Conteotiempo();
    }

    if (!asteroideActivo) {
        inicializarAsteroide();
    }

    if (!asteroideActivo1) {
        inicializarAsteroide1();
    }

     if (disparoActivo) {
        screen.fillRect(prevDisparoX, prevDisparoY, disparoAncho, disparoAlto, ILI9341_BLACK);

        prevDisparoX = disparoX;
        prevDisparoY = disparoY;

        disparoY -= 20;

        if (disparoY < 0) {
            disparoActivo = false;
        } else {
            screen.fillRect(disparoX, disparoY, disparoAncho, disparoAlto, ILI9341_BLUE);
        }
    }
}

void mostrarPantallaInicio(){
    screen.fillScreen(ILI9341_BLACK);
    int playerX = (2 * (XMAX / 3)) - 16;
    int playerY = YMAX - 75;
    int enemigoX = (XMAX / 3) - 16;
    int enemigoY = YMAX - 75;

    Jugadorcanvas.drawBitmap(0, 0, Player[count], 32, 32, ILI9341_WHITE);
    screen.drawBitmap(playerX, playerY, Jugadorcanvas.getBuffer(), Jugadorcanvas.width(), Jugadorcanvas.height(), ILI9341_YELLOW);
    Enemigocanvas.drawBitmap(0, 0, Enemigo[count], 32, 32, ILI9341_WHITE);
    screen.drawBitmap(enemigoX, enemigoY, Enemigocanvas.getBuffer(), Enemigocanvas.width(), Enemigocanvas.height(), ILI9341_RED);

    screen.setTextSize(3);
    screen.setTextColor(ILI9341_WHITE);
    screen.setCursor(10, YMAX / 2 - 40);
    screen.print("Presiona");
    screen.setCursor(10, YMAX / 2 - 20);
    screen.print("Start");
    screen.setCursor(10, YMAX / 2);
    screen.print("para iniciar");
}

void iniciarJuego() {
    if (digitalRead(botonStart) == HIGH) {
        juegoIniciado = true;
        screen.fillScreen(ILI9341_BLACK); // Limpiar la pantalla
        setPlayerPosition(XMAX / 2 - 16, YMAX - 50); // Inicializar la posición del jugador
        tiempo = 0; // Reiniciar tiempo
        prevmillis = millis();
        Serial.println("Juego iniciado");
    }
}

void setPlayerPosition(int x1, int y1) {
    x = x1;
    y = y1;
}

void animatePlayer(void) {

    screen.fillRect(prev_x, prev_y, 32, 32, ILI9341_BLACK); 
    Jugadorcanvas.drawBitmap(0, 0, Player[count], 32, 32, ILI9341_WHITE); 
    screen.drawBitmap(x, y, Jugadorcanvas.getBuffer(), Jugadorcanvas.width(), Jugadorcanvas.height(), ILI9341_CYAN, ILI9341_BLACK);

    prev_x = x;
    prev_y = y;

    count++;
        if(count == 2)
            count = 0;
    
}

void Conteotiempo(void) {

    tiempocanvas.fillRect(0, 0, XMAX, 16, ILI9341_BLACK); // Limpia el área del texto
    tiempocanvas.setTextColor(ILI9341_WHITE);
    tiempocanvas.setCursor(0, 0);
    tiempocanvas.setTextSize(2);
    tiempocanvas.print(tiempo);
    screen.drawBitmap((XMAX - tiempocanvas.width()) / 2, 0, tiempocanvas.getBuffer(), tiempocanvas.width(), tiempocanvas.height(), ILI9341_WHITE, ILI9341_BLACK);

}

void disparoPlayer() {
   if (digitalRead(botonDisparo) == HIGH) {
        Serial.println("Boton Disparo");
        if (!disparoActivo) {
            Serial.println("Cambiando a Disparo Activo"); 
            disparoX = x + 15;  // Centra el disparo en el jugador 
            disparoY = y - disparoAlto;  // Coloca el disparo justo encima del jugador
            disparoActivo = true; 
            reproducirSonido(disJugador); 
        }
    }
}

void moverPlayerDerecha() {
    delay(500);
    if(digitalRead(botonRight) == HIGH) {
        Serial.println("Boton RIGHT");
        moverPlayer(RIGHT); 
    }    
}

void moverPlayerIzquierda() {
    delay(500);
    if(digitalRead(botonLeft) == HIGH) {
        Serial.println("Boton LEFT");
        moverPlayer(LEFT);  
    }
}

void moverPlayer(uint8_t direccion) {
    uint8_t delta = 2;
    
        if (direccion == RIGHT && x < XMAX - 32) { // 32 es el ancho del jugador
            x += delta;
        } else if (direccion == LEFT && x > 0) {
            x -= delta;
    }
}

void setEnemigoPosition(int x2, int y2) {
    x = x2;
    y = y2;
}

void animateEnemigo(void) {

    unsigned long currentMillis = millis();

    if (currentMillis - prevEnemyMillis >= enemySpeed) {
        prevEnemyMillis = currentMillis;

        screen.fillRect(prev_xE, prev_yE, 32, 32, ILI9341_BLACK);

        prev_xE = xE;
        prev_yE = yE;

        xE += 10 * enemyDirection;

        if (xE <= 0 || xE >= XMAX - 32) {
            enemyDirection *= -1; // Cambiar dirección
        }

        yE += 5; // Baja un poco en cada actualización
        if (yE > YMAX) {
            yE = 0; // Reiniciar a la parte superior
        }

        Enemigocanvas.drawBitmap(0, 0, Enemigo[count], 32, 32, ILI9341_WHITE); 
        screen.drawBitmap(xE, yE, Enemigocanvas.getBuffer(), Enemigocanvas.width(), Enemigocanvas.height(), ILI9341_RED);
    }

    count++;
        if(count == 2)
            count = 0;

    if (currentMillis - prevEnemyShotMillis >= enemyShotSpeed && !disparoEnemigoActivo) {
    prevEnemyShotMillis = currentMillis;
    disparoEnemigoX = xE + 15;  // Centra el disparo en el enemigo
    disparoEnemigoY = yE + 32; // Aparece debajo del enemigo
    disparoEnemigoActivo = true;
    }
  
}

void DisparoEnemigo() {
    if (disparoEnemigoActivo) {

        if (prevDisparoEnemigoY != disparoEnemigoY) {
            reproducirSonido(disEnemigo);
        }
        // Borrar el disparo en la posición anterior
        screen.fillRect(prevDisparoEnemigoX, prevDisparoEnemigoY, disparoEnemigoAncho, disparoEnemigoAlto, ILI9341_BLACK);

        // Actualizar posición previa
        prevDisparoEnemigoX = disparoEnemigoX;
        prevDisparoEnemigoY = disparoEnemigoY;

        // Mover el disparo hacia abajo
        disparoEnemigoY += 15;

        // Si el disparo sale de la pantalla, desactivarlo
        if (disparoEnemigoY > YMAX) {
            disparoEnemigoActivo = false;
        } else {
            // Dibujar el disparo en la nueva posición
            screen.fillRect(disparoEnemigoX, disparoEnemigoY, disparoEnemigoAncho, disparoEnemigoAlto, ILI9341_GREEN);
        }

        // colisión con el jugador
        if (disparoEnemigoX >= x && disparoEnemigoX <= x + 32 &&
            disparoEnemigoY >= y && disparoEnemigoY <= y + 32) {
            // Colisión detectada
            Serial.println("Jugador impactado por el disparo enemigo!");
            disparoEnemigoActivo = false;
            mostrarGameOver(); 
            reproducirSonido(colision);
        }
      } 
}

void ColisionDisparoJugador() {
    // Verifica si el disparo está en contacto con el enemigo
       // Verifica si el disparo está en contacto con el enemigo
    if (disparoActivo && disparoX >= xE && disparoX <= xE + 32 && disparoY <= yE + 32) {
        // Colisión detectada
        disparoActivo = false; // Desactiva el disparo

        // Borra el disparo y el enemigo en la posición de la colisión
        screen.fillRect(disparoX, disparoY, disparoAncho, disparoAlto, ILI9341_BLACK); // Borra el disparo
        screen.fillRect(xE, yE, 32, 32, ILI9341_BLACK); // Borra el enemigo

        xE = random(0, XMAX - 32); 
        yE = 16;  

        Enemigocanvas.drawBitmap(0, 0, Enemigo[count], 32, 32, ILI9341_WHITE);
        screen.drawBitmap(xE, yE, Enemigocanvas.getBuffer(), Enemigocanvas.width(), Enemigocanvas.height(), ILI9341_YELLOW);
   
        reproducirSonido(colision);

        enemigosDerrotados++;
    }

}

void ColisionEnemigoJugador() {
    if (xE + 32 >= x && xE <= x + 32 && 
        yE + 32 >= y && yE <= y + 32) {
        reproducirSonido(colision);
        Serial.println("¡Jugador impactado por el enemigo!");
        mostrarGameOver();
    } 
}

void inicializarAsteroide() {
    xAsteroide = -40;                     
    yAsteroide = random(20, YMAX - 80); 
    velXAsteroide = random(10, 14);        
    velYAsteroide = random(10, 14); 
    asteroideActivo = true;  
} 

void inicializarAsteroide1() {
    xAsteroide1 = XMAX;                     
    yAsteroide1 = random(20, YMAX - 80); 
    velXAsteroide1 = random(15, 19);        
    velYAsteroide1 = random(15, 19); 
    asteroideActivo1 = true;             
}

void moverAsteroide() {
    if (!asteroideActivo) return;

    // Borrar la posición previa del asteroide
    screen.fillRect(xAsteroide, yAsteroide, 40, 40, ILI9341_BLACK);
    
    // Actualizar la posición
    xAsteroide += velXAsteroide;
    yAsteroide += velYAsteroide;

    screen.drawBitmap(xAsteroide, yAsteroide, AsteroideIzquierda, 40, 40, ILI9341_ORANGE, ILI9341_BLACK);

    // Si el asteroide sale de la pantalla, reiniciar
    if (xAsteroide > XMAX || yAsteroide > YMAX) {
        inicializarAsteroide();
    }

    if (!asteroideActivo1) return;

    // Borrar la posición previa del asteroide
    screen.fillRect(xAsteroide1, yAsteroide1, 40, 40, ILI9341_BLACK);
    
    // Actualizar la posición
    xAsteroide1 -= velXAsteroide1;
    yAsteroide1 += velYAsteroide1;

    screen.drawBitmap(xAsteroide1, yAsteroide1, AsteroideDerecha, 40, 40, ILI9341_ORANGE, ILI9341_BLACK);
    

    // Si el asteroide sale de la pantalla, reiniciar
    if (xAsteroide1 < -40 || yAsteroide1 > YMAX) {
        inicializarAsteroide1();
    }
    
}

void ColisionAsteroideJugador() {
    if (asteroideActivo && 
        xAsteroide + 32 >= x && xAsteroide <= x + 32 &&
        yAsteroide + 32 >= y && yAsteroide <= y + 32) {
        reproducirSonido(colision);
        Serial.println("¡Colisión con asteroide 1!");
        mostrarGameOver();
    }

    if (asteroideActivo1 && 
        xAsteroide1 + 32 >= x && xAsteroide1 <= x + 32 &&
        yAsteroide1 + 32 >= y && yAsteroide1 <= y + 32) {
        reproducirSonido(colision);
        Serial.println("¡Colisión con asteroide 2!");
        mostrarGameOver();
    }
}

void mostrarGameOver() {
    screen.fillScreen(ILI9341_BLACK); 

    screen.setTextSize(3);
    screen.setTextColor(ILI9341_RED);
    screen.setCursor((XMAX - 150) / 2, (YMAX / 2) - 50);
    screen.print("GAME OVER");

    screen.setTextSize(2);
    screen.setTextColor(ILI9341_WHITE);
    screen.setCursor((XMAX - 100) / 2, (YMAX / 2) - 10);
    screen.print("Tiempo: ");
    screen.print(tiempo);

    screen.setTextSize(2);
    screen.setTextColor(ILI9341_WHITE);
    screen.setCursor((XMAX - 100) / 2, (YMAX / 2) + 30);
    screen.print("Enemigos ");
    screen.setCursor((XMAX - 120) / 2, (YMAX / 2) + 50);
    screen.print("Derrotados: ");
    screen.print(enemigosDerrotados);  

    delay(3000); 

    enemigosDerrotados = 0; 
    juegoIniciado = false;
    juegoTerminado = false;
    tiempo = 0;  // Reiniciar el tiempo
    prevmillis = millis();  // Reiniciar el tiempo de cuenta
    screen.fillScreen(ILI9341_BLACK);  // Limpiar la pantalla al reiniciar
    mostrarPantallaInicio();  // Mostrar pantalla de inicio
}

void reproducirSonido(int sonido) {

   switch (sonido) {
        case disEnemigo:
            tone(buzzer, TONO_DIS_ENEMIGO, DURACION_DIS_ENEMIGO); // Reproduce tono para disparo enemigo
            break;

        case disJugador:
            tone(buzzer, TONO_DIS_JUGADOR, DURACION_DIS_JUGADOR); // Reproduce tono para disparo jugador
            break;

        case colision:
            tone(buzzer, TONO_COLISION, DURACION_COLISION); // Reproduce tono para colisión jugador-enemigo
            break;

        default:
            noTone(buzzer); // Detiene cualquier tono si no se reconoce la acción
            break;
    }
}