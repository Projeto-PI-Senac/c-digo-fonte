#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>                       // guia de linguagem C
#include <stdbool.h>                     // adicionar o bool
#include <string.h>                      // manipulação de strings (strlen, strcpy)
#include <allegro5/allegro.h>            // controla as janelas
#include <allegro5/allegro_primitives.h> // formas geométricas básicas
#include <allegro5/allegro_font.h>       // sistema básico de fontes e texto
#include <allegro5/allegro_ttf.h>        // carregar fontes personalizadas
#include <allegro5/allegro_image.h>      // carregar imagens
#include <math.h>
#include "sprites.h"


#define LARGURA_TELA 1600
#define ALTURA_TELA 1000
#define LINHAS_MAPA 25
#define COLUNAS_MAPA 40
#define TAMANHO_PIXEL 40

#define MAPA_LARGURA_PIXELS (COLUNAS_MAPA * TAMANHO_PIXEL)
#define MAPA_ALTURA_PIXELS (LINHAS_MAPA * TAMANHO_PIXEL)

typedef struct
{
    float x, y;
    float origemX, origemY;
    float velocidade;
    int alerta;          // 0 = Normal/Parado, 1 = Desconfiado, 2 = Perseguindo, 3 = Voltando
    float visao;
    int tempo_suspeito;
} Inimigo;

typedef struct
{
    int linha, coluna;
    float x, y;
    bool ativo;
    char falas[4][220];
    int indice_fala;
    bool interagindo;
} NPC;

typedef struct {
    int linha, coluna;
    float x, y;
    bool coletado;
    char texto_pista[150];
} PistaMapa;

typedef struct {
    int linha, coluna;
    float x, y;
} Figurante;

bool guarda_tem_visao_livre(int mapa[LINHAS_MAPA][COLUNAS_MAPA], float x_guarda, float y_guarda, float x_jogador, float y_jogador)
{
    int passos = 20;
    for (int i = 1; i < passos; i++)
    {
        float t = (float)i / passos;
        float check_x = x_guarda + (x_jogador - x_guarda) * t + (TAMANHO_PIXEL / 2.0f);
        float check_y = y_guarda + (y_jogador - y_guarda) * t + (TAMANHO_PIXEL / 2.0f);

        int col = (int)(check_x / TAMANHO_PIXEL);
        int lin = (int)(check_y / TAMANHO_PIXEL);

        if (lin >= 0 && lin < LINHAS_MAPA && col >= 0 && col < COLUNAS_MAPA)
        {
            if (mapa[lin][col] == 1)
            {
                return false;
            }
        }
    }
    return true;
}

int jogo()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();

    al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);

    ALLEGRO_DISPLAY* janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!janela)
    {
        printf("Erro critico: nao foi possivel criar a janela do jogo!\n");
        return -1;
    }

    sprites_inicializar();

    ALLEGRO_FONT* fonte_dialogo = al_load_ttf_font("Arial.ttf", 22, 0);
    if (!fonte_dialogo) fonte_dialogo = al_load_ttf_font("arial.ttf", 22, 0);
    if (!fonte_dialogo) {
        fonte_dialogo = al_create_builtin_font();
    }

    ALLEGRO_BITMAP* mapa_fundo = al_load_bitmap("mapa.png");
    if (!mapa_fundo) {
        mapa_fundo = al_load_bitmap("Sprites/mapa.png");
    }

    ALLEGRO_BITMAP* mapa_noite = al_load_bitmap("Sprites/Mapa A Noite.png");
    if (!mapa_noite) {
        mapa_noite = al_load_bitmap("./Sprites/Mapa A Noite.png");
    }

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* eventos = al_create_event_queue();

    al_register_event_source(eventos, al_get_keyboard_event_source());
    al_register_event_source(eventos, al_get_display_event_source(janela));
    al_register_event_source(eventos, al_get_mouse_event_source());
    al_register_event_source(eventos, al_get_timer_event_source(timer));

    al_start_timer(timer);

    int jogador_linha = 12;
    int jogador_coluna = 20;

    bool jogador_virado_esquerda = false;
    bool guarda_virado_esquerda = false;

    int jogador_direcao = DIR_FRENTE;
    int timer_andar_jogador = 0;

    int guarda_direcao = DIR_FRENTE;
    bool guarda_andando = false;

    int estado_missao = 0;
    bool itens_visiveis = false;
    bool jogador_fardado = false;

    bool peca1_coletada = false;
    int peca1_linha = 7, peca1_coluna = 4;

    bool peca2_coletada = false;
    int peca2_linha = 17, peca2_coluna = 34;

    bool farda_coletada = false;
    int farda_linha = 11, farda_coluna = 23;

    PistaMapa pistas[2];
    pistas[0].linha = 15; pistas[0].coluna = 5; pistas[0].coletado = false;
    strcpy(pistas[0].texto_pista, "PISTA 1: 'O regime militar foi instaurado em 1964.'");

    pistas[1].linha = 10; pistas[1].coluna = 35; pistas[1].coletado = false;
    strcpy(pistas[1].texto_pista, "PISTA 2: 'O AI-5 foi o ato mais rigoroso de cassaçao.'");

    bool chave_ditadura_coletada = false;
    int chave_linha = 16, chave_coluna = 35;

    int timer_aviso_pista = 0;
    char texto_pista_atual[160] = "";

    bool modo_digitacao = false;
    char buffer_resposta[64] = "";
    int pergunta_ditadura_atual = 0;
    bool quiz_ditadura_concluido = false;
    bool terminal_interagindo = false;

    // Estados extras para telas de Game Over e recado de farda
    bool tela_game_over = false;
    char texto_game_over[200] = "";
    int timer_game_over = 0;

    int mapa[LINHAS_MAPA][COLUNAS_MAPA] = {
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    };
    
    int mapa_ditadura[LINHAS_MAPA][COLUNAS_MAPA] = {
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
        { 1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1 },
        { 1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1 },
        { 1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
    };

    // Mapa do último estágio (Busca pelo documento secreto final)
    int mapa_final[LINHAS_MAPA][COLUNAS_MAPA] = {
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,0,1 },
        { 1,0,1,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1 },
        { 1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
    };

    bool jogando = true;
    bool redesenhar = true;

    Inimigo guarda;
    int linhaInicial = 6;
    int colunaInicial = 35;

    guarda.origemX = colunaInicial * TAMANHO_PIXEL;
    guarda.origemY = linhaInicial * TAMANHO_PIXEL;
    guarda.x = guarda.origemX;
    guarda.y = guarda.origemY;
    guarda.velocidade = 2.2;
    guarda.alerta = 0;
    guarda.visao = 200.0;
    guarda.tempo_suspeito = 0;

    NPC mentor;
    mentor.linha = 12;
    mentor.coluna = 5;
    mentor.x = mentor.coluna * TAMANHO_PIXEL;
    mentor.y = mentor.linha * TAMANHO_PIXEL;
    mentor.ativo = false;
    mentor.indice_fala = 0;
    mentor.interagindo = false;

    int terminal_linha = 7;
    int terminal_coluna = 23;
    bool terminal_ativo = false;

    // Coordenadas do documento secreto final no mapa 8
    int doc_final_linha = 3;
    int doc_final_coluna = 35;
    bool documento_final_coletado = false;

    Figurante figurantes[2];
    figurantes[0].linha = 8;
    figurantes[0].coluna = 12;
    figurantes[0].x = figurantes[0].coluna * TAMANHO_PIXEL;
    figurantes[0].y = figurantes[0].linha * TAMANHO_PIXEL;
    mapa[figurantes[0].linha][figurantes[0].coluna] = 1;

    figurantes[1].linha = 16;
    figurantes[1].coluna = 15;
    figurantes[1].x = figurantes[1].coluna * TAMANHO_PIXEL;
    figurantes[1].y = figurantes[1].linha * TAMANHO_PIXEL;
    mapa[figurantes[1].linha][figurantes[1].coluna] = 1;

    strcpy(mentor.falas[0], "Ola, jovem! Que bom te ver por aqui nesta praça.");
    strcpy(mentor.falas[1], "Estive planejando uma maquina revolucionaria ultimamente...");
    strcpy(mentor.falas[2], "Mas acabei perdendo algumas pecas essenciais dela enquanto passeava por ai.");
    strcpy(mentor.falas[3], "Voce poderia me dar uma mao para encontra-las?");

    int contador_passo = 0;
    int velocidade_passo = 12;

    while (jogando)
    {
        ALLEGRO_EVENT event;
        al_wait_for_event(eventos, &event);

        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            redesenhar = true;

            if (tela_game_over)
            {
                timer_game_over--;
                if (timer_game_over <= 0)
                {
                    tela_game_over = false;
                    terminal_interagindo = false;
                    modo_digitacao = false;
                    pergunta_ditadura_atual = 0;
                    // Reseta posição para fora da base
                    jogador_linha = 9;
                    jogador_coluna = 1;
                }
                continue;
            }

            ALLEGRO_KEYBOARD_STATE key_state;
            al_get_keyboard_state(&key_state);

            if (!modo_digitacao)
            {
                contador_passo++;
                if (contador_passo >= velocidade_passo)
                {
                    int linha = jogador_linha;
                    int coluna = jogador_coluna;
                    bool moveu = false;

                    if (al_key_down(&key_state, ALLEGRO_KEY_UP)) { linha--; moveu = true; jogador_direcao = DIR_COSTAS; timer_andar_jogador = 15; }
                    else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN)) { linha++; moveu = true; jogador_direcao = DIR_FRENTE; timer_andar_jogador = 15; }
                    else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) { coluna--; moveu = true; jogador_virado_esquerda = true; jogador_direcao = DIR_ESQUERDA; timer_andar_jogador = 15; }
                    else if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) { coluna++; moveu = true; jogador_virado_esquerda = false; jogador_direcao = DIR_DIREITA; timer_andar_jogador = 15; }

                    if (timer_andar_jogador > 0) timer_andar_jogador--;

                    int linhas_atual_mapa = (estado_missao == 8) ? 9 : LINHAS_MAPA;
                    int colunas_atual_mapa = (estado_missao == 8) ? COLUNAS_MAPA : COLUNAS_MAPA;

                    if (moveu && linha >= 0 && linha < linhas_atual_mapa && coluna >= 0 && coluna < colunas_atual_mapa)
                    {
                        int obstaculo = 0;
                        if (estado_missao == 8) obstaculo = mapa_final[linha][coluna];
                        else obstaculo = mapa[linha][coluna];

                        if (obstaculo != 1 && !(estado_missao != 8 && linha == mentor.linha && coluna == mentor.coluna))
                        {
                            jogador_linha = linha;
                            jogador_coluna = coluna;
                            contador_passo = 0;
                        }
                    }
                }
            }

            int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
            int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

            if (itens_visiveis && estado_missao != 3 && estado_missao != 8)
            {
                if (!peca1_coletada && jogador_linha == peca1_linha && jogador_coluna == peca1_coluna)
                {
                    peca1_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "PECA 1: Engrenagem coletada!");
                }

                if (!peca2_coletada && jogador_linha == peca2_linha && jogador_coluna == peca2_coluna)
                {
                    peca2_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "PECA 2: Componente coletado!");
                }
            }

            if (estado_missao == 3 && !farda_coletada)
            {
                if (jogador_linha == farda_linha && jogador_coluna == farda_coluna)
                {
                    farda_coletada = true;
                    jogador_fardado = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "DISFARCE OBTIDO: Voce vestiu o uniforme do batalhao!");
                }
            }

            if (estado_missao == 3)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (!pistas[i].coletado && jogador_linha == pistas[i].linha && jogador_coluna == pistas[i].coluna)
                    {
                        pistas[i].coletado = true;
                        timer_aviso_pista = 300;
                        strcpy(texto_pista_atual, pistas[i].texto_pista);
                    }
                }
            }

            if (estado_missao == 3 && !chave_ditadura_coletada)
            {
                if (jogador_linha == chave_linha && jogador_coluna == chave_coluna)
                {
                    chave_ditadura_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "CHAVE COLETADA: Arquivos secretos liberados!");
                }
            }

            if (estado_missao == 8 && !documento_final_coletado)
            {
                if (jogador_linha == doc_final_linha && jogador_coluna == doc_final_coluna)
                {
                    documento_final_coletado = true;
                    timer_aviso_pista = 300;
                    strcpy(texto_pista_atual, "PARABENS! Voce encontrou o documento secreto final!");
                }
            }

            float dist_npc = sqrt(pow(jogador_x_mundo - mentor.x, 2) + pow(jogador_y_mundo - mentor.y, 2));
            mentor.ativo = (dist_npc < 60.0);

            float term_x = terminal_coluna * TAMANHO_PIXEL;
            float term_y = terminal_linha * TAMANHO_PIXEL;
            float dist_term = sqrt(pow(jogador_x_mundo - term_x, 2) + pow(jogador_y_mundo - term_y, 2));
            terminal_ativo = (dist_term < 60.0);

            if (!mentor.ativo && mentor.interagindo && estado_missao != 4 && estado_missao != 5 && estado_missao != 6 && estado_missao != 7)
            {
                mentor.interagindo = false;
                mentor.indice_fala = 0;
            }

            // Lógica mecânica do guarda
            if (estado_missao == 3)
            {
                float dx = jogador_x_mundo - guarda.x;
                float dy = jogador_y_mundo - guarda.y;
                float distancia = sqrt(dx * dx + dy * dy);
                bool visao_livre = guarda_tem_visao_livre(mapa, guarda.x, guarda.y, jogador_x_mundo, jogador_y_mundo);

                if (guarda.alerta == 0 && distancia <= guarda.visao && visao_livre)
                {
                    guarda.alerta = 1;
                    guarda.tempo_suspeito = 0;
                }
                else if (guarda.alerta == 1)
                {
                    if (distancia > guarda.visao || !visao_livre)
                    {
                        guarda.tempo_suspeito++;
                        if (guarda.tempo_suspeito > 150)
                        {
                            guarda.alerta = 0;
                            guarda.tempo_suspeito = 0;
                        }
                    }
                    else
                    {
                        guarda.tempo_suspeito += 2;
                        if (guarda.tempo_suspeito > 150)
                        {
                            guarda.alerta = 2;
                        }
                    }
                }

                if (guarda.alerta == 2 && distancia < 30.0f)
                {
                    if (jogador_fardado)
                    {
                        guarda.x = guarda.origemX;
                        guarda.y = guarda.origemY;
                        guarda.alerta = 0;
                        guarda.tempo_suspeito = 0;
                    }
                    else
                    {
                        jogador_linha = 9;
                        jogador_coluna = 1;
                        guarda.x = guarda.origemX;
                        guarda.y = guarda.origemY;
                        guarda.alerta = 0;
                        guarda.tempo_suspeito = 0;
                    }
                }

                if (guarda.alerta == 2)
                {
                    float distancia_da_origem = sqrt(pow(guarda.x - guarda.origemX, 2) + pow(guarda.y - guarda.origemY, 2));

                    if (distancia > 450.0 || distancia_da_origem > 550.0)
                    {
                        guarda.alerta = 3;
                    }
                    else if (distancia > 1.0)
                    {
                        float novo_x = guarda.x + (dx / distancia) * guarda.velocidade;
                        int coluna_futura = (int)(novo_x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                        int linha_atual = (int)(guarda.y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                        if (coluna_futura >= 0 && coluna_futura < COLUNAS_MAPA && linha_atual >= 0 && linha_atual < LINHAS_MAPA) {
                            if (mapa[linha_atual][coluna_futura] != 1) guarda.x = novo_x;
                        }

                        float novo_y = guarda.y + (dy / distancia) * guarda.velocidade;
                        int coluna_atual = (int)(guarda.x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                        int linha_futura = (int)(novo_y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                        if (coluna_atual >= 0 && coluna_atual < COLUNAS_MAPA && linha_futura >= 0 && linha_futura < LINHAS_MAPA) {
                            if (mapa[linha_futura][coluna_atual] != 1) guarda.y = novo_y;
                        }
                    }
                }
                else if (guarda.alerta == 3)
                {
                    if (distancia <= guarda.visao && visao_livre)
                    {
                        guarda.alerta = 1;
                        guarda.tempo_suspeito = 0;
                    }
                    else
                    {
                        float dx_origem = guarda.origemX - guarda.x;
                        float dy_origem = guarda.origemY - guarda.y;
                        float dist_origem = sqrt(dx_origem * dx_origem + dy_origem * dy_origem);

                        if (dist_origem <= 2.0)
                        {
                            guarda.x = guarda.origemX;
                            guarda.y = guarda.origemY;
                            guarda.alerta = 0;
                            guarda.tempo_suspeito = 0;
                        }
                        else
                        {
                            float velocidade_volta = guarda.velocidade * 1.1f;
                            float novo_x = guarda.x + (dx_origem / dist_origem) * velocidade_volta;
                            int coluna_futura = (int)(novo_x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                            int linha_atual = (int)(guarda.y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                            if (coluna_futura >= 0 && coluna_futura < COLUNAS_MAPA && linha_atual >= 0 && linha_atual < LINHAS_MAPA) {
                                if (mapa[linha_atual][coluna_futura] != 1) guarda.x = novo_x;
                            }

                            float novo_y = guarda.y + (dy_origem / dist_origem) * velocidade_volta;
                            int coluna_atual = (int)(guarda.x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                            int linha_futura = (int)(guarda.y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                            if (coluna_atual >= 0 && coluna_atual < COLUNAS_MAPA && linha_futura >= 0 && linha_futura < LINHAS_MAPA) {
                                if (mapa[linha_futura][coluna_atual] != 1) guarda.y = novo_y;
                            }
                        }
                    }
                }
            }
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            jogando = false;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                jogando = false;
            }

            if (modo_digitacao)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
                {
                    int len = strlen(buffer_resposta);
                    if (len > 0) buffer_resposta[len - 1] = '\0';
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    if (pergunta_ditadura_atual == 0)
                    {
                        if (strcmp(buffer_resposta, "1964") == 0)
                        {
                            pergunta_ditadura_atual = 1;
                            strcpy(buffer_resposta, "");
                        }
                        else
                        {
                            // ERRO NA PERGUNTA 1: Game Over imediato
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Voce foi descoberto.");
                        }
                    }
                    else if (pergunta_ditadura_atual == 1)
                    {
                        if (strcmp(buffer_resposta, "AI-5") == 0 || strcmp(buffer_resposta, "AI5") == 0 || strcmp(buffer_resposta, "ai-5") == 0 || strcmp(buffer_resposta, "ai5") == 0)
                        {
                            pergunta_ditadura_atual = 2;
                            strcpy(buffer_resposta, "");
                        }
                        else
                        {
                            // ERRO NA PERGUNTA 2: Game Over imediato
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Voce foi descoberto.");
                        }
                    }
                    else if (pergunta_ditadura_atual == 2)
                    {
                        if (strcmp(buffer_resposta, "COMUNISMO") == 0 || strcmp(buffer_resposta, "SUBVERSAO") == 0 || strcmp(buffer_resposta, "comunismo") == 0 || strcmp(buffer_resposta, "subversao") == 0)
                        {
                            quiz_ditadura_concluido = true;
                            modo_digitacao = false;
                            terminal_interagindo = false;
                            estado_missao = 8; // Vai para o último mapa de busca do documento final!
                            jogador_linha = 7;
                            jogador_coluna = 1;
                        }
                        else
                        {
                            // ERRO NA PERGUNTA 3: Game Over imediato
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Voce foi descoberto.");
                        }
                    }
                }
                else
                {
                    char letra = '\0';
                    if (event.keyboard.keycode >= ALLEGRO_KEY_A && event.keyboard.keycode <= ALLEGRO_KEY_Z)
                    {
                        letra = 'A' + (event.keyboard.keycode - ALLEGRO_KEY_A);
                    }
                    else if (event.keyboard.keycode >= ALLEGRO_KEY_0 && event.keyboard.keycode <= ALLEGRO_KEY_9)
                    {
                        letra = '0' + (event.keyboard.keycode - ALLEGRO_KEY_0);
                    }
                    else if (event.keyboard.keycode == ALLEGRO_KEY_MINUS || event.keyboard.keycode == ALLEGRO_KEY_PAD_MINUS)
                    {
                        letra = '-';
                    }

                    if (letra != '\0' && strlen(buffer_resposta) < 50)
                    {
                        char temp[2] = { letra, '\0' };
                        strcat(buffer_resposta, temp);
                    }
                }
            }
            else if (event.keyboard.keycode == ALLEGRO_KEY_E)
            {
                if (estado_missao == 3 && terminal_ativo && !quiz_ditadura_concluido)
                {
                    // VERIFICAÇÃO DE UNIFORME NO TERMINAL
                    if (!jogador_fardado)
                    {
                        tela_game_over = true;
                        timer_game_over = 180;
                        strcpy(texto_game_over, "GAME OVER: Sem uniforme! Acesso negado, voce foi descoberto.");
                    }
                    else
                    {
                        modo_digitacao = true;
                        terminal_interagindo = true;
                        strcpy(buffer_resposta, "");
                    }
                }
                else if (estado_missao == 3 && terminal_ativo && quiz_ditadura_concluido)
                {
                    terminal_interagindo = !terminal_interagindo;
                }
                else if (mentor.ativo && !mentor.interagindo)
                {
                    mentor.interagindo = true;

                    if (estado_missao == 2 && peca1_coletada && peca2_coletada)
                    {
                        estado_missao = 7;
                        mentor.indice_fala = 0;
                    }
                }
                else if (mentor.interagindo && estado_missao == 0)
                {
                    mentor.indice_fala++;
                    if (mentor.indice_fala > 2)
                    {
                        mentor.indice_fala = 3;
                    }
                }
                else if (mentor.interagindo && estado_missao == 4)
                {
                    mentor.interagindo = false;
                    mentor.indice_fala = 0;
                    estado_missao = 6;
                }
                else if (mentor.interagindo && estado_missao == 5)
                {
                    mentor.interagindo = false;
                    mentor.indice_fala = 0;
                    estado_missao = 2;
                    itens_visiveis = true;
                }
                else if (mentor.interagindo && estado_missao == 7)
                {
                    mentor.indice_fala++;
                    if (mentor.indice_fala > 2)
                    {
                        memcpy(mapa, mapa_ditadura, sizeof(mapa));
                        estado_missao = 3;

                        jogador_linha = 9;
                        jogador_coluna = 1;

                        mentor.interagindo = false;
                        mentor.indice_fala = 0;
                    }
                }
            }

            if (mentor.interagindo && !modo_digitacao && ((estado_missao == 0 && mentor.indice_fala == 3) || estado_missao == 6))
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_1)
                {
                    estado_missao = 5;
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_2)
                {
                    estado_missao = 4;
                }
            }
        }

        if (redesenhar && al_is_event_queue_empty(eventos))
        {
            redesenhar = false;

            int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
            int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

            float zoom = 2.0f;
            float largura_visivel = LARGURA_TELA / zoom;
            float altura_visivel = ALTURA_TELA / zoom;

            int largura_atual_pixels = (estado_missao == 8) ? (COLUNAS_MAPA * TAMANHO_PIXEL) : MAPA_LARGURA_PIXELS;
            int altura_atual_pixels = (estado_missao == 8) ? (9 * TAMANHO_PIXEL) : MAPA_ALTURA_PIXELS;

            float camera_x = (jogador_x_mundo + TAMANHO_PIXEL / 2.0f) - (largura_visivel / 2.0f);
            float camera_y = (jogador_y_mundo + TAMANHO_PIXEL / 2.0f) - (altura_visivel / 2.0f);

            if (camera_x < 0) camera_x = 0;
            if (camera_y < 0) camera_y = 0;
            if (camera_x > largura_atual_pixels - largura_visivel) camera_x = largura_atual_pixels - largura_visivel;
            if (camera_y > altura_atual_pixels - altura_visivel) camera_y = altura_atual_pixels - altura_visivel;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            ALLEGRO_TRANSFORM camera;
            al_identity_transform(&camera);
            al_translate_transform(&camera, -camera_x, -camera_y);
            al_scale_transform(&camera, zoom, zoom);
            al_use_transform(&camera);

            if (estado_missao == 8)
            {
                // Desenha o fundo do mapa final (cor escura de base secreta)
                al_draw_filled_rectangle(0, 0, largura_atual_pixels, altura_atual_pixels, al_map_rgb(25, 25, 35));
            }
            else if (estado_missao == 3 && mapa_noite != NULL)
            {
                al_draw_bitmap(mapa_noite, 0, 0, 0);
            }
            else if (mapa_fundo != NULL)
            {
                al_draw_bitmap(mapa_fundo, 0, 0, 0);
            }

            if (estado_missao == 8 && !documento_final_coletado)
            {
                // Desenha o documento final no mapa 8
                al_draw_filled_rectangle(doc_final_coluna * TAMANHO_PIXEL + 5, doc_final_linha * TAMANHO_PIXEL + 5,
                    (doc_final_coluna + 1) * TAMANHO_PIXEL - 5, (doc_final_linha + 1) * TAMANHO_PIXEL - 5, al_map_rgb(255, 215, 0));
            }

            if (estado_missao == 3 && !chave_ditadura_coletada)
            {
                sprites_desenhar_chave(chave_coluna * TAMANHO_PIXEL, chave_linha * TAMANHO_PIXEL);
            }

            if (estado_missao == 3 && !farda_coletada)
            {
                al_draw_filled_rectangle(farda_coluna * TAMANHO_PIXEL + 10, farda_linha * TAMANHO_PIXEL + 10,
                    (farda_coluna + 1) * TAMANHO_PIXEL - 10, (farda_linha + 1) * TAMANHO_PIXEL - 10, al_map_rgb(100, 150, 255));

                float dist_farda = sqrt(pow(jogador_x_mundo - (farda_coluna * TAMANHO_PIXEL), 2) + pow(jogador_y_mundo - (farda_linha * TAMANHO_PIXEL), 2));
                if (dist_farda < 50.0f) {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), farda_coluna * TAMANHO_PIXEL, farda_linha * TAMANHO_PIXEL - 25, 0, "[E] Vestir Farda");
                }
            }

            if (estado_missao == 3)
            {
                for (int i = 0; i < 2; i++)
                {
                    if (!pistas[i].coletado)
                    {
                        al_draw_filled_circle(pistas[i].coluna * TAMANHO_PIXEL + 20, pistas[i].linha * TAMANHO_PIXEL + 20, 10, al_map_rgb(255, 215, 0));
                        float dist_pista = sqrt(pow(jogador_x_mundo - (pistas[i].coluna * TAMANHO_PIXEL), 2) + pow(jogador_y_mundo - (pistas[i].linha * TAMANHO_PIXEL), 2));
                        if (dist_pista < 50.0f)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), pistas[i].coluna * TAMANHO_PIXEL - 15, pistas[i].linha * TAMANHO_PIXEL - 25, 0, "[E] Ler Pista");
                        }
                    }
                }
            }

            if (estado_missao != 3 && estado_missao != 8)
            {
                for (int i = 0; i < 2; i++)
                {
                    sprites_desenhar_npc(figurantes[i].x, figurantes[i].y);
                }
            }

            if (itens_visiveis && estado_missao != 3 && estado_missao != 8)
            {
                if (!peca1_coletada)
                {
                    sprites_desenhar_jornal(peca1_coluna * TAMANHO_PIXEL, peca1_linha * TAMANHO_PIXEL);
                }
                if (!peca2_coletada)
                {
                    sprites_desenhar_documento(peca2_coluna * TAMANHO_PIXEL, peca2_linha * TAMANHO_PIXEL);
                }
            }

            guarda_virado_esquerda = (jogador_x_mundo < guarda.x);
            sprites_desenhar_jogador_animado(jogador_x_mundo, jogador_y_mundo, jogador_direcao, (timer_andar_jogador > 0), jogador_fardado);


            if (estado_missao == 3)
            {
                float diff_x = jogador_x_mundo - guarda.x;
                float diff_y = jogador_y_mundo - guarda.y;
                guarda_andando = (guarda.alerta == 2 || guarda.alerta == 3);

                if (fabs(diff_x) > fabs(diff_y))
                    guarda_direcao = (diff_x < 0) ? DIR_ESQUERDA : DIR_DIREITA;
                else
                    guarda_direcao = (diff_y < 0) ? DIR_COSTAS : DIR_FRENTE;

                sprites_desenhar_guarda_animado(guarda.x, guarda.y, guarda_direcao, guarda_andando, guarda.alerta);

                if (guarda.alerta == 1)
                {
                    float centro_x = guarda.x + (TAMANHO_PIXEL / 2.0f);
                    float topo_y = guarda.y - 32.0f;
                    al_draw_filled_rectangle(centro_x - 4, topo_y, centro_x + 4, topo_y + 22, al_map_rgb(255, 255, 0));
                    al_draw_filled_rectangle(centro_x - 4, topo_y + 26, centro_x + 4, topo_y + 34, al_map_rgb(255, 255, 0));
                }
            }

            if (estado_missao != 3 && estado_missao != 8)
            {
                int mentor_direcao = DIR_FRENTE;
                if (mentor.ativo)
                {
                    float diff_mx = jogador_x_mundo - mentor.x;
                    float diff_my = jogador_y_mundo - mentor.y;
                    if (fabs(diff_mx) > fabs(diff_my))
                        mentor_direcao = (diff_mx < 0) ? DIR_ESQUERDA : DIR_DIREITA;
                    else
                        mentor_direcao = (diff_my < 0) ? DIR_COSTAS : DIR_FRENTE;
                }
                sprites_desenhar_mentor_animado(mentor.x, mentor.y, mentor_direcao, false);

                if (mentor.ativo && !mentor.interagindo)
                {
                    al_draw_filled_circle(mentor.x + 20, mentor.y - 15, 14, al_map_rgb(240, 240, 240));
                    al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), mentor.x + 13, mentor.y - 27, 0, "E");
                }
            }
            else if (estado_missao == 3)
            {
                float tx = terminal_coluna * TAMANHO_PIXEL;
                float ty = terminal_linha * TAMANHO_PIXEL;
                if (terminal_ativo)
                {
                    al_draw_filled_circle(tx + 20, ty - 15, 14, al_map_rgb(240, 240, 240));
                    al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), tx + 13, ty - 27, 0, "E");
                }
            }

            ALLEGRO_TRANSFORM padrao;
            al_identity_transform(&padrao);
            al_use_transform(&padrao);

            // TELA DE GAME OVER (TELA PRETA COM MENSAGEM)
            if (tela_game_over)
            {
                al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgb(0, 0, 0));
                al_draw_text(fonte_dialogo, al_map_rgb(255, 50, 50), LARGURA_TELA / 2 - 250, ALTURA_TELA / 2 - 20, 0, texto_game_over);
                al_flip_display();
                continue;
            }

            if (itens_visiveis && estado_missao != 3 && estado_missao != 8)
            {
                int total_pecas = (peca1_coletada ? 1 : 0) + (peca2_coletada ? 1 : 0);
                char hud_pecas[50];
                sprintf(hud_pecas, "Pecas da maquina: %d/2", total_pecas);
                al_draw_filled_rectangle(20, 20, 310, 62, al_map_rgba(15, 15, 15, 210));
                al_draw_rectangle(20, 20, 310, 62, al_map_rgb(220, 200, 80), 2.0);
                al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 35, 29, 0, hud_pecas);
            }

            if (timer_aviso_pista > 0)
            {
                timer_aviso_pista--;
                al_draw_filled_rectangle(150, 80, LARGURA_TELA - 150, 135, al_map_rgb(20, 30, 50));
                al_draw_rectangle(150, 80, LARGURA_TELA - 150, 135, al_map_rgb(255, 215, 0), 2.0);
                al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 175, 95, 0, texto_pista_atual);
            }

            if (mentor.interagindo || terminal_interagindo)
            {
                al_draw_filled_rectangle(150, ALTURA_TELA - 170, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(15, 15, 15));
                al_draw_rectangle(150, ALTURA_TELA - 170, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(200, 200, 200), 2.0);

                if (estado_missao == 0)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 135, 0, mentor.falas[mentor.indice_fala]);

                    if (mentor.indice_fala == 3)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 100), 180, ALTURA_TELA - 95, 0, "[1] Aceitar ajudar");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 80, 80), 380, ALTURA_TELA - 95, 0, "[2] Recusar");
                    }
                    else
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                }
                else if (estado_missao == 3)
                {
                    if (modo_digitacao)
                    {
                        if (pergunta_ditadura_atual == 0)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 145, 0, "Pergunta 1: Em que ano comecou o regime militar no Brasil?");
                        }
                        else if (pergunta_ditadura_atual == 1)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 145, 0, "Pergunta 2: Qual Ato Institucional cassou mandatos?");
                        }
                        else if (pergunta_ditadura_atual == 2)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 145, 0, "Pergunta 3: Qual era a justificativa oficial do regime?");
                        }

                        char display_digito[100];
                        sprintf(display_digito, "Sua resposta: %s_", buffer_resposta);
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 255), 180, ALTURA_TELA - 105, 0, display_digito);
                        al_draw_text(fonte_dialogo, al_map_rgb(200, 200, 200), 180, ALTURA_TELA - 65, 0, "[Digite e pressione ENTER para confirmar]");
                    }
                }
                else if (estado_missao == 4)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(200, 200, 200), 180, ALTURA_TELA - 110, 0, "Ok, estou aqui caso mude de ideia.");
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para fechar]");
                }
                else if (estado_missao == 5)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 110, 0, "Maravilha! Elas não estão tão longes assim.");
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para começar]");
                }
                else if (estado_missao == 6)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 135, 0, "Que bom que voltou! Está interessado ainda?");
                    al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 100), 180, ALTURA_TELA - 95, 0, "[1] Aceitar ajudar");
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 80, 80), 380, ALTURA_TELA - 95, 0, "[2] Recusar");
                }
                else if (estado_missao == 7)
                {
                    if (mentor.indice_fala == 0)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 135, 0, "Incrível, você achou todas as peças! Deixe-me encaixá-las aqui...");
                    }
                    else if (mentor.indice_fala == 1)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 135, 0, "Sabe... esta não é uma máquina comum. Ela manipula a linha temporal.");
                    }
                    else if (mentor.indice_fala == 2)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 100, 100), 180, ALTURA_TELA - 135, 0, "Precisamos investigar os arquivos de 1964. Preparado? Vamos lá!");
                    }
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                }
            }

            al_flip_display();
        }
    }

    if (mapa_fundo) {
        al_destroy_bitmap(mapa_fundo);
    }

    if (mapa_noite) {
        al_destroy_bitmap(mapa_noite);
    }

    if (fonte_dialogo) {
        al_destroy_font(fonte_dialogo);
    }

    sprites_finalizar();
    al_destroy_timer(timer);
    al_destroy_event_queue(eventos);
    al_destroy_display(janela);
    return 0;
}