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

#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define LINHAS_MAPA 20
#define COLUNAS_MAPA 30
#define TAMANHO_PIXEL 40
// Constantes

typedef struct
{
    float x, y;
    float origemX, origemY;
    float velocidade;
    int alerta;          // 0 = Normal/Parado, 1 = Desconfiado/Segunda Checagem, 2 = Perseguindo, 3 = Voltando
    float visao;
    int tempo_suspeito;  // Contador para o tempo de hesitação da segunda checagem
} Inimigo;

typedef struct
{
    float x, y;
    bool ativo;
    char texto[150];
} NPC;

int jogo()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();
    al_install_keyboard();
    // Inicialização

    ALLEGRO_DISPLAY *janela = al_create_display(LARGURA_TELA, ALTURA_TELA);

    sprites_inicializar();

    ALLEGRO_EVENT_QUEUE *eventos = al_create_event_queue();

    al_register_event_source(eventos, al_get_keyboard_event_source());
    al_register_event_source(eventos, al_get_display_event_source(janela));

    int mapa[LINHAS_MAPA][COLUNAS_MAPA];
    int jogador_linha = 1;
    int jogador_coluna = 2;
    // Posição do jogador

    for (int i = 0; i < LINHAS_MAPA; i++)
    {
        for (int j = 0; j < COLUNAS_MAPA; j++)
        {
            if (i == 0 || i == LINHAS_MAPA - 1 || j == 0 || j == COLUNAS_MAPA - 1)
            {
                mapa[i][j] = 1;
            }
            else if ((i == 6 && j < 22) || (i == 13 && j > 8))
            {
                mapa[i][j] = 1;
            }
            else if ((j == 10 && i < 6) || (j == 20 && i > 6 && i < 13))
            {
                mapa[i][j] = 1;
            }
            else
            {
                mapa[i][j] = 0;
            }
        }
    }

    int linha_da_porta = 3;
    int coluna_da_porta = 10;
    mapa[linha_da_porta][coluna_da_porta] = 3;

    mapa[10][20] = 0;
    bool jogando = true;

    bool chave_coletada = false;
    int linha_da_chave = 3;
    int coluna_da_chave = 3;

    Inimigo guarda;

    int linhaInicial = 16;
    int colunaInicial = 25;

    guarda.origemX = colunaInicial * TAMANHO_PIXEL;
    guarda.origemY = linhaInicial * TAMANHO_PIXEL;

    guarda.x = guarda.origemX;
    guarda.y = guarda.origemY;

    guarda.velocidade = 1.5;
    guarda.alerta = 0;
    guarda.visao = 150.0;
    guarda.tempo_suspeito = 0;

    NPC mentor;
    mentor.x = 5 * TAMANHO_PIXEL;
    mentor.y = 5 * TAMANHO_PIXEL;
    mentor.ativo = false;
    strcpy(mentor.texto, "Ola, aluno! Colete a chave para abrir a porta e escapar do guarda!");

    int contador_passo = 0;
    int velocidade_passo = 15;

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
            }
        }

        // Continuar andando ao segurar a tecla
        ALLEGRO_KEYBOARD_STATE key_state;
        al_get_keyboard_state(&key_state);

        contador_passo++;
        if (contador_passo >= velocidade_passo)
        {
            int linha = jogador_linha;
            int coluna = jogador_coluna;
            bool moveu = false;

            if (al_key_down(&key_state, ALLEGRO_KEY_UP))
            {
                linha--;
                moveu = true;
            }
            else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN))
            {
                linha++;
                moveu = true;
            }
            else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT))
            {
                coluna--;
                moveu = true;
            }
            else if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT))
            {
                coluna++;
                moveu = true;
            }

            if (moveu && mapa[linha][coluna] != 1)
            {
                jogador_linha = linha;
                jogador_coluna = coluna;
                contador_passo = 0;
            }
        }
        
        // Se a chave for pega, a porta desaparece
        if (!chave_coletada && jogador_linha == linha_da_chave && jogador_coluna == coluna_da_chave)
        {
            chave_coletada = true;
            mapa[linha_da_porta][coluna_da_porta] = 0;
            printf("Chave coletada e porta aberta!\n");
        }

        int jogador_x_mundo = jogador_coluna * TAMANHO_PIXEL;
        int jogador_y_mundo = jogador_linha * TAMANHO_PIXEL;

        int cam_x = jogador_x_mundo - (LARGURA_TELA / 2);
        int cam_y = jogador_y_mundo - (ALTURA_TELA / 2);

        if (cam_x < 0)
            cam_x = 0;
        if (cam_y < 0)
            cam_y = 0;
        if (cam_x > (COLUNAS_MAPA * TAMANHO_PIXEL) - LARGURA_TELA)
            cam_x = (COLUNAS_MAPA * TAMANHO_PIXEL) - LARGURA_TELA;
        if (cam_y > (LINHAS_MAPA * TAMANHO_PIXEL) - ALTURA_TELA)
            cam_y = (LINHAS_MAPA * TAMANHO_PIXEL) - ALTURA_TELA;

        float dist_npc = sqrt(pow(jogador_x_mundo - mentor.x, 2) + pow(jogador_y_mundo - mentor.y, 2));

        if (dist_npc < 40.0)
        {
            mentor.ativo = true;
        }
        else
        {
            mentor.ativo = false;
        }

        al_clear_to_color(al_map_rgb(0, 0, 0));

        for (int i = 0; i < LINHAS_MAPA; i++)
        {
            for (int j = 0; j < COLUNAS_MAPA; j++)
            {
                int x = (j * TAMANHO_PIXEL) - cam_x;
                int y = (i * TAMANHO_PIXEL) - cam_y;

                if (mapa[i][j] == 1)
                {
                    al_draw_filled_rectangle(x, y, x + TAMANHO_PIXEL, y + TAMANHO_PIXEL, al_map_rgb(0, 0, 255));
                }
                else if (mapa[i][j] == 3)
                {
                    al_draw_filled_rectangle(x, y, x + TAMANHO_PIXEL, y + TAMANHO_PIXEL, al_map_rgb(0, 120, 255));
                }
                else
                {
                    al_draw_filled_rectangle(x, y, x + TAMANHO_PIXEL, y + TAMANHO_PIXEL, al_map_rgb(40, 40, 40));
                    al_draw_rectangle(x, y, x + TAMANHO_PIXEL, y + TAMANHO_PIXEL, al_map_rgb(60, 60, 60), 1.0);
                }
            }
        }

        if (!chave_coletada)
        {
            int chave_x_mundo = coluna_da_chave * TAMANHO_PIXEL;
            int chave_y_mundo = linha_da_chave * TAMANHO_PIXEL;
            int chave_x_tela = chave_x_mundo - cam_x;
            int chave_y_tela = chave_y_mundo - cam_y;

            sprites_desenhar_chave(chave_x_tela, chave_y_tela);
        }

        int player_x_tela = jogador_x_mundo - cam_x;
        int player_y_tela = jogador_y_mundo - cam_y;

        sprites_desenhar_jogador(player_x_tela, player_y_tela);

        int inimigo_x_tela = guarda.x - cam_x;
        int inimigo_y_tela = guarda.y - cam_y;

        // Corpo do guarda
        sprites_desenhar_guarda(inimigo_x_tela, inimigo_y_tela);

        // ponto de exclamação encima do guarda
        if (guarda.alerta == 1)
        {
            float centro_x = inimigo_x_tela + (TAMANHO_PIXEL / 2.0f);
            float topo_y = inimigo_y_tela - 32.0f; // Mais alta ainda em cima da cabeça

            // Corpo largo e alto da exclamação
            al_draw_filled_rectangle(centro_x - 4, topo_y, centro_x + 4, topo_y + 22, al_map_rgb(255, 255, 0));
            // Ponto grande da exclamação embaixo
            al_draw_filled_rectangle(centro_x - 4, topo_y + 26, centro_x + 4, topo_y + 34, al_map_rgb(255, 255, 0));
        }

        int npc_x_tela = mentor.x - cam_x;
        int npc_y_tela = mentor.y - cam_y;
        
        sprites_desenhar_npc(npc_x_tela, npc_y_tela);

       // Se o NPC estiver ativo, desenha a caixa de diálogo
        if (mentor.ativo)
        {
            al_draw_filled_rectangle(50, ALTURA_TELA - 130, LARGURA_TELA - 50, ALTURA_TELA - 20, al_map_rgb(20, 20, 20));
            al_draw_rectangle(50, ALTURA_TELA - 130, LARGURA_TELA - 50, ALTURA_TELA - 20, al_map_rgb(255, 255, 255), 2.0);

            ALLEGRO_TRANSFORM transformacao;
            al_identity_transform(&transformacao);
            al_scale_transform(&transformacao, 2.0, 2.0);
            al_use_transform(&transformacao);

            ALLEGRO_FONT *fonte = al_create_builtin_font();
            float inicio_x = 32.0;
            float inicio_y = (ALTURA_TELA - 115) / 2.0;
            float largura_maxima = 330.0; 

            char linha_atual[100] = "";
            char palavra[50];
            int p = 0;
            int y_offset = 0;

            for (int i = 0; i <= strlen(mentor.texto); i++)
            {
                if (mentor.texto[i] == ' ' || mentor.texto[i] == '\0')
                {
                    palavra[p] = '\0';
                    p = 0;

                    char teste[150];
                    strcpy(teste, linha_atual);
                    if (strlen(linha_atual) > 0) strcat(teste, " ");
                    strcat(teste, palavra);

                    if (al_get_text_width(fonte, teste) > largura_maxima)
                    {
                        al_draw_text(fonte, al_map_rgb(255, 255, 255), inicio_x, inicio_y + y_offset, 0, linha_atual);
                        y_offset += 12;
                        strcpy(linha_atual, palavra);
                    }
                    else
                    {
                        strcpy(linha_atual, teste);
                    }
                }
                else
                {
                    if (p < 49) palavra[p++] = mentor.texto[i];
                }
            }
            if (strlen(linha_atual) > 0)
            {
                al_draw_text(fonte, al_map_rgb(255, 255, 255), inicio_x, inicio_y + y_offset, 0, linha_atual);
            }

            al_identity_transform(&transformacao);
            al_use_transform(&transformacao);
        }

        float dx = jogador_x_mundo - guarda.x;
        float dy = jogador_y_mundo - guarda.y;
        float distancia = sqrt(dx * dx + dy * dy);

        // LÓGICA DA SEGUNDA CHANCE E DA CHECAGEM:
        if (guarda.alerta == 0 && distancia <= guarda.visao)
        {
            // O guarda desconfiou pela primeira vez
            guarda.alerta = 1;
            guarda.tempo_suspeito = 0; 
            printf("O guarda desconfiou de algo...\n");
        }
        else if (guarda.alerta == 1)
        {
            // SEGUNDA CHANCE: Se o jogador fugir do campo de visão durante a desconfiança, o guarda desiste!
            if (distancia > guarda.visao)
            {
                guarda.alerta = 0;
                guarda.tempo_suspeito = 0;
                printf("Ufa! O guarda perdeu você de vista a tempo e voltou ao normal.\n");
            }
            else
            {
                // Se continuar na mira, o contador avança
                guarda.tempo_suspeito++;
                // 80 frames de janela de escape (o dobro do anterior, dando bastante tempo para reagir)
                if (guarda.tempo_suspeito > 80) 
                {
                    guarda.alerta = 2; // Acabou a chance: vira perseguição real!
                    printf("O guarda te pegou no flagra e começou a correr!\n");
                }
            }
        }

        // Se o jogador for pego na perseguição real, reinicia
        if (guarda.alerta == 2 && distancia < 20.0f)
        {
            printf("O guarda te alcançou! Recomeçando...\n");

            jogador_linha = 1;
            jogador_coluna = 2;

            guarda.x = guarda.origemX;
            guarda.y = guarda.origemY;
            guarda.alerta = 0;
            guarda.tempo_suspeito = 0;
        }

        // Estados do Guarda
        if (guarda.alerta == 2)
        {
            // ESTADO 2: Perseguindo o Jogador
            float distancia_da_origem = sqrt(pow(guarda.x - guarda.origemX, 2) + pow(guarda.y - guarda.origemY, 2));

            // Se o jogador fugiu da perseguição real, o guarda desiste e volta (Estado 3)
            if (distancia > 350.0 || distancia_da_origem > 400.0)
            {
                guarda.alerta = 3;
                printf("O guarda perdeu você de vista e está voltando...\n");
            }
            else if (distancia > 1.0)
            {
                // Eixo X
                float novo_x = guarda.x + (dx / distancia) * guarda.velocidade;
                int coluna_futura = (int)(novo_x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                int linha_atual = (int)(guarda.y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                if (mapa[linha_atual][coluna_futura] != 1)
                {
                    guarda.x = novo_x;
                }

                // Eixo Y
                float novo_y = guarda.y + (dy / distancia) * guarda.velocidade;
                int coluna_atual = (int)(guarda.x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                int linha_futura = (int)(novo_y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                if (mapa[linha_futura][coluna_atual] != 1)
                {
                    guarda.y = novo_y;
                }
            }
        }
        else if (guarda.alerta == 3)
        {
            // ESTADO 3: VOLTANDO PARA A ORIGEM ANDANDO

            // Se ele te ver no caminho de volta, entra em alerta de novo
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

                // Se chegou perto o suficiente da origem, reseta para o estado normal
                if (dist_origem <= 2.0)
                {
                    guarda.x = guarda.origemX;
                    guarda.y = guarda.origemY;
                    guarda.alerta = 0;
                    guarda.tempo_suspeito = 0;
                }
                else
                {
                    // Anda de volta com colisão nas paredes
                    float velocidade_volta = guarda.velocidade * 0.8f;

                    float novo_x = guarda.x + (dx_origem / dist_origem) * velocidade_volta;
                    int coluna_futura = (int)(novo_x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                    int linha_atual = (int)(guarda.y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                    if (mapa[linha_atual][coluna_futura] != 1)
                    {
                        guarda.x = novo_x;
                    }

                    float novo_y = guarda.y + (dy_origem / dist_origem) * velocidade_volta;
                    int coluna_atual = (int)(guarda.x + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;
                    int linha_futura = (int)(novo_y + TAMANHO_PIXEL / 2) / TAMANHO_PIXEL;

                    if (mapa[linha_futura][coluna_atual] != 1)
                    {
                        guarda.y = novo_y;
                    }
                }
            }
        }

        al_flip_display();
    } // Fim do while(jogando)

    sprites_finalizar();

    al_destroy_event_queue(eventos);
    al_destroy_display(janela);
    return 0;
}