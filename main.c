#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <stdbool.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

int jogo();

// Desenha um botão em estilo Pixel Art combinando com a paleta da capa
static void desenhar_botao_pixel(ALLEGRO_FONT *fonte, int x1, int y1, int x2, int y2, const char *texto, bool hover)
{
    int px = 4; // Tamanho do "pixel" da borda

    // Cores da paleta da capa (Roxo sombrio + Vermelho carmesim + Brilho magico)
    ALLEGRO_COLOR cor_sombra = al_map_rgb(10, 5, 15);
    ALLEGRO_COLOR cor_fundo  = hover ? al_map_rgb(85, 22, 48) : al_map_rgb(26, 14, 36);
    ALLEGRO_COLOR cor_borda  = hover ? al_map_rgb(255, 85, 75) : al_map_rgb(185, 35, 45);
    ALLEGRO_COLOR cor_brilho = hover ? al_map_rgb(255, 190, 110) : al_map_rgb(110, 35, 75);
    ALLEGRO_COLOR cor_texto  = hover ? al_map_rgb(255, 240, 180) : al_map_rgb(240, 235, 245);

    // 1. Sombra deslocada em pixel art
    al_draw_filled_rectangle(x1 + px * 2, y1 + px * 2, x2 + px * 2, y2 + px * 2, cor_sombra);

    // 2. Corpo principal do botão
    al_draw_filled_rectangle(x1, y1, x2, y2, cor_fundo);

    // 3. Borda externa pixelada (com cantos chanfrados estilo 16/32-bits)
    al_draw_filled_rectangle(x1 + px, y1, x2 - px, y1 + px, cor_borda);             // Topo
    al_draw_filled_rectangle(x1 + px, y2 - px, x2 - px, y2, cor_borda);             // Base
    al_draw_filled_rectangle(x1, y1 + px, x1 + px, y2 - px, cor_borda);             // Esquerda
    al_draw_filled_rectangle(x2 - px, y1 + px, x2, y2 - px, cor_borda);             // Direita

    // 4. Linha interna de destaque superior/esquerda (efeito relevo pixelado)
    al_draw_filled_rectangle(x1 + px * 2, y1 + px, x2 - px * 2, y1 + px * 2, cor_brilho);
    al_draw_filled_rectangle(x1 + px, y1 + px * 2, x1 + px * 2, y2 - px * 2, cor_brilho);

    // 5. Texto centralizado com sombra pixelada
    int cx = (x1 + x2) / 2;
    int cy = (y1 + y2) / 2 - 20;

    char texto_exibido[64];
    if (hover)
        sprintf(texto_exibido, "> %s <", texto);
    else
        sprintf(texto_exibido, "%s", texto);

    al_draw_text(fonte, cor_sombra, cx + 3, cy + 3, ALLEGRO_ALIGN_CENTER, texto_exibido);
    al_draw_text(fonte, cor_texto, cx, cy, ALLEGRO_ALIGN_CENTER, texto_exibido);
}

int main()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();

    int largura_tela = 1280;
    int altura_tela = 960;

    // Botões alinhados na direita, logo abaixo do título "FRAGMENTATION OF DICTATORSHIP"
    int largura_botao = 360;
    int altura_botao = 76;
    int espacamento = 96;

    int x1 = 765;
    int y1 = 445;
    int x2 = x1 + largura_botao;
    int y2 = y1 + altura_botao;

    int estado_menu = 0;
    int mouse_atual_x = 0;
    int mouse_atual_y = 0;

    al_set_new_display_flags(ALLEGRO_WINDOWED);

    ALLEGRO_DISPLAY *janela = al_create_display(largura_tela, altura_tela);
    if (!janela)
    {
        printf("Erro critico: nao foi possivel criar a janela!\n");
        return -1;
    }

    ALLEGRO_FONT *fonte = al_load_ttf_font("Arial.ttf", 34, 0);
    ALLEGRO_FONT *fonte_titulo = al_load_ttf_font("Arial.ttf", 46, 0);
    ALLEGRO_FONT *fonte_texto = al_load_ttf_font("Arial.ttf", 26, 0);
    ALLEGRO_BITMAP *background = al_load_bitmap("fundo.png");
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);

    if (!fonte || !fonte_titulo || !fonte_texto)
    {
        printf("Erro critico: Nao foi possivel carregar a fonte Arial.ttf!\n");
        return -1;
    }

    if (!background)
    {
        printf("Aviso: 'fundo.png' nao encontrado na raiz. O menu rodara com fundo preto.\n");
    }

    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(janela));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    bool rodando_programa = true;
    bool redesenhar = true;

    while (rodando_programa)
    {
        ALLEGRO_EVENT evento;
        al_wait_for_event(event_queue, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            rodando_programa = false;
        }
        else if (evento.type == ALLEGRO_EVENT_TIMER)
        {
            redesenhar = true;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_AXES)
        {
            mouse_atual_x = evento.mouse.x;
            mouse_atual_y = evento.mouse.y;
        }
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            int mouse_x = evento.mouse.x;
            int mouse_y = evento.mouse.y;

            if (estado_menu == 0)
            {
                // Botão 1: JOGAR
                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2)
                {
                    al_destroy_font(fonte);
                    al_destroy_font(fonte_titulo);
                    al_destroy_font(fonte_texto);
                    if (background) al_destroy_bitmap(background);
                    al_destroy_display(janela);
                    al_destroy_event_queue(event_queue);
                    al_destroy_timer(timer);

                    jogo();

                    janela = al_create_display(largura_tela, altura_tela);
                    if (!janela) return -1;
                    fonte = al_load_ttf_font("Arial.ttf", 34, 0);
                    fonte_titulo = al_load_ttf_font("Arial.ttf", 46, 0);
                    fonte_texto = al_load_ttf_font("Arial.ttf", 26, 0);
                    background = al_load_bitmap("fundo.png");
                    timer = al_create_timer(1.0 / 60.0);

                    event_queue = al_create_event_queue();
                    al_register_event_source(event_queue, al_get_mouse_event_source());
                    al_register_event_source(event_queue, al_get_display_event_source(janela));
                    al_register_event_source(event_queue, al_get_timer_event_source(timer));
                    al_start_timer(timer);
                }

                // Botão 2: TUTORIAL
                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 + espacamento && mouse_y <= y2 + espacamento)
                {
                    estado_menu = 1;
                    redesenhar = true;
                }

                // Botão 3: SAIR
                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 + espacamento * 2 && mouse_y <= y2 + espacamento * 2)
                {
                    rodando_programa = false;
                }
            }
            else if (estado_menu == 1)
            {
                // Botão Voltar na tela de Tutorial
                if (mouse_x >= 60 && mouse_x <= 260 && mouse_y >= 60 && mouse_y <= 125)
                {
                    estado_menu = 0;
                    redesenhar = true;
                }
            }
        }

        if (redesenhar && al_is_event_queue_empty(event_queue))
        {
            redesenhar = false;

            if (estado_menu == 0)
            {
                if (background)
                {
                    // Escala a imagem 1672x941 para caber 100% na janela 1280x960 sem cortar o título
                    al_draw_scaled_bitmap(
                        background, 0, 0,
                        al_get_bitmap_width(background),
                        al_get_bitmap_height(background),
                        0, 0, largura_tela, altura_tela, 0
                    );
                }
                else
                {
                    al_clear_to_color(al_map_rgb(15, 10, 22));
                }

                bool hover_jogar    = (mouse_atual_x >= x1 && mouse_atual_x <= x2 && mouse_atual_y >= y1 && mouse_atual_y <= y2);
                bool hover_tutorial = (mouse_atual_x >= x1 && mouse_atual_x <= x2 && mouse_atual_y >= y1 + espacamento && mouse_atual_y <= y2 + espacamento);
                bool hover_sair     = (mouse_atual_x >= x1 && mouse_atual_x <= x2 && mouse_atual_y >= y1 + espacamento * 2 && mouse_atual_y <= y2 + espacamento * 2);

                desenhar_botao_pixel(fonte, x1, y1, x2, y2, "PLAY", hover_jogar);
                desenhar_botao_pixel(fonte, x1, y1 + espacamento, x2, y2 + espacamento, "TUTORIAL", hover_tutorial);
                desenhar_botao_pixel(fonte, x1, y1 + espacamento * 2, x2, y2 + espacamento * 2, "SAIR", hover_sair);
            }
            else if (estado_menu == 1)
            {
                al_clear_to_color(al_map_rgb(18, 10, 26));

                // Moldura pixel art da tela de tutorial
                al_draw_rectangle(140, 150, largura_tela - 140, altura_tela - 150, al_map_rgb(185, 35, 45), 4.0);
                al_draw_rectangle(148, 158, largura_tela - 148, altura_tela - 158, al_map_rgb(110, 35, 75), 2.0);

                al_draw_text(fonte_titulo, al_map_rgb(255, 85, 75), largura_tela / 2, 200, ALLEGRO_ALIGN_CENTER, "COMO JOGAR");
                al_draw_text(fonte_texto, al_map_rgb(240, 235, 245), largura_tela / 2, 320, ALLEGRO_ALIGN_CENTER, "SETAS DO TECLADO: Movimentar o personagem");
                al_draw_text(fonte_texto, al_map_rgb(240, 235, 245), largura_tela / 2, 390, ALLEGRO_ALIGN_CENTER, "TECLA E: Conversar, coletar itens e ler pistas");
                al_draw_text(fonte_texto, al_map_rgb(240, 235, 245), largura_tela / 2, 460, ALLEGRO_ALIGN_CENTER, "TECLA P: Pausar o jogo a qualquer momento");
                al_draw_text(fonte_texto, al_map_rgb(255, 210, 120), largura_tela / 2, 550, ALLEGRO_ALIGN_CENTER, "DICA: Use as caixas para se esconder e vista a farda para se infiltrar!");

Use o código com cuidado.
bool hover_voltar = (mouse_atual_x >= 60 && mouse_atual_x <= 260 && mouse_atual_y >= 60 && mouse_atual_y <= 125);
desenhar_botao_pixel(fonte, 60, 60, 260, 125, "VOLTAR", hover_voltar);
}
al_flip_display();
}
}
al_destroy_font(fonte);
al_destroy_font(fonte_titulo);
al_destroy_font(fonte_texto);
if (background) al_destroy_bitmap(background);
al_destroy_display(janela);
al_destroy_event_queue(event_queue);
al_destroy_timer(timer);
return 0;
}