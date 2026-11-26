#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define TAMANHO_SPRITE 40

static ALLEGRO_BITMAP* personagem = NULL;
static ALLEGRO_BITMAP* guarda_fallback = NULL;
static ALLEGRO_BITMAP* npc_fallback = NULL;
static ALLEGRO_BITMAP* chave = NULL;
static ALLEGRO_BITMAP* jornal = NULL;
static ALLEGRO_BITMAP* documento = NULL;

#define MAX_FRAMES 10
#define MAX_CAMINHO 256

// Arrays de animação do Soldado Militar
static ALLEGRO_BITMAP* guarda_andar_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_andar_esq[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_andar_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_andar_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_alerta[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_parado_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_parado_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_parado_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* guarda_parado_esq[MAX_FRAMES] = { NULL };

// Arrays de animação do Mentor
static ALLEGRO_BITMAP* mentor_andar_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_andar_esq[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_andar_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_andar_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_parado_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_parado_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_parado_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* mentor_parado_esq[MAX_FRAMES] = { NULL };

// Arrays de animação do Protagonista (Skin Normal)
static ALLEGRO_BITMAP* jog_andar_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_andar_esq[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_andar_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_andar_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_parado_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_parado_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_parado_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_parado_esq[MAX_FRAMES] = { NULL };

// Arrays de animação do Protagonista Militar (Skin Fardada)
static ALLEGRO_BITMAP* jog_mil_andar_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_andar_esq[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_andar_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_andar_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_parado_frente[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_parado_tras[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_parado_dir[MAX_FRAMES] = { NULL };
static ALLEGRO_BITMAP* jog_mil_parado_esq[MAX_FRAMES] = { NULL };

static int contador_anim_guarda = 0;
static int contador_anim_mentor = 0;
static int contador_anim_jogador = 0;

// Função auxiliar para carregar sequências de frames numerados
static void carregar_sequencia(ALLEGRO_BITMAP** lista, int qtd, const char* padrao, int inicio_indice)
{
    char caminho[MAX_CAMINHO];
    for (int i = 0; i < qtd; i++)
    {
        sprintf(caminho, padrao, i + inicio_indice);
        lista[i] = al_load_bitmap(caminho);
    }
}


static void destruir_sequencia(ALLEGRO_BITMAP** lista, int qtd)
{
    for (int i = 0; i < qtd; i++)
    {
        if (lista[i] != NULL)
        {
            al_destroy_bitmap(lista[i]);
            lista[i] = NULL;
        }
    }
}

void sprites_inicializar(void)
{
    if (!al_is_image_addon_initialized()) {
        al_init_image_addon();
    }

    personagem = al_load_bitmap("Sprites/Protagonista/ParadoDeFrente/parado_0.png");
    if (!personagem) personagem = al_load_bitmap("Sprites/personagem.png");

    guarda_fallback = al_load_bitmap("Sprites/guarda.png");

    npc_fallback = al_load_bitmap("Sprites/npc_mentor.png");
    if (!npc_fallback) npc_fallback = al_load_bitmap("./Sprites/npc_mentor.png");

    chave = al_load_bitmap("Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("./Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("Sprites/chave.png");

    jornal = al_load_bitmap("Sprites/jornal.png");
    documento = al_load_bitmap("Sprites/documento.png");

    // Soldado Militar
    carregar_sequencia(guarda_andar_dir, 4, "Sprites/Soldado militar/AndandoPraDireita/AndandoDir_%d.png", 0);
    carregar_sequencia(guarda_andar_esq, 4, "Sprites/Soldado militar/AndandoPraEsquerda/AndandoEsq_%d.png", 0);
    carregar_sequencia(guarda_andar_frente, 5, "Sprites/Soldado militar/AndandoPraFrente/Andando_frente_%d.png", 0);
    carregar_sequencia(guarda_andar_tras, 5, "Sprites/Soldado militar/AndandoPraTras/Andando_trás_%d.png", 0);
    carregar_sequencia(guarda_alerta, 6, "Sprites/Soldado militar/EstadoDeAlerta/Alerta_%d.png", 0);
    carregar_sequencia(guarda_parado_frente, 5, "Sprites/Soldado militar/ParadoDeFrente/parado%d.png.png", 1);
    carregar_sequencia(guarda_parado_tras, 5, "Sprites/Soldado militar/ParadoDeCostas/parado_trás_%d.png", 0);
    carregar_sequencia(guarda_parado_dir, 4, "Sprites/Soldado militar/ParadoPraDireita/ParadoDir_%d.png", 0);
    carregar_sequencia(guarda_parado_esq, 5, "Sprites/Soldado militar/ParadoPraEsquerda/ParadoEsq_%d.png", 0);

    // Mentor
    carregar_sequencia(mentor_andar_dir, 8, "Sprites/Mentor/AndandoPraDireita/AndandoDir_%d.png", 0);
    carregar_sequencia(mentor_andar_esq, 8, "Sprites/Mentor/AndandoPraEsquerda/AndandoEsq_%d.png", 0);
    carregar_sequencia(mentor_andar_frente, 8, "Sprites/Mentor/AndandoPraFrente/Andando_frente_%d.png", 0);
    carregar_sequencia(mentor_andar_tras, 6, "Sprites/Mentor/AndandoPraTras/Andando_tras_%d.png", 0);
    carregar_sequencia(mentor_parado_frente, 5, "Sprites/Mentor/ParadoDeFrente/parado_%d.png", 0);
    carregar_sequencia(mentor_parado_tras, 5, "Sprites/Mentor/ParadoDeCostas/parado_tras_%d.png", 0);
    carregar_sequencia(mentor_parado_dir, 5, "Sprites/Mentor/ParadoPraDireita/ParadoDir_%d.png", 0);
    carregar_sequencia(mentor_parado_esq, 5, "Sprites/Mentor/ParadoPraEsquerda/ParadoEsq_%d.png", 0);

    // Protagonista (Skin Normal)
    carregar_sequencia(jog_andar_dir, 4, "Sprites/Protagonista/AndandoPraDireita/AndandoDir_%d.png", 0);
    carregar_sequencia(jog_andar_esq, 4, "Sprites/Protagonista/AndandoPraEsquerda/AndandoEsq_%d.png", 0);
    carregar_sequencia(jog_andar_frente, 5, "Sprites/Protagonista/AndandoPraFrente/Andando_frente_%d.png", 0);
    carregar_sequencia(jog_andar_tras, 4, "Sprites/Protagonista/AndandoPraTras/Andando_tras_%d.png", 0);
    carregar_sequencia(jog_parado_frente, 4, "Sprites/Protagonista/ParadoDeFrente/parado_%d.png", 0);
    carregar_sequencia(jog_parado_tras, 5, "Sprites/Protagonista/ParadoDeCostas/parado_tras_%d.png", 0);
    carregar_sequencia(jog_parado_dir, 2, "Sprites/Protagonista/ParadoPraDireita/ParadoDir_%d.png", 0);
    carregar_sequencia(jog_parado_esq, 2, "Sprites/Protagonista/ParadoPraEsquerda/ParadoEsq_%d.png", 0);

    // Protagonista Militar (Skin Fardada)
    carregar_sequencia(jog_mil_andar_dir, 6, "Sprites/ProtagonistaMilitar/AndandoPraDireita/AndandoDir_%d.png", 0);
    carregar_sequencia(jog_mil_andar_esq, 6, "Sprites/ProtagonistaMilitar/AndandoPraEsquerda/AndandoEsq_%d.png", 0);
    carregar_sequencia(jog_mil_andar_frente, 6, "Sprites/ProtagonistaMilitar/AndandoPraFrente/Andando_frente_%d.png", 0);
    carregar_sequencia(jog_mil_andar_tras, 6, "Sprites/ProtagonistaMilitar/AndandoPraTras/Andando_tras_%d.png", 0);
    carregar_sequencia(jog_mil_parado_frente, 4, "Sprites/ProtagonistaMilitar/ParadoDeFrente/Parado_%d.png", 0);
    carregar_sequencia(jog_mil_parado_tras, 5, "Sprites/ProtagonistaMilitar/ParadoDeCostas/Parado_tras_%d.png", 0);
    carregar_sequencia(jog_mil_parado_dir, 2, "Sprites/ProtagonistaMilitar/ParadoPraDireita/ParadoDir_%d.png", 0);
    carregar_sequencia(jog_mil_parado_esq, 2, "Sprites/ProtagonistaMilitar/ParadoPraEsquerda/ParadoEsq_%d.png", 0);

    if (jog_parado_frente[0] && jog_mil_parado_frente[0])
        printf("[OK] Animacoes do Protagonista (Normal e Militar) carregadas!\n");
}

void sprites_desenhar_tamanho(ALLEGRO_BITMAP* imagem,
    float x, float y,
    float largura, float altura,
    bool virado_esquerda,
    ALLEGRO_COLOR cor_fallback)
{
    if (imagem != NULL)
    {
        int flags = virado_esquerda ? ALLEGRO_FLIP_HORIZONTAL : 0;

        al_draw_scaled_bitmap(
            imagem, 0, 0,
            al_get_bitmap_width(imagem),
            al_get_bitmap_height(imagem),
            x, y, largura, altura, flags
        );
    }
    else
    {
        al_draw_filled_rectangle(x, y, x + largura, y + altura, cor_fallback);
    }
}

void sprites_desenhar_jogador(float x, float y, bool virado_esquerda)
{
    int dir = virado_esquerda ? DIR_ESQUERDA : DIR_DIREITA;
    sprites_desenhar_jogador_animado(x, y, dir, false, false);
}

void sprites_desenhar_jogador_animado(float x, float y, int direcao, bool andando, bool fardado)
{
    contador_anim_jogador++;
    int passo_anim = contador_anim_jogador / 7;

    ALLEGRO_BITMAP* frame_atual = NULL;

    if (fardado)
    {
        if (andando)
        {
            if (direcao == DIR_DIREITA)       frame_atual = jog_mil_andar_dir[passo_anim % 6];
            else if (direcao == DIR_ESQUERDA) frame_atual = jog_mil_andar_esq[passo_anim % 6];
            else if (direcao == DIR_COSTAS)   frame_atual = jog_mil_andar_tras[passo_anim % 6];
            else                              frame_atual = jog_mil_andar_frente[passo_anim % 6];
        }
        else
        {
            if (direcao == DIR_DIREITA)       frame_atual = jog_mil_parado_dir[passo_anim % 2];
            else if (direcao == DIR_ESQUERDA) frame_atual = jog_mil_parado_esq[passo_anim % 2];
            else if (direcao == DIR_COSTAS)   frame_atual = jog_mil_parado_tras[passo_anim % 5];
            else                              frame_atual = jog_mil_parado_frente[passo_anim % 4];
        }
        if (frame_atual == NULL) frame_atual = jog_mil_parado_frente[0];
    }
    else
    {
        if (andando)
        {
            if (direcao == DIR_DIREITA)       frame_atual = jog_andar_dir[passo_anim % 4];
            else if (direcao == DIR_ESQUERDA) frame_atual = jog_andar_esq[passo_anim % 4];

            else if (direcao == DIR_COSTAS)   frame_atual = jog_andar_tras[passo_anim % 4];
            else                              frame_atual = jog_andar_frente[passo_anim % 5];
        }
        else
        {
            if (direcao == DIR_DIREITA)       frame_atual = jog_parado_dir[passo_anim % 2];
            else if (direcao == DIR_ESQUERDA) frame_atual = jog_parado_esq[passo_anim % 2];
            else if (direcao == DIR_COSTAS)   frame_atual = jog_parado_tras[passo_anim % 5];
            else                              frame_atual = jog_parado_frente[passo_anim % 4];
        }
        if (frame_atual == NULL) frame_atual = jog_parado_frente[0];
    }
    if (frame_atual == NULL) frame_atual = personagem;
    float tamanho_visual = 100.0f;
    float offset = (TAMANHO_SPRITE - tamanho_visual) / 2.0f;
    sprites_desenhar_tamanho(frame_atual, x + offset, y + offset, tamanho_visual, tamanho_visual, false, al_map_rgb(50, 150, 255));
}
void sprites_desenhar_guarda(float x, float y, bool virado_esquerda)
{
    int dir = virado_esquerda ? DIR_ESQUERDA : DIR_DIREITA;
    sprites_desenhar_guarda_animado(x, y, dir, false, 0);
}
void sprites_desenhar_guarda_animado(float x, float y, int direcao, bool andando, int alerta)
{
    contador_anim_guarda++;
    int passo_anim = contador_anim_guarda / 8;
    ALLEGRO_BITMAP* frame_atual = NULL;
    if (alerta == 1)
    {
        frame_atual = guarda_alerta[passo_anim % 6];
    }
    else if (andando)
    {
        if (direcao == DIR_DIREITA)       frame_atual = guarda_andar_dir[passo_anim % 4];
        else if (direcao == DIR_ESQUERDA) frame_atual = guarda_andar_esq[passo_anim % 4];
        else if (direcao == DIR_COSTAS)   frame_atual = guarda_andar_tras[passo_anim % 5];
        else                              frame_atual = guarda_andar_frente[passo_anim % 5];
    }
    else
    {
        if (direcao == DIR_DIREITA)       frame_atual = guarda_parado_dir[passo_anim % 4];
        else if (direcao == DIR_ESQUERDA) frame_atual = guarda_parado_esq[passo_anim % 5];
        else if (direcao == DIR_COSTAS)   frame_atual = guarda_parado_tras[passo_anim % 5];
        else                              frame_atual = guarda_parado_frente[passo_anim % 5];
    }
    if (frame_atual == NULL) frame_atual = guarda_parado_frente[0];
    if (frame_atual == NULL) frame_atual = guarda_fallback;
    float tamanho_visual = 100.0f; // Mantido no tamanho ideal que você curtiu
    float offset = (TAMANHO_SPRITE - tamanho_visual) / 2.0f;
    sprites_desenhar_tamanho(frame_atual, x + offset, y + offset, tamanho_visual, tamanho_visual, false, al_map_rgb(200, 40, 40));
}
void sprites_desenhar_npc(float x, float y)
{
    sprites_desenhar_tamanho(npc_fallback, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, false, al_map_rgb(220, 180, 40));
}
void sprites_desenhar_mentor_animado(float x, float y, int direcao, bool andando)
{
    contador_anim_mentor++;
    int passo_anim = contador_anim_mentor / 8;
    ALLEGRO_BITMAP* frame_atual = NULL;
    if (andando)
    {
        if (direcao == DIR_DIREITA)       frame_atual = mentor_andar_dir[passo_anim % 8];
        else if (direcao == DIR_ESQUERDA) frame_atual = mentor_andar_esq[passo_anim % 8];
        else if (direcao == DIR_COSTAS)   frame_atual = mentor_andar_tras[passo_anim % 6];
        else                              frame_atual = mentor_andar_frente[passo_anim % 8];
    }
    else
    {
        if (direcao == DIR_DIREITA)       frame_atual = mentor_parado_dir[passo_anim % 5];
        else if (direcao == DIR_ESQUERDA) frame_atual = mentor_parado_esq[passo_anim % 5];
        else if (direcao == DIR_COSTAS)   frame_atual = mentor_parado_tras[passo_anim % 5];
        else                              frame_atual = mentor_parado_frente[passo_anim % 5];
    }
    if (frame_atual == NULL) frame_atual = mentor_parado_frente[0];
    if (frame_atual == NULL) frame_atual = npc_fallback;
    float tamanho_visual = 100.0f; // Mantido no tamanho ideal que você curtiu
    float offset = (TAMANHO_SPRITE - tamanho_visual) / 2.0f;
    sprites_desenhar_tamanho(frame_atual, x + offset, y + offset, tamanho_visual, tamanho_visual, false, al_map_rgb(220, 180, 40));
}
void sprites_desenhar_chave(float x, float y)
{
    float tamanho_chave = 50.0f;
    float offset = (TAMANHO_SPRITE - tamanho_chave) / 2.0f;
    sprites_desenhar_tamanho(chave, x + offset, y + offset, tamanho_chave, tamanho_chave, false, al_map_rgb(255, 215, 0));
}
void sprites_desenhar_jornal(float x, float y)
{
    float tamanho_item = 28.0f;
    float offset = (TAMANHO_SPRITE - tamanho_item) / 2.0f;
    sprites_desenhar_tamanho(jornal, x + offset, y + offset, tamanho_item, tamanho_item, false, al_map_rgb(235, 235, 235));
}
void sprites_desenhar_documento(float x, float y)
{
    float tamanho_item = 26.0f;
    float offset = (TAMANHO_SPRITE - tamanho_item) / 2.0f;
    sprites_desenhar_tamanho(documento, x + offset, y + offset, tamanho_item, tamanho_item, false, al_map_rgb(190, 140, 75));
}
void sprites_finalizar(void)
{
    if (personagem) { al_destroy_bitmap(personagem); personagem = NULL; }
    if (guarda_fallback) { al_destroy_bitmap(guarda_fallback); guarda_fallback = NULL; }
    if (npc_fallback) { al_destroy_bitmap(npc_fallback); npc_fallback = NULL; }
    if (chave) { al_destroy_bitmap(chave); chave = NULL; }
    if (jornal) { al_destroy_bitmap(jornal); jornal = NULL; }
    if (documento) { al_destroy_bitmap(documento); documento = NULL; }
    destruir_sequencia(guarda_andar_dir, 4);
    destruir_sequencia(guarda_andar_esq, 4);
    destruir_sequencia(guarda_andar_frente, 5);
    destruir_sequencia(guarda_andar_tras, 5);
    destruir_sequencia(guarda_alerta, 6);
    destruir_sequencia(guarda_parado_frente, 5);
    destruir_sequencia(guarda_parado_tras, 5);
    destruir_sequencia(guarda_parado_dir, 4);
    destruir_sequencia(guarda_parado_esq, 5);
    destruir_sequencia(mentor_andar_dir, 8);
    destruir_sequencia(mentor_andar_esq, 8);
    destruir_sequencia(mentor_andar_frente, 8);
    destruir_sequencia(mentor_andar_tras, 6);
    destruir_sequencia(mentor_parado_frente, 5);
    destruir_sequencia(mentor_parado_tras, 5);
    destruir_sequencia(mentor_parado_dir, 5);
    destruir_sequencia(mentor_parado_esq, 5);
    destruir_sequencia(jog_andar_dir, 4);
    destruir_sequencia(jog_andar_esq, 4);
    destruir_sequencia(jog_andar_frente, 5);
    destruir_sequencia(jog_andar_tras, 4);
    destruir_sequencia(jog_parado_frente, 4);
    destruir_sequencia(jog_parado_tras, 5);
    destruir_sequencia(jog_parado_dir, 2);
    destruir_sequencia(jog_parado_esq, 2);
    destruir_sequencia(jog_mil_andar_dir, 6);
    destruir_sequencia(jog_mil_andar_esq, 6);
    destruir_sequencia(jog_mil_andar_frente, 6);
    destruir_sequencia(jog_mil_andar_tras, 6);
    destruir_sequencia(jog_mil_parado_frente, 4);
    destruir_sequencia(jog_mil_parado_tras, 5);
    destruir_sequencia(jog_mil_parado_dir, 2);
    destruir_sequencia(jog_mil_parado_esq, 2);
}