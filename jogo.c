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

// Estrutura simples para os figurantes de fundo da praça
typedef struct {
    int linha, coluna;
    float x, y;
} Figurante;

int jogo()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();

    // CONFIGURAÇÃO DO V-SYNC: Desativa o V-Sync forçado do monitor, 
    // deixando o controle de quadros 100% na mão do Timer de 60 FPS.
    al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);

    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA_TELA, ALTURA_TELA);

    sprites_inicializar();

    ALLEGRO_FONT *fonte_dialogo = al_load_ttf_font("arial.ttf", 22, 0);
    if (!fonte_dialogo) {
        fonte_dialogo = al_create_builtin_font(); 
    }

    ALLEGRO_BITMAP *mapa_fundo = al_load_bitmap("mapa.png");
    if (!mapa_fundo) {
        mapa_fundo = al_load_bitmap("Sprites/mapa.png");
    }

     ALLEGRO_BITMAP *mapa_noite = al_load_bitmap("Sprites/Mapa A Noite.png");
    if (!mapa_noite) {
        mapa_noite = al_load_bitmap("./Sprites/Mapa A Noite.png");
    }

    // CRIANDO O TIMER DE 60 FPS FIXOS
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE *eventos = al_create_event_queue();

    al_register_event_source(eventos, al_get_keyboard_event_source());
    al_register_event_source(eventos, al_get_display_event_source(janela));
    al_register_event_source(eventos, al_get_mouse_event_source());
    al_register_event_source(eventos, al_get_timer_event_source(timer));

    al_start_timer(timer);

    int jogador_linha = 12; 
    int jogador_coluna = 20;

    bool jogador_virado_esquerda = false;
    bool guarda_virado_esquerda = false;
    int guarda_direcao = DIR_FRENTE;
    bool guarda_andando = false;

    // CONTROLE DE ESTADOS
    int estado_missao = 0; 
    bool itens_visiveis = false;

    // Peças perdidas da máquina
    bool peca1_coletada = false;
    int peca1_linha = 7, peca1_coluna = 4;

    bool peca2_coletada = false;
    int peca2_linha = 17, peca2_coluna = 34;

     // Chave da Ditadura (aparece no mapa noturno da Fase 3)
    bool chave_ditadura_coletada = false;
    int chave_linha = 16, chave_coluna = 35;

    int timer_aviso_pista = 0;
    char texto_pista_atual[160] = "";

    int mapa[LINHAS_MAPA][COLUNAS_MAPA] = {
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    };

    bool jogando = true;
    bool redesenhar = true; // Flag para controlar o redesenho apenas no tick do timer

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

        // O TIMER DEFINE A TAXA DE ATUALIZAÇÃO E RENDERIZAÇÃO A 60 FPS FIXOS
        if (event.type == ALLEGRO_EVENT_TIMER)
        {
            redesenhar = true;

            // LEITURA CONTÍNUA DO TECLADO DENTRO DO TIMER (Evita travamentos de movimento)
            ALLEGRO_KEYBOARD_STATE key_state;
            al_get_keyboard_state(&key_state);

            contador_passo++;
            if (contador_passo >= velocidade_passo)
            {
                int linha = jogador_linha;
                int coluna = jogador_coluna;
                bool moveu = false;

                if (al_key_down(&key_state, ALLEGRO_KEY_UP)) { linha--; moveu = true; }
                else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN)) { linha++; moveu = true; }
                else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) { coluna--; moveu = true; jogador_virado_esquerda = true; }
                else if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) { coluna++; moveu = true; jogador_virado_esquerda = false; }

                if (moveu && linha >= 0 && linha < LINHAS_MAPA && coluna >= 0 && coluna < COLUNAS_MAPA)
                {
                    if (mapa[linha][coluna] != 1 && !(linha == mentor.linha && coluna == mentor.coluna))
                    {
                        jogador_linha = linha;
                        jogador_coluna = coluna;
                        contador_passo = 0;
                    }
                }
            }

            int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
            int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

            // COLETA DAS PEÇAS SE VISÍVEIS
            if (itens_visiveis)
            {
                if (!peca1_coletada && jogador_linha == peca1_linha && jogador_coluna == peca1_coluna)
                {
                    peca1_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "PECA 1: Engrenagem da maquina coletada!");
                }

                if (!peca2_coletada && jogador_linha == peca2_linha && jogador_coluna == peca2_coluna)
                {
                    peca2_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "PECA 2: Componente misterioso coletado!");
                }
            }

            // COLETA DA CHAVE NO MAPA NOTURNO DA DITADURA
            if (estado_missao == 3 && !chave_ditadura_coletada)
            {
                if (jogador_linha == chave_linha && jogador_coluna == chave_coluna)
                {
                    chave_ditadura_coletada = true;
                    timer_aviso_pista = 240;
                    strcpy(texto_pista_atual, "CHAVE COLETADA: Voce encontrou a chave dos arquivos secretos!");
                }
            }


            float dist_npc = sqrt(pow(jogador_x_mundo - mentor.x, 2) + pow(jogador_y_mundo - mentor.y, 2));
            mentor.ativo = (dist_npc < 60.0);

            if (!mentor.ativo && mentor.interagindo && estado_missao != 4 && estado_missao != 5 && estado_missao != 6 && estado_missao != 7)
            {
                mentor.interagindo = false;
                mentor.indice_fala = 0;
            }

            // LÓGICA DO GUARDA DA DITADURA (SÓ ATIVA NA FASE 3)
            if (estado_missao == 3)
            {
                float dx = jogador_x_mundo - guarda.x;
                float dy = jogador_y_mundo - guarda.y;
                float distancia = sqrt(dx * dx + dy * dy);

                if (guarda.alerta == 0 && distancia <= guarda.visao)
                {
                    guarda.alerta = 1;
                    guarda.tempo_suspeito = 0; 
                }
                else if (guarda.alerta == 1)
                {
                    if (distancia > guarda.visao)
                    {
                        guarda.alerta = 0;
                        guarda.tempo_suspeito = 0;
                    }
                    else
                    {
                        guarda.tempo_suspeito++;
                        if (guarda.tempo_suspeito > 30) 
                        {
                            guarda.alerta = 2; 
                        }
                    }
                }

                if (guarda.alerta == 2 && distancia < 25.0f)
                {
                    jogador_linha = 12;
                    jogador_coluna = 20;
                    guarda.x = guarda.origemX;
                    guarda.y = guarda.origemY;
                    guarda.alerta = 0;
                    guarda.tempo_suspeito = 0;
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
                    if (distancia <= guarda.visao)
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
        // TRATAMENTO DOS CLIQUES DE TECLADO (DIÁLOGOS E ESCOLHAS)
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                jogando = false;
            }

            if (event.keyboard.keycode == ALLEGRO_KEY_E)
            {
                if (mentor.ativo && !mentor.interagindo)
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
                        estado_missao = 3; 
                        mentor.interagindo = false;
                        mentor.indice_fala = 0;
                    }
                }
            }

            if (mentor.interagindo && ((estado_missao == 0 && mentor.indice_fala == 3) || estado_missao == 6))
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

        // RENDERIZAÇÃO EXECUTADA APENAS QUANDO O TIMER SINALIZA E A FILA ESTÁ VAZIA
        if (redesenhar && al_is_event_queue_empty(eventos))
        {
            redesenhar = false;

            int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
            int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

            al_clear_to_color(al_map_rgb(0, 0, 0));



            if (estado_missao == 3 && mapa_noite != NULL)
            {
                al_draw_bitmap(mapa_noite, 0, 0, 0);
            }
            else if (mapa_fundo != NULL)
            {
                al_draw_bitmap(mapa_fundo, 0, 0, 0);
            }

            // Desenha o sprite da chave (KEY.png) no mapa noturno da Ditadura
            if (estado_missao == 3 && !chave_ditadura_coletada)
            {
                sprites_desenhar_chave(chave_coluna * TAMANHO_PIXEL, chave_linha * TAMANHO_PIXEL);
            }


            if (estado_missao != 3)
            {
                for (int i = 0; i < 2; i++)
                {
                    sprites_desenhar_npc(figurantes[i].x, figurantes[i].y);
                }
            }

            if (itens_visiveis)
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
            sprites_desenhar_jogador(jogador_x_mundo, jogador_y_mundo, jogador_virado_esquerda);
            


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
            }

            

            if (itens_visiveis)
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

            if (estado_missao == 3 && guarda.alerta == 1)
            {
                float centro_x = guarda.x + (TAMANHO_PIXEL / 2.0f);
                float topo_y = guarda.y - 32.0f;
                al_draw_filled_rectangle(centro_x - 4, topo_y, centro_x + 4, topo_y + 22, al_map_rgb(255, 255, 0));
                al_draw_filled_rectangle(centro_x - 4, topo_y + 26, centro_x + 4, topo_y + 34, al_map_rgb(255, 255, 0));
            }

            sprites_desenhar_npc(mentor.x, mentor.y);

            if (mentor.ativo && !mentor.interagindo && estado_missao != 3)
            {
                al_draw_filled_circle(mentor.x + 20, mentor.y - 15, 14, al_map_rgb(240, 240, 240));
                al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), mentor.x + 13, mentor.y - 27, 0, "E");
            }

            if (mentor.interagindo)
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
                else if (estado_missao == 2)
                {
                    if (!peca1_coletada || !peca2_coletada)
                    {
                        al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 110, 0, "Ainda faltam pecas da maquina na praça! Continue procurando.");
                        al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para fechar]");
                    }
                }
                else if (estado_missao == 3)
                {
                    al_draw_text(fonte_dialogo, al_map_rgb(255, 100, 100), 180, ALTURA_TELA - 110, 0, "[VIAGEM CONCLUÍDA] Bem-vindo a Ditadura Militar! Cuidado com os guardas!");
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