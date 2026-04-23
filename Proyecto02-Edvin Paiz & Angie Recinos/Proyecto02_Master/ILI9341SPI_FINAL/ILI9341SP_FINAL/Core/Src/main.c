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
 * Proyecto 02 - Angie Recinos 23294 | Edvin Paiz 23072
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "bitmaps.h"
#include <stdio.h>
#include <stdlib.h>
#include "fatfs_sd.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    ESTADO_PORTADA,
    ESTADO_NIVEL_1,
    ESTADO_NIVEL_2,
	ESTADO_NIVEL_3,
	ESTADO_NIVEL_4
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
} EstadoBarril;

typedef struct {
    int x, y;
    int x_ant, y_ant;
    int dir;           // 1: derecha, -1: izquierda
    EstadoBarril estado;
    int activo;        // 0: libre para reusar, 1: en pantalla
    int anim;          // Cuadro de animación

    int plataforma_actual;
    int plataforma_destino;
    int escalera_objetivo;
} Barril;

typedef struct {
    int x0, y0;
    int x1, y1;
    int altura;     // qué tan alto sube el arco
    int duracion;   // cuántos ticks tarda este salto
} SaltoRuta;

typedef struct {
    int x, y;
    int x_ant, y_ant;
    int activo;
    int anim;

    int ruta_idx;   // en qué salto va
    int t;          // progreso dentro del salto actual
    const SaltoRuta *ruta;
    int ruta_len;
} Fueguito;

typedef enum {
    TILE_FONDO    = 0,
    TILE_ESCALERA = 1,
    TILE_VIGA     = 2,
    TILE_V11      = 3,
    TILE_V12      = 4,
    TILE_V13      = 5,
    TILE_V14      = 6,
    TILE_V15      = 7,
    TILE_V16      = 8,
    TILE_V17      = 9,
    TILE_V21      = 10,
    TILE_V22      = 11,
    TILE_V23      = 12,
    TILE_V24      = 13,
    TILE_V25      = 14,
    TILE_V26      = 15,
    TILE_V27      = 16
} TileID;

typedef struct {
    int x, y;
    int x_ant, y_ant;
    int flip;
    int y_past;
    int jumpState;
    int jumpProgress;
    int inercia_x;
    int sube;       // Reemplaza a mario_sube
    uint8_t muriendo;
    uint8_t frame;
    uint8_t tick;
    const uint16_t *actual_bmp;
    int ancho_hoja;
    uint16_t color_transparente; // 0x0000 usualmente
    uint8_t frame_ant;
} Personaje;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
char buffer[100];

#define BTN_LEFT    (1 << 5)
#define BTN_RIGHT   (1 << 4)
#define BTN_UP      (1 << 3)
#define BTN_DOWN    (1 << 2)
#define BTN_SQUARE  (1 << 1)
#define BTN_X       (1 << 0)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
// ------------- Variables para jugadores --------------
Personaje p1, p2; // Mario y Luigi

// ------------- Variables para fondos -----------------
int dk_anim_prev = -1;
int princess_anim_prev = -1;

extern const uint16_t escalera1[];
extern const uint16_t escalera_2[];
extern const uint16_t escalera_4[];

extern const uint16_t viga[];
extern const uint16_t viga11[];
extern const uint16_t viga12[];
extern const uint16_t viga13[];
extern const uint16_t viga14[];
extern const uint16_t viga15[];
extern const uint16_t viga16[];
extern const uint16_t viga17[];

extern const uint16_t viga21[];
extern const uint16_t viga22[];
extern const uint16_t viga23[];
extern const uint16_t viga24[];
extern const uint16_t viga25[];
extern const uint16_t viga26[];
extern const uint16_t viga27[];

extern const uint16_t viga_2[];
extern const uint16_t plataforma_2[];
extern const uint16_t malla_2[];

extern const uint16_t viga_4[];


#define MAP_W 40
#define MAP_H 29

extern const uint8_t nivel1_mapa[MAP_H][MAP_W];
extern const uint8_t nivel2_mapa[MAP_H][MAP_W];
extern const uint8_t nivel3_mapa[MAP_H][MAP_W];
extern const uint8_t nivel4_mapa[MAP_H][MAP_W];


// ------------- Variables para los UART -------------
volatile uint8_t ctrl_state1;
volatile uint8_t ctrl_cmd2;
volatile uint8_t ctrl_state6;
uint8_t rx_data1;
uint8_t rx_data2;
uint8_t rx_data6;
uint8_t prev_state1 = 0;
uint8_t prev_state2 = 0;

// ------------- Variables para menu -------------------------------
uint8_t menu = 1;
uint8_t menu_print = 1;
uint8_t menu_up = 0;
uint8_t menu_down = 0;
uint8_t select = 0;
uint8_t jugadores = 1;
uint8_t nivel = 1;

// ------------- Posiciones de mario (actual y anterior) -------------
#define LVL2_MARIO_START_X 15

int mario_cayendo = 0;
int mario_vel_caida = 0;
#define MARIO_GRAVEDAD  1
#define MARIO_CAIDA_MAX 6

// ------------- Buffer para sprites -------------
extern uint16_t sprite_buffer[1024];

// ------------- Cambios de pantalla -------------
EstadoJuego estadoActual = ESTADO_PORTADA;
uint8_t cambioDePantalla = 1;

// ------------- Variables para salto -------------
int anim = 0;
const uint16_t *mario_actual = mario_camina; // Puntero al arreglo que se va a dibujar
int mario_ancho_hoja = 48;             // Por defecto la de caminar tiene 3 cuadros
int en_aire = 0;					   // Bandera para ver si está saltando
int salto_timer = 0; 				   // Cuenta tiempo de salto
int jumpState = 0;    // 0: Suelo, 1: Subiendo, 2: Bajando
int jumpProgress = 0; // Qué tanto ha saltado
int jumpMax = 22;     // Altura máxima del salto en píxeles
int jumpEvent = 0;
char comando;
int inercia_x  = 0;
int timer_inercia = 0;
char comando_anterior = '0';
int mario_y_objetivo = 0;
// --------------- Variables para donkey --------------
uint32_t frame_counter = 0;
uint8_t dk_lanzando = 0;
uint8_t dk_anim = 0;
uint8_t dk_anim_tick = 0;
uint8_t dk_barril_pendiente = 0;
uint8_t dk_spawns_pendientes = 0;

#define LVL2_DK_X 16
#define LVL2_DK_Y 32
#define LVL2_PRINCESS_X 120
#define LVL2_PRINCESS_Y 8


// --------------- Plataformas Nivel 1 -----------------
const Plataforma nivel1[] = {
    {130, 182, 47, 48},   // Viga 7 (arriba)
    {12, 235, 71, 74},    // Viga 6
    {23, 267, 105, 99},  // Viga 5
    {12, 246, 130, 134},  // Viga 4
    {32, 266, 166, 159},  // Viga 3
    {11, 287, 188, 196},  // Viga 2
    {0, 320, 223, 219}    // Viga 1 (abajo)
};


const Plataforma *plataformas_actuales = nivel1;
int numPlataformas_actual = 7;

// --------------- Plataformas Nivel 2 -----------------
const Plataforma nivel2[] = {
    {112, 176, 32, 32},   // plataforma superior
    {0,   320, 64, 64},   // fila 9
    {0,   320, 104, 104}, // fila 14

    {0,   56,  144, 144}, // fila 19 izquierda
    {88,  232, 144, 144}, // fila 19 centro
    {264, 320, 144, 144}, // fila 19 derecha

    {40,  280, 184, 184}, // fila 24
    {0,   320, 224, 224}  // piso
};

// ----------------- Plataformas Nivel 3 ----------------
const Plataforma nivel3[] = {
    {120, 184,  32,  32},  // fila 5
    {  0, 248,  64,  64},  // fila 9
    {296, 320,  80,  80},  // fila 11
    {272, 288,  88,  88},  // fila 12
    {240, 264,  96,  96},  // fila 13
    {200, 232, 104, 104},  // fila 14
    {  0,  40, 112, 112},  // fila 15 izquierda
    { 80, 120, 112, 112},  // fila 15 centro
    {128, 168, 128, 128},  // fila 17 centro
    {272, 320, 128, 128},  // fila 17 derecha
    { 48,  72, 136, 136},  // fila 18 izquierda
    {216, 232, 136, 136},  // fila 18 derecha
    {240, 264, 144, 144},  // fila 19
    {272, 288, 152, 152},  // fila 20
    {  0,  40, 160, 160},  // fila 21 izquierda
    {296, 320, 160, 160},  // fila 21 derecha
    { 48,  72, 176, 176},  // fila 23
    { 80, 120, 184, 184},  // fila 24 izquierda
    {136, 176, 184, 184},  // fila 24 centro
    {288, 320, 184, 184},  // fila 24 derecha
    {264, 280, 192, 192},  // fila 25
    {240, 256, 200, 200},  // fila 26
    {  0,  40, 208, 208},  // fila 27 izquierda
    {208, 232, 208, 208},  // fila 27 derecha
    {  0, 320, 224, 224}   // fila 29 piso
};

// --------------- Plataformas Nivel 4 -----------------
const Plataforma nivel4[] = {
    { 80, 232,  32,  32},   // fila 5

    { 64,  88,  72,  72},   // fila 10 izq
    {112, 200,  72,  72},   // fila 10 centro
    {224, 248,  72,  72},   // fila 10 der

    { 48,  88, 112, 112},   // fila 15 izq
    {112, 200, 112, 112},   // fila 15 centro
    {224, 264, 112, 112},   // fila 15 der

    { 32,  88, 152, 152},   // fila 20 izq
    {112, 200, 152, 152},   // fila 20 centro
    {224, 280, 152, 152},   // fila 20 der

    { 16,  88, 192, 192},   // fila 25 izq
    {112, 200, 192, 192},   // fila 25 centro
    {224, 296, 192, 192},   // fila 25 der

    {  0, 312, 224, 224}    // fila 29 piso
};

// ---------------- Escaleras -------------------
const Escalera nivel1_escaleras[] = {
    {255, 218, 179, 11},   // Esc. 1
    {138, 179, 148, 11},   // Esc. 2
    {54, 174, 145, 11},    // Esc. 3
    {158, 150, 116, 10},   // Esc. 4
    {108, 116, 90, 10},    // Esc. 5
    {43, 115, 92, 9},      // Esc. 6
    {212, 90, 60, 11},     // Esc. 7
    {169, 58, 35, 11}      // Esc. 8
};

const Escalera nivel2_escaleras[] = {
    {168,  48,   8,  8},   // arriba derecha
    {288,  88,  48,  8},   // 2a sección derecha
    {32,  128,  88,  8},   // 3a sección
    {120, 128,  88,  8},
    {192, 128,  88,  8},
    {280, 128,  88,  8},
    {88,  168, 128,  8},   // 4a sección
    {224, 168, 128,  8},
    {40,  208, 168,  8},   // 5a sección
    {120, 208, 168,  8},
    {192, 208, 168,  8},
    {272, 208, 168,  8}
};

const Escalera nivel3_escaleras[] = {
    {176,  56,  24, 8},   // entre fila 5 y fila 9
    {240,  88,  56, 8},   // entre fila 9 y fila 13
    {296, 120,  72, 8},   // entre fila 11 y fila 17
    {216, 128,  96, 8},   // entre fila 14 y fila 18
    { 32, 152, 104, 8},   // entre fila 15 izq y fila 21 izq
    { 80, 176, 104, 8},   // entre fila 15 centro y fila 24 izq
    {112, 176, 104, 8},   // entre fila 15 centro y fila 24 izq
    {272, 144, 120, 8},   // entre fila 17 der y fila 20
    {296, 176, 152, 8},   // entre fila 21 der y fila 24 der
    { 16, 200, 152, 8}    // entre fila 21 izq y fila 27 izq
};

const Escalera nivel4_escaleras[] = {
    // Entre fila 10 y fila 15
    { 64,  96,  56, 8 },   // izquierda

    // Entre fila 15 y fila 20
    {264, 136,  96, 8 },   // derecha

    // Entre fila 20 y fila 25
    { 32, 176, 136, 8 },   // izquierda

    // Entre fila 25 y fila 29
    {296, 208, 176, 8 }    // derecha
};

int numEscaleras_actual = 8;
const Escalera *escaleras_actuales = nivel1_escaleras;
int escalera_id = 0; // Para recordar cuál escalera está usando Mario
int mario_sube = 0;	// Bandera para escaleras

// -------------------- Barriles --------------
#define MAX_BARRILES 3
Barril barriles[MAX_BARRILES];
int spawn_timer = 0;
#define SCREEN_TOP      30
#define SCREEN_BOTTOM   240
#define SPRITE_BARRIL   16
const uint16_t *sprite_barril;
int hoja_barril;
int frame_barril;

// -------------------- Fueguitos --------------
#define MAX_FUEGOS 3
Fueguito fuegos[MAX_FUEGOS];
uint32_t fuego_tick = 0;

const SaltoRuta ruta_fuego_1[] = {
    // Ida
	{  48, 176,  128, 176, 18, 14 },
    { 128, 176, 200, 176, 18, 14 },
    { 200, 176, 260,  176, 18, 14 },
	// Regreso
	{ 260, 176, 200,  176, 18, 14 },
	{ 200, 176, 128,  176, 18, 14 },
	{ 128, 176, 48,  176, 18, 14 }
};

const SaltoRuta ruta_fuego_2[] = {
	// Ida
    { 96, 136, 150, 136, 18, 14 },
    { 150, 136, 200, 136, 18, 14 },
	// Regreso
    { 200, 136, 150,  136, 18, 14 },
    { 150, 136, 96,  136, 18, 14 }
};

const SaltoRuta ruta_fuego_3[] = {
	// Ida
    { 176, 96, 250, 96, 18, 14 },
    { 250, 96, 300, 96, 18, 14 },
	// Regreso
    { 300, 96, 250,  96, 18, 14 },
    { 250,  96, 176,  96, 18, 14 }
};

const SaltoRuta ruta_fuego_4[] = {
	// Ida
	{  48, 160,  128, 168, 18, 14 },
	{ 128, 168, 150, 168, 18, 14 },
	{ 150, 168, 260,  176, 18, 14 },
	// Regreso
	{ 260, 176, 150,  168, 18, 14 },
	{ 150, 168, 128,  168, 18, 14 },
	{ 128, 168, 48,  160, 18, 14 }
};

const SaltoRuta ruta_fuego_5[] = {
	// Ida
	{ 16, 96, 56, 120, 18, 14 },
	{ 56, 120, 96, 96, 18, 14 },
	// Regreso
	{ 96, 96, 56,  120, 18, 14 },
	{ 56, 120, 16,  96, 18, 14 }
};

const SaltoRuta ruta_fuego_6[] = {
	// Ida
    { 200, 88, 250, 80, 18, 14 },
    { 250, 80, 300, 64, 18, 14 },
	// Regreso
    { 300, 64, 250,  80, 18, 14 },
    { 250, 88, 200,  88, 18, 14 }
};

const SaltoRuta ruta_fuego_7[] = {
    // Ida
	{  48, 176,  128, 176, 18, 14 },
    { 128, 176, 200, 176, 18, 14 },
    { 200, 176, 260,  176, 18, 14 },
	// Regreso
	{ 260, 176, 200,  176, 18, 14 },
	{ 200, 176, 128,  176, 18, 14 },
	{ 128, 176, 48,  176, 18, 14 }
};

const SaltoRuta ruta_fuego_8[] = {
	// Ida
    { 96, 136, 150, 136, 18, 14 },
    { 150, 136, 200, 136, 18, 14 },
	// Regreso
    { 200, 136, 150,  136, 18, 14 },
    { 150, 136, 96,  136, 18, 14 }
};

const SaltoRuta ruta_fuego_9[] = {
	// Ida
    { 176, 96, 250, 96, 18, 14 },
    { 250, 96, 300, 96, 18, 14 },
	// Regreso
    { 300, 96, 250,  96, 18, 14 },
    { 250,  96, 176,  96, 18, 14 }
};

// --------------- Victoria --------------
uint8_t victoria_activa = 0;
uint8_t victoria_pintada = 0;
uint32_t victoria_inicio = 0;

#define VICTORIA_DURACION_MS 3000

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
static inline int MaxBarrilesActivosNivel1(void);
void actualizarY_Personaje(Personaje *p);
void crearBarril(void);
void actualizarY_Objeto(int *obj_x, int *obj_y);
void actualizarBarriles(void);
void RestaurarFondoBarrilSeguro(int x, int y);

void ProcesarMovimientoNivel1(Personaje *p);

int alturaPlataformaEnX(int idx, int x);
int detectarPlataforma(int x, int y);
int direccionPlataforma(int idx);
int plataformaInferiorDeEscalera(int esc, int plataforma_actual);
int elegirEscaleraObjetivo(Barril *b);
int alturaPisoPlataformaEnX(int idx, int x);

int hayColision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void iniciarMuerteMario(void);
void reiniciarJuego(void);

void FixColorEndianness(uint16_t *buffer, uint32_t size);
void Dibujar_Imagen_Bin(char* nombre, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static inline void DibujarTile8(int x, int y, const uint16_t *tile);
int RectIntersects(int x1, int y1, int w1, int h1,
                   int x2, int y2, int w2, int h2);
void DibujarDecoracionNivel1(void);
void DibujarNivel1Tileado(void);
void RestaurarRectNivel1(int rx, int ry, int rw, int rh);
void DibujarDecoracionNivel2(void);
void DibujarNivel2Tileado(void);
void RestaurarRectNivel2(int rx, int ry, int rw, int rh);

void InicializarFuegosNivel2(void);
void ActualizarFuegosNivel2(void);
void DibujarFuegosNivel2(void);

int actualizarY_Personaje_N2(Personaje *p);
void ProcesarMovimientoNivel2(Personaje *p);
void ColisionPersonajeFuegosNivel2(Personaje *p);

void DibujarDecoracionNivel3(void);
void DibujarNivel3Tileado(void);
void RestaurarRectNivel3(int rx, int ry, int rw, int rh);

void InicializarFuegosNivel3(void);
void ActualizarFuegosNivel3(void);
void DibujarFuegosNivel3(void);

int actualizarY_Personaje_N3(Personaje *p);
void ProcesarMovimientoNivel3(Personaje *p);
void ColisionPersonajeFuegosNivel3(Personaje *p);

void RestaurarRectNivel4(int rx, int ry, int rw, int rh);
void DibujarNivel4Tileado(void);

void InicializarFuegosNivel4(void);
void ActualizarFuegosNivel4(void);
void DibujarFuegosNivel4(void);

int actualizarY_Personaje_N4(Personaje *p);
void ProcesarMovimientoNivel4(Personaje *p);
void ColisionPersonajeFuegosNivel4(Personaje *p);

void DibujarSelectorMenu(void);
void BorrarSelectorMenu(void);
void ConfirmarSeleccionMenu(void);

void victoria(void);
int marioEnPlataformaVictoria_P(Personaje *p, int idx);
void transmit_uart(char *string);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TILE_W 8
#define TILE_H 8

static const uint16_t *const TILESET1[] = {
	NULL,       // 0 = fondo
	escalera1,  // 1
	viga,		// 2
	viga11,     // 3
	viga12,     // 4
	viga13,     // 5
	viga14,     // 6
	viga15,     // 7
	viga16,     // 8
	viga17,     // 9
	viga21,     // 10
	viga22,     // 11
	viga23,     // 12
	viga24,     // 13
	viga25,     // 14
	viga26,     // 15
	viga27      // 16
};

static const uint16_t *const TILESET2[] = {
	NULL,       // 0 = fondo
	escalera_2,  // 1
	viga_2,		// 2
	plataforma_2,     // 3
	malla_2,     // 4
};

static const uint16_t *const TILESET3[] = {
    NULL,       // 0 = fondo
    escalera1,  // 1
    viga        // 2
};

static const uint16_t *const TILESET4[] = {
	NULL,       // 0 = fondo
	escalera_4,  // 1
	viga_4,		// 2
};

static inline int MaxBarrilesActivosNivel1(void) {
    return (jugadores == 2) ? 2 : 3;
}

void actualizarY_Personaje(Personaje *p) {
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int checkX = p->x + 8; // Centro del personaje
    int y_objetivo_local = 0;

    for (int i = 0; i < numPlataformas_actual; i++) {
        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int y_teorico = alturaPlataformaEnX(i, checkX);
            int distancia = p->y - y_teorico;

            // Detección de suelo (margen de 5 píxeles)
            if (distancia >= -5 && distancia < menor_distancia) {
                menor_distancia = distancia;
                viga_encontrada = i;
                y_objetivo_local = y_teorico;
            }
        }
    }

    if (viga_encontrada != -1) {
        p->y = y_objetivo_local;
    }
}

void crearBarril(void) {
    int activos = 0;

    for (int i = 0; i < MAX_BARRILES; i++) {
        if (barriles[i].activo) activos++;
    }

    if (activos >= MaxBarrilesActivosNivel1()) {
        return;
    }

    for (int i = 0; i < MAX_BARRILES; i++) {
        if (barriles[i].activo == 0) {

            // Nacer sobre la segunda plataforma (índice 1)
            barriles[i].x = 75;
            barriles[i].y = alturaPlataformaEnX(1, barriles[i].x + 8);

            barriles[i].x_ant = barriles[i].x;
            barriles[i].y_ant = barriles[i].y;
            barriles[i].estado = BARRIL_RODANDO;
            barriles[i].activo = 1;
            barriles[i].anim = 0;

            barriles[i].plataforma_actual = 1;
            barriles[i].plataforma_destino = -1;
            barriles[i].escalera_objetivo = -1;
            barriles[i].dir = direccionPlataforma(1);

            break;
        }
    }
}

void actualizarY_Objeto(int *obj_x, int *obj_y) {
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int y_final = *obj_y;
    int checkX = (*obj_x) + 8;

    for (int i = 0; i < numPlataformas_actual; i++) {
        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int y_teorico = alturaPlataformaEnX(i, checkX);
            int distancia = (*obj_y) - y_teorico;

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

        barriles[i].x_ant = barriles[i].x;
        barriles[i].y_ant = barriles[i].y;

        if (barriles[i].estado == BARRIL_RODANDO) {

            barriles[i].plataforma_actual = detectarPlataforma(barriles[i].x, barriles[i].y);

            if (barriles[i].plataforma_actual != -1) {
                barriles[i].dir = direccionPlataforma(barriles[i].plataforma_actual);
            }

            if (barriles[i].escalera_objetivo == -1 && barriles[i].plataforma_actual != -1) {
                barriles[i].escalera_objetivo = elegirEscaleraObjetivo(&barriles[i]);
                if (barriles[i].escalera_objetivo != -1) {
                    barriles[i].plataforma_destino =
                        plataformaInferiorDeEscalera(barriles[i].escalera_objetivo,
                                                     barriles[i].plataforma_actual);
                }
            }

            int x_prev = barriles[i].x;
            barriles[i].x += barriles[i].dir * 2;
            actualizarY_Objeto(&barriles[i].x, &barriles[i].y);

            if (barriles[i].escalera_objetivo != -1) {
                int xc = escaleras_actuales[barriles[i].escalera_objetivo].x_centro;

                if ((barriles[i].dir == 1 && x_prev <= xc && barriles[i].x >= xc) ||
                    (barriles[i].dir == -1 && x_prev >= xc && barriles[i].x <= xc)) {

                    barriles[i].x = xc;
                    barriles[i].estado = BARRIL_BAJANDO_ESC;
                }
            }
        }
        else if (barriles[i].estado == BARRIL_BAJANDO_ESC) {

            barriles[i].x = escaleras_actuales[barriles[i].escalera_objetivo].x_centro;
            barriles[i].y += 2;

            if (barriles[i].plataforma_destino != -1) {
                int y_dest = alturaPlataformaEnX(barriles[i].plataforma_destino, barriles[i].x);

                if (barriles[i].y >= y_dest) {
                    barriles[i].y = y_dest;
                    barriles[i].plataforma_actual = barriles[i].plataforma_destino;
                    barriles[i].dir = direccionPlataforma(barriles[i].plataforma_actual);

                    barriles[i].estado = BARRIL_RODANDO;
                    barriles[i].plataforma_destino = -1;
                    barriles[i].escalera_objetivo = -1;
                }
            }
        }

        if (barriles[i].estado == BARRIL_BAJANDO_ESC) {
            barriles[i].anim = (barriles[i].y / 4) % 2;   // 2 frames para barril_frontal
        } else {
            barriles[i].anim = (barriles[i].x / 8) % 4;   // 4 frames para barril_lateral
        }

        // Si toca el oil, desaparece y DK agenda otro lanzamiento
        if (hayColision(barriles[i].x + 4, barriles[i].y + 4, 8, 8,
                        34, 198, 20, 24)) {
            RestaurarFondoBarrilSeguro(barriles[i].x_ant, barriles[i].y_ant);
            barriles[i].activo = 0;

            if (dk_spawns_pendientes < MaxBarrilesActivosNivel1()) {
                dk_spawns_pendientes++;
            }
            continue;
        }

        if (barriles[i].x > 320 || barriles[i].x < -16 || barriles[i].y >= 240) {
            RestaurarFondoBarrilSeguro(barriles[i].x_ant, barriles[i].y_ant);
            barriles[i].activo = 0;
            continue;
        }
    }
}

void RestaurarFondoBarrilSeguro(int x, int y) {
    int h_visible = SPRITE_BARRIL;

    if (y >= SCREEN_BOTTOM) return;

    if (y + h_visible > SCREEN_BOTTOM) {
        h_visible = SCREEN_BOTTOM - y;
    }

    if (y < SCREEN_TOP) {
        int recorte = SCREEN_TOP - y;
        y = SCREEN_TOP;
        h_visible -= recorte;
    }

    if (h_visible <= 0) return;

    RestaurarRectNivel1(x, y, SPRITE_BARRIL, h_visible);
}

void ProcesarMovimientoNivel1(Personaje *p) {
    if (p->muriendo) return;

    // --- Lógica de Salto ---
    if (p->jumpState == 1) { // Subiendo
        p->y -= 2;
        p->jumpProgress += 2;
        if (p->jumpProgress >= jumpMax) p->jumpState = 2;
    }
    else if (p->jumpState == 2) { // Bajando
        p->y += 2;
        if (p->y >= p->y_past) {
            p->y = p->y_past;
            p->jumpState = 0;
            p->jumpProgress = 0;
            p->inercia_x = 0;
            p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
            p->ancho_hoja = 48;
        }
    }

    if (p->jumpState != 0) p->x += p->inercia_x;

    // --- Escaleras y Rampas ---
    if (p->jumpState == 0) {
        int enEscalera = 0;
        for (int i = 0; i < numEscaleras_actual; i++) { // <--- AÑADIR 'int' AQUÍ
            if (p->x > (nivel1_escaleras[i].x_centro - 8) && p->x < (nivel1_escaleras[i].x_centro + 8)) {
                if (p->y <= nivel1_escaleras[i].y_piso && p->y >= (nivel1_escaleras[i].y_techo - 2)) {
                    enEscalera = 1;
                    if (p->sube != 0) {
                        p->x = nivel1_escaleras[i].x_centro;
                        if (p->sube == 1) p->y -= 2;
                        if (p->sube == 2) p->y += 2;
                        p->actual_bmp = (p == &p1) ? mario_subiendo : luigi_sube;
                        p->ancho_hoja = 32;
                    }
                    if (p->y <= nivel1_escaleras[i].y_techo) { p->y = nivel1_escaleras[i].y_techo; p->y_past = p->y; }
                    if (p->y >= nivel1_escaleras[i].y_piso) { p->y = nivel1_escaleras[i].y_piso; }
                    break;
                }
            }
        }
        if (!enEscalera) {
            actualizarY_Personaje(p);
            p->y_past = p->y; // <--- CRITICO: Actualizar el suelo para el siguiente salto
        }
    }
    if (p->x < 0) p->x = 0;
    if (p->x > 320 - 16) p->x = 320 - 16;
}

int alturaPlataformaEnX(int idx, int x) {
    if (plataformas_actuales[idx].y_inicio == plataformas_actuales[idx].y_fin) {
        return plataformas_actuales[idx].y_inicio - 16;
    }

    int ancho = plataformas_actuales[idx].x2 - plataformas_actuales[idx].x1;
    int avance = x - plataformas_actuales[idx].x1;
    int dy = plataformas_actuales[idx].y_fin - plataformas_actuales[idx].y_inicio;

    return plataformas_actuales[idx].y_inicio + ((avance * dy) / ancho) - 16;
}

int detectarPlataforma(int x, int y) {
    int mejor = -1;
    int menor_dist = 1000;

    for (int i = 0; i < numPlataformas_actual; i++) {
        int xc = x + 8;

        if (xc >= plataformas_actuales[i].x1 && xc <= plataformas_actuales[i].x2) {
            int y_plat = alturaPlataformaEnX(i, xc);
            int dist = y - y_plat;

            if (dist >= -10 && dist < menor_dist) {
                menor_dist = dist;
                mejor = i;
            }
        }
    }
    return mejor;
}

int direccionPlataforma(int idx) {
    if (plataformas_actuales[idx].y_fin > plataformas_actuales[idx].y_inicio)
        return 1;   // baja hacia la derecha
    else
        return -1;  // baja hacia la izquierda
}

int plataformaInferiorDeEscalera(int esc, int plataforma_actual) {
    switch (esc) {
        case 7: return (plataforma_actual == 0) ? 1 : -1;
        case 6: return (plataforma_actual == 1) ? 2 : -1;
        case 4: return (plataforma_actual == 2) ? 3 : -1;
        case 5: return (plataforma_actual == 2) ? 3 : -1;
        case 3: return (plataforma_actual == 3) ? 4 : -1;
        case 1: return (plataforma_actual == 4) ? 5 : -1;
        case 2: return (plataforma_actual == 4) ? 5 : -1;
        case 0: return (plataforma_actual == 5) ? 6 : -1;
        default: return -1;
    }
}

int elegirEscaleraObjetivo(Barril *b) {
    int candidatos[8];
    int distancias[8];
    int n = 0;

    for (int e = 0; e < numEscaleras_actual; e++) {
        int p_dest = plataformaInferiorDeEscalera(e, b->plataforma_actual);
        if (p_dest == -1) continue;

        int dx = escaleras_actuales[e].x_centro - b->x;

        if ((b->dir == 1 && dx >= 0) || (b->dir == -1 && dx <= 0)) {
            candidatos[n] = e;
            distancias[n] = abs(dx);
            n++;
        }
    }

    if (n == 0) return -1;

    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (distancias[j] < distancias[i]) {
                int tmp = distancias[i]; distancias[i] = distancias[j]; distancias[j] = tmp;
                tmp = candidatos[i]; candidatos[i] = candidatos[j]; candidatos[j] = tmp;
            }
        }
    }

    int r = rand() % 100;
    if (n == 1 || r < 65) return candidatos[0];
    if (n == 2 || r < 90) return candidatos[1];
    return candidatos[rand() % n];
}

int alturaPisoPlataformaEnX(int idx, int x) {
    if (plataformas_actuales[idx].y_inicio == plataformas_actuales[idx].y_fin) {
        return plataformas_actuales[idx].y_inicio;
    }

    int ancho = plataformas_actuales[idx].x2 - plataformas_actuales[idx].x1;
    int avance = x - plataformas_actuales[idx].x1;
    int dy = plataformas_actuales[idx].y_fin - plataformas_actuales[idx].y_inicio;

    return plataformas_actuales[idx].y_inicio + ((avance * dy) / ancho);
}

int hayColision(int x1, int y1, int w1, int h1,
                int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2) &&
           (x1 + w1 > x2) &&
           (y1 < y2 + h2) &&
           (y1 + h1 > y2);
}

void iniciarMuerteMario(void) {
    jumpState = 0;
    jumpProgress = 0;
    inercia_x = 0;
    mario_sube = 0;
    comando = '0';

    mario_actual = mario_muere;
    mario_ancho_hoja = 80;   // 5 frames x 16 px
}
void reiniciarJuego(void) {
    // Limpiar barriles activos
    for (int i = 0; i < MAX_BARRILES; i++) {
        barriles[i].activo = 0;
        barriles[i].x = 0;
        barriles[i].y = 0;
        barriles[i].x_ant = 0;
        barriles[i].y_ant = 0;
        barriles[i].anim = 0;
        barriles[i].estado = BARRIL_RODANDO;
        barriles[i].plataforma_actual = -1;
        barriles[i].plataforma_destino = -1;
        barriles[i].escalera_objetivo = -1;
        barriles[i].dir = 1;
    }
    // Reiniciar personajes multiplayer
    p1.x = 15;
    p1.y = 219;
    p1.x_ant = p1.x;
    p1.y_ant = p1.y;
    p1.flip = 0;
    p1.y_past = p1.y;
    p1.jumpState = 0;
    p1.jumpProgress = 0;
    p1.inercia_x = 0;
    p1.sube = 0;
    p1.muriendo = 0;
    p1.frame = 0;
    p1.tick = 0;
    p1.actual_bmp = mario_camina;
    p1.ancho_hoja = 48;
    p1.color_transparente = 0x0000;
    p1.frame_ant = 255;

    p2.x = 40;
    p2.y = 219;
    p2.x_ant = p2.x;
    p2.y_ant = p2.y;
    p2.flip = 0;
    p2.y_past = p2.y;
    p2.jumpState = 0;
    p2.jumpProgress = 0;
    p2.inercia_x = 0;
    p2.sube = 0;
    p2.muriendo = 0;
    p2.frame = 0;
    p2.tick = 0;
    p2.actual_bmp = luigi_camina;
    p2.ancho_hoja = 48;
    p2.color_transparente = 0x0000;
    p2.frame_ant = 255;

    // Reiniciar DK / nivel 1
    spawn_timer = 0;
    dk_lanzando = 0;
    dk_anim = 0;
    dk_anim_tick = 0;
    dk_barril_pendiente = 0;
    dk_spawns_pendientes = 0;
    dk_anim_prev = -1;
    princess_anim_prev = -1;
    frame_counter = 0;

    // Reiniciar menú
    menu = 1;
    menu_print = 1;
    menu_up = 0;
    menu_down = 0;
    select = 0;
    jugadores = 1;
    nivel = 1;

    // Limpiar estados de entrada para que el menú vuelva a responder bien
    prev_state1 = 0;
    prev_state2 = 0;
    ctrl_state1 = 0;
    ctrl_state6 = 0;
    ctrl_cmd2 = 0;

    // Reset de victoria
    victoria_activa = 0;
    victoria_pintada = 0;
    victoria_inicio = 0;

    // Volver a portada
    estadoActual = ESTADO_PORTADA;
    cambioDePantalla = 1;
}

void FixColorEndianness(uint16_t *buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = (buffer[i] << 8) | (buffer[i] >> 8);
    }
}

void Dibujar_Imagen_Bin(char* nombre, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    FIL fil;
    UINT bytesRead;
    uint16_t fila_buffer[320];
    FRESULT res;

    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // LCD OFF
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET); // SD ON
    HAL_Delay(1);

    res = f_open(&fil, nombre, FA_READ);
    if (res == FR_OK) {
        transmit_uart("Archivo abierto correctamente\r\n");

        for (int i = 0; i < h; i++) {
            if (f_read(&fil, fila_buffer, w * 2, &bytesRead) == FR_OK && bytesRead == (w * 2)) {

                FixColorEndianness(fila_buffer, w);

                HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);     // SD OFF
                HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // LCD ON

                LCD_Bitmap(x, y + i, w, 1, fila_buffer);

                HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);   // LCD OFF
                HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);   // SD ON
            } else {
                transmit_uart("Error leyendo fila\r\n");
                break;
            }
        }

        f_close(&fil);
        HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET); // SD OFF
    } else {
        sprintf(buffer, "Error f_open = %d\r\n", res);
        transmit_uart(buffer);
    }
}

static inline void DibujarTile8(int x, int y, const uint16_t *tile)
{
    if (tile == NULL) return;
    if (x < 0 || y < 0 || x > 320 - TILE_W || y > 240 - TILE_H) return;
    LCD_Bitmap(x, y, TILE_W, TILE_H, (uint16_t *)tile);
}

int RectIntersects(int x1, int y1, int w1, int h1,
                   int x2, int y2, int w2, int h2)
{
    return (x1 < x2 + w2) &&
           (x1 + w1 > x2) &&
           (y1 < y2 + h2) &&
           (y1 + h1 > y2);
}

void DibujarDecoracionNivel1(void)
{
    // Reemplaza barriles_stack por el nombre real de tu sprite de barriles apilados
    LCD_Bitmap(8, 40, 28, 32, (uint16_t *)barriles_piled);

    LCD_Bitmap(34, 198, 20, 24, (uint16_t *)oil);
}

void DibujarNivel1Tileado(void)
{
    LCD_Clear(0x0000);

    for (int fila = 0; fila < MAP_H; fila++) {
        for (int col = 0; col < MAP_W; col++) {
            uint8_t id = nivel1_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET1[id]);
            }
        }
    }

    DibujarDecoracionNivel1();
}

void RestaurarRectNivel1(int rx, int ry, int rw, int rh)
{
    FillRect(rx, ry, rw, rh, 0x0000);

    int col_ini = rx / TILE_W;
    int col_fin = (rx + rw - 1) / TILE_W;
    int fila_ini = ry / TILE_H;
    int fila_fin = (ry + rh - 1) / TILE_H;

    if (col_ini < 0) col_ini = 0;
    if (fila_ini < 0) fila_ini = 0;
    if (col_fin >= MAP_W) col_fin = MAP_W - 1;
    if (fila_fin >= MAP_H) fila_fin = MAP_H - 1;

    for (int fila = fila_ini; fila <= fila_fin; fila++) {
        for (int col = col_ini; col <= col_fin; col++) {
            uint8_t id = nivel1_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET1[id]);
            }
        }
    }

    // decoración fija encima del tilemap
    if (RectIntersects(rx, ry, rw, rh, 8, 40, 28, 32)) {
        LCD_Bitmap(8, 40, 28, 32, (uint16_t *)barriles_piled);
    }

    if (RectIntersects(rx, ry, rw, rh, 34, 198, 20, 24)) {
        LCD_Bitmap(34, 198, 20, 24, (uint16_t *)oil);
    }
}

void DibujarDecoracionNivel2(void)
{
    LCD_Bitmap(152, 95, 20, 24, (uint16_t *)oil);
}

void DibujarNivel2Tileado(void)
{
    LCD_Clear(0x0000);

    for (int fila = 0; fila < MAP_H; fila++) {
        for (int col = 0; col < MAP_W; col++) {
            uint8_t id = nivel2_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET2[id]);
            }
        }
    }

    DibujarDecoracionNivel2();
}

void RestaurarRectNivel2(int rx, int ry, int rw, int rh)
{
    FillRect(rx, ry, rw, rh, 0x0000);

    int col_ini = rx / TILE_W;
    int col_fin = (rx + rw - 1) / TILE_W;
    int fila_ini = ry / TILE_H;
    int fila_fin = (ry + rh - 1) / TILE_H;

    if (col_ini < 0) col_ini = 0;
    if (fila_ini < 0) fila_ini = 0;
    if (col_fin >= MAP_W) col_fin = MAP_W - 1;
    if (fila_fin >= MAP_H) fila_fin = MAP_H - 1;

    for (int fila = fila_ini; fila <= fila_fin; fila++) {
        for (int col = col_ini; col <= col_fin; col++) {
            uint8_t id = nivel2_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET2[id]);
            }
        }
    }

    if (RectIntersects(rx, ry, rw, rh, 152, 95, 20, 24)) {
        LCD_Bitmap(152, 95, 20, 24, (uint16_t *)oil);
    }
}

void InicializarFuegosNivel2(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        fuegos[i].activo = 0;
        fuegos[i].anim = 0;
        fuegos[i].ruta_idx = 0;
        fuegos[i].t = 0;
    }

    fuegos[0].activo = 1;
    fuegos[0].ruta = ruta_fuego_1;
    fuegos[0].ruta_len = sizeof(ruta_fuego_1) / sizeof(ruta_fuego_1[0]);
    fuegos[0].x = ruta_fuego_1[0].x0;
    fuegos[0].y = ruta_fuego_1[0].y0;
    fuegos[0].x_ant = fuegos[0].x;
    fuegos[0].y_ant = fuegos[0].y;

    fuegos[1].activo = 1;
    fuegos[1].ruta = ruta_fuego_2;
    fuegos[1].ruta_len = sizeof(ruta_fuego_2) / sizeof(ruta_fuego_2[0]);
    fuegos[1].x = ruta_fuego_2[0].x0;
    fuegos[1].y = ruta_fuego_2[0].y0;
    fuegos[1].x_ant = fuegos[1].x;
    fuegos[1].y_ant = fuegos[1].y;

    fuegos[2].activo = 1;
    fuegos[2].ruta = ruta_fuego_3;
    fuegos[2].ruta_len = sizeof(ruta_fuego_3) / sizeof(ruta_fuego_3[0]);
    fuegos[2].x = ruta_fuego_3[0].x0;
    fuegos[2].y = ruta_fuego_3[0].y0;
    fuegos[2].x_ant = fuegos[2].x;
    fuegos[2].y_ant = fuegos[2].y;
}

void ActualizarFuegosNivel2(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        fuegos[i].x_ant = fuegos[i].x;
        fuegos[i].y_ant = fuegos[i].y;

        const SaltoRuta *s = &fuegos[i].ruta[fuegos[i].ruta_idx];

        float p = (float)fuegos[i].t / (float)s->duracion;
        if (p > 1.0f) p = 1.0f;

        // Interpolación horizontal
        fuegos[i].x = s->x0 + (int)((s->x1 - s->x0) * p);

        // Base vertical lineal
        int y_base = s->y0 + (int)((s->y1 - s->y0) * p);

        // Arco parabólico: sube y baja
        float arco = 4.0f * p * (1.0f - p);   // máximo en medio
        fuegos[i].y = y_base - (int)(s->altura * arco);

        fuegos[i].anim = (fuegos[i].anim + 1) % 4;
        fuegos[i].t++;

        if (fuegos[i].t > s->duracion) {
            fuegos[i].ruta_idx++;
            fuegos[i].t = 0;

            if (fuegos[i].ruta_idx >= fuegos[i].ruta_len) {
                fuegos[i].ruta_idx = 0;
            }
        }
    }
}

void DibujarFuegosNivel2(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        RestaurarRectNivel2(fuegos[i].x_ant, fuegos[i].y_ant, 16, 16);

        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
        LCD_DibujarSpriteTransparente(
            fuegos[i].x, fuegos[i].y,
            16, 16,
            fueguito,
            fuegos[i].anim,
            64,
            0x0000,
            0
        );
    }
}

int actualizarY_Personaje_N2(Personaje *p)
{
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int checkX = p->x + 8;
    int y_objetivo_local = p->y;

    for (int i = 0; i < numPlataformas_actual; i++) {
        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int y_teorico = alturaPlataformaEnX(i, checkX);
            int distancia = y_teorico - p->y;

            if (distancia >= 0 && distancia <= 10 && distancia < menor_distancia) {
                menor_distancia = distancia;
                viga_encontrada = i;
                y_objetivo_local = y_teorico;
            }
        }
    }

    if (viga_encontrada != -1) {
        p->y = y_objetivo_local;
        return 1;
    }

    return 0;
}

void ProcesarMovimientoNivel2(Personaje *p)
{
    if (p->muriendo) return;

    if (p->jumpState != 0) {
        p->x += p->inercia_x;
    }

    if (p->jumpState == 0) {
        int enEscaleraActiva = 0;

        for (int i = 0; i < numEscaleras_actual; i++) {
            if (p->x > (escaleras_actuales[i].x_centro - 8) &&
                p->x < (escaleras_actuales[i].x_centro + 8)) {

                if (p->y <= escaleras_actuales[i].y_piso &&
                    p->y >= (escaleras_actuales[i].y_techo - 2)) {

                    enEscaleraActiva = 1;

                    if (p->sube == 1 || p->sube == 2) {
                        p->x = escaleras_actuales[i].x_centro;

                        if (p->sube == 1) p->y -= 2;
                        if (p->sube == 2) p->y += 2;

                        int dist = p->y - escaleras_actuales[i].y_techo;
                        if (dist <= 4 && dist > 0) {
                            p->actual_bmp = (p == &p1) ? mario_escala : luigi_escala;
                            p->ancho_hoja = 64;
                        } else {
                            p->actual_bmp = (p == &p1) ? mario_subiendo : luigi_sube;
                            p->ancho_hoja = 32;
                        }

                        p->sube = 0;
                    }

                    if (p->y <= escaleras_actuales[i].y_techo) {
                        p->y = escaleras_actuales[i].y_techo;
                        p->y_past = p->y;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    if (p->y >= escaleras_actuales[i].y_piso) {
                        p->y = escaleras_actuales[i].y_piso;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    break;
                }
            }
        }

        if (!enEscaleraActiva) {
            if (!actualizarY_Personaje_N2(p)) {
                p->y += 2;   // luego esto lo puedes volver gravedad real por personaje
                if (actualizarY_Personaje_N2(p)) {
                    p->y_past = p->y;
                }
            } else {
                p->y_past = p->y;
            }
        }
    }

    if (p->jumpState == 1) {
        int step = 2;
        if (p->jumpProgress + step > jumpMax) {
            step = jumpMax - p->jumpProgress;
        }

        p->y -= step;
        p->jumpProgress += step;

        if (p->jumpProgress >= jumpMax) {
            p->jumpState = 2;
        }
    }
    else if (p->jumpState == 2) {
        int step = 2;
        if (p->jumpProgress - step < 0) {
            step = p->jumpProgress;
        }

        p->y += step;
        p->jumpProgress -= step;

        if (p->y >= p->y_past) {
            p->y = p->y_past;
            p->jumpState = 0;
            p->jumpProgress = 0;
            p->inercia_x = 0;
            p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
            p->ancho_hoja = 48;
        }
    }

    if (p->x < 0) p->x = 0;
    if (p->x > 320 - 16) p->x = 320 - 16;
    if (p->y < 8) p->y = 8;
    if (p->y > 240 - 16) p->y = 240 - 16;
}

void ColisionPersonajeFuegosNivel2(Personaje *p)
{
    if (p->muriendo) return;

    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        if (hayColision(p->x, p->y, 16, 16,
                        fuegos[i].x + 3, fuegos[i].y + 3, 10, 10)) {
            p->muriendo = 1;
            p->frame = 0;
            p->tick = 0;
            p->actual_bmp = (p == &p1) ? mario_muere : luigi_muere;
            p->ancho_hoja = 80;
            break;
        }
    }
}

void DibujarDecoracionNivel3(void)
{
    // si quieres oil, princesa, etc., lo pones aquí
}

void DibujarNivel3Tileado(void)
{
    LCD_Clear(0x0000);

    for (int fila = 0; fila < MAP_H; fila++) {
        for (int col = 0; col < MAP_W; col++) {
            uint8_t id = nivel3_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET3[id]);
            }
        }
    }

    DibujarDecoracionNivel3();
}

void RestaurarRectNivel3(int rx, int ry, int rw, int rh)
{
    FillRect(rx, ry, rw, rh, 0x0000);

    int col_ini = rx / TILE_W;
    int col_fin = (rx + rw - 1) / TILE_W;
    int fila_ini = ry / TILE_H;
    int fila_fin = (ry + rh - 1) / TILE_H;

    if (col_ini < 0) col_ini = 0;
    if (fila_ini < 0) fila_ini = 0;
    if (col_fin >= MAP_W) col_fin = MAP_W - 1;
    if (fila_fin >= MAP_H) fila_fin = MAP_H - 1;

    for (int fila = fila_ini; fila <= fila_fin; fila++) {
        for (int col = col_ini; col <= col_fin; col++) {
            uint8_t id = nivel3_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET3[id]);
            }
        }
    }

    DibujarDecoracionNivel3();
}

void DibujarFuegosNivel3(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        RestaurarRectNivel3(fuegos[i].x_ant, fuegos[i].y_ant, 16, 16);

        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
        LCD_DibujarSpriteTransparente(
            fuegos[i].x, fuegos[i].y,
            16, 16,
            fueguito,
            fuegos[i].anim,
            64,
            0x0000,
            0
        );
    }
}

void ActualizarFuegosNivel3(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        fuegos[i].x_ant = fuegos[i].x;
        fuegos[i].y_ant = fuegos[i].y;

        const SaltoRuta *s = &fuegos[i].ruta[fuegos[i].ruta_idx];

        float p = (float)fuegos[i].t / (float)s->duracion;
        if (p > 1.0f) p = 1.0f;

        fuegos[i].x = s->x0 + (int)((s->x1 - s->x0) * p);
        int y_base = s->y0 + (int)((s->y1 - s->y0) * p);

        float arco = 4.0f * p * (1.0f - p);
        fuegos[i].y = y_base - (int)(s->altura * arco);

        fuegos[i].anim = (fuegos[i].anim + 1) % 4;
        fuegos[i].t++;

        if (fuegos[i].t > s->duracion) {
            fuegos[i].ruta_idx++;
            fuegos[i].t = 0;

            if (fuegos[i].ruta_idx >= fuegos[i].ruta_len) {
                fuegos[i].ruta_idx = 0;
            }
        }
    }
}

void InicializarFuegosNivel3(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        fuegos[i].activo = 0;
        fuegos[i].anim = 0;
        fuegos[i].ruta_idx = 0;
        fuegos[i].t = 0;
    }

    fuegos[0].activo = 1;
    fuegos[0].ruta = ruta_fuego_4;
    fuegos[0].ruta_len = sizeof(ruta_fuego_4) / sizeof(ruta_fuego_4[0]);
    fuegos[0].x = ruta_fuego_4[0].x0;
    fuegos[0].y = ruta_fuego_4[0].y0;
    fuegos[0].x_ant = fuegos[0].x;
    fuegos[0].y_ant = fuegos[0].y;

    fuegos[1].activo = 1;
    fuegos[1].ruta = ruta_fuego_5;
    fuegos[1].ruta_len = sizeof(ruta_fuego_5) / sizeof(ruta_fuego_5[0]);
    fuegos[1].x = ruta_fuego_5[0].x0;
    fuegos[1].y = ruta_fuego_5[0].y0;
    fuegos[1].x_ant = fuegos[1].x;
    fuegos[1].y_ant = fuegos[1].y;

    fuegos[2].activo = 1;
    fuegos[2].ruta = ruta_fuego_6;
    fuegos[2].ruta_len = sizeof(ruta_fuego_6) / sizeof(ruta_fuego_6[0]);
    fuegos[2].x = ruta_fuego_6[0].x0;
    fuegos[2].y = ruta_fuego_6[0].y0;
    fuegos[2].x_ant = fuegos[2].x;
    fuegos[2].y_ant = fuegos[2].y;
}

int actualizarY_Personaje_N3(Personaje *p)
{
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int checkX = p->x + 8;
    int y_objetivo_local = p->y;

    for (int i = 0; i < numPlataformas_actual; i++) {
        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int y_teorico = alturaPlataformaEnX(i, checkX);
            int distancia = y_teorico - p->y;

            if (distancia >= -4 && distancia <= 10 && abs(distancia) < menor_distancia) {
                menor_distancia = abs(distancia);
                viga_encontrada = i;
                y_objetivo_local = y_teorico;
            }
        }
    }

    if (viga_encontrada != -1) {
        p->y = y_objetivo_local;
        return 1;
    }

    return 0;
}

void ColisionPersonajeFuegosNivel3(Personaje *p)
{
    if (p->muriendo) return;

    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        if (hayColision(p->x, p->y, 16, 16,
                        fuegos[i].x + 3, fuegos[i].y + 3, 10, 10)) {
            p->muriendo = 1;
            p->frame = 0;
            p->tick = 0;
            p->actual_bmp = (p == &p1) ? mario_muere : luigi_muere;
            p->ancho_hoja = 80;
            break;
        }
    }
}

void ProcesarMovimientoNivel3(Personaje *p)
{
    if (p->muriendo) return;

    // Movimiento horizontal durante salto
    if (p->jumpState != 0) {
        p->x += p->inercia_x;
    }

    // =========================
    // ESCALERAS / SUELO
    // =========================
    if (p->jumpState == 0) {
        int enEscaleraActiva = 0;

        for (int i = 0; i < numEscaleras_actual; i++) {
            if (p->x > (escaleras_actuales[i].x_centro - 8) &&
                p->x < (escaleras_actuales[i].x_centro + 8)) {

                if (p->y <= escaleras_actuales[i].y_piso &&
                    p->y >= (escaleras_actuales[i].y_techo - 2)) {

                    enEscaleraActiva = 1;

                    if (p->sube == 1 || p->sube == 2) {
                        p->x = escaleras_actuales[i].x_centro;

                        if (p->sube == 1) p->y -= 2;
                        if (p->sube == 2) p->y += 2;

                        int dist = p->y - escaleras_actuales[i].y_techo;
                        if (dist <= 4 && dist > 0) {
                            p->actual_bmp = (p == &p1) ? mario_escala : luigi_escala;
                            p->ancho_hoja = 64;
                        } else {
                            p->actual_bmp = (p == &p1) ? mario_subiendo : luigi_sube;
                            p->ancho_hoja = 32;
                        }

                        p->sube = 0;
                    }

                    if (p->y <= escaleras_actuales[i].y_techo) {
                        p->y = escaleras_actuales[i].y_techo;
                        p->y_past = p->y;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    if (p->y >= escaleras_actuales[i].y_piso) {
                        p->y = escaleras_actuales[i].y_piso;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    break;
                }
            }
        }

        // Si no está en escalera, intenta pegarse al piso/plataforma
        if (!enEscaleraActiva) {
            // Si no está sobre una plataforma válida, empieza a caer
            if (!actualizarY_Personaje_N3(p)) {
                p->y += 2;  // gravedad básica

                // después de caer, intenta aterrizar
                if (actualizarY_Personaje_N3(p)) {
                    p->y_past = p->y;
                }
            } else {
                p->y_past = p->y;
            }
        }
    }

    // =========================
    // SALTO: SUBIENDO
    // =========================
    if (p->jumpState == 1) {
        int step = 2;

        if (p->jumpProgress + step > jumpMax) {
            step = jumpMax - p->jumpProgress;
        }

        p->y -= step;
        p->jumpProgress += step;

        if (p->jumpProgress >= jumpMax) {
            p->jumpState = 2;
        }
    }
    // =========================
    // SALTO: CAYENDO
    // =========================
    else if (p->jumpState == 2) {
        int step = 2;
        p->y += step;
        p->jumpProgress -= step;

        // En nivel 3, primero intentamos aterrizar en cualquier plataforma válida
        if (actualizarY_Personaje_N3(p)) {
            p->jumpState = 0;
            p->jumpProgress = 0;
            p->inercia_x = 0;
            p->y_past = p->y;
            p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
            p->ancho_hoja = 48;
        }
        // Fallback: si no aterriza en plataforma, vuelve al suelo original del salto
        else if (p->y >= p->y_past) {
            p->y = p->y_past;
            p->jumpState = 0;
            p->jumpProgress = 0;
            p->inercia_x = 0;
            p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
            p->ancho_hoja = 48;
        }
    }

    // =========================
    // LÍMITES DE PANTALLA
    // =========================
    if (p->x < 0) p->x = 0;
    if (p->x > 320 - 16) p->x = 320 - 16;
    if (p->y < 8) p->y = 8;
    if (p->y > 240 - 16) p->y = 240 - 16;
}

void DibujarNivel4Tileado(void) {
    LCD_Clear(0x0000);
    for (int fila = 0; fila < MAP_H; fila++) {
        for (int col = 0; col < MAP_W; col++) {
            uint8_t id = nivel4_mapa[fila][col];
            if (id != TILE_FONDO) {
                DibujarTile8(col * TILE_W, fila * TILE_H, TILESET4[id]);
            }
        }
    }
}

void RestaurarRectNivel4(int rx, int ry, int rw, int rh) {
    FillRect(rx, ry, rw, rh, 0x0000);
    int col_ini = rx / TILE_W, col_fin = (rx + rw - 1) / TILE_W;
    int fila_ini = ry / TILE_H, fila_fin = (ry + rh - 1) / TILE_H;

    for (int fila = (fila_ini < 0 ? 0 : fila_ini); fila <= (fila_fin >= MAP_H ? MAP_H - 1 : fila_fin); fila++) {
        for (int col = (col_ini < 0 ? 0 : col_ini); col <= (col_fin >= MAP_W ? MAP_W - 1 : col_fin); col++) {
            uint8_t id = nivel4_mapa[fila][col];
            if (id != TILE_FONDO) DibujarTile8(col * TILE_W, fila * TILE_H, TILESET4[id]);
        }
    }
}

void InicializarFuegosNivel4(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        fuegos[i].activo = 0;
        fuegos[i].anim = 0;
        fuegos[i].ruta_idx = 0;
        fuegos[i].t = 0;
    }

    fuegos[0].activo = 1;
    fuegos[0].ruta = ruta_fuego_7;
    fuegos[0].ruta_len = sizeof(ruta_fuego_7) / sizeof(ruta_fuego_7[0]);
    fuegos[0].x = ruta_fuego_7[0].x0;
    fuegos[0].y = ruta_fuego_7[0].y0;
    fuegos[0].x_ant = fuegos[0].x;
    fuegos[0].y_ant = fuegos[0].y;

    fuegos[1].activo = 1;
    fuegos[1].ruta = ruta_fuego_8;
    fuegos[1].ruta_len = sizeof(ruta_fuego_8) / sizeof(ruta_fuego_8[0]);
    fuegos[1].x = ruta_fuego_8[0].x0;
    fuegos[1].y = ruta_fuego_8[0].y0;
    fuegos[1].x_ant = fuegos[1].x;
    fuegos[1].y_ant = fuegos[1].y;

    fuegos[2].activo = 1;
    fuegos[2].ruta = ruta_fuego_9;
    fuegos[2].ruta_len = sizeof(ruta_fuego_9) / sizeof(ruta_fuego_9[0]);
    fuegos[2].x = ruta_fuego_9[0].x0;
    fuegos[2].y = ruta_fuego_9[0].y0;
    fuegos[2].x_ant = fuegos[2].x;
    fuegos[2].y_ant = fuegos[2].y;
}

void ActualizarFuegosNivel4(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        fuegos[i].x_ant = fuegos[i].x;
        fuegos[i].y_ant = fuegos[i].y;

        const SaltoRuta *s = &fuegos[i].ruta[fuegos[i].ruta_idx];

        float p = (float)fuegos[i].t / (float)s->duracion;
        if (p > 1.0f) p = 1.0f;

        // Interpolación horizontal
        fuegos[i].x = s->x0 + (int)((s->x1 - s->x0) * p);

        // Base vertical lineal
        int y_base = s->y0 + (int)((s->y1 - s->y0) * p);

        // Arco parabólico: sube y baja
        float arco = 4.0f * p * (1.0f - p);   // máximo en medio
        fuegos[i].y = y_base - (int)(s->altura * arco);

        fuegos[i].anim = (fuegos[i].anim + 1) % 4;
        fuegos[i].t++;

        if (fuegos[i].t > s->duracion) {
            fuegos[i].ruta_idx++;
            fuegos[i].t = 0;

            if (fuegos[i].ruta_idx >= fuegos[i].ruta_len) {
                fuegos[i].ruta_idx = 0;
            }
        }
    }
}

void DibujarFuegosNivel4(void) {
    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        RestaurarRectNivel4(fuegos[i].x_ant, fuegos[i].y_ant, 16, 16);

        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
        LCD_DibujarSpriteTransparente(
            fuegos[i].x, fuegos[i].y,
            16, 16,
            fueguito,
            fuegos[i].anim,
            64,
            0x0000,
            0
        );
    }
}

int actualizarY_Personaje_N4(Personaje *p)
{
    int viga_encontrada = -1;
    int menor_distancia = 1000;
    int checkX = p->x + 8;
    int y_objetivo_local = p->y;

    for (int i = 0; i < numPlataformas_actual; i++) {
        if (checkX >= plataformas_actuales[i].x1 && checkX <= plataformas_actuales[i].x2) {
            int y_teorico = alturaPlataformaEnX(i, checkX);

            // Igual que tu lógica original de nivel 4:
            // solo acepta plataformas al mismo nivel o debajo
            int distancia = y_teorico - p->y;

            if (distancia >= 0 && distancia <= 10 && distancia < menor_distancia) {
                menor_distancia = distancia;
                viga_encontrada = i;
                y_objetivo_local = y_teorico;
            }
        }
    }

    if (viga_encontrada != -1) {
        p->y = y_objetivo_local;
        return 1;
    }

    return 0;
}

void ProcesarMovimientoNivel4(Personaje *p)
{
    if (p->muriendo) return;

    // Movimiento horizontal durante salto
    if (p->jumpState != 0) {
        p->x += p->inercia_x;
    }

    // =========================
    // ESCALERAS / SUELO
    // =========================
    if (p->jumpState == 0) {
        int enEscaleraActiva = 0;

        for (int i = 0; i < numEscaleras_actual; i++) {
            int centro_personaje = p->x + 8;

            if (centro_personaje >= (escaleras_actuales[i].x_centro - escaleras_actuales[i].ancho) &&
                centro_personaje <= (escaleras_actuales[i].x_centro + escaleras_actuales[i].ancho)) {

                if (p->y <= escaleras_actuales[i].y_piso &&
                    p->y >= (escaleras_actuales[i].y_techo - 2)) {

                    enEscaleraActiva = 1;

                    if (p->sube == 1 || p->sube == 2) {
                        // Igual que tu lógica original de nivel 4
                        p->x = escaleras_actuales[i].x_centro - 4;

                        if (p->sube == 1) p->y -= 2;
                        if (p->sube == 2) p->y += 2;

                        int dist = p->y - escaleras_actuales[i].y_techo;
                        if (dist <= 4 && dist > 0) {
                            p->actual_bmp = (p == &p1) ? mario_escala : luigi_escala;
                            p->ancho_hoja = 64;
                        } else {
                            p->actual_bmp = (p == &p1) ? mario_subiendo : luigi_sube;
                            p->ancho_hoja = 32;
                        }

                        p->sube = 0;
                    }

                    if (p->y <= escaleras_actuales[i].y_techo) {
                        p->y = escaleras_actuales[i].y_techo;
                        p->y_past = p->y;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    if (p->y >= escaleras_actuales[i].y_piso) {
                        p->y = escaleras_actuales[i].y_piso;
                        p->y_past = p->y;
                        p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
                        p->ancho_hoja = 48;
                    }

                    break;
                }
            }
        }

        if (!enEscaleraActiva) {
            // Si no está sobre una plataforma válida, cae
            if (!actualizarY_Personaje_N4(p)) {
                p->y += 2;

                // Después de caer, intenta aterrizar
                if (actualizarY_Personaje_N4(p)) {
                    p->y_past = p->y;
                }
            } else {
                p->y_past = p->y;
            }
        }
    }

    // =========================
    // SALTO: SUBIENDO
    // =========================
    if (p->jumpState == 1) {
        int step = 2;

        if (p->jumpProgress + step > jumpMax) {
            step = jumpMax - p->jumpProgress;
        }

        p->y -= step;
        p->jumpProgress += step;

        if (p->jumpProgress >= jumpMax) {
            p->jumpState = 2;
        }
    }
    // =========================
    // SALTO: CAYENDO
    // =========================
    else if (p->jumpState == 2) {
        int step = 2;

        if (p->jumpProgress - step < 0) {
            step = p->jumpProgress;
        }

        p->y += step;
        p->jumpProgress -= step;

        if (p->y >= p->y_past) {
            p->y = p->y_past;
            p->jumpState = 0;
            p->jumpProgress = 0;
            p->inercia_x = 0;
            p->actual_bmp = (p == &p1) ? mario_camina : luigi_camina;
            p->ancho_hoja = 48;
        }
    }

    // =========================
    // LÍMITES DE PANTALLA
    // =========================
    if (p->x < 0) p->x = 0;
    if (p->x > 320 - 16) p->x = 320 - 16;
    if (p->y < 8) p->y = 8;
    if (p->y > 240 - 16) p->y = 240 - 16;
}

void ColisionPersonajeFuegosNivel4(Personaje *p)
{
    if (p->muriendo) return;

    for (int i = 0; i < MAX_FUEGOS; i++) {
        if (!fuegos[i].activo) continue;

        if (hayColision(p->x, p->y, 16, 16,
                        fuegos[i].x + 3, fuegos[i].y + 3, 10, 10)) {
            p->muriendo = 1;
            p->frame = 0;
            p->tick = 0;
            p->actual_bmp = (p == &p1) ? mario_muere : luigi_muere;
            p->ancho_hoja = 80;
            break;
        }
    }
}

void DibujarSelectorMenu(void) {
    if (menu == 1) {
        if (jugadores == 1) {
            LCD_Bitmap(94, 140, 7, 7, (uint16_t *)selector);
        } else {
            LCD_Bitmap(94, 161, 7, 7, (uint16_t *)selector);
        }
    }
    else if (menu == 2) {
        switch (nivel) {
            case 1:
                LCD_Bitmap(126, 125, 7, 7, (uint16_t *)selector);
                break;
            case 2:
                LCD_Bitmap(126, 144, 7, 7, (uint16_t *)selector);
                break;
            case 3:
                LCD_Bitmap(126, 163, 7, 7, (uint16_t *)selector);
                break;
            case 4:
                LCD_Bitmap(126, 182, 7, 7, (uint16_t *)selector);
                break;
        }
    }
}

void BorrarSelectorMenu(void) {
    if (menu == 1) {
        if (jugadores == 1) {
            FillRect(94, 140, 7, 7, 0x0000);
        } else {
            FillRect(94, 161, 7, 7, 0x0000);
        }
    }
    else if (menu == 2) {
        switch (nivel) {
            case 1:
                FillRect(126, 125, 7, 7, 0x0000);
                break;
            case 2:
                FillRect(126, 144, 7, 7, 0x0000);
                break;
            case 3:
                FillRect(126, 163, 7, 7, 0x0000);
                break;
            case 4:
                FillRect(126, 182, 7, 7, 0x0000);
                break;
        }
    }
}

void ConfirmarSeleccionMenu(void) {
    if (menu == 1) {
        menu = 2; menu_print = 1; select = 0; nivel = 1;
        cambioDePantalla = 1;
    }
    else if (menu == 2) {
        select = 0;
        // Inicializamos personajes antes de entrar al nivel
        // P1 Siempre activo
        p1.x = 15;
        p1.y = 279;
        p1.actual_bmp = mario_camina;
        p1.ancho_hoja = 48;
        p1.x_ant = p1.x;
        p1.y_ant = p1.y;

        if (jugadores == 2) {
            // P2 inicia un poco a la derecha de P1
            p2.x = 40;
            p2.y = 279;
            p2.actual_bmp = luigi_camina;
            p2.ancho_hoja = 48;
            p2.x_ant = p2.x;
            p2.y_ant = p2.y;
        }

        switch (nivel) {
            case 1: estadoActual = ESTADO_NIVEL_1; break;
            case 2: estadoActual = ESTADO_NIVEL_2; break;
            case 3: estadoActual = ESTADO_NIVEL_3; break;
            case 4: estadoActual = ESTADO_NIVEL_4; break;
        }
        cambioDePantalla = 1;
    }
}

void victoria(void) {
    if (!victoria_pintada) {
    	HAL_UART_Transmit(&huart3, (uint8_t*)"V", 1, 1000);
        LCD_Clear(0x0000);
        Dibujar_Imagen_Bin("ganaste.bin", 0, 0, 320, 240);
        victoria_pintada = 1;
        victoria_inicio = HAL_GetTick();
    }

    if ((HAL_GetTick() - victoria_inicio) >= VICTORIA_DURACION_MS) {
        victoria_activa = 0;
        victoria_pintada = 0;
        victoria_inicio = 0;
        reiniciarJuego();
    }
}

int marioEnPlataformaVictoria_P(Personaje *p, int idx) {
    int checkX = p->x + 8;
    return (p->jumpState == 0 &&
            checkX >= plataformas_actuales[idx].x1 &&
            checkX <= plataformas_actuales[idx].x2 &&
            p->y == alturaPlataformaEnX(idx, checkX));
}

void transmit_uart(char *string)
{
	uint8_t len = strlen(string);
	HAL_UART_Transmit(&huart2,(uint8_t*)string, len,200);
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
  MX_FATFS_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  LCD_Clear(0x0000);

  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);   // SD OFF
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); // LCD OFF
  HAL_Delay(20);

  fres = f_mount(&fs, "", 1); // Ahora CMD0 debería funcionar

  if (fres == FR_OK) {
      transmit_uart("SD Montada!\r\n");
  } else {
      sprintf(buffer, "f_mount error = %d\r\n", fres);
      transmit_uart(buffer);
  }
	// Inicio de UARTs
	HAL_UART_Receive_IT(&huart1, &rx_data1, 1);
	HAL_UART_Receive_IT(&huart2, &rx_data2, 1);
	HAL_UART_Receive_IT(&huart6, &rx_data6, 1);
	//FillRect(90, 60, 20, 20, 0x001F);

	//LCD_Bitmap(0, 31, 320, 209, fondo);

	srand(HAL_GetTick());
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		if (!victoria_activa && !p1.muriendo && (jugadores == 1 || !p2.muriendo)) {
		    // =====================================================
		    // 1. LÓGICA JUGADOR 1 (Control 1 - UART1)
		    // =====================================================
		    if (ctrl_state1 != prev_state1) {
		        uint8_t changed = ctrl_state1 ^ prev_state1;

		        if (estadoActual == ESTADO_PORTADA) {
		            if ((changed & BTN_UP) && (ctrl_state1 & BTN_UP)) menu_up = 1;
		            if ((changed & BTN_DOWN) && (ctrl_state1 & BTN_DOWN)) menu_down = 1;
		            if ((changed & BTN_SQUARE) && (ctrl_state1 & BTN_SQUARE)) select = 1;
		        } else {
		            // Salto P1
		            if ((changed & BTN_X) && (ctrl_state1 & BTN_X)) {
		                if (p1.jumpState == 0) {
		                    p1.jumpState = 1;
		                    p1.jumpProgress = 0;
		                    p1.y_past = p1.y;
		                    p1.actual_bmp = mario_brinca;
		                    p1.ancho_hoja = 16;
		                }
		            }
		        }
		        prev_state1 = ctrl_state1;
		    }

		    // Lógica continua P1 (Caminar / Escalar)
		    if (estadoActual != ESTADO_PORTADA) {
		        if (p1.jumpState == 0) {
		            if (ctrl_state1 & BTN_RIGHT) {
		                p1.x += 3; p1.flip = 1; p1.inercia_x = 2;
		                p1.actual_bmp = mario_camina; p1.ancho_hoja = 48;
		            } else if (ctrl_state1 & BTN_LEFT) {
		                p1.x -= 3; p1.flip = 0; p1.inercia_x = -2;
		                p1.actual_bmp = mario_camina; p1.ancho_hoja = 48;
		            } else { p1.inercia_x = 0; }

		            if (ctrl_state1 & BTN_UP) p1.sube = 1;
		            else if (ctrl_state1 & BTN_DOWN) p1.sube = 2;
		            else p1.sube = 0;
		        }
		    }

		    // =====================================================
		    // 2. LÓGICA JUGADOR 2 (Control 2 - UART6)
		    // =====================================================
		    if (jugadores == 2) {
		        if (ctrl_state6 != prev_state2) {
		            uint8_t changed = ctrl_state6 ^ prev_state2;

		            // Salto P2
		            if ((changed & BTN_X) && (ctrl_state6 & BTN_X)) {
		                if (p2.jumpState == 0) {
		                    p2.jumpState = 1;
		                    p2.jumpProgress = 0;
		                    p2.y_past = p2.y;
		                    p2.actual_bmp = luigi_brinca; // Usar sprite de Luigi
		                    p2.ancho_hoja = 16;
		                }
		            }
		            prev_state2 = ctrl_state6;
		        }

		        // Lógica continua P2
		        if (estadoActual != ESTADO_PORTADA) {
		            if (p2.jumpState == 0) {
		                if (ctrl_state6 & BTN_RIGHT) {
		                    p2.x += 3; p2.flip = 1; p2.inercia_x = 2;
		                    p2.actual_bmp = luigi_camina; p2.ancho_hoja = 48;
		                } else if (ctrl_state6 & BTN_LEFT) {
		                    p2.x -= 3; p2.flip = 0; p2.inercia_x = -2;
		                    p2.actual_bmp = luigi_camina; p2.ancho_hoja = 48;
		                } else { p2.inercia_x = 0; }

		                if (ctrl_state6 & BTN_UP) p2.sube = 1;
		                else if (ctrl_state6 & BTN_DOWN) p2.sube = 2;
		                else p2.sube = 0;
		            }
		        }
		    }

		    // =====================================================
		    // 3. LÓGICA TERMINAL (Debug / Comando UART2)
		    // =====================================================
		    if (ctrl_cmd2 != 0) {
		        comando = ctrl_cmd2; ctrl_cmd2 = 0;
		        // Aplicar comandos de terminal solo a P1 para debug
		        if (estadoActual == ESTADO_PORTADA) {
		            if (comando == 'u') menu_up = 1;
		            if (comando == 'd') menu_down = 1;
		            if (comando == 's') select = 1;
		        } else {
		            if (comando == 'j' && p1.jumpState == 0) {
		                p1.jumpState = 1; p1.y_past = p1.y;
		                p1.actual_bmp = mario_brinca; p1.ancho_hoja = 16;
		            }
		            if (comando == 'r') { p1.x += 5; p1.flip = 1; p1.inercia_x = 2; }
		            if (comando == 'l') { p1.x -= 5; p1.flip = 0; p1.inercia_x = -2; }
		        }
		    }
		}
		if (victoria_activa) {
		    victoria();
		    HAL_Delay(10);
		    continue;
		}
		            // --- DIBUJAR SEGÚN EL ESTADO ---
		            switch (estadoActual) {
// === Menu ============================================================================================
		            case ESTADO_PORTADA:
		                switch (menu) {
		                    case 1: // Menú selector de jugadores
		                        if (menu_print == 1) {
		                            LCD_Clear(0x0000);
		                            Dibujar_Imagen_Bin("pantalla_de_inicio_f.bin", 0, 0, 320, 240);
		                            DibujarSelectorMenu();
		                            menu_print = 0;
		                            cambioDePantalla = 0;
		                            HAL_UART_Transmit(&huart3, (uint8_t*)"M", 1, 1000);
		                            //HAL_UART_Transmit(&huart2, (uint8_t*)"M", 1, 1000);
		                        }

		                        if (menu_up == 1) {
		                            BorrarSelectorMenu();

		                            if (jugadores == 1) jugadores = 2;
		                            else jugadores = 1;

		                            DibujarSelectorMenu();
		                            menu_up = 0;
		                        }
		                        else if (menu_down == 1) {
		                            BorrarSelectorMenu();

		                            if (jugadores == 1) jugadores = 2;
		                            else jugadores = 1;

		                            DibujarSelectorMenu();
		                            menu_down = 0;
		                        }

		                        if (select == 1) {
		                            ConfirmarSeleccionMenu();
		                        }
		                        break;

		                    case 2: // Menú selector de nivel
		                        if (menu_print == 1) {
		                            LCD_Clear(0x0000);
		                            Dibujar_Imagen_Bin("pantalla_de_inicio_l.bin", 0, 0, 320, 240);
		                            DibujarSelectorMenu();
		                            menu_print = 0;
		                            cambioDePantalla = 0;
		                        }

		                        if (menu_up == 1) {
		                            BorrarSelectorMenu();

		                            if (nivel == 1) nivel = 4;
		                            else nivel--;

		                            DibujarSelectorMenu();
		                            menu_up = 0;
		                        }
		                        else if (menu_down == 1) {
		                            BorrarSelectorMenu();

		                            if (nivel == 4) nivel = 1;
		                            else nivel++;

		                            DibujarSelectorMenu();
		                            menu_down = 0;
		                        }

		                        if (select == 1) {
		                            ConfirmarSeleccionMenu();
		                        }
		                        break;
		                }
		                break;

// === NIVEL 1 ===================================================================================
		                case ESTADO_NIVEL_1:
		                    if (cambioDePantalla) {
		                    	HAL_UART_Transmit(&huart3, (uint8_t*)"1", 1, 1000);

		                        LCD_Clear(0x0000);

		                        plataformas_actuales = nivel1;
		                        numPlataformas_actual = 7;
		                        escaleras_actuales = nivel1_escaleras;
		                        numEscaleras_actual = 8;

		                        DibujarNivel1Tileado();

		                        cambioDePantalla = 0;
		                        frame_counter = 0;
		                        spawn_timer = 0;
		                        dk_lanzando = 0;
		                        dk_anim = 0;
		                        dk_anim_tick = 0;
		                        dk_barril_pendiente = 0;
		                        dk_spawns_pendientes = 0;
		                        dk_anim_prev = -1;
		                        princess_anim_prev = -1;

		                        for (int i = 0; i < MAX_BARRILES; i++) {
		                            barriles[i].activo = 0;
		                            barriles[i].x = 0;
		                            barriles[i].y = 0;
		                            barriles[i].x_ant = 0;
		                            barriles[i].y_ant = 0;
		                            barriles[i].anim = 0;
		                            barriles[i].estado = BARRIL_RODANDO;
		                            barriles[i].plataforma_actual = -1;
		                            barriles[i].plataforma_destino = -1;
		                            barriles[i].escalera_objetivo = -1;
		                            barriles[i].dir = 1;
		                        }

		                        // Inicializar P1
		                        p1.x = 15;
		                        p1.y = 219;
		                        p1.x_ant = p1.x;
		                        p1.y_ant = p1.y;
		                        p1.flip = 0;
		                        p1.y_past = p1.y;
		                        p1.jumpState = 0;
		                        p1.jumpProgress = 0;
		                        p1.inercia_x = 0;
		                        p1.sube = 0;
		                        p1.muriendo = 0;
		                        p1.frame = 0;
		                        p1.tick = 0;
		                        p1.actual_bmp = mario_camina;
		                        p1.ancho_hoja = 48;
		                        p1.color_transparente = 0x0000;
		                        p1.frame_ant = 255;   // fuerza primer dibujo

		                        // Inicializar P2
		                        p2.muriendo = 0;
		                        p2.frame = 0;
		                        p2.tick = 0;
		                        p2.frame_ant = 255;
		                        p2.jumpState = 0;
		                        p2.jumpProgress = 0;
		                        p2.inercia_x = 0;
		                        p2.sube = 0;
		                        p2.flip = 0;
		                        p2.color_transparente = 0x0000;

		                        if (jugadores == 2) {
		                            p2.x = 40;
		                            p2.y = 219;
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.y_past = p2.y;
		                            p2.actual_bmp = luigi_camina;
		                            p2.ancho_hoja = 48;
		                        }
		                    }

		                    // --- 1. LÓGICA DE DONKEY KONG (Spawn de barriles) ---
		                    spawn_timer++;
		                    if (!dk_lanzando) {
		                        int activos = 0;
		                        for (int i = 0; i < MAX_BARRILES; i++) {
		                            if (barriles[i].activo) activos++;
		                        }

		                        if ((dk_spawns_pendientes > 0 || spawn_timer > 150) &&
		                            activos < MaxBarrilesActivosNivel1()) {

		                            dk_lanzando = 1;
		                            dk_anim = 0;
		                            dk_anim_tick = 0;
		                            dk_barril_pendiente = 1;

		                            if (dk_spawns_pendientes > 0) dk_spawns_pendientes--;
		                            spawn_timer = 0;
		                        }
		                    }

		                    if (dk_lanzando) {
		                        dk_anim_tick++;
		                        if (dk_anim_tick >= 6) {
		                            dk_anim_tick = 0;
		                            dk_anim++;

		                            if (dk_anim == 2 && dk_barril_pendiente) {
		                                crearBarril();
		                                dk_barril_pendiente = 0;
		                            }

		                            if (dk_anim > 2) {
		                                dk_anim = 0;
		                                dk_lanzando = 0;
		                            }
		                        }
		                    }

		                    // --- 2. ACTUALIZAR OBJETOS Y JUGADORES ---
		                    actualizarBarriles();
		                    ProcesarMovimientoNivel1(&p1);
		                    if (jugadores == 2) ProcesarMovimientoNivel1(&p2);

		                    // --- 3. DETECTAR COLISIONES (Barriles contra P1 y P2) ---
		                    for (int i = 0; i < MAX_BARRILES; i++) {
		                        if (!barriles[i].activo) continue;

		                        if (!p1.muriendo &&
		                            hayColision(p1.x, p1.y, 16, 16, barriles[i].x + 3, barriles[i].y + 2, 8, 8)) {
		                            p1.muriendo = 1;
		                            p1.frame = 0;
		                            p1.tick = 0;
		                        }

		                        if (jugadores == 2 && !p2.muriendo &&
		                            hayColision(p2.x, p2.y, 16, 16, barriles[i].x + 3, barriles[i].y + 2, 8, 8)) {
		                            p2.muriendo = 1;
		                            p2.frame = 0;
		                            p2.tick = 0;
		                        }
		                    }

		                    // --- 4. GESTIÓN DE MUERTE (Animación) ---
		                    if (p1.muriendo) {
		                        p1.tick++;
		                        if (p1.tick >= 6) {
		                            p1.tick = 0;
		                            p1.frame++;
		                            if (p1.frame >= 5) {
		                            	HAL_UART_Transmit(&huart3, (uint8_t*)"D", 1, 1000);
		                                reiniciarJuego();
		                                break;
		                            }
		                        }
		                        p1.actual_bmp = mario_muere;
		                        p1.ancho_hoja = 80;
		                    }

		                    if (jugadores == 2 && p2.muriendo) {
		                        p2.tick++;
		                        if (p2.tick >= 6) {
		                            p2.tick = 0;
		                            p2.frame++;
		                            if (p2.frame >= 5) {
		                                p2.muriendo = 0;   // o aquí luego manejas vidas
		                                p2.frame = 0;
		                                p2.tick = 0;
		                                p2.actual_bmp = luigi_camina;
		                                p2.ancho_hoja = 48;
		                            }
		                        }
		                        if (p2.muriendo) {
		                            p2.actual_bmp = luigi_muere;
		                            p2.ancho_hoja = 80;
		                        }
		                    }

		                    // --- 5. PREPARAR FRAMES ACTUALES ---
		                    frame_counter++;

		                    int princess_anim = (frame_counter / 15) % 2;

		                    int f1 = p1.muriendo ? p1.frame :
		                             (p1.sube != 0 ? ((p1.y / 8) % 2) : ((p1.x / 10) % 3));

		                    int f2 = 0;
		                    if (jugadores == 2) {
		                        f2 = p2.muriendo ? p2.frame :
		                             (p2.sube != 0 ? ((p2.y / 8) % 2) : ((p2.x / 10) % 3));
		                    }

		                    int redraw_p1 = (p1.x != p1.x_ant || p1.y != p1.y_ant || f1 != p1.frame_ant);
		                    int redraw_p2 = (jugadores == 2) &&
		                                    (p2.x != p2.x_ant || p2.y != p2.y_ant || f2 != p2.frame_ant);

		                    if (!p1.muriendo && !victoria_activa) {
		                        if (marioEnPlataformaVictoria_P(&p1, 0)) {
		                            victoria_activa = 1;
		                            break;
		                        }
		                    }

		                    if (jugadores == 2 && !p2.muriendo && !victoria_activa) {
		                        if (marioEnPlataformaVictoria_P(&p2, 0)) {
		                            victoria_activa = 1;
		                            break;
		                        }
		                    }

		                    // --- 6. RESTAURAR TODO LO VIEJO PRIMERO ---

		                    // A. DK
		                    if (dk_anim != dk_anim_prev) {
		                        RestaurarRectNivel1(35, 39, 32, 32);
		                    }

		                    // B. Princesa
		                    if (princess_anim != princess_anim_prev) {
		                        RestaurarRectNivel1(127, 24, 44, 22);
		                    }

		                    // C. Barriles
		                    for (int i = 0; i < MAX_BARRILES; i++) {
		                        if (barriles[i].activo) {
		                            RestaurarFondoBarrilSeguro(barriles[i].x_ant, barriles[i].y_ant);
		                        }
		                    }

		                    // D. Jugadores
		                    if (redraw_p1) {
		                        RestaurarRectNivel1(p1.x_ant, p1.y_ant, 16, 16);
		                    }

		                    if (redraw_p2) {
		                        RestaurarRectNivel1(p2.x_ant, p2.y_ant, 16, 16);
		                    }

		                    // --- 7. DIBUJAR TODO LO NUEVO DESPUÉS ---

		                    // A. Donkey Kong
		                    if (dk_anim != dk_anim_prev) {
		                        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                        LCD_DibujarSpriteTransparente(
		                            35, 39,
		                            32, 32,
		                            donkey_barril,
		                            dk_anim,
		                            96,
		                            0x0000,
		                            0
		                        );
		                    }

		                    // B. Princesa
		                    if (princess_anim != princess_anim_prev) {
		                        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                        LCD_DibujarSpriteTransparente(
		                            127, 24,
		                            44, 22,
		                            princesa,
		                            princess_anim,
		                            88,
		                            0x0000,
		                            0
		                        );
		                    }

		                    // C. Barriles
		                    for (int i = 0; i < MAX_BARRILES; i++) {
		                        if (!barriles[i].activo) continue;

		                        const uint16_t *sprite_b =
		                            (barriles[i].estado == BARRIL_BAJANDO_ESC) ? barril_frontal : barril_lateral;
		                        int hoja_b =
		                            (barriles[i].estado == BARRIL_BAJANDO_ESC) ? 32 : 64;

		                        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                        LCD_DibujarSpriteTransparente(
		                            barriles[i].x, barriles[i].y,
		                            16, 16,
		                            sprite_b,
		                            barriles[i].anim % (hoja_b / 16),
		                            hoja_b,
		                            0x0000,
		                            0
		                        );
		                    }

		                    // D. P1
		                    if (redraw_p1) {
		                        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                        LCD_DibujarSpriteTransparente(
		                            p1.x, p1.y,
		                            16, 16,
		                            p1.actual_bmp,
		                            f1,
		                            p1.ancho_hoja,
		                            0x0000,
		                            p1.flip
		                        );
		                    }

		                    // E. P2
		                    if (redraw_p2) {
		                        while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                        LCD_DibujarSpriteTransparente(
		                            p2.x, p2.y,
		                            16, 16,
		                            p2.actual_bmp,
		                            f2,
		                            p2.ancho_hoja,
		                            0x0000,
		                            p2.flip
		                        );
		                    }

		                    // --- 8. COMMIT DE ESTADOS NUEVOS ---
		                    if (dk_anim != dk_anim_prev) {
		                        dk_anim_prev = dk_anim;
		                    }

		                    if (princess_anim != princess_anim_prev) {
		                        princess_anim_prev = princess_anim;
		                    }

		                    for (int i = 0; i < MAX_BARRILES; i++) {
		                        if (barriles[i].activo) {
		                            barriles[i].x_ant = barriles[i].x;
		                            barriles[i].y_ant = barriles[i].y;
		                        }
		                    }

		                    if (redraw_p1) {
		                        p1.x_ant = p1.x;
		                        p1.y_ant = p1.y;
		                        p1.frame_ant = f1;
		                    }

		                    if (redraw_p2) {
		                        p2.x_ant = p2.x;
		                        p2.y_ant = p2.y;
		                        p2.frame_ant = f2;
		                    }

		                    break;

// === Nivel 2 ========================================================================================
		                case ESTADO_NIVEL_2:
		                    if (cambioDePantalla) {
		                    	HAL_UART_Transmit(&huart3, (uint8_t*)"2", 1, 1000);

		                        DibujarNivel2Tileado();

		                        plataformas_actuales = nivel2;
		                        numPlataformas_actual = 8;
		                        escaleras_actuales = nivel2_escaleras;
		                        numEscaleras_actual = 12;

		                        // ---------- P1 ----------
		                        p1.x = LVL2_MARIO_START_X;
		                        p1.y = alturaPlataformaEnX(7, p1.x + 8);
		                        p1.x_ant = p1.x;
		                        p1.y_ant = p1.y;
		                        p1.y_past = p1.y;
		                        p1.flip = 0;
		                        p1.actual_bmp = mario_camina;
		                        p1.ancho_hoja = 48;
		                        p1.jumpState = 0;
		                        p1.jumpProgress = 0;
		                        p1.inercia_x = 0;
		                        p1.sube = 0;
		                        p1.muriendo = 0;
		                        p1.frame = 0;
		                        p1.tick = 0;
		                        p1.frame_ant = 255;
		                        p1.color_transparente = 0x0000;

		                        // ---------- P2 ----------
		                        p2.muriendo = 0;
		                        p2.frame = 0;
		                        p2.tick = 0;
		                        p2.frame_ant = 255;
		                        p2.jumpState = 0;
		                        p2.jumpProgress = 0;
		                        p2.inercia_x = 0;
		                        p2.sube = 0;
		                        p2.flip = 0;
		                        p2.color_transparente = 0x0000;

		                        if (jugadores == 2) {
		                            p2.x = LVL2_MARIO_START_X + 24;
		                            p2.y = alturaPlataformaEnX(7, p2.x + 8);
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.y_past = p2.y;
		                            p2.actual_bmp = luigi_camina;
		                            p2.ancho_hoja = 48;
		                        }

		                        dk_anim_prev = -1;
		                        princess_anim_prev = -1;
		                        frame_counter = 0;

		                        InicializarFuegosNivel2();
		                        fuego_tick = HAL_GetTick();

		                        cambioDePantalla = 0;
		                    }

		                    // ---------- DK bailando ----------
		                    {
		                        int dk2_anim = (frame_counter / 10) % 4;

		                        if (dk2_anim != dk_anim_prev) {
		                            RestaurarRectNivel2(LVL2_DK_X, LVL2_DK_Y, 32, 32);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                LVL2_DK_X, LVL2_DK_Y,
		                                32, 32,
		                                dkong_dance,
		                                dk2_anim,
		                                128,
		                                0x0000,
		                                0
		                            );

		                            dk_anim_prev = dk2_anim;
		                        }
		                    }

		                    // ---------- Princesa ----------
		                    {
		                        int princess_anim2 = (frame_counter / 15) % 2;

		                        if (princess_anim2 != princess_anim_prev) {
		                            RestaurarRectNivel2(LVL2_PRINCESS_X, LVL2_PRINCESS_Y, 44, 22);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                LVL2_PRINCESS_X, LVL2_PRINCESS_Y,
		                                44, 22,
		                                princesa,
		                                princess_anim2,
		                                88,
		                                0x0000,
		                                0
		                            );

		                            princess_anim_prev = princess_anim2;
		                        }
		                    }

		                    frame_counter++;

		                    // ---------- Fuegos ----------
		                    if (HAL_GetTick() - fuego_tick >= 90) {
		                        fuego_tick = HAL_GetTick();
		                        ActualizarFuegosNivel2();
		                        DibujarFuegosNivel2();
		                    }

		                    // ---------- Colisiones con fuegos ----------
		                    ColisionPersonajeFuegosNivel2(&p1);
		                    if (jugadores == 2) {
		                        ColisionPersonajeFuegosNivel2(&p2);
		                    }

		                    // ---------- Animación de muerte ----------
		                    if (p1.muriendo) {
		                        p1.tick++;

		                        if (p1.tick >= 6) {
		                            p1.tick = 0;

		                            if (p1.frame < 4) {
		                                p1.frame++;
		                            } else {
		                            	HAL_UART_Transmit(&huart3, (uint8_t*)"D", 1, 1000);
		                                reiniciarJuego();
		                                break;
		                            }
		                        }

		                        p1.actual_bmp = mario_muere;
		                        p1.ancho_hoja = 80;
		                    }

		                    if (jugadores == 2 && p2.muriendo) {
		                        p2.tick++;

		                        if (p2.tick >= 6) {
		                            p2.tick = 0;

		                            if (p2.frame < 4) {
		                                p2.frame++;
		                            } else {
		                                // respawn simple de P2
		                                p2.muriendo = 0;
		                                p2.frame = 0;
		                                p2.tick = 0;
		                                p2.x = LVL2_MARIO_START_X + 24;
		                                p2.y = alturaPlataformaEnX(7, p2.x + 8);
		                                p2.x_ant = p2.x;
		                                p2.y_ant = p2.y;
		                                p2.y_past = p2.y;
		                                p2.jumpState = 0;
		                                p2.jumpProgress = 0;
		                                p2.inercia_x = 0;
		                                p2.sube = 0;
		                                p2.flip = 0;
		                                p2.actual_bmp = luigi_camina;
		                                p2.ancho_hoja = 48;
		                                p2.frame_ant = 255;
		                            }
		                        }

		                        if (p2.muriendo) {
		                            p2.actual_bmp = luigi_muere;
		                            p2.ancho_hoja = 80;
		                        }
		                    }

		                    // ---------- Movimiento ----------
		                    ProcesarMovimientoNivel2(&p1);
		                    if (jugadores == 2) {
		                        ProcesarMovimientoNivel2(&p2);
		                    }

		                    // ---------- Frames actuales ----------
		                    {
		                        int f1 = p1.muriendo ? p1.frame :
		                                 (p1.sube != 0 ? ((p1.y / 8) % 2) : ((p1.x / 10) % 3));

		                        int redraw_p1 = (p1.x != p1.x_ant || p1.y != p1.y_ant || f1 != p1.frame_ant);

		                        int f2 = 0;
		                        int redraw_p2 = 0;

		                        if (jugadores == 2) {
		                            f2 = p2.muriendo ? p2.frame :
		                                 (p2.sube != 0 ? ((p2.y / 8) % 2) : ((p2.x / 10) % 3));

		                            redraw_p2 = (p2.x != p2.x_ant || p2.y != p2.y_ant || f2 != p2.frame_ant);
		                        }

		                        // ---------- Restaurar fondos viejos ----------
		                        if (redraw_p1) {
		                            RestaurarRectNivel2(p1.x_ant, p1.y_ant, 16, 16);
		                        }

		                        if (redraw_p2) {
		                            RestaurarRectNivel2(p2.x_ant, p2.y_ant, 16, 16);
		                        }

		                        // si está muriendo, limpia su posición actual antes de redibujar animación
		                        if (p1.muriendo) {
		                            RestaurarRectNivel2(p1.x, p1.y, 16, 16);
		                        }

		                        if (jugadores == 2 && p2.muriendo) {
		                            RestaurarRectNivel2(p2.x, p2.y, 16, 16);
		                        }

		                        // ---------- Victoria ----------
		                        if (!p1.muriendo && !victoria_activa) {
		                            if (marioEnPlataformaVictoria_P(&p1, 0)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        if (jugadores == 2 && !p2.muriendo && !victoria_activa) {
		                            if (marioEnPlataformaVictoria_P(&p2, 0)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        // ---------- Dibujar personajes ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p1.x, p1.y,
		                                16, 16,
		                                p1.actual_bmp,
		                                f1,
		                                p1.ancho_hoja,
		                                0x0000,
		                                p1.flip
		                            );
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p2.x, p2.y,
		                                16, 16,
		                                p2.actual_bmp,
		                                f2,
		                                p2.ancho_hoja,
		                                0x0000,
		                                p2.flip
		                            );
		                        }

		                        // ---------- Commit ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            p1.x_ant = p1.x;
		                            p1.y_ant = p1.y;
		                            p1.frame_ant = f1;
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.frame_ant = f2;
		                        }
		                    }

		                    HAL_Delay(10);
		                    break;
// === Nivel 3 =========================================================================================
		                case ESTADO_NIVEL_3:
		                    if (cambioDePantalla) {
		                    	HAL_UART_Transmit(&huart3, (uint8_t*)"3", 1, 1000);

		                        DibujarNivel3Tileado();

		                        plataformas_actuales = nivel3;
		                        numPlataformas_actual = 25;
		                        escaleras_actuales = nivel3_escaleras;
		                        numEscaleras_actual = 10;

		                        // ---------- P1 ----------
		                        p1.x = 15;
		                        p1.y = alturaPlataformaEnX(24, p1.x + 8);
		                        p1.x_ant = p1.x;
		                        p1.y_ant = p1.y;
		                        p1.y_past = p1.y;
		                        p1.flip = 0;
		                        p1.actual_bmp = mario_camina;
		                        p1.ancho_hoja = 48;
		                        p1.jumpState = 0;
		                        p1.jumpProgress = 0;
		                        p1.inercia_x = 0;
		                        p1.sube = 0;
		                        p1.muriendo = 0;
		                        p1.frame = 0;
		                        p1.tick = 0;
		                        p1.frame_ant = 255;
		                        p1.color_transparente = 0x0000;

		                        // ---------- P2 ----------
		                        p2.muriendo = 0;
		                        p2.frame = 0;
		                        p2.tick = 0;
		                        p2.frame_ant = 255;
		                        p2.jumpState = 0;
		                        p2.jumpProgress = 0;
		                        p2.inercia_x = 0;
		                        p2.sube = 0;
		                        p2.flip = 0;
		                        p2.color_transparente = 0x0000;

		                        if (jugadores == 2) {
		                            p2.x = 39;
		                            p2.y = alturaPlataformaEnX(24, p2.x + 8);
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.y_past = p2.y;
		                            p2.actual_bmp = luigi_camina;
		                            p2.ancho_hoja = 48;
		                        }

		                        dk_anim_prev = -1;
		                        princess_anim_prev = -1;
		                        frame_counter = 0;

		                        InicializarFuegosNivel3();
		                        fuego_tick = HAL_GetTick();

		                        cambioDePantalla = 0;
		                    }

		                    // ---------- DK bailando ----------
		                    {
		                        int dk3_anim = (frame_counter / 10) % 4;

		                        if (dk3_anim != dk_anim_prev) {
		                            RestaurarRectNivel3(LVL2_DK_X, LVL2_DK_Y, 32, 32);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                LVL2_DK_X, LVL2_DK_Y,
		                                32, 32,
		                                dkong_dance,
		                                dk3_anim,
		                                128,
		                                0x0000,
		                                0
		                            );

		                            dk_anim_prev = dk3_anim;
		                        }
		                    }

		                    // ---------- Princesa ----------
		                    {
		                        int princess_anim3 = (frame_counter / 15) % 2;

		                        if (princess_anim3 != princess_anim_prev) {
		                            RestaurarRectNivel3(LVL2_PRINCESS_X, LVL2_PRINCESS_Y, 44, 22);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                LVL2_PRINCESS_X, LVL2_PRINCESS_Y,
		                                44, 22,
		                                princesa,
		                                princess_anim3,
		                                88,
		                                0x0000,
		                                0
		                            );

		                            princess_anim_prev = princess_anim3;
		                        }
		                    }

		                    frame_counter++;

		                    // ---------- Fuegos ----------
		                    if (HAL_GetTick() - fuego_tick >= 90) {
		                        fuego_tick = HAL_GetTick();
		                        ActualizarFuegosNivel3();
		                        DibujarFuegosNivel3();
		                    }

		                    // ---------- Colisiones con fuegos ----------
		                    ColisionPersonajeFuegosNivel3(&p1);
		                    if (jugadores == 2) {
		                        ColisionPersonajeFuegosNivel3(&p2);
		                    }

		                    // ---------- Animación de muerte ----------
		                    if (p1.muriendo) {
		                        p1.tick++;

		                        if (p1.tick >= 6) {
		                            p1.tick = 0;

		                            if (p1.frame < 4) {
		                                p1.frame++;
		                            } else {
		                            	HAL_UART_Transmit(&huart3, (uint8_t*)"D", 1, 1000);
		                                reiniciarJuego();
		                                break;
		                            }
		                        }

		                        p1.actual_bmp = mario_muere;
		                        p1.ancho_hoja = 80;
		                    }

		                    if (jugadores == 2 && p2.muriendo) {
		                        p2.tick++;

		                        if (p2.tick >= 6) {
		                            p2.tick = 0;

		                            if (p2.frame < 4) {
		                                p2.frame++;
		                            } else {
		                                // respawn simple de P2
		                                p2.muriendo = 0;
		                                p2.frame = 0;
		                                p2.tick = 0;
		                                p2.x = 39;
		                                p2.y = alturaPlataformaEnX(24, p2.x + 8);
		                                p2.x_ant = p2.x;
		                                p2.y_ant = p2.y;
		                                p2.y_past = p2.y;
		                                p2.jumpState = 0;
		                                p2.jumpProgress = 0;
		                                p2.inercia_x = 0;
		                                p2.sube = 0;
		                                p2.flip = 0;
		                                p2.actual_bmp = luigi_camina;
		                                p2.ancho_hoja = 48;
		                                p2.frame_ant = 255;
		                            }
		                        }

		                        if (p2.muriendo) {
		                            p2.actual_bmp = luigi_muere;
		                            p2.ancho_hoja = 80;
		                        }
		                    }

		                    // ---------- Movimiento ----------
		                    if (!p1.muriendo) {
		                        if (comando == 'j' && p1.jumpState == 0) {
		                            p1.jumpState = 1;
		                            p1.jumpProgress = 0;
		                            comando = '0';
		                            p1.y_past = p1.y;
		                            p1.actual_bmp = mario_brinca;
		                            p1.ancho_hoja = 16;
		                        }
		                    }

		                    ProcesarMovimientoNivel3(&p1);
		                    if (jugadores == 2) {
		                        ProcesarMovimientoNivel3(&p2);
		                    }

		                    // ---------- Frames actuales ----------
		                    {
		                        int f1 = p1.muriendo ? p1.frame :
		                                 (p1.sube != 0 ? ((p1.y / 8) % 2) : ((p1.x / 10) % 3));

		                        int redraw_p1 = (p1.x != p1.x_ant || p1.y != p1.y_ant || f1 != p1.frame_ant);

		                        int f2 = 0;
		                        int redraw_p2 = 0;

		                        if (jugadores == 2) {
		                            f2 = p2.muriendo ? p2.frame :
		                                 (p2.sube != 0 ? ((p2.y / 8) % 2) : ((p2.x / 10) % 3));

		                            redraw_p2 = (p2.x != p2.x_ant || p2.y != p2.y_ant || f2 != p2.frame_ant);
		                        }

		                        // ---------- Restaurar fondos viejos ----------
		                        if (redraw_p1) {
		                            RestaurarRectNivel3(p1.x_ant, p1.y_ant, 16, 16);
		                        }

		                        if (redraw_p2) {
		                            RestaurarRectNivel3(p2.x_ant, p2.y_ant, 16, 16);
		                        }

		                        if (p1.muriendo) {
		                            RestaurarRectNivel3(p1.x, p1.y, 16, 16);
		                        }

		                        if (jugadores == 2 && p2.muriendo) {
		                            RestaurarRectNivel3(p2.x, p2.y, 16, 16);
		                        }

		                        // ---------- Victoria ----------
		                        if (!p1.muriendo && !victoria_activa) {
		                            if (marioEnPlataformaVictoria_P(&p1, 0)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        if (jugadores == 2 && !p2.muriendo && !victoria_activa) {
		                            if (marioEnPlataformaVictoria_P(&p2, 0)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        // ---------- Dibujar personajes ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p1.x, p1.y,
		                                16, 16,
		                                p1.actual_bmp,
		                                f1,
		                                p1.ancho_hoja,
		                                0x0000,
		                                p1.flip
		                            );
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p2.x, p2.y,
		                                16, 16,
		                                p2.actual_bmp,
		                                f2,
		                                p2.ancho_hoja,
		                                0x0000,
		                                p2.flip
		                            );
		                        }

		                        // ---------- Commit ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            p1.x_ant = p1.x;
		                            p1.y_ant = p1.y;
		                            p1.frame_ant = f1;
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.frame_ant = f2;
		                        }
		                    }

		                    HAL_Delay(10);
		                    break;

// === Nivel 4 ===================================================================================
		                case ESTADO_NIVEL_4:
		                    if (cambioDePantalla) {
		                    	HAL_UART_Transmit(&huart3, (uint8_t*)"4", 1, 1000);

		                        DibujarNivel4Tileado();

		                        plataformas_actuales = nivel4;
		                        numPlataformas_actual = 14;
		                        escaleras_actuales = nivel4_escaleras;
		                        numEscaleras_actual = 4;

		                        // ---------- P1 ----------
		                        p1.x = 156;
		                        p1.y = alturaPlataformaEnX(13, p1.x + 8);
		                        p1.x_ant = p1.x;
		                        p1.y_ant = p1.y;
		                        p1.y_past = p1.y;
		                        p1.flip = 0;
		                        p1.actual_bmp = mario_camina;
		                        p1.ancho_hoja = 48;
		                        p1.jumpState = 0;
		                        p1.jumpProgress = 0;
		                        p1.inercia_x = 0;
		                        p1.sube = 0;
		                        p1.muriendo = 0;
		                        p1.frame = 0;
		                        p1.tick = 0;
		                        p1.frame_ant = 255;
		                        p1.color_transparente = 0x0000;

		                        // ---------- P2 ----------
		                        p2.muriendo = 0;
		                        p2.frame = 0;
		                        p2.tick = 0;
		                        p2.frame_ant = 255;
		                        p2.jumpState = 0;
		                        p2.jumpProgress = 0;
		                        p2.inercia_x = 0;
		                        p2.sube = 0;
		                        p2.flip = 0;
		                        p2.color_transparente = 0x0000;

		                        if (jugadores == 2) {
		                            p2.x = 180;
		                            p2.y = alturaPlataformaEnX(13, p2.x + 8);
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.y_past = p2.y;
		                            p2.actual_bmp = luigi_camina;
		                            p2.ancho_hoja = 48;
		                        }

		                        dk_anim_prev = -1;
		                        princess_anim_prev = -1;
		                        frame_counter = 0;

		                        InicializarFuegosNivel4();
		                        fuego_tick = HAL_GetTick();

		                        cambioDePantalla = 0;
		                    }

		                    // ---------- DK bailando ----------
		                    {
		                        int dk4_anim = (frame_counter / 10) % 4;

		                        if (dk4_anim != dk_anim_prev) {
		                            RestaurarRectNivel4(144, 48, 32, 32);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                144, 48,
		                                32, 32,
		                                dkong_dance,
		                                dk4_anim,
		                                128,
		                                0x0000,
		                                0
		                            );

		                            dk_anim_prev = dk4_anim;
		                        }
		                    }

		                    // ---------- Princesa ----------
		                    {
		                        int princess_anim4 = (frame_counter / 15) % 2;

		                        if (princess_anim4 != princess_anim_prev) {
		                            RestaurarRectNivel4(152, 10, 44, 22);

		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                152, 10,
		                                44, 22,
		                                princesa,
		                                princess_anim4,
		                                88,
		                                0x0000,
		                                0
		                            );

		                            princess_anim_prev = princess_anim4;
		                        }
		                    }

		                    frame_counter++;

		                    // ---------- Fuegos ----------
		                    if (HAL_GetTick() - fuego_tick >= 90) {
		                        fuego_tick = HAL_GetTick();
		                        ActualizarFuegosNivel4();
		                        DibujarFuegosNivel4();
		                    }

		                    // ---------- Colisiones con fuegos ----------
		                    ColisionPersonajeFuegosNivel4(&p1);
		                    if (jugadores == 2) {
		                        ColisionPersonajeFuegosNivel4(&p2);
		                    }

		                    // ---------- Animación de muerte ----------
		                    if (p1.muriendo) {
		                        p1.tick++;

		                        if (p1.tick >= 6) {
		                            p1.tick = 0;

		                            if (p1.frame < 4) {
		                                p1.frame++;
		                            } else {
		                            	HAL_UART_Transmit(&huart3, (uint8_t*)"D", 1, 1000);
		                                reiniciarJuego();
		                                break;
		                            }
		                        }

		                        p1.actual_bmp = mario_muere;
		                        p1.ancho_hoja = 80;
		                    }

		                    if (jugadores == 2 && p2.muriendo) {
		                        p2.tick++;

		                        if (p2.tick >= 6) {
		                            p2.tick = 0;

		                            if (p2.frame < 4) {
		                                p2.frame++;
		                            } else {
		                                // Respawn simple de P2
		                                p2.muriendo = 0;
		                                p2.frame = 0;
		                                p2.tick = 0;
		                                p2.x = 180;
		                                p2.y = alturaPlataformaEnX(13, p2.x + 8);
		                                p2.x_ant = p2.x;
		                                p2.y_ant = p2.y;
		                                p2.y_past = p2.y;
		                                p2.jumpState = 0;
		                                p2.jumpProgress = 0;
		                                p2.inercia_x = 0;
		                                p2.sube = 0;
		                                p2.flip = 0;
		                                p2.actual_bmp = luigi_camina;
		                                p2.ancho_hoja = 48;
		                                p2.frame_ant = 255;
		                            }
		                        }

		                        if (p2.muriendo) {
		                            p2.actual_bmp = luigi_muere;
		                            p2.ancho_hoja = 80;
		                        }
		                    }

		                    // ---------- Movimiento ----------
		                    ProcesarMovimientoNivel4(&p1);
		                    if (jugadores == 2) {
		                        ProcesarMovimientoNivel4(&p2);
		                    }

		                    // ---------- Frames actuales ----------
		                    {
		                        int f1 = p1.muriendo ? p1.frame :
		                                 (p1.sube != 0 ? ((p1.y / 8) % 2) : ((p1.x / 10) % 3));

		                        int redraw_p1 = (p1.x != p1.x_ant || p1.y != p1.y_ant || f1 != p1.frame_ant);

		                        int f2 = 0;
		                        int redraw_p2 = 0;

		                        if (jugadores == 2) {
		                            f2 = p2.muriendo ? p2.frame :
		                                 (p2.sube != 0 ? ((p2.y / 8) % 2) : ((p2.x / 10) % 3));

		                            redraw_p2 = (p2.x != p2.x_ant || p2.y != p2.y_ant || f2 != p2.frame_ant);
		                        }

		                        // ---------- Restaurar fondos viejos ----------
		                        if (redraw_p1) {
		                            RestaurarRectNivel4(p1.x_ant, p1.y_ant, 16, 16);
		                        }

		                        if (redraw_p2) {
		                            RestaurarRectNivel4(p2.x_ant, p2.y_ant, 16, 16);
		                        }

		                        if (p1.muriendo) {
		                            RestaurarRectNivel4(p1.x, p1.y, 16, 16);
		                        }

		                        if (jugadores == 2 && p2.muriendo) {
		                            RestaurarRectNivel4(p2.x, p2.y, 16, 16);
		                        }

		                        // ---------- Victoria ----------
		                        if (!p1.muriendo && !victoria_activa) {
		                            if (hayColision(p1.x, p1.y, 16, 16, 144, 48, 32, 32)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        if (jugadores == 2 && !p2.muriendo && !victoria_activa) {
		                            if (hayColision(p2.x, p2.y, 16, 16, 144, 48, 32, 32)) {
		                                victoria_activa = 1;
		                                break;
		                            }
		                        }

		                        // ---------- Dibujar personajes ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p1.x, p1.y,
		                                16, 16,
		                                p1.actual_bmp,
		                                f1,
		                                p1.ancho_hoja,
		                                0x0000,
		                                p1.flip
		                            );
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            while (hspi1.State == HAL_SPI_STATE_BUSY_TX);
		                            LCD_DibujarSpriteTransparente(
		                                p2.x, p2.y,
		                                16, 16,
		                                p2.actual_bmp,
		                                f2,
		                                p2.ancho_hoja,
		                                0x0000,
		                                p2.flip
		                            );
		                        }

		                        // ---------- Commit ----------
		                        if (redraw_p1 || p1.muriendo) {
		                            p1.x_ant = p1.x;
		                            p1.y_ant = p1.y;
		                            p1.frame_ant = f1;
		                        }

		                        if (jugadores == 2 && (redraw_p2 || p2.muriendo)) {
		                            p2.x_ant = p2.x;
		                            p2.y_ant = p2.y;
		                            p2.frame_ant = f2;
		                        }
		                    }

		                    HAL_Delay(10);
		                    break;

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|SD_CS_Pin, GPIO_PIN_SET);

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
	    	ctrl_state1 = rx_data1;

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
        	ctrl_state6 = rx_data6;

            //HAL_UART_Transmit(&huart2, &rx_data, 1, 100);

            HAL_UART_Receive_IT(&huart6, &rx_data6, 1);
        }

}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // Esto es lo mismo que LCD_CS_H()
    	//HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
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
