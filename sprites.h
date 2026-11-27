#ifndef SPRITES_H
#define SPRITES_H

#include <stdbool.h>
#include <allegro5/allegro.h>

// Direções possíveis dos personagens
#define DIR_FRENTE   0
#define DIR_COSTAS   1
#define DIR_ESQUERDA 2
#define DIR_DIREITA  3

// Inicialização e carregamento das imagens
void sprites_inicializar(void);

// Desenho com tamanho personalizado e opção de espelhamento horizontal
void sprites_desenhar_tamanho(ALLEGRO_BITMAP *imagem, float x, float y, float largura, float altura, bool virado_esquerda, ALLEGRO_COLOR cor_fallback);

// Desenho dos personagens e objetos
void sprites_desenhar_jogador(float x, float y, bool virado_esquerda);
void sprites_desenhar_jogador_animado(float x, float y, int direcao, bool andando, bool fardado);
void sprites_desenhar_guarda(float x, float y, bool virado_esquerda);
void sprites_desenhar_guarda_animado(float x, float y, int direcao, bool andando, int alerta);
void sprites_desenhar_npc(float x, float y);
void sprites_desenhar_mentor_animado(float x, float y, int direcao, bool andando);
void sprites_desenhar_figurante_animado(int tipo, float x, float y, int direcao);
void sprites_desenhar_chave(float x, float y);
void sprites_desenhar_jornal(float x, float y);
void sprites_desenhar_documento(float x, float y);

// Liberação da memória
void sprites_finalizar(void);

#endif
