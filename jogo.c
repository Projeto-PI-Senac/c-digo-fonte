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
    char falas[3][220]; 
    int indice_fala;    
    bool interagindo;
} NPC;

int jogo()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();

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

    ALLEGRO_EVENT_QUEUE *eventos = al_create_event_queue();

    al_register_event_source(eventos, al_get_keyboard_event_source());
    al_register_event_source(eventos, al_get_display_event_source(janela));
    al_register_event_source(eventos, al_get_mouse_event_source());

    int jogador_linha = 12; 
    int jogador_coluna = 20;

    int mapa[LINHAS_MAPA][COLUNAS_MAPA] = {
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        {  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
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

    // DIÁLOGOS DE INFILTRAÇÃO E INTELIGÊNCIA SOBRE A DITADURA
    strcpy(mentor.falas[0], "Atencao na infiltração! Estamos dentro do prédio do regime reunindo provas.");
    strcpy(mentor.falas[1], "Converse com os contatos discretamente e recupere os arquivos confidenciais.");
    strcpy(mentor.falas[2], "Cuidado com os guardas nas esquinas e prepare-se caso seja interrogado.");

    int contador_passo = 0;
    int velocidade_passo = 12;

    while (jogando)
    {
        ALLEGRO_EVENT event;

        while (al_get_next_event(eventos, &event))
        {
            if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                jogando = false;
            }

            if (event.type == ALLEGRO_EVENT_KEY_DOWN)
            {
                if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                {
                    jogando = false;
                }

                if (event.keyboard.keycode == ALLEGRO_KEY_E)
                {
                    if (mentor.ativo)
                    {
                        if (!mentor.interagindo)
                        {
                            mentor.interagindo = true;
                            mentor.indice_fala = 0;
                        }
                        else
                        {
                            mentor.indice_fala++;
                            if (mentor.indice_fala >= 3)
                            {
                                mentor.interagindo = false;
                                mentor.indice_fala = 0;
                            }
                        }
                    }
                }
            }
        }

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
            else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) { coluna--; moveu = true; }
            else if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) { coluna++; moveu = true; }

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

        float dist_npc = sqrt(pow(jogador_x_mundo - mentor.x, 2) + pow(jogador_y_mundo - mentor.y, 2));
        mentor.ativo = (dist_npc < 60.0);

        if (!mentor.ativo && mentor.interagindo)
        {
            mentor.interagindo = false;
            mentor.indice_fala = 0;
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        if (mapa_fundo != NULL)
        {
            al_draw_bitmap(mapa_fundo, 0, 0, 0);
        }

        sprites_desenhar_jogador(jogador_x_mundo, jogador_y_mundo);
        sprites_desenhar_guarda(guarda.x, guarda.y);

        if (guarda.alerta == 1)
        {
            float centro_x = guarda.x + (TAMANHO_PIXEL / 2.0f);
            float topo_y = guarda.y - 32.0f;
            al_draw_filled_rectangle(centro_x - 4, topo_y, centro_x + 4, topo_y + 22, al_map_rgb(255, 255, 0));
            al_draw_filled_rectangle(centro_x - 4, topo_y + 26, centro_x + 4, topo_y + 34, al_map_rgb(255, 255, 0));
        }

        sprites_desenhar_npc(mentor.x, mentor.y);

        if (mentor.ativo && !mentor.interagindo)
        {
            al_draw_filled_circle(mentor.x + 20, mentor.y - 15, 14, al_map_rgb(240, 240, 240));
            al_draw_text(fonte_dialogo, al_map_rgb(20, 20, 20), mentor.x + 13, mentor.y - 27, 0, "E");
        }

        if (mentor.interagindo)
        {
            al_draw_filled_rectangle(150, ALTURA_TELA - 140, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(15, 15, 15));
            al_draw_rectangle(150, ALTURA_TELA - 140, LARGURA_TELA - 150, ALTURA_TELA - 40, al_map_rgb(200, 200, 200), 2.0);

            al_draw_text(fonte_dialogo, al_map_rgb(255, 255, 255), 180, ALTURA_TELA - 110, 0, mentor.falas[mentor.indice_fala]);
            al_draw_text(fonte_dialogo, al_map_rgb(220, 200, 80), LARGURA_TELA - 450, ALTURA_TELA - 75, 0, "[Pressione E para continuar]");
        }

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

        al_flip_display();
    } 

    if (mapa_fundo) {
        al_destroy_bitmap(mapa_fundo);
    }
    if (fonte_dialogo) {
        al_destroy_font(fonte_dialogo);
    }

    sprites_finalizar();
    al_destroy_event_queue(eventos);
    al_destroy_display(janela);
    return 0;
}