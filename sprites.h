#ifndef SPRITES_H
#define SPRITES_H

#include <allegro5/allegro.h>

// Inicialização e carregamento das imagens
void sprites_inicializar(void);

// Desenho com tamanho personalizado
void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem, float x, float y, float largura, float altura, ALLEGRO_COLOR cor_fallback);

// Desenho dos personagens e objetos
void sprites_desenhar_jogador(float x, float y);
void sprites_desenhar_guarda(float x, float y);
void sprites_desenhar_npc(float x, float y);
void sprites_desenhar_chave(float x, float y);

// Liberação da memória
void sprites_finalizar(void);

#endif