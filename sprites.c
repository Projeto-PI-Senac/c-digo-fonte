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
static ALLEGRO_BITMAP *jornal = NULL;
static ALLEGRO_BITMAP *documento = NULL;

void sprites_inicializar(void)
{
    if (!al_is_image_addon_initialized()) {
        al_init_image_addon();
    }

    personagem = al_load_bitmap("Sprites/personagem.png");
    if (!personagem) personagem = al_load_bitmap("./Sprites/personagem.png");
    if (!personagem) personagem = al_load_bitmap("personagem.png");

    guarda = al_load_bitmap("Sprites/guarda.png");
    if (!guarda) guarda = al_load_bitmap("./Sprites/guarda.png");
    if (!guarda) guarda = al_load_bitmap("guarda.png");

    npc = al_load_bitmap("Sprites/npc_mentor.png");
    if (!npc) npc = al_load_bitmap("./Sprites/npc_mentor.png");
    if (!npc) npc = al_load_bitmap("npc_mentor.png");

     chave = al_load_bitmap("Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("./Sprites/KEY.png");
    if (!chave) chave = al_load_bitmap("Sprites/chave.png");

    jornal = al_load_bitmap("Sprites/jornal.png");
    if (!jornal) jornal = al_load_bitmap("./Sprites/jornal.png");
    if (!jornal) jornal = al_load_bitmap("jornal.png");

    documento = al_load_bitmap("Sprites/documento.png");
    if (!documento) documento = al_load_bitmap("./Sprites/documento.png");
    if (!documento) documento = al_load_bitmap("documento.png");

    if (!personagem) printf("[INFO] Usando bloco colorido para o personagem.\n");
    if (!guarda)     printf("[INFO] Usando bloco colorido para o guarda.\n");
    if (!npc)        printf("[INFO] Usando bloco colorido para o NPC.\n");
    if (!chave)      printf("[INFO] Usando bloco colorido para a chave.\n");
    if (!jornal)     printf("[INFO] Usando bloco colorido para o jornal.\n");
    if (!documento)  printf("[INFO] Usando bloco colorido para o documento.\n");
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
    sprites_desenhar_tamanho(guarda, x, y, TAMANHO_SPRITE, TAMANHO_SPRITE, virado_esquerda, al_map_rgb(200, 40, 40));
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
    if (personagem) { al_destroy_bitmap(personagem); personagem = NULL; }
    if (guarda) { al_destroy_bitmap(guarda); guarda = NULL; }
    if (npc) { al_destroy_bitmap(npc); npc = NULL; }
    if (chave) { al_destroy_bitmap(chave); chave = NULL; }
    if (jornal) { al_destroy_bitmap(jornal); jornal = NULL; }
    if (documento) { al_destroy_bitmap(documento); documento = NULL; }
}