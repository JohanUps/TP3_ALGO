#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

#define DEF_SEED 0

typedef struct s_Node Node;

typedef struct s_Link{
    Node* next;
    Node* prev;
}Link;

struct s_Node{
    int key;
    unsigned int level;
    Link** link; //Tableau de lien, link[0] correspond à un pointeur vers une structure link qui contient les pointeurs next et prev de niveau 0 du noeud
};



struct s_SkipList {
    Node*  sentinel;
    unsigned int nblevels;
    int size;
};


SkipList* skiplist_create(int nblevels) {
    //Le nombre de niveau d'une skiplist est au moins égal à 1
    if(nblevels<1){
        fprintf(stderr, "Erreur : Le niveau de la skiplist est inférieur à 1");
        exit(EXIT_FAILURE);
    }
    
    //Allocation mememoire pour la skiplist et sa sentinel
    SkipList* skiplist = malloc(sizeof(SkipList));
    skiplist->sentinel = malloc(sizeof(Node));
    skiplist->nblevels = nblevels;
    
    //Allocation des tableaux de pointeur next et prev de la skiplist
    skiplist->sentinel->link = malloc(sizeof(Link)*nblevels);
    
    //Initialisation des noeuds vers la sentinel
    for(unsigned int i = 0; i<skiplist->nblevels; i++){
        skiplist->sentinel->link[i]->next = skiplist->sentinel;
        skiplist->sentinel->link[i]->prev = skiplist->sentinel;
    }
    
    skiplist->size = 0;
    

	return skiplist;
}

void skiplist_delete(SkipList** d) {
    //Nous allons passer par le noeud de plus bas niveau d'element en element pour être sur de passer par tout les noeuds
    SkipList* skiplist = *d;
	Node* cursor = skiplist->sentinel->link[0]->next;
    while(cursor != skiplist->sentinel){
        Node* toDelete = cursor;
        cursor = cursor->link[0]->next;
        free(toDelete);
    }
    free(skiplist);
}



SkipList* skiplist_insert(SkipList* d, int value) {
    
    //Recherche de l'endroit ou le nouveau noeud doit être insérer (le curseur va pointer sur le plus grand element dont la key est inférieur ou égale à value) /*A optimiser (ne pas passer necessairement par link[0]) */ //TODO
    Node* cursor = d->sentinel->link[0]->next;
    while(cursor->key <= cursor->link[0]->next->key){
        cursor = cursor->link[0]->next;
    }
    //Si l'element est deja dans la liste on ne l'ajoute pas
    if(cursor->key != value){
        
        //Utilisation de la structure RNG qui va donner la valeur du champ level de l'element à inserer
        RNG rng = rng_initialize(DEF_SEED, d->nblevels);
        unsigned int newElemLevel = rng_get_value(&rng);
        
        //Allocation memoire du nouvel element
        Node* newElem = malloc(sizeof(Node));
        newElem->key = value;
        newElem->level = newElemLevel;
        
        //Creation du pointeur sur l'element qui suit le curseur ainsi que sur l'element qui le precede  
        Node* precedent = cursor->link[0]->prev;
        Node* suivant = cursor->link[0]->next;
        
        //*Mise a jour des pointeurs de newElem
        
            //Mise a jour des pointeurs next des éléments précédents newElem et mise à jour des champs 
        unsigned int nbPointeursMAJ = 0;
        while(nbPointeursMAJ < newElemLevel){
            //Mettre a jour autant de pointeurs prev que possible vers l'element precedent
            for(unsigned int i = nbPointeursMAJ; i < newElemLevel && i < precedent->level; i++){
                newElem->link[i]->prev = precedent;
                precedent->link[i]->next = newElem;
                nbPointeursMAJ++;
            }
            
            //Recherche d'un noeud de plus haut niveau pour mettre les pointeurs manquant (deplacement sur le pointeur prev de plus haut niveau)
            precedent = precedent->link[nbPointeursMAJ - 1]->prev;
        }
        
            
            
        //Mise a jour des pointeurs prev des éléments suivant newElem et mise à jour des champs next
        nbPointeursMAJ = 0;
        while(nbPointeursMAJ < newElemLevel){
            //Mettre a jour autant de pointeurs next que possible vers l'element suivant
            for(unsigned int i = nbPointeursMAJ; i < newElemLevel && i < suivant->level; i++){
            newElem->link[i]->next = suivant;
            suivant->link[i]->prev = newElem;
            nbPointeursMAJ++;
            }
            suivant = suivant->link[nbPointeursMAJ - 1]->next;
        }
        
        //Incrementation de la taille de la liste
        d->size++;
    }
	return d;
}
