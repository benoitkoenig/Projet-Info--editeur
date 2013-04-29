#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

int EPAISSEUR_TRAIT;

typedef enum {Noir, Rouge} Couleur;
typedef enum {Selection, Edition, Dessin} Mode;
typedef enum {Vrai, Faux} Bool;

typedef struct point{
    int x;
    int y;
    struct point* next;
} Point;

typedef struct fragment{
    Couleur couleur;
    Point* chaine;
    SDL_Surface* spt;
    int lench;
    struct fragment* next;
} Fragment;


void eventDessin(SDL_Event ev, Fragment** , Bool * debut, Couleur couleur, int w, int h);
void affichage(Mode, Bool, Fragment*, SDL_Surface*, Couleur couleur);
void couleurevent(SDL_Event ev, Couleur *couleur, Mode mode, Bool debut, Fragment* fragments);


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


    cairo_surface_t *surfaceFond;
    SDL_Surface *surfLigne=NULL;
    surfLigne = SDL_CreateRGBSurface(SDL_HWSURFACE, ecran->w, ecran->h, 32, 0, 0, 0, 0);
    surfaceFond = cairo_image_surface_create_for_data (surfLigne->pixels,
                                                      CAIRO_FORMAT_ARGB32,
                                                      surfLigne->w,
                                                      surfLigne->h,
                                                      surfLigne->pitch);
    cairo_t *droite = cairo_create(surfaceFond);

    Mode mode = Dessin;
    Bool continuer = Vrai;
    SDL_Event ev;
    Couleur couleur = Noir;
    Fragment* fragments = NULL;

	//Variables en mode Selection
	int selection = -1; //-1 = pas de selection
	Bool deplacer = Faux; //Vrai -> l'objet suit la sélection, ne redessine PAS l'objet avec Cairo.
												//La position de l'objet est relative à sa position d'origine

	//Variables en mode Dessin
	Bool debut = Vrai; //dès qu'on clique, on crée un nouvel objet.

	//Variables en mode Edition
	//La vaiable selection sert ici aussi
	Point* edit = NULL;

    while (continuer==Vrai) {
        while (SDL_PollEvent(&ev)==1) {
            if (mode==Dessin)
                eventDessin(ev, &fragments, &debut, couleur, ecran->w, ecran->h);
            /*if (mode==Selection)
                eventSelection(ev);
            if (mode==Edition)
                eventEdition(ev);*/
            couleurevent(ev, &couleur, mode, debut, fragments);/*
            changermode();
            deplacerterrain();*/
            if (ev.type==SDL_KEYDOWN)
                if (ev.key.keysym.sym==SDLK_ESCAPE)
                    continuer=Faux;
        }
		affichage(mode, debut, fragments, ecran, couleur);
    }




    TTF_CloseFont(police);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
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

void affichage(Mode mode, Bool debut, Fragment* fragments, SDL_Surface* ecran, Couleur couleur) {
    int n=1, x, y;
    SDL_GetMouseState(&x, &y);
    cairo_surface_t *surfaceFond;
    SDL_Surface *surfLigne=NULL;
    surfLigne = SDL_CreateRGBSurface(SDL_HWSURFACE, ecran->w, ecran->h, 32, 0, 0, 0, 255);
    SDL_FillRect(surfLigne, NULL, 0xFFFFFFFF);
    surfaceFond = cairo_image_surface_create_for_data (surfLigne->pixels,
                                                      CAIRO_FORMAT_ARGB32,
                                                      surfLigne->w,
                                                      surfLigne->h,
                                                      surfLigne->pitch);
    cairo_t *droite = cairo_create(surfaceFond);
    cairo_set_line_width(droite, EPAISSEUR_TRAIT);



    Fragment* neuf = NULL;
    Point* nvp = NULL;
    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));

    if (fragments!=NULL) {
        neuf = fragments;
        if (neuf->spt!=NULL)    SDL_BlitSurface(neuf->spt, NULL, ecran, NULL);
        neuf = neuf->next;
        while (neuf!=NULL) {
            if (neuf->spt!=NULL)    SDL_BlitSurface(neuf->spt, NULL, ecran, NULL);
            neuf = neuf->next;
        }
    }


    if (mode==Dessin) {
        if (debut==Faux) {
            neuf = fragments;
            while (neuf->next != NULL)
                neuf = neuf->next;
            //On est maintenant au dernier fragment. On va le dessiner.
            if (neuf->couleur==Rouge)
                cairo_set_source_rgba (droite, 255, 0, 0, 1);
            nvp = neuf->chaine;

            cairo_move_to(droite, nvp->x, nvp->y);

            if (neuf->lench == 2)
                cairo_curve_to(droite, nvp->next->x, nvp->next->y, x, y, x, y);
            if (neuf->lench == 3)
                cairo_curve_to(droite, nvp->next->x, nvp->next->y, x, y, nvp->next->next->x, nvp->next->next->y);
            if (neuf->lench >= 4)
                cairo_curve_to(droite, nvp->next->x, nvp->next->y, nvp->next->next->next->x, nvp->next->next->next->y, nvp->next->next->x, nvp->next->next->y);
            nvp = nvp->next;
            while (neuf->lench>=4+3*n) {
                n++;
                nvp = nvp->next->next->next;
                cairo_curve_to(droite, nvp->x, nvp->y, nvp->next->next->x, nvp->next->next->y, nvp->next->x, nvp->next->y);
            }
            if (neuf->lench==4+3*n-1) {
                nvp = nvp->next->next->next;
                cairo_curve_to(droite, nvp->x, nvp->y, x, y, nvp->next->x, nvp->next->y);
            }
            if (neuf->lench==4+3*n-2) {
                nvp = nvp->next->next->next;
                cairo_curve_to(droite, nvp->x, nvp->y, x, y, x, y);
            }





            cairo_stroke(droite);
            SDL_BlitSurface(surfLigne, NULL, ecran, NULL);
        }
    }


    SDL_Flip(ecran);
}


void eventDessin(SDL_Event ev, Fragment** fragments, Bool * debut, Couleur couleur, int w, int h) {
    int n=1;
    Fragment* neuf = NULL;
    Point* neufchaine = NULL;
    Point* nvp=NULL;
    if (ev.type == SDL_MOUSEBUTTONDOWN) {
        if (ev.button.button == SDL_BUTTON_LEFT) {
            if (*debut==Vrai) {
                *debut = Faux;
                if (*fragments==NULL) {
                    *fragments = malloc(sizeof(Fragment));
                    (*fragments)->couleur = couleur;
                    (*fragments)->next = NULL;
                    (*fragments)->chaine = malloc(sizeof(Point));
                    ((*fragments)->chaine)->x = ev.button.x;
                    ((*fragments)->chaine)->y = ev.button.y;
                    ((*fragments)->chaine)->next = NULL;
                    (*fragments)->lench = 1;
                    (*fragments)->spt = NULL;
                }
                else {
                    neuf = (*fragments);
                    while (neuf->next != NULL)
                        neuf = neuf->next;
                    neuf->next = malloc(sizeof(Fragment));
                    (neuf->next)->couleur = couleur;
                    (neuf->next)->next = NULL;
                    neuf->next->chaine = malloc(sizeof(Point));
                    (neuf->next->chaine)->x = ev.button.x;
                    (neuf->next->chaine)->y = ev.button.y;
                    (neuf->next->chaine)->next = NULL;
                    (neuf->next)->lench = 1;
                    (neuf->next)->spt = NULL;
                }
            }
            else {
                neuf = (*fragments);
                while (neuf->next != NULL)
                    neuf = neuf->next;
                neufchaine = neuf->chaine;
                while (neufchaine->next!=NULL)
                    neufchaine = neufchaine->next;
                neufchaine->next = malloc(sizeof(Point));
                (neufchaine->next)->x = ev.button.x;
                (neufchaine->next)->y = ev.button.y;
                (neuf->lench)++;
            }
        }
        if (ev.button.button == SDL_BUTTON_RIGHT) {
            *debut = Vrai;
            neuf = (*fragments);
            while (neuf->next != NULL)
                neuf = neuf->next;


    nvp = neuf->chaine;
    cairo_surface_t *surfaceFond;

    neuf->spt = SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, 32, 0, 0, 0, 0);
    SDL_FillRect(neuf->spt, NULL, 0xFFFFFFFF);
    SDL_SetAlpha(neuf->spt, SDL_SRCALPHA, 100);
    SDL_SetColorKey(neuf->spt, SDL_SRCCOLORKEY, SDL_MapRGB(neuf->spt->format, 255, 255, 255));

    surfaceFond = cairo_image_surface_create_for_data (neuf->spt->pixels,
                                                      CAIRO_FORMAT_ARGB32,
                                                      neuf->spt->w,
                                                      neuf->spt->h,
                                                      neuf->spt->pitch);
    cairo_t *droite = cairo_create(surfaceFond);
    cairo_set_source_rgba (droite, 0, 0, 0, 1);
    cairo_set_line_width(droite, EPAISSEUR_TRAIT);
    if (neuf->couleur==Rouge)
        cairo_set_source_rgba (droite, 255, 0, 0, 1);
    cairo_move_to(droite, nvp->x, nvp->y);
    if (neuf->lench >= 4)
    cairo_curve_to(droite, nvp->next->x, nvp->next->y, nvp->next->next->next->x, nvp->next->next->next->y, nvp->next->next->x, nvp->next->next->y);
    nvp = nvp->next;
        while (neuf->lench>=4+3*n) {
            n++;
            nvp = nvp->next->next->next;
            cairo_curve_to(droite, nvp->x, nvp->y, nvp->next->next->x, nvp->next->next->y, nvp->next->x, nvp->next->y);
        }


    cairo_stroke(droite);






        }
    }
}
