#include <stdio.h>                       // guia de linguagem C
#include <allegro5/allegro.h>            // controla as janelas
#include <allegro5/allegro_primitives.h> // formas geométricas báscias
#include <allegro5/allegro_font.h>       // sistema básico de fontes e texto
#include <allegro5/allegro_ttf.h>        // carregar fontes personalizadas
#include <allegro5/allegro_image.h>      // carregar imagens

int main()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();
    // inicialização

    int largura_tela = 1280;
    int altura_tela = 720;
    int largura_botao = 350;
    int altura_botao = 100;
    // tamanho da tela e botão

    int x1 = (largura_tela - largura_botao) / 2;
    int y1 = 500;
    int x2 = x1 + largura_botao;
    int y2 = y1 + altura_botao;
    // posição do retangulo

    ALLEGRO_DISPLAY *janela = al_create_display(1280, 960);     // variavel da janela
    ALLEGRO_FONT *fonte = al_load_ttf_font("Arial.ttf", 50, 0); // variavel da fonte
    ALLEGRO_BITMAP *background = al_load_bitmap("fundo.png");   // varievel de imagem de fundo
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);         // variavel de contador de frame

    ALLEGRO_EVENT_QUEUE *mouse = al_create_event_queue();
    al_register_event_source(mouse, al_get_mouse_event_source());
    al_register_event_source(mouse, al_get_display_event_source(janela));
    al_register_event_source(mouse, al_get_timer_event_source(timer));
    // eventos que envolvem o MOUSE

    al_start_timer(timer);

    bool jogando = true;
    bool desenho = true;

    if (!fonte)
    {
        printf("Erro ao carregar a fonte!\n");
        return -1;
    }

    while (jogando)
    {
        ALLEGRO_EVENT evento;
        al_wait_for_event(mouse, &evento);

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            jogando = false;
        }

        if (evento.type == ALLEGRO_EVENT_TIMER)
        {
            desenho = true;
        }

        if (desenho && al_is_event_queue_empty(mouse))
        {
            desenho = false;

            al_draw_bitmap(background, 0, 0, 0);

            al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(0, 100, 255));
            al_draw_filled_rectangle(x1, y1+120, x2, y2+120, al_map_rgb(0, 100, 255));
            al_draw_filled_rectangle(x1, y1+240, x2, y2+240, al_map_rgb(0, 100, 255));

            int texto_x = x1 + (largura_botao / 2);
            int texto_y = y1 + (altura_botao / 2 - 25);
            al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y, ALLEGRO_ALIGN_CENTER, "PLAY");
            al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y+120, ALLEGRO_ALIGN_CENTER, "TUTORIAL");
            al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y+240, ALLEGRO_ALIGN_CENTER, "SAIR");
        }

        if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            int mouse_x = evento.mouse.x;
            int mouse_y = evento.mouse.y;

            if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2)
            {
                printf("JOGAR!\n");
            }
            // botão jogar

            if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1+120 && mouse_y <= y2+120)
            {
                printf("TUTORIAL!\n");
            }
            // botão tutorial

            if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1+240 && mouse_y <= y2+240)
            {
                printf("SAIR!\n");
            }
            // botão sair
        }

        al_flip_display();
        // mostrar na tela :D
    }

    al_destroy_font(fonte);
    al_destroy_display(janela);
    al_destroy_bitmap(background);
    // limpar armazenamento

    return 0;
}
