
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


// Desenha uma imagem ou um retangulo provisório com largura e altura personalizadas
void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem,
                              float x, float y,
                              float largura, float altura,
                              ALLEGRO_COLOR cor_fallback)
{
    if (imagem != NULL)
    {
        al_draw_scaled_bitmap(
            imagem,
            0, 0,
            al_get_bitmap_width(imagem),
            al_get_bitmap_height(imagem),
            x, y,
            largura,
            altura,
            0
        );
    }
    else
    {
        al_draw_filled_rectangle(
            x, y,
            x + largura,
            y + altura,
            cor_fallback
        );
    }
}


// Desenha o jogador (40x40)
void sprites_desenhar_jogador(float x, float y)
{
    sprites_desenhar_tamanho(
        personagem, x, y,
        TAMANHO_SPRITE, TAMANHO_SPRITE,
        al_map_rgb(255, 0, 0)
    );
}


// Desenha o guarda (40x40)
void sprites_desenhar_guarda(float x, float y)
{
    sprites_desenhar_tamanho(
        guarda, x, y,
        TAMANHO_SPRITE, TAMANHO_SPRITE,
        al_map_rgb(0, 255, 0)
    );
}


// Desenha o NPC (40x40)
void sprites_desenhar_npc(float x, float y)
{
    sprites_desenhar_tamanho(
        npc, x, y,
        TAMANHO_SPRITE, TAMANHO_SPRITE,
        al_map_rgb(255, 0, 255)
    );
}


// Desenha a chave menor (24x24) e centralizada dentro do bloco 40x40
void sprites_desenhar_chave(float x, float y)
{
    float tamanho_chave = 24.0f;
    float offset = (TAMANHO_SPRITE - tamanho_chave) / 2.0f;

    sprites_desenhar_tamanho(
        chave, x + offset, y + offset,
        tamanho_chave, tamanho_chave,
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
