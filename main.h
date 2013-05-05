extern int EPAISSEUR_TRAIT;
extern int W_SUR_H;


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


void eventDessin(SDL_Event ev, Fragment** , Bool * debut, Couleur couleur, int w, int h, int posecran);
void couleurevent(SDL_Event ev, Couleur *couleur, Mode mode, Bool debut, Fragment* fragments);
void ecranevent(SDL_Event ev, int *posecran, SDL_Rect* tailleecran);
void changermode(SDL_Event ev, Mode *mode, Bool *debut);
Bool finevent(SDL_Event ev);
