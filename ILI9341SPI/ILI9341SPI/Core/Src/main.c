/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 * Proyecto 02 - Angie Recinos 23294 | Edvin Paiz
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "bitmaps.h"
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    ESTADO_PORTADA,
    ESTADO_NIVEL_1,
    ESTADO_NIVEL_2,
    ESTADO_GAMEOVER
} EstadoJuego;

typedef struct {
    int x1, x2;      // Rango horizontal de la plataforma
    int y_inicio;    // Altura en x1
    int y_fin;       // Altura en x2 (si y_inicio != y_fin, está inclinada)
} Plataforma;

typedef struct {
    int x_centro;    // Posición X de la escalera
    int y_piso;     // Donde empieza abajo
    int y_techo;     // Donde termina arriba
    int ancho;       // Margen de la escalera (detectar que Mario está cerca)
} Escalera;
typedef enum {
    BARRIL_RODANDO,
    BARRIL_BAJANDO_ESC,
    BARRIL_CAYENDO // Por si cae
} EstadoBarril;

typedef struct {
    int x, y;
    int x_ant, y_ant;
    int dir;           // 1: derecha, -1: izquierda
    EstadoBarril estado;
    int activo;        // 0: libre para reusar, 1: en pantalla
    int anim;          // Cuadro de animación
} Barril;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
// ------------- Variables para fondos -----------------
extern const uint16_t inicio[];
extern const uint16_t fondo_nivel1[];
extern const uint16_t fondo_nivel2[];

// ------------- Variables para los UART -------------
volatile uint8_t ctrl_cmd1;
volatile uint8_t ctrl_cmd2;
volatile uint8_t ctrl_cmd6;
volatile uint8_t rx_data1;
volatile uint8_t rx_data2;
volatile uint8_t rx_data6;

// ------------- Posiciones de mario (actual y anterior) -------------
//int mario_x = 0;
//int mario_y = 210;
int mario_x = 15;
int mario_y = 287;
int mario_x_ant = 50;
int mario_y_ant = 50;
int mario_flip = 0;
int mario_y_past = 0;

// ------------- Buffer para sprites -------------
extern uint16_t sprite_buffer[1024];

// ------------- Cambios de pantalla -------------
EstadoJuego estadoActual = ESTADO_PORTADA;
uint8_t cambioDePantalla = 1;

// ------------- Variables para salto -------------
int anim = 0;
uint16_t *mario_actual = mario_camina; // Puntero al arreglo que se va a dibujar
int mario_ancho_hoja = 48;             // Por defecto la de caminar tiene 3 cuadros
int en_aire = 0;					   // Bandera para ver si está saltando
int salto_timer = 0; 				   // Cuenta tiempo de salto
int jumpState = 0;    // 0: Suelo, 1: Subiendo, 2: Bajando
int jumpProgress = 0; // Qué tanto ha saltado
int jumpMax = 16;     // Altura máxima del salto en píxeles
int jumpEvent = 0;
char comando;
int inercia_x  = 0;
int timer_inercia = 0;
char comando_anterior = '0';
int mario_y_objetivo = 0;
// --------------- Variables para donkey --------------
uint32_t frame_counter = 0;
/* USER CODE END PV */

// --------------- Plataformas -----------------
/*const Plataforma nivel1[] = {
    {0, 320, 230, 226},   // Viga 1: De izquierda a derecha, subiendo
    {11, 287, 195, 203},  // Viga 2: De izquierda a derecha, bajando
    {32, 266, 173, 166},   // Viga 3: Izquierda a derecha, subiendo
    {12, 246, 135, 141},   // Viga 4: De izquierda a derecha, bajando
    {23, 267, 112, 106},  // Viga 5: De izquierda a derecha, subiendo
    {12, 235, 78, 81},   // Viga 6: Izquierda a derecha, bajando
	{130, 182, 54, 55},	 // Viga 7: Levemente bajando
};*/

const Plataforma nivel1[] = {
    {130, 182, 54, 55},   // Viga 7 (arriba)
    {12, 235, 78, 81},    // Viga 6
    {23, 267, 112, 106},  // Viga 5
    {12, 246, 135, 141},  // Viga 4
    {32, 266, 173, 166},  // Viga 3
    {11, 287, 195, 203},  // Viga 2
    {0, 320, 230, 226}    // Viga 1 (abajo)
};
const Plataforma *plataformas_actuales = nivel1;
int numPlataformas_actual = 7;
// ---------------- Escaleras -------------------
const Escalera nivel1_escaleras[] = {
    {255, 226, 187, 11}, 	// Esc. 1 (rangos de altura)
    {138, 187, 156, 11},    // Esc. 2
    {54, 182, 153, 11},		// Esc. 3
	{158, 158, 124, 10},	// Esc. 4
	{108, 124, 98, 10},		// Esc. 5
	{43, 123, 100, 9},		// Esc. 6
	{212, 98, 68, 11},		// Esc. 7
	{169, 66, 43, 11}		// Esc. 8
};
int numEscaleras_actual = 8;
const Escalera *escaleras_actuales = nivel1_escaleras;
int escalera_id = 0; // Para recordar cuál escalera está usando Mario
int mario_sube = 0;	// Bandera para escaleras

// -------------------- Barriles --------------
#define MAX_BARRILES 4
Barril barriles[MAX_BARRILES];
int spawn_timer = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void actualizarY_Mario(void);
void crearBarril(void);
void actualizarY_Objeto(int *obj_x, int *obj_y);
void actualizarBarriles(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*void actualizarY_Mario(void) {
    for (int i = 0; i < numPlataformas_actual; i++) {
        // Se revisa si mario está parado sobre la viga
        if (mario_x >= plataformas_actuales[i].x1 && mario_x <= plataformas_actuales[i].x2) {
            // Se calcula el ancho de la viga con x2-x1
            int anchoViga = plataformas_actuales[i].x2 - plataformas_actuales[i].x1;
            // El avance será la posición de mario - x1 (que tanto ha avanzado en esa viga)
            int avanceX = mario_x - plataformas_actuales[i].x1;
            // Se calcula la diferencia en y
            int diferenciaY = plataformas_actuales[i].y_fin - plataformas_actuales[i].y_inicio;

            // FÓRMULA DE LA PENDIENTE
            // Se resta la altura de Mario para que toquen los pies y no la cabeza
            mario_y = plataformas_actuales[i].y_inicio + ((avanceX * diferenciaY) / anchoViga) - 16;

            break; // Salimos del for porque ya encontramos el suelo
        }
    }
}*/
void actualizarY_Mario(void) {
    int viga_encontrada = -1;
    int menor_distancia = 1000;

    for (int i = 0; i < numPlataformas_actual; i++) {
        // Revisamos si mario está en la viga
        if (mario_x + 8 >= plataformas_actuales[i].x1 && mario_x + 8 <= plataformas_actuales[i].x2) {

            // Calculamos la altura para la plataforma en este punto X
            int anchoViga = plataformas_actuales[i].x2 - plataformas_actuales[i].x1;
            int avanceX = (mario_x + 8) - plataformas_actuales[i].x1;
            int diferenciaY = plataformas_actuales[i].y_fin - plataformas_actuales[i].y_inicio;

            // Altura teórica en este punto (se usa la fórmila de la pendiente)
            int y_teorico = plataformas_actuales[i].y_inicio + ((avanceX * diferenciaY) / anchoViga) - 16;

            // Se busca la viga que teóricamente tiene debajo de los pies (la más cercana)
            int distancia = mario_y - y_teorico;

            // Solo la tomamos en cuenta si está cerca (evita saltar a vigas que están lejos)
            // y si es la más cercana que tenemos actualmente
            if (distancia >= -5 && distancia < menor_distancia) {
                menor_distancia = distancia;
                viga_encontrada = i;
                mario_y_objetivo = y_teorico;
            }
        }
    }

    // Si encontramos la viga correcta, se actualiza la posición en y
    if (viga_encontrada != -1) {
        mario_y = mario_y_objetivo;
    }
}

void crearBarril(void) {
    for (int i = 0; i < MAX_BARRILES; i++) {
        if (barriles[i].activo == 0) {
            barriles[i].x = 67; // Donde está Donkey Kong
            barriles[i].y = 45; // Ajustado a la altura de la viga 7
            barriles[i].dir = 1;
            barriles[i].estado = BARRIL_RODANDO;
            barriles[i].activo = 1;
            break;
        }
    }
}

void actualizarY_Objeto(int *obj_x, int *obj_y) {
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int y_final = *obj_y; // Empezamos con la Y actual

    for (int i = 0; i < numPlataformas_actual; i++) {
        // Usamos el centro del objeto (8 porque el barril mide 16)
        int checkX = (*obj_x) + 8;

        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int anchoViga = plataformas_actuales[i].x2 - plataformas_actuales[i].x1;
            int avanceX = checkX - plataformas_actuales[i].x1;
            int diferenciaY = plataformas_actuales[i].y_fin - plataformas_actuales[i].y_inicio;

            // Calculamos el suelo en ese punto X
            int y_teorico = plataformas_actuales[i].y_inicio + ((avanceX * diferenciaY) / anchoViga) - 16;

            int distancia = (*obj_y) - y_teorico;

            // Busca la viga más cercana
            if (distancia >= -10 && distancia < menor_distancia) {
                menor_distancia = distancia;
                y_final = y_teorico;
                viga_encontrada = i;
            }
        }
    }

    if (viga_encontrada != -1) {
        *obj_y = y_final;
    }
}

void actualizarBarriles(void) {
    for (int i = 0; i < MAX_BARRILES; i++) {
        if (!barriles[i].activo) continue;

        // Guardar posición anterior para borrar rastro
        barriles[i].x_ant = barriles[i].x;
        barriles[i].y_ant = barriles[i].y;

        if (barriles[i].estado == BARRIL_RODANDO) {
            // Mover en X
            barriles[i].x += (barriles[i].dir * 3); // Velocidad del barril

            // Aquí se llama a una versión simplificada de actualizarY para el barril
            actualizarY_Objeto(&barriles[i].x, &barriles[i].y);

            //Decidir si baja por una escalera (random)
            for (int e = 0; e < numEscaleras_actual; e++) {
                if (abs(barriles[i].x - nivel1_escaleras[e].x_centro) < 5) {
                    // Si está sobre una escalera, hay un 10% de probabilidad de que baje
                    if (rand() % 100 < 10) {
                        barriles[i].estado = BARRIL_BAJANDO_ESC;
                        barriles[i].x = nivel1_escaleras[e].x_centro; // Alineación
                    }
                }
            }
        }
        else if (barriles[i].estado == BARRIL_BAJANDO_ESC) {
            barriles[i].y += 4; // Baja más rápido de lo que rueda

            // Se detecta si tocó la viga de abajo
            // Si la distancia a la siguiente viga es casi 0, vuelve a rodar
            // y cambiar de dirección: barriles[i].dir *= -1;
        }

        // Se calcula el sprite
        barriles[i].anim = (barriles[i].x / 8) % 4;

        // 5. Salida de pantalla (Desactivar los barriles)
        if (barriles[i].x > 320 || barriles[i].x < -16 || barriles[i].y > 240) {
        	LCD_RestaurarFondo(barriles[i].x, barriles[i].y, 16, 16, fondo_nivel1, 320, 31);
        	barriles[i].activo = 0;
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	LCD_Init();

	LCD_Clear(0x00);

	// Inicio de UARTs
	HAL_UART_Receive_IT(&huart1, &rx_data1, 1);
	HAL_UART_Receive_IT(&huart2, &rx_data2, 1);
	HAL_UART_Receive_IT(&huart6, &rx_data6, 1);
	//FillRect(90, 60, 20, 20, 0x001F);

	//LCD_Bitmap(0, 31, 320, 209, fondo);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		if (ctrl_cmd2 != 0) {
		        comando = ctrl_cmd2;
		        ctrl_cmd2 = 0; // Limpiar comando

		        // Transiciones de estado (Cambiar de nivel)
		                if (estadoActual == ESTADO_PORTADA && comando == 's') { // s para Start
		                    estadoActual = ESTADO_NIVEL_1;
		                    cambioDePantalla = 1;
		                }
		                else if (estadoActual == ESTADO_NIVEL_1 && comando == 'n') { // n para Next
		                    estadoActual = ESTADO_NIVEL_2;
		                    cambioDePantalla = 1;
		                }

		                // Movimiento (Solo si estamos en un nivel)
		                if (estadoActual == ESTADO_NIVEL_1 || estadoActual == ESTADO_NIVEL_2) {
		                	// Revisamos comandos enviados por UART
		                	// Para desplazamientos actualizamos posición de mario
		                	if (comando == 'j'){
		                		jumpEvent = 1;
		                		}
		                	if (comando == 'r'){
		                    	mario_x += 5; // Derecha
		                    	mario_flip = 1;
		                    	//comando_anterior = 'r';
		                    	inercia_x = 2;

		                    }
		                    if (comando == 'l'){
		                    	mario_x -= 5; // Izquierda
		                    	mario_flip = 0;
		                    	//comando_anterior = 'l';
		                    	inercia_x = -2;

		                    }
		                    if (comando == 'u') {

		                    	mario_sube = 1;
		                    }
		                    if (comando == 'd') {

		                    	mario_sube = 2;
		                    }

		                }
		}

		            // --- DIBUJAR SEGÚN EL ESTADO ---
		            switch (estadoActual) {
		                case ESTADO_PORTADA:
		                    if (cambioDePantalla) {
		                        LCD_Clear(0x0000);
		                        LCD_Bitmap(0, 0, 320, 240, inicio);
		                        LCD_Print("PRESIONA 'S' PARA INICIAR", 65, 180, 1, 0xFFFF, 0x0000);
		                        cambioDePantalla = 0;
		                    }
		                    break;

		                case ESTADO_NIVEL_1:
		                    if (cambioDePantalla) {
		                        LCD_Clear(0x0000);
		                        LCD_Bitmap(0, 31, 320, 209, fondo_nivel1);
		                        cambioDePantalla = 0;
		                        frame_counter = 0;
		                        spawn_timer = 0;

		                        //barriles[0].x = 100; barriles[0].y = 100; barriles[0].activo = 1; barriles[0].dir = 1;
		                    }
		                    spawn_timer++;
		                        if (spawn_timer > 150) {
		                            crearBarril();       // se crean barriles
		                            spawn_timer = 0;     // Reiniciamos el reloj para el siguiente barril
		                        }
		                    // Muestra sprites fijos
		                    // Muestra a donkey kong
		                    int dk_anim = (frame_counter / 15) % 3;

		                    while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                    LCD_DibujarSpriteUniversal(35, 45, 32, 32, donkey_barril, dk_anim, 96,
		                                               fondo_nivel1, 320, 0x0000, 31, 0);
		                    HAL_Delay(10);
		                    // Mostrar a la princesa
		                    int princess_anim = (frame_counter /15) % 2;
		                    while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                    LCD_DibujarSpriteUniversal(127, 32, 44, 22, princesa, princess_anim, 88,
		                   		                       fondo_nivel1, 320, 0x0000, 31, 0);
		                    HAL_Delay(10);
		                    frame_counter++;
		                    actualizarBarriles();

		                    // codigo para barriles
		                    for (int i = 0; i < MAX_BARRILES; i++) {
		                        if (barriles[i].activo) {
		                            // 1. Borrar posición vieja
		                            LCD_RestaurarFondo(barriles[i].x_ant, barriles[i].y_ant, 16, 16, fondo_nivel1, 320, 31);

		                            // 2. Dibujar posición nueva
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteUniversal(
		                                barriles[i].x, barriles[i].y,
		                                16, 16,
		                                barril_lateral,
		                                barriles[i].anim,
		                                64,            // El ancho total 64x16
		                                fondo_nivel1, 320, 0x0000, 31, 0
		                            );
		                        }
		                    }

		                    // Lógica del salto con inercia
		                    if (comando == 'j' && jumpState == 0) {
		                            jumpState = 1;     // Empezar a subir
		                            jumpProgress = 0;

		                            comando = '0';
		                            mario_y_past = mario_y;
		                            mario_actual = mario_brinca;
		                            mario_ancho_hoja = 16;
		                    }

		                    // Si está saltando mueve en x simulando inercia
		                    if(jumpState != 0){
		                    	mario_x += inercia_x;
		                    }
		                    // Aquí va caminado
		                    if (jumpState == 0){
		                    	int enEscaleraActiva = 0;

		                    	for (int i = 0; i < numEscaleras_actual; i++) {
		                    	        // Se revisa si está alineado con la escalera
		                    	        if (mario_x > (nivel1_escaleras[i].x_centro - 8) &&
		                    	            mario_x < (nivel1_escaleras[i].x_centro + 8)) {

		                    	            // Se revisa si está entre el piso y el techo, se añade un margen de 2 como +/-
		                    	            if (mario_y <= nivel1_escaleras[i].y_piso && mario_y >= (nivel1_escaleras[i].y_techo - 2)) {

		                    	                // Mientras Mario esté en este rango, bloqueamos la rampa
		                    	            	// indicando que está en una escalera
		                    	                enEscaleraActiva = 1;

		                    	                // Solo si hay comando, movemos la posición
		                    	                if (mario_sube == 1 || mario_sube == 2) {
		                    	                    mario_x = nivel1_escaleras[i].x_centro; // Se jala a mario para el centro de la escalera

		                    	                    if (mario_sube == 1) mario_y -= 4;
		                    	                    if (mario_sube == 2) mario_y += 4;

		                    	                    // Identificamos qué sprite usar
		                    	                    int dist = mario_y - nivel1_escaleras[i].y_techo;
		                    	                    if (dist <= 4 && dist > 0) {
		                    	                        mario_actual = mario_escala;	// Terminando de subir
		                    	                        mario_ancho_hoja = 64;
		                    	                        anim = (mario_y / 8) % 4;

		                    	                    } else {
		                    	                        mario_actual = mario_subiendo; // Espaldas
		                    	                        mario_ancho_hoja = 32;
		                    	                        anim = (mario_y / 8) % 2;
		                    	                    }

		                    	                    mario_sube = 0; // Limpiamos bandera
		                    	                }

		                    	                // Se actualiza la posicion de mario siempre que este dentro del rango
		                    	                if (mario_y <= nivel1_escaleras[i].y_techo) {
		                    	                    mario_y = nivel1_escaleras[i].y_techo;
		                    	                    mario_y_past = mario_y;
		                    	                    // Solo si no está bajando, cambiamos de sprite
		                    	                    mario_actual = mario_camina;
		                    	                    mario_ancho_hoja = 48;
		                    	                }
		                    	                if (mario_y >= nivel1_escaleras[i].y_piso) {
		                    	                    mario_y = nivel1_escaleras[i].y_piso;
		                    	                    mario_actual = mario_camina;
		                    	                    mario_ancho_hoja = 48;

		                    	                }

		                    	                break;
		                    	            }
		                    	        }
		                    	    }

		                    	    // Si no hay escalera, entonces hace la rampa
		                    	    if (enEscaleraActiva == 0) {
		                    	        actualizarY_Mario();
		                    	    }
		                    }

		                    if (jumpState == 1) { // SUBIENDO - en el salto
		                            int step = 2; // Velocidad de subida

		                            if (jumpProgress +step > 16) {
		                                step = 16 - jumpProgress; // Llegó al tope, empezar a bajar
		                            }
		                            mario_y -= step;			// La y es inversa (si va para arriba -)
		                            jumpProgress += step;

		                            if (jumpProgress >= 16){
		                            	jumpState = 2;
		                            }

		                        }
		                    else if (jumpState == 2) { // BAJANDO
		                            int step = 2; // Velocidad de bajada
		                            if(jumpProgress - step < 0){
		                            	step  = jumpProgress;
		                            }
		                            mario_y += step;
		                            jumpProgress -= step;

		                            if (mario_y >= mario_y_past) { // altura del "suelo/viga"
		                                mario_y = mario_y_past;
		                                jumpState = 0;    // Tocar suelo
		                                jumpProgress = 0;
		                                inercia_x = 0;
		                                mario_actual = mario_camina;
		                                mario_ancho_hoja = 48;

		                            }
		                        }

		                    // Límites Horizontales
		                    if (mario_x < 0) mario_x = 0;
		                    if (mario_x > 320 - 16) mario_x = 320 - 16;

		                    // Límites Verticales (Respetando inicio de fondo en 31)
		                    if (mario_y < 31) mario_y = 31;
		                    if (mario_y > 240 - 16) mario_y = 240 - 16;

		                    if (mario_x != mario_x_ant || mario_y != mario_y_ant) {
		                        // Actualizamos posición, guardamos la posición actual como la nueva "anterior"
		                    	LCD_RestaurarFondo(mario_x_ant, mario_y_ant, 16, 16, fondo_nivel1, 320, 31);
		                    	mario_x_ant = mario_x;
		                        mario_y_ant = mario_y;
		                    	}
		                    anim = (mario_x / 10) % 3;

		                    while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                    LCD_DibujarSpriteUniversal(
		                        	mario_x, mario_y,           // Posición actual
		                        	16, 16,                     // Ancho y alto de Mario
		                        	mario_actual,                 // El arreglo de sprites
		                        	anim,                       // Cuál cuadro de la animación
		                        	mario_ancho_hoja,           // Ancho total del sprite sheet (ej: 30px * 6 frames)
		                        	fondo_nivel1,               // El arreglo con el fondo de Donkey Kong
		                        	320,                        // Ancho total del fondo (320px)
		                        	0x0000,                      // COLOR TRANSPARENTE (Negro)
									31,							// Offset del fondo
									mario_flip
		                       );
		                    HAL_Delay(20);
		                    break;
		                case ESTADO_NIVEL_2:
		                	if (cambioDePantalla) {
		                		LCD_Clear(0x0000);
		                		LCD_Bitmap(0, 31, 320, 209, fondo_nivel2); // Dibuja el mapa
		                		cambioDePantalla = 0;
		                	}
		                	anim = (mario_x / 10) % 3;
		                	//LCD_Sprite(mario_x, mario_y, 16, 16, mario_camina, 3, anim, 0, 0);
		                	LCD_DibujarSpriteUniversal(
		                			mario_x, mario_y,           // Posición actual
									16, 16,                     // Ancho y alto de Mario
									mario_camina,                 // El arreglo de sprites
									anim,                       // Cuál cuadro de la animación
									48,                        // Ancho total del sprite sheet (ej: 30px * 6 frames)
									fondo_nivel2,               // El arreglo con el fondo de Donkey Kong
									320,                        // Ancho total del fondo (320px)
									0x0000,                      // COLOR TRANSPARENTE (Negro)
									31,
									mario_flip
		                	);
		                	break;

		                    //int anim = (mario_x / 10) % 6;
		                    //LCD_Sprite(mario_x, mario_y, 30, 28, mario_mart, 6, anim, 0, 0);



		            }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RESET_Pin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DC_Pin */
  GPIO_InitStruct.Pin = LCD_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin SD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1) // Control 1
	    {
	    	ctrl_cmd1 = rx_data1;

	        //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

	        HAL_UART_Receive_IT(&huart1, &rx_data1, 1);

	    }

    if (huart->Instance == USART2) // Terminal para pruebas
    {
    	ctrl_cmd2 = rx_data2;

        //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

        HAL_UART_Receive_IT(&huart2, &rx_data2, 1);

    }

    if (huart->Instance == USART6) // Control 2
        {
        	ctrl_cmd6 = rx_data6;

            //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

            HAL_UART_Receive_IT(&huart6, &rx_data6, 1);

            switch (ctrl_cmd6)
            			  	  {
            			  		  	  case 'X':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:X\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		  		  break;

            			  		  	  case 'C':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:C\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		 		  break;
            			  		  	  case 'U':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:U\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		 		  break;

            			  		  	  case 'D':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:D\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		 		  break;

            			  		  	  case 'R':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:R\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		 		  break;

            			  		  	  case 'L':
            			  		  		  HAL_UART_Transmit(&huart2, (uint8_t*)"C2:L\r\n", 6, 1000);
            			  		  		  ctrl_cmd6 = 0;
            			  		 		  break;
            			  		  	  }
        }

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // Esto es lo mismo que LCD_CS_H()
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
