#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define TAMANHO_SPRITE 40

static ALLEGRO_BITMAP *personagem = NULL;
static ALLEGRO_BITMAP *guarda_fallback = NULL;
static ALLEGRO_BITMAP *npc = NULL;
static ALLEGRO_BITMAP *chave = NULL;
static ALLEGRO_BITMAP *jornal = NULL;
static ALLEGRO_BITMAP *documento = NULL;

// Arrays de animação do Soldado Militar
static ALLEGRO_BITMAP *guarda_andar_dir[4]    = {NULL};
static ALLEGRO_BITMAP *guarda_andar_esq[4]    = {NULL};
static ALLEGRO_BITMAP *guarda_andar_frente[5] = {NULL};
static ALLEGRO_BITMAP *guarda_andar_tras[5]   = {NULL};
static ALLEGRO_BITMAP *guarda_alerta[6]       = {NULL};
static ALLEGRO_BITMAP *guarda_parado_frente[5]= {NULL};
static ALLEGRO_BITMAP *guarda_parado_tras[5]  = {NULL};
static ALLEGRO_BITMAP *guarda_parado_dir[4]   = {NULL};
static ALLEGRO_BITMAP *guarda_parado_esq[5]   = {NULL};

static int contador_anim_guarda = 0;

// Função auxiliar para carregar sequências de frames numerados
static void carregar_sequencia(ALLEGRO_BITMAP **lista, int qtd, const char *padrao, int inicio_indice)
{
    char caminho[256];
    for (int i = 0; i < qtd; i++)
    {
        sprintf(caminho, padrao, i + inicio_indice);
        lista[i] = al_load_bitmap(caminho);
    }
}

static void destruir_sequencia(ALLEGRO_BITMAP **lista, int qtd)
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

    personagem = al_load_bitmap("Sprites/personagem.png");
    if (!personagem) personagem = al_load_bitmap("./Sprites/personagem.png");

    guarda_fallback = al_load_bitmap("Sprites/guarda.png");

    npc = al_load_bitmap("Sprites/npc_mentor.png");
    if (!npc) npc = al_load_bitmap("./Sprites/npc_mentor.png");

    chave = al_load_bitmap("Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("./Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("Sprites/chave.png");

    jornal = al_load_bitmap("Sprites/jornal.png");
    documento = al_load_bitmap("Sprites/documento.png");

    // Carrega todas as pastas de animação do Soldado Militar
    carregar_sequencia(guarda_andar_dir,    4, "Sprites/Soldado militar/AndandoPraDireita/AndandoDir_%d.png", 0);
    carregar_sequencia(guarda_andar_esq,    4, "Sprites/Soldado militar/AndandoPraEsquerda/AndandoEsq_%d.png", 0);
    carregar_sequencia(guarda_andar_frente, 5, "Sprites/Soldado militar/AndandoPraFrente/Andando_frente_%d.png", 0);
    carregar_sequencia(guarda_andar_tras,   5, "Sprites/Soldado militar/AndandoPraTras/Andando_trás_%d.png", 0);
    carregar_sequencia(guarda_alerta,       6, "Sprites/Soldado militar/EstadoDeAlerta/Alerta_%d.png", 0);
    carregar_sequencia(guarda_parado_frente,5, "Sprites/Soldado militar/ParadoDeFrente/parado%d.png", 1);
    carregar_sequencia(guarda_parado_tras,  5, "Sprites/Soldado militar/ParadoDeCostas/parado_trás_%d.png", 0);
    carregar_sequencia(guarda_parado_dir,   4, "Sprites/Soldado militar/ParadoPraDireita/ParadoDir_%d.png", 0);
    carregar_sequencia(guarda_parado_esq,   5, "Sprites/Soldado militar/ParadoPraEsquerda/ParadoEsq_%d.png", 0);

    if (guarda_parado_frente[0] != NULL)
        printf("[OK] Animacoes do Soldado Militar carregadas com sucesso!\n");
    else
        printf("[AVISO] Verifique o caminho da pasta Sprites/Soldado militar.\n");
}

void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem,
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
    sprites_desenhar_tamanho(personagem, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, virado_esquerda, al_map_rgb(50, 150, 255));
}

void sprites_desenhar_guarda(float x, float y, bool virado_esquerda)
{
    int dir = virado_esquerda ? DIR_ESQUERDA : DIR_DIREITA;
    sprites_desenhar_guarda_animado(x, y, dir, false, 0);
}

void sprites_desenhar_guarda_animado(float x, float y, int direcao, bool andando, int alerta)
{
    contador_anim_guarda++;
    int passo_anim = contador_anim_guarda / 8; // Troca o frame a cada 8 ticks (~7.5 FPS de animação)

    ALLEGRO_BITMAP *frame_atual = NULL;

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

    if (frame_atual == NULL)
    {
        frame_atual = guarda_fallback;
    }

    // Desenha um pouco maior (48x48) centralizado no tile 40x40 para o soldado ficar bem visível
    float tamanho_visual = 48.0f;
    float offset = (TAMANHO_SPRITE - tamanho_visual) / 2.0f;
    sprites_desenhar_tamanho(frame_atual, x + offset, y + offset, tamanho_visual, tamanho_visual, false, al_map_rgb(200, 40, 40));
}

void sprites_desenhar_npc(float x, float y)
{
    sprites_desenhar_tamanho(npc, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, false, al_map_rgb(220, 180, 40));
}

void sprites_desenhar_chave(float x, float y)
{
    float tamanho_chave = 24.0f;
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
    if (personagem)      { al_destroy_bitmap(personagem); personagem = NULL; }
    if (guarda_fallback) { al_destroy_bitmap(guarda_fallback); guarda_fallback = NULL; }
    if (npc)             { al_destroy_bitmap(npc); npc = NULL; }
    if (chave)           { al_destroy_bitmap(chave); chave = NULL; }
    if (jornal)          { al_destroy_bitmap(jornal); jornal = NULL; }
    if (documento)       { al_destroy_bitmap(documento); documento = NULL; }

    destruir_sequencia(guarda_andar_dir, 4);
    destruir_sequencia(guarda_andar_esq, 4);
    destruir_sequencia(guarda_andar_frente, 5);
    destruir_sequencia(guarda_andar_tras, 5);
    destruir_sequencia(guarda_alerta, 6);
    destruir_sequencia(guarda_parado_frente, 5);
    destruir_sequencia(guarda_parado_tras, 5);
    destruir_sequencia(guarda_parado_dir, 4);
    destruir_sequencia(guarda_parado_esq, 5);
}
