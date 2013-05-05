#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <unistd.h>


#include "main.h"
#include "affichage.h"

int EPAISSEUR_TRAIT;
int W_SUR_H;

/*
typedef enum {KeyPress, KeyUp, KeyRelease, MouseButtonPress, MouseButtonRelease, MouseButtonDown} Type;
typedef enum {n, r, e, s, d, left, right} Key;
typedef enum {Type type, int x, int y, Key key} Event;
*/





int main(int argc, char *argv[]){

    if(SDL_Init(SDL_INIT_VIDEO) == -1){
        fprintf(stderr, "Erreur d'initialisation de la SDL : %s\n", SDL_GetError()); // Écriture de l'erreur
        exit(EXIT_FAILURE); // On quitte le programme
    }
    TTF_Init();

    SDL_Surface *ecran = NULL;
    TTF_Font *police = NULL;

    ecran = SDL_SetVideoMode(500, 500, 32, SDL_HWSURFACE | SDL_DOUBLEBUF/* | SDL_FULLSCREEN*/);
    if (ecran == NULL){  // Si l'ouverture a échoué, on le note et on arrête
        fprintf(stderr, "Impossible de charger le mode vidéo : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_WM_SetCaption("Editeur de terrain", NULL);
    police = TTF_OpenFont("VirtualVectorVortex.ttf", 85);
    EPAISSEUR_TRAIT = 7*ecran->h/768;
    W_SUR_H = 10;

    Mode mode = Dessin;
    Bool continuer = Vrai;
    SDL_Event ev;
    SDL_EnableKeyRepeat(200, 100);
    Couleur couleur = Noir;
    Fragment* fragments = NULL;
    int posecran = 0;

    //Variables pour l'affichage
    cairo_surface_t *surfaceFond = NULL;
    SDL_Surface *surfLigne = NULL;
    SDL_Rect positionecran, tailleecran;
    tailleecran.x = 0;
    tailleecran.y = 0;
    tailleecran.h = ecran->h;
    tailleecran.w = ecran->w;
    positionecran.y = 0;
    positionecran.x = 0;
    positionecran.h = ecran->h;
    positionecran.w = ecran->w;
    cairo_t *droite = NULL;

    surfLigne = SDL_CreateRGBSurface(SDL_HWSURFACE, W_SUR_H * ecran->h, ecran->h, 32, 0, 0, 0, 255);
    surfaceFond = cairo_image_surface_create_for_data (surfLigne->pixels,
                                                        CAIRO_FORMAT_ARGB32,
                                                        surfLigne->w,
                                                        surfLigne->h,
                                                        surfLigne->pitch);



	//Variables en mode Selection
	//int selection = -1; //-1 = pas de selection
	//Bool deplacer = Faux; //Vrai -> l'objet suit la sélection, ne redessine PAS l'objet avec Cairo.
												//La position de l'objet est relative à sa position d'origine

	//Variables en mode Dessin
	Bool debut = Vrai; //dès qu'on clique, on crée un nouvel objet.

	//Variables en mode Edition
	//La variable selection sert ici aussi
	//Point* edit = NULL;

    //racourcis clavier : n:noir, r:rouge, s:Selection, e:Edition, d:Dessin
    affichage(mode, debut, fragments, ecran, posecran, surfaceFond, surfLigne, positionecran, droite, tailleecran);
    while (continuer==Vrai) {
        while (SDL_PollEvent(&ev)==1) {
            if (mode==Dessin)
                eventDessin(ev, &fragments, &debut, couleur, ecran->w, ecran->h, posecran);
            /*if (mode==Selection)
                eventSelection(ev);
            if (mode==Edition)
                eventEdition(ev);*/
            couleurevent(ev, &couleur, mode, debut, fragments);
            ecranevent(ev, &posecran, &tailleecran);
            changermode(ev, &mode, &debut);
            continuer = finevent(ev);
        }
        affichage(mode, debut, fragments, ecran, posecran, surfaceFond, surfLigne, positionecran, droite, tailleecran);
		usleep(20000);
    }

    TTF_CloseFont(police);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}

void changermode(SDL_Event ev, Mode *mode, Bool *debut) {
    if (ev.type==SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_s) {
            *mode = Selection;
            *debut = Vrai;
        }
        if (ev.key.keysym.sym == SDLK_d)
            *mode = Dessin;
        if (ev.key.keysym.sym == SDLK_e) {
            *mode = Edition;
            *debut = Vrai;
        }
    }
}

Bool finevent(SDL_Event ev) {
    if (ev.type==SDL_KEYDOWN)
        if (ev.key.keysym.sym==SDLK_ESCAPE)
            return Faux;
    if (ev.type==SDL_QUIT)
        return Faux;
    return Vrai;
}

void ecranevent(SDL_Event ev, int *posecran, SDL_Rect* tailleecran) {
    if (ev.type==SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_RIGHT)
            (*posecran)+=20;
        if (ev.key.keysym.sym == SDLK_LEFT && (*posecran)>0)
            (*posecran)-=20;
    }
    tailleecran->x = (*posecran);
}

void couleurevent(SDL_Event ev, Couleur *couleur, Mode mode, Bool debut, Fragment* fragments) {
    Fragment* neuf = NULL;
    if (ev.type==SDL_KEYDOWN) {
        if (ev.key.keysym.sym==SDLK_r)
            *couleur = Rouge;
        if (ev.key.keysym.sym==SDLK_n)
            *couleur = Noir;
    }
    if (mode==Dessin && debut==Faux) {
        neuf = fragments;
        while(neuf->next!=NULL)
            neuf = neuf->next;
        neuf->couleur = *couleur;
    }
}




void eventDessin(SDL_Event ev, Fragment** fragments, Bool * debut, Couleur couleur, int w, int h, int posecran) {
    Fragment* neuf = NULL;
    Point* neufchaine = NULL;
    if (ev.type == SDL_MOUSEBUTTONDOWN) {
        if (ev.button.button == SDL_BUTTON_LEFT) {
            if (*debut==Vrai) {
                *debut = Faux;
                if (*fragments==NULL) {
                    *fragments = malloc(sizeof(Fragment));
                    neuf = *fragments;
                }
                else {
                    neuf = (*fragments);
                    while (neuf->next != NULL)
                        neuf = neuf->next;
                    neuf->next = malloc(sizeof(Fragment));
                    neuf = neuf->next;
                }
                neuf->couleur = couleur;
                neuf->next = NULL;
                neuf->chaine = malloc(sizeof(Point));
                neuf->chaine->x = ev.button.x+posecran;
                neuf->chaine->y = ev.button.y;
                neuf->lench = 1;
                neuf->spt = NULL;
            }
            else {
                neuf = (*fragments);
                while (neuf->next != NULL)
                    neuf = neuf->next;
                neufchaine = neuf->chaine;
                while (neufchaine->next!=NULL)
                    neufchaine = neufchaine->next;
                neufchaine->next = malloc(sizeof(Point));
                (neufchaine->next)->x = ev.button.x+posecran;
                (neufchaine->next)->y = ev.button.y;
                (neuf->lench)++;
            }
        }
        if (ev.button.button == SDL_BUTTON_RIGHT) {
            *debut = Vrai;
        }
    }
}
