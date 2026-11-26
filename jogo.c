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
    char texto_pista[550];
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
        printf("Erro crítico: não foi possível criar a janela do jogo!\n");
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

    // Coordenadas exatas mantidas conforme solicitado
    bool farda_coletada = false;
    int farda_linha = 8, farda_coluna = 38;

    PistaMapa pistas[2];
    pistas[0].linha = 16; pistas[0].coluna = 23; pistas[0].coletado = false;
    strcpy(pistas[0].texto_pista, "ARQUIVO HISTORICO - 1964:\nEm março de 1964, um golpe civil-militar depôs o\npresidente João Goulart. O regime instituiu leis de\nexceção, cassou direitos políticos e endureceu o\ncontrole estatal sob a justificativa de segurança nacional.");

    pistas[1].linha = 21; pistas[1].coluna = 1; pistas[1].coletado = false;
    strcpy(pistas[1].texto_pista, "DOCUMENTO OFICIAL - O AI-5:\nO Ato Institucional nº 5 (1968) marcou o período mais\nviolento da ditadura. Fechou o Congresso Nacional,\ncassou mandatos, instaurou censura prévia à imprensa\ne suspendeu o habeas corpus para crimes políticos.");

    bool lendo_pista_tela = false;
    char texto_pista_lendo[500] = "";

    bool chave_ditadura_coletada = false;
    int chave_linha = 16, chave_coluna = 35;

    bool modo_digitacao = false;
    char buffer_resposta[64] = "";
    int pergunta_ditadura_atual = 0; // Vai de 0 a 4 (total de 5 perguntas)
    bool quiz_ditadura_concluido = false;
    bool terminal_interagindo = false;
    int terminal_estado_dialogo = 0;

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
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
        { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
    };

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

    strcpy(mentor.falas[0], "Olá, jovem! Que bom te ver por aqui nesta praça.");
    strcpy(mentor.falas[1], "Estive planejando uma máquina revolucionária ultimamente...");
    strcpy(mentor.falas[2], "Mas acabei perdendo algumas peças essenciais dela enquanto passeava por aí.");
    strcpy(mentor.falas[3], "Você poderia me dar uma mão para encontrá-las?");

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
                    terminal_estado_dialogo = 0;
                    pergunta_ditadura_atual = 0;
                    jogador_linha = 9;
                    jogador_coluna = 1;
                }
                continue;
            }

            ALLEGRO_KEYBOARD_STATE key_state;
            al_get_keyboard_state(&key_state);

            if (!modo_digitacao && !lendo_pista_tela)
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

                            if (itens_visiveis && estado_missao != 3 && estado_missao != 8)
                            {
                                if (!peca1_coletada && jogador_linha == peca1_linha && jogador_coluna == peca1_coluna)
                                {
                                    peca1_coletada = true;
                                    lendo_pista_tela = true;
                                    strcpy(texto_pista_lendo, "COMPONENTE ENCONTRADO:\nVocê coletou a primeira engrenagem mecânica da máquina do tempo.");
                                }
                                if (!peca2_coletada && jogador_linha == peca2_linha && jogador_coluna == peca2_coluna)
                                {
                                    peca2_coletada = true;
                                    lendo_pista_tela = true;
                                    strcpy(texto_pista_lendo, "COMPONENTE ENCONTRADO:\nVocê coletou o segundo componente essencial para o funcionamento do aparato.");
                                }
                            }
                        }
                    }
                }
            }

            int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
            int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

            if (estado_missao == 8 && !documento_final_coletado)
            {
                if (jogador_linha == doc_final_linha && jogador_coluna == doc_final_coluna)
                {
                    documento_final_coletado = true;
                    lendo_pista_tela = true;
                    strcpy(texto_pista_lendo, "MISSAO CONCLUIDA!\nVocê encontrou o documento secreto final guardado no coração do regime militar.");
                }
            }

            float dist_npc = sqrt(pow(jogador_x_mundo - mentor.x, 2) + pow(jogador_y_mundo - mentor.y, 2));
            mentor.ativo = (dist_npc < 60.0);

            float term_x = terminal_coluna * TAMANHO_PIXEL;
            float term_y = terminal_linha * TAMANHO_PIXEL;
            float dist_term = sqrt(pow(jogador_x_mundo - term_x, 2) + pow(jogador_y_mundo - term_y, 2));
            terminal_ativo = (dist_term < 60.0);

            if (!mentor.ativo && mentor.interagindo && estado_missao != 2 && estado_missao != 4 && estado_missao != 5 && estado_missao != 6 && estado_missao != 7)
            {
                mentor.interagindo = false;
                mentor.indice_fala = 0;
            }

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
                            int linha_futura = (int)(novo_y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                            if (coluna_futura >= 0 && coluna_futura < COLUNAS_MAPA && linha_futura >= 0 && linha_futura < LINHAS_MAPA) {
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

            if (lendo_pista_tela)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_E)
                {
                    lendo_pista_tela = false;
                }
            }
            else if (modo_digitacao)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_BACKSPACE)
                {
                    int len = strlen(buffer_resposta);
                    if (len > 0) buffer_resposta[len - 1] = '\0';
                }
                else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER)
                {
                    // Validação das 5 perguntas educacionais
                    if (pergunta_ditadura_atual == 0)
                    {
                        if (strcmp(buffer_resposta, "1964") == 0)
                        {
                            pergunta_ditadura_atual = 1;
                            strcpy(buffer_resposta, "");
                        }
                        else
                        {
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! O regime não tolera erros.");
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
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Você foi descoberto.");
                        }
                    }
                    else if (pergunta_ditadura_atual == 2)
                    {
                        if (strcmp(buffer_resposta, "COMUNISMO") == 0 || strcmp(buffer_resposta, "SUBVERSAO") == 0 || strcmp(buffer_resposta, "comunismo") == 0 || strcmp(buffer_resposta, "subversao") == 0)
                        {
                            pergunta_ditadura_atual = 3;
                            strcpy(buffer_resposta, "");
                        }
                        else
                        {
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Você foi descoberto.");
                        }
                    }
                    else if (pergunta_ditadura_atual == 3)
                    {
                        // Pergunta 4 nova: Culpado pela cassação de mandatos e fechamento do congresso (AI-5 / CENSURA / CONGRESSO)
                        if (strcmp(buffer_resposta, "CENSURA") == 0 || strcmp(buffer_resposta, "censura") == 0)
                        {
                            pergunta_ditadura_atual = 4;
                            strcpy(buffer_resposta, "");
                        }
                        else
                        {
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta! Falha nos protocolos de censura.");
                        }
                    }
                    else if (pergunta_ditadura_atual == 4)
                    {
                        // Pergunta 5 nova: Presidente deposto em 1964 (JOAO GOULART / GOULART / JANGO)
                        if (strcmp(buffer_resposta, "JOAO GOULART") == 0 || strcmp(buffer_resposta, "GOULART") == 0 || strcmp(buffer_resposta, "JANGO") == 0 ||
                            strcmp(buffer_resposta, "joao goulart") == 0 || strcmp(buffer_resposta, "goulart") == 0 || strcmp(buffer_resposta, "jango") == 0)
                        {
                            quiz_ditadura_concluido = true;
                            modo_digitacao = false;
                            terminal_estado_dialogo = 5; 
                        }
                        else
                        {
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Resposta incorreta sobre a liderança depuesta!");
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
                    else if (event.keyboard.keycode == ALLEGRO_KEY_SPACE)
                    {
                        letra = ' ';
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
                int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
                int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

                if (estado_missao == 3 && !farda_coletada)
                {
                    float dist_farda = sqrt(pow(jogador_x_mundo - (farda_coluna * TAMANHO_PIXEL), 2) + pow(jogador_y_mundo - (farda_linha * TAMANHO_PIXEL), 2));
                    if (dist_farda < 60.0f)
                    {
                        farda_coletada = true;
                        jogador_fardado = true;
                        lendo_pista_tela = true;
                        strcpy(texto_pista_lendo, "DISFARCE OBTIDO - UNIFORME MILITAR:\nVocê encontrou o vestuário oficial do batalhão. Vestir esta farda\né essencial para despistar os sentinelas e circular pela base sem\nser imediatamente preso por suspeita de subversão.");
                        continue;
                    }
                }

                if (estado_missao == 3)
                {
                    bool pista_lida = false;
                    for (int i = 0; i < 2; i++)
                    {
                        if (!pistas[i].coletado)
                        {
                            float dist_pista = sqrt(pow(jogador_x_mundo - (pistas[i].coluna * TAMANHO_PIXEL), 2) + pow(jogador_y_mundo - (pistas[i].linha * TAMANHO_PIXEL), 2));
                            if (dist_pista < 60.0f)
                            {
                                pistas[i].coletado = true;
                                lendo_pista_tela = true;
                                strcpy(texto_pista_lendo, pistas[i].texto_pista);
                                pista_lida = true;
                                break;
                            }
                        }
                    }
                    if (pista_lida) continue;
                }

                if (estado_missao == 3 && terminal_ativo)
                {
                    if (!terminal_interagindo)
                    {
                        terminal_interagindo = true;
                        if (!jogador_fardado)
                        {
                            terminal_estado_dialogo = 1; 
                        }
                        else if (!quiz_ditadura_concluido && terminal_estado_dialogo == 0)
                        {
                            terminal_estado_dialogo = 2; 
                        }
                    }
                    else
                    {
                        if (terminal_estado_dialogo == 1)
                        {
                            terminal_interagindo = false;
                            terminal_estado_dialogo = 0;
                            tela_game_over = true;
                            timer_game_over = 180;
                            strcpy(texto_game_over, "GAME OVER: Você tentou se infiltrar sem a farda militar apropriada!");
                        }
                        else if (terminal_estado_dialogo == 2)
                        {
                            terminal_estado_dialogo = 3; 
                        }
                        else if (terminal_estado_dialogo == 3)
                        {
                            terminal_estado_dialogo = 4; 
                        }
                        else if (terminal_estado_dialogo == 4)
                        {
                            terminal_estado_dialogo = 0;
                            modo_digitacao = true;
                            strcpy(buffer_resposta, "");
                        }
                        else if (terminal_estado_dialogo == 5)
                        {
                            terminal_estado_dialogo = 6; 
                        }
                        else if (terminal_estado_dialogo == 6)
                        {
                            terminal_estado_dialogo = 7; 
                        }
                        else if (terminal_estado_dialogo == 7)
                        {
                            // AQUI: Após o último aviso do guarda, ao apertar E ele vai direto para o mapa final de forma fluida
                            terminal_interagindo = false;
                            terminal_estado_dialogo = 0;
                            estado_missao = 8;
                            jogador_linha = 7;
                            jogador_coluna = 1;
                        }
                    }
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
                else if (mentor.interagindo && estado_missao == 6)
                {
                    mentor.interagindo = false;
                    mentor.indice_fala = 0;
                }
                else if (mentor.interagindo && estado_missao == 2)
                {
                    mentor.interagindo = false;
                    mentor.indice_fala = 0;
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
                    mentor.interagindo = true; 
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
                if (dist_farda < 60.0f) {
                    al_draw_filled_circle((farda_coluna * TAMANHO_PIXEL) + 20, (farda_linha * TAMANHO_PIXEL) - 15, 14, al_map_rgb(240, 240, 240));
                    al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), (farda_coluna * TAMANHO_PIXEL) + 13, (farda_linha * TAMANHO_PIXEL) - 27, 0, "E");
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
                        if (dist_pista < 60.0f)
                        {
                            al_draw_filled_circle((pistas[i].coluna * TAMANHO_PIXEL) + 20, (pistas[i].linha * TAMANHO_PIXEL) - 15, 14, al_map_rgb(240, 240, 240));
                            al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), (pistas[i].coluna * TAMANHO_PIXEL) + 13, (pistas[i].linha * TAMANHO_PIXEL) - 27, 0, "E");
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

            if (tela_game_over)
            {
                al_draw_filled_rectangle(0, 0, LARGURA_TELA, ALTURA_TELA, al_map_rgb(0, 0, 0));
                al_draw_text(fonte_dialogo, al_map_rgb(255, 50, 50), LARGURA_TELA / 2 - 350, ALTURA_TELA / 2 - 20, 0, texto_game_over);
                al_flip_display();
                continue;
            }

            if (lendo_pista_tela)
            {
                al_draw_filled_rectangle(180, 230, LARGURA_TELA - 180, ALTURA_TELA - 230, al_map_rgba(20, 25, 35, 245));
                al_draw_rectangle(180, 230, LARGURA_TELA - 180, ALTURA_TELA - 230, al_map_rgb(220, 200, 80), 3.0);
                
                char l1[150] = "", l2[150] = "", l3[150] = "", l4[150] = "", l5[150] = "";
                sscanf(texto_pista_lendo, "%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]", l1, l2, l3, l4, l5);

                al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 220, 270, 0, l1);
                al_draw_text(fonte_dialogo, al_map_rgb(240, 240, 240), 220, 310, 0, l2);
                al_draw_text(fonte_dialogo, al_map_rgb(240, 240, 240), 220, 350, 0, l3);
                if (strlen(l4) > 0) al_draw_text(fonte_dialogo, al_map_rgb(240, 240, 240), 220, 390, 0, l4);
                if (strlen(l5) > 0) al_draw_text(fonte_dialogo, al_map_rgb(240, 240, 240), 220, 430, 0, l5);

                al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), LARGURA_TELA - 520, ALTURA_TELA - 285, 0, "[Pressione E para fechar e continuar]");
                al_flip_display();
                continue;
            }

            if (itens_visiveis && estado_missao != 3 && estado_missao != 8)
            {
                int total_pecas = (peca1_coletada ? 1 : 0) + (peca2_coletada ? 1 : 0);
                char hud_pecas[50];
                sprintf(hud_pecas, "Peças da máquina: %d/2", total_pecas);
                al_draw_filled_rectangle(20, 20, 310, 62, al_map_rgba(15, 15, 15, 210));
                al_draw_rectangle(20, 20, 310, 62, al_map_rgb(220, 200, 80), 2.0);
                al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 35, 29, 0, hud_pecas);
            }

            if (mentor.interagindo || terminal_interagindo)
            {
                al_draw_filled_rectangle(150, ALTURA_TELA - 220, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(15, 15, 15));
                al_draw_rectangle(150, ALTURA_TELA - 220, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(200, 200, 200), 2.0);

                if (estado_missao == 0)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, mentor.falas[mentor.indice_fala]);

                    if (mentor.indice_fala == 3)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 100), 180, ALTURA_TELA - 120, 0, "[1] Aceitar ajudar na busca das peças temporais");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 80, 80), 180, ALTURA_TELA - 85, 0, "[2] Recusar e seguir seu próprio caminho");
                    }
                    else
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                }
                else if (estado_missao == 2)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Ainda faltam componentes vitais da máquina espalhados pela praça.");
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Continue explorando com cautela antes de iniciarmos a viagem ao passado.");
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para fechar]");
                }
                else if (estado_missao == 3)
                {
                    if (terminal_estado_dialogo == 1)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 80, 80), 180, ALTURA_TELA - 180, 0, "Alerta de segurança! Você está circulando pela base sem o uniforme regulamentar!");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Isso é estritamente proibido sob as ordens do comando militar.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 2)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 180, 0, "Parado aí! O acesso a este setor restrito exige identificação imediata.");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Não reconheço seu rosto entre as patrulhas regulares.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 3)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 180, 0, "Para garantir que não é um elemento subversivo infiltrado...");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Vou submetê-lo a um rigoroso questionário de segurança nacional.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 4)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 180, 0, "Responda corretamente às 5 questões do comitê.");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Qualquer erro resultará em prisão sumária por atividade subversiva.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 5)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 180, 0, "Excelente. Suas respostas coincidem perfeitamente com os registros.");
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 140, 0, "Você provou total alinhamento com as diretrizes do regime.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 6)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 180, 0, "Acesso autorizado aos arquivos confidenciais do quartel.");
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 140, 0, "Mantenha discrição absoluta durante sua permanência interna.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
                    }
                    else if (terminal_estado_dialogo == 7)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 180, 0, "Portões abertos. Prepare-se para entrar no setor restrito.");
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 140, 0, "Busque o documento secreto final guardado no coração do regime.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para prosseguir]");
                    }
                    else if (modo_digitacao)
                    {
                        if (pergunta_ditadura_atual == 0)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Questão 1 de 5 (Segurança Nacional):");
                            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), 180, ALTURA_TELA - 145, 0, "Em que ano foi instaurado o regime militar no Brasil?");
                        }
                        else if (pergunta_ditadura_atual == 1)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Questão 2 de 5 (Decretos Especiais):");
                            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), 180, ALTURA_TELA - 145, 0, "Qual foi o Ato Institucional considerado o mais rigoroso da ditadura?");
                        }
                        else if (pergunta_ditadura_atual == 2)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Questão 3 de 5 (Doutrina Ideológica):");
                            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), 180, ALTURA_TELA - 145, 0, "Qual era a suposta ameaça interna combatida pelo Estado?");
                        }
                        else if (pergunta_ditadura_atual == 3)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Questão 4 de 5 (Controle da Mídia):");
                            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), 180, ALTURA_TELA - 145, 0, "O que o governo impôs aos jornais para controlar a circulação de notícias?");
                        }
                        else if (pergunta_ditadura_atual == 4)
                        {
                            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Questão 5 de 5 (Contexto Histórico):");
                            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), 180, ALTURA_TELA - 145, 0, "Qual era o sobrenome do presidente deposto em 1964?");
                        }

                        char display_digito[100];
                        sprintf(display_digito, "Sua resposta: %s_", buffer_resposta);
                        al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 255), 180, ALTURA_TELA - 100, 0, display_digito);
                        al_draw_text(fonte_dialogo, al_map_rgb(200, 200, 200), 180, ALTURA_TELA - 65, 0, "[Digite no teclado e pressione ENTER para validar]");
                    }
                }
                else if (estado_missao == 4)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(200, 200, 200), 180, ALTURA_TELA - 150, 0, "Compreendo. Estarei por aqui caso mude de ideia e decida ajudar na missão.");
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para fechar]");
                }
                else if (estado_missao == 5)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 150), 180, ALTURA_TELA - 180, 0, "Maravilha! Sabia que podia contar com a sua ajuda.");
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 140, 0, "As peças perdidas estão escondidas pelos arredores da praça. Vamos lá!");
                    al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para começar]");
                }
                else if (estado_missao == 6)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Que bom que retornou! Repensou nossa proposta de investigação temporal?");
                    al_draw_text(fonte_dialogo, al_map_rgb(0, 255, 100), 180, ALTURA_TELA - 120, 0, "[1] Aceitar ajudar na busca das peças temporais");
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 80, 80), 180, ALTURA_TELA - 85, 0, "[2] Recusar e seguir seu próprio caminho");
                }
                else if (estado_missao == 7)
                {
                    if (mentor.indice_fala == 0)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 180, 0, "Incrível! Você recolheu todos os componentes necessários.");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Deixe-me acoplar as peças na estrutura principal da máquina...");
                    }
                    else if (mentor.indice_fala == 1)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 180, 0, "Sabe... esta tecnologia tem o poder de reescrever pontos críticos da história.");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 230, 120), 180, ALTURA_TELA - 140, 0, "Teremos que nos infiltrar discretamente nos registros de 1964.");
                    }
                    else if (mentor.indice_fala == 2)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 100, 100), 180, ALTURA_TELA - 180, 0, "Lembre-se: qualquer deslize e a patrulha militar o tratará como subversivo.");
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 100, 100), 180, ALTURA_TELA - 140, 0, "Prepare-se para a transição temporal. Vamos entrar no quartel!");
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