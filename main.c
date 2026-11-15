#include <stdio.h>                       
#include <stdbool.h>                     
#include <allegro5/allegro.h>            
#include <allegro5/allegro_primitives.h> 
#include <allegro5/allegro_font.h>       
#include <allegro5/allegro_ttf.h>        
#include <allegro5/allegro_image.h>      

bool jogo();

int main()
{
    al_init();
    al_init_primitives_addon();
    al_init_ttf_addon();
    al_init_image_addon();
    al_install_mouse();

    int largura_tela = 1280;
    int altura_tela = 720;
    int largura_botao = 350;
    int altura_botao = 100;

    int x1 = (largura_tela - largura_botao) / 2;
    int y1 = 500;
    int x2 = x1 + largura_botao;
    int y2 = y1 + altura_botao;

    int estado_menu = 0;

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_PROGRAMMABLE_PIPELINE);

    ALLEGRO_DISPLAY *janela = al_create_display(1280, 960);     
    ALLEGRO_FONT *fonte = al_load_ttf_font("Arial.ttf", 50, 0); 
    ALLEGRO_BITMAP *background = al_load_bitmap("fundo.png");   
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);         

    if (!fonte)
    {
        printf("Erro crítico: Nao foi possivel carregar a fonte Arial.ttf!\n");
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
        else if (evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
            int mouse_x = evento.mouse.x;
            int mouse_y = evento.mouse.y;

            if (estado_menu == 0)
            {
                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 && mouse_y <= y2)
                {
                    al_destroy_font(fonte);
                    if (background) al_destroy_bitmap(background);
                    al_destroy_display(janela);
                    al_destroy_event_queue(event_queue);
                    al_destroy_timer(timer);

                    jogo();

                    janela = al_create_display(1280, 960);
                    fonte = al_load_ttf_font("Arial.ttf", 50, 0);
                    background = al_load_bitmap("fundo.png");
                    timer = al_create_timer(1.0 / 60.0);

                    event_queue = al_create_event_queue();
                    al_register_event_source(event_queue, al_get_mouse_event_source());
                    al_register_event_source(event_queue, al_get_display_event_source(janela));
                    al_register_event_source(event_queue, al_get_timer_event_source(timer));
                    al_start_timer(timer);
                }

                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 + 120 && mouse_y <= y2 + 120)
                {
                    estado_menu = 1;
                    redesenhar = true;
                }

                if (mouse_x >= x1 && mouse_x <= x2 && mouse_y >= y1 + 240 && mouse_y <= y2 + 240)
                {
                    rodando_programa = false;
                }
            }
            else if (estado_menu == 1)
            {
                if (mouse_x >= 50 && mouse_x <= 200 && mouse_y >= 50 && mouse_y <= 110)
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
                    al_draw_bitmap(background, 0, 0, 0);
                }
                else
                {
                    al_clear_to_color(al_map_rgb(0, 0, 0));
                }

                al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(0, 100, 255));
                al_draw_filled_rectangle(x1, y1 + 120, x2, y2 + 120, al_map_rgb(0, 100, 255));
                al_draw_filled_rectangle(x1, y1 + 240, x2, y2 + 240, al_map_rgb(0, 100, 255));

                int texto_x = x1 + (largura_botao / 2);
                int texto_y = y1 + (altura_botao / 2 - 25);
                al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y, ALLEGRO_ALIGN_CENTER, "PLAY");
                al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y + 120, ALLEGRO_ALIGN_CENTER, "TUTORIAL");
                al_draw_text(fonte, al_map_rgb(255, 255, 255), texto_x, texto_y + 240, ALLEGRO_ALIGN_CENTER, "SAIR");
            }
            else if (estado_menu == 1)
            {
                al_clear_to_color(al_map_rgb(20, 20, 20));

                al_draw_text(fonte, al_map_rgb(255, 255, 255), largura_tela / 2, 150, ALLEGRO_ALIGN_CENTER, "COMO JOGAR");
                al_draw_text(fonte, al_map_rgb(200, 200, 200), largura_tela / 2, 300, ALLEGRO_ALIGN_CENTER, "Use o teclado para mover o personagem.");
                al_draw_text(fonte, al_map_rgb(200, 200, 200), largura_tela / 2, 380, ALLEGRO_ALIGN_CENTER, "Evite os guardas e converse com os NPCs.");

                al_draw_filled_rectangle(50, 50, 200, 110, al_map_rgb(100, 100, 100));
                al_draw_text(fonte, al_map_rgb(255, 255, 255), 125, 58, ALLEGRO_ALIGN_CENTER, "Voltar");
            }

            al_flip_display();
        }
    }

    al_destroy_font(fonte);
    if (background) al_destroy_bitmap(background);
    al_destroy_display(janela);
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);

    return 0;
}