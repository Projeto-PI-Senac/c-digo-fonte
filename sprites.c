
#include <stdio.h>
#include "sprites.h"
#include <allegro5/allegro_primitives.h>

// Tamanho dos personagens no jogo
#define TAMANHO_SPRITE 40

// Imagens dos personagens e objetos
static ALLEGRO_BITMAP *personagem = NULL;
static ALLEGRO_BITMAP *guarda = NULL;
static ALLEGRO_BITMAP *npc = NULL;
static ALLEGRO_BITMAP *chave = NULL;


// Inicializa e carrega as imagens
void sprites_inicializar(void)
{
    personagem = al_load_bitmap("Sprites/personagem.png");
    guarda = al_load_bitmap("Sprites/guarda.png");
    npc = al_load_bitmap("Sprites/npc_mentor.png");
    chave = al_load_bitmap("Sprites/chave.png");

    if (personagem == NULL)
        printf("Aviso: Sprite do jogador nao encontrado.\n");

    if (guarda == NULL)
        printf("Aviso: Sprite do guarda nao encontrado.\n");

    if (npc == NULL)
        printf("Aviso: Sprite do NPC nao encontrado.\n");

    if (chave == NULL)
        printf("Aviso: Sprite da chave nao encontrado.\n");
}


// Desenha uma imagem ou um retangulo provisório
static void desenhar_sprite(ALLEGRO_BITMAP *imagem,
                            float x, float y,
                            ALLEGRO_COLOR cor)
{
    if (imagem != NULL)
    {
        al_draw_scaled_bitmap(
            imagem,
            0, 0,
            al_get_bitmap_width(imagem),
            al_get_bitmap_height(imagem),
            x, y,
            TAMANHO_SPRITE,
            TAMANHO_SPRITE,
            0
        );
    }
    else
    {
        al_draw_filled_rectangle(
            x, y,
            x + TAMANHO_SPRITE,
            y + TAMANHO_SPRITE,
            cor
        );
    }
}


// Desenha o jogador
void sprites_desenhar_jogador(float x, float y)
{
    desenhar_sprite(
        personagem, x, y,
        al_map_rgb(255, 0, 0)
    );
}


// Desenha o guarda
void sprites_desenhar_guarda(float x, float y)
{
    desenhar_sprite(
        guarda, x, y,
        al_map_rgb(0, 255, 0)
    );
}


// Desenha o NPC
void sprites_desenhar_npc(float x, float y)
{
    desenhar_sprite(
        npc, x, y,
        al_map_rgb(255, 0, 255)
    );
}


// Desenha a chave
void sprites_desenhar_chave(float x, float y)
{
    desenhar_sprite(
        chave, x, y,
        al_map_rgb(255, 255, 0)
    );
}


// Libera a memória das imagens
void sprites_finalizar(void)
{
    if (personagem != NULL)
    {
        al_destroy_bitmap(personagem);
        personagem = NULL;
    }

    if (guarda != NULL)
    {
        al_destroy_bitmap(guarda);
        guarda = NULL;
    }

    if (npc != NULL)
    {
        al_destroy_bitmap(npc);
        npc = NULL;
    }

    if (chave != NULL)
    {
        al_destroy_bitmap(chave);
        chave = NULL;
    }
}
