#ifndef SPRITES_H
#define SPRITES_H

#include <stdbool.h>
#include <allegro5/allegro.h>

// Inicialização e carregamento das imagens
void sprites_inicializar(void);

// Desenho com tamanho personalizado e opção de espelhamento horizontal
void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem, float x, float y, float largura, float altura, bool virado_esquerda, ALLEGRO_COLOR cor_fallback);

// Desenho dos personagens (com direção) e objetos
void sprites_desenhar_jogador(float x, float y, bool virado_esquerda);
void sprites_desenhar_guarda(float x, float y, bool virado_esquerda);
void sprites_desenhar_npc(float x, float y);
void sprites_desenhar_chave(float x, float y);
void sprites_desenhar_jornal(float x, float y);
void sprites_desenhar_documento(float x, float y);

// Liberação da memória
void sprites_finalizar(void);

#endif