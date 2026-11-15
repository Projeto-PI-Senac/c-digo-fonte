#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprites.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define TAMANHO_SPRITE 40

static ALLEGRO_BITMAP *personagem = NULL;
static ALLEGRO_BITMAP *guarda = NULL;
static ALLEGRO_BITMAP *npc = NULL;
static ALLEGRO_BITMAP *chave = NULL;

void sprites_inicializar(void)
{
    if (!al_is_image_addon_initialized()) {
        al_init_image_addon();
    }

    // Tenta carregar testando o caminho local e variações comuns
    personagem = al_load_bitmap("Sprites/personagem.png");
    if (!personagem) personagem = al_load_bitmap("./Sprites/personagem.png");
    if (!personagem) personagem = al_load_bitmap("personagem.png");

    guarda = al_load_bitmap("Sprites/guarda.png");
    if (!guarda) guarda = al_load_bitmap("./Sprites/guarda.png");
    if (!guarda) guarda = al_load_bitmap("guarda.png");

    npc = al_load_bitmap("Sprites/npc_mentor.png");
    if (!npc) npc = al_load_bitmap("./Sprites/npc_mentor.png");
    if (!npc) npc = al_load_bitmap("npc_mentor.png");

    chave = al_load_bitmap("Sprites/chave.png");
    if (!chave) chave = al_load_bitmap("./Sprites/chave.png");
    if (!chave) chave = al_load_bitmap("chave.png");

    // Se ainda assim falhar, avisa mas NÃO trava o jogo (usa fallback colorido)
    if (!personagem) printf("[INFO] Usando bloco colorido para o personagem.\n");
    if (!guarda)     printf("[INFO] Usando bloco colorido para o guarda.\n");
    if (!npc)        printf("[INFO] Usando bloco colorido para o NPC.\n");
    if (!chave)      printf("[INFO] Usando bloco colorido para a chave.\n");
}

void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem,
                              float x, float y,
                              float largura, float altura,
                              ALLEGRO_COLOR cor_fallback)
{
    if (imagem != NULL)
    {
        al_draw_scaled_bitmap(
            imagem, 0, 0,
            al_get_bitmap_width(imagem),
            al_get_bitmap_height(imagem),
            x, y, largura, altura, 0
        );
    }
    else
    {
        al_draw_filled_rectangle(x, y, x + largura, y + altura, cor_fallback);
    }
}

void sprites_desenhar_jogador(float x, float y)
{
    sprites_desenhar_tamanho(personagem, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, al_map_rgb(50, 150, 255));
}

void sprites_desenhar_guarda(float x, float y)
{
    sprites_desenhar_tamanho(guarda, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, al_map_rgb(200, 40, 40));
}

void sprites_desenhar_npc(float x, float y)
{
    sprites_desenhar_tamanho(npc, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, al_map_rgb(220, 180, 40));
}

void sprites_desenhar_chave(float x, float y)
{
    float tamanho_chave = 24.0f;
    float offset = (TAMANHO_SPRITE - tamanho_chave) / 2.0f;
    sprites_desenhar_tamanho(chave, x + offset, y + offset, tamanho_chave, tamanho_chave, al_map_rgb(255, 215, 0));
}

void sprites_finalizar(void)
{
    if (personagem) { al_destroy_bitmap(personagem); personagem = NULL; }
    if (guarda) { al_destroy_bitmap(guarda); guarda = NULL; }
    if (npc) { al_destroy_bitmap(npc); npc = NULL; }
    if (chave) { al_destroy_bitmap(chave); chave = NULL; }
}