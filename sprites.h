
#ifndef SPRITES_H
#define SPRITES_H

#include <allegro5/allegro.h>

// Inicialização e carregamento das imagens
void sprites_inicializar(void);

// Desenho dos personagens e objetos
void sprites_desenhar_jogador(float x, float y);
void sprites_desenhar_guarda(float x, float y);
void sprites_desenhar_npc(float x, float y);
void sprites_desenhar_chave(float x, float y);

// Liberação da memória
void sprites_finalizar(void);

#endif