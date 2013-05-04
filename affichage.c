#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <unistd.h>

#include <sys/time.h>


#include "main.h"
#include "affichage.h"


struct timeval t1, t2;


void affichage(Mode mode, Bool debut, Fragment* fragments, SDL_Surface* ecran, int posecran,
                cairo_surface_t *surfaceFond, SDL_Surface *surfLigne, SDL_Rect positionecran, cairo_t *droite, SDL_Rect tailleecran) {

    int n, x, y;
    SDL_GetMouseState(&x, &y);

    Fragment* neuf = NULL;

    SDL_FillRect(surfLigne, &tailleecran, 0xFFFFFFFF);
    droite = cairo_create(surfaceFond);
    cairo_set_line_width(droite, EPAISSEUR_TRAIT);
    Point* nvp = NULL;

    neuf = fragments;
    while(neuf!=NULL) {
        n=0;
        if (neuf->couleur==Rouge)
            cairo_set_source_rgba (droite, 1, 0, 0, 1);
        else
            cairo_set_source_rgba (droite, 0, 0, 0, 1);
        nvp = neuf->chaine;

        cairo_move_to(droite, nvp->x, nvp->y);
        nvp = nvp->next;
        while (neuf->lench>=4+3*n) {
            cairo_curve_to(droite, nvp->x, nvp->y, nvp->next->next->x, nvp->next->next->y, nvp->next->x, nvp->next->y);
            n++;
            nvp = nvp->next->next->next;
        }
        if (mode==Dessin && debut==Faux && neuf->next==NULL) {
            if (neuf->lench==4+3*n-1) {
                cairo_curve_to(droite, nvp->x, nvp->y, x+posecran, y, nvp->next->x, nvp->next->y);
            }
            if (neuf->lench==4+3*n-2) {
                cairo_curve_to(droite, nvp->x, nvp->y, x+posecran, y, x+posecran, y);
            }
        }
        cairo_stroke(droite);
        neuf = neuf->next;
    }



    SDL_BlitSurface(surfLigne, &tailleecran, ecran, &positionecran);
    SDL_Flip(ecran);

}
