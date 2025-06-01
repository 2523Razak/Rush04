# Rush04

Un mini terminal personnalisable en C, avec des fonctionnalités pratiques et une logique bien maîtrisée.

# Description Courte

Le programme est un shell (interpréteur de commandes) minimaliste écrit en **langage C**. Il permet d'exécuter des commandes système, de manipuler des fichiers et dossiers, et d’éditer du texte,tout cela dans une interface simplifiée faite maison.

# Fonctionnalités Principales

- **cd** : Change de répertoire

- **ls** : Liste les fichiers du dossier courant

- **mkdir** : Crée un nouveau dossier

- **nano** : Ouvre un fichier dans l’éditeur nano

- **help** : Affiche les commandes disponibles

- **exit** : Ferme le shell

# Technologies Utilisées

## Langage : C

## Librairies : 
- ***stdio.h, stdlib.h, string.h*** : gestion des entrées/sorties, allocations mémoire, manipulation de texte.

- ***unistd.h*** : exécution de commandes, gestion des processus.

- ***dirent.h*** : lecture des dossiers (pour ls).

- ***sys/types.h, sys/wait.h,sys/stat.h*** : manipulation des processus, des statuts et des dossiers.


# Compilez le code

```c
gcc -o Rus04 Rush04.c
     ou bien
clang Rush04 -o Rush04
```

## Lancez le shell
```c
./minishell
```


🧪 Utilisation

shell> mkdir test
shell> cd test
shell> nano notes.txt
shell> ls
notes.txt
shell> help
shell> exit


---

# 👥 Équipe

Développé avec passion par : 
- ***Al***
- ***Boureima_Issa_Adamou_R***
- ***Rid***
- ***Ri***


# Code Source Commenté et Expliqué

## Fichier : shell.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
```
Ces en-têtes permettent d'utiliser les fonctions nécessaires pour la **gestion des fichiers, processus et entrées/sorties**. C'est la base de tout programme UNIX-like.

```c
#define MAX_INPUT_SIZE 1024
#define MAX_TOKENS 64
#define DELIMITERS " \t\r\n\a"
```
Ces constantes définissent la **taille maximale de l'entrée utilisateur, le nombre maximal de mots par commande, et les caractères utilisés pour séparer ces mots** (espace, tabulation, retour à la ligne).

## Fonction d’aide
```c
void display_help() {
    printf("\n=== AIDE ===\n");
    printf("exit      - Quitter le shell\n");
    printf("cd <dir>  - Changer de dossier\n");
    printf("ls        - Lister les fichiers\n");
    printf("mkdir <d> - Créer un dossier\n");
    printf("nano <f>  - Éditer un fichier\n");
    printf("help      - Afficher cette aide\n\n");
}
```

Une simple fonction qui imprime un menu d'aide dans la console. Elle est appelée lorsque l'utilisateur tape help.


## Fonction principale d'exécution
```c
void execute_command(char** args) {
    if (args[0] == NULL) return;
```
Si l'utilisateur n'a rien tapé, on ne fait rien.
```c
if (strcmp(args[0], "exit") == 0) {
        printf("Merci d'avoir utilisé Super Mini Shell!\n");
        exit(0);
    }
```
Quitte le programme si la commande est exit.
```c
else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Usage: cd <directory>\n");
        } else if (chdir(args[1]) != 0) {
            perror("Erreur lors du changement de dossier");
        }
    }
```
Gère la commande **cd** avec **chdir**, qui change le répertoire courant.

```c
else if (strcmp(args[0], "ls") == 0) {
        DIR* dir = opendir(".");
        struct dirent* entry;
        if (dir == NULL) {
            perror("Erreur lors de l'ouverture du dossier");
            return;
        }
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }
        closedir(dir);
    }
```
Liste les fichiers et dossiers du répertoire courant à l'aide de opendir, readdir, puis closedir.

```c

else if (strcmp(args[0], "mkdir") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Usage: mkdir <directory>\n");
        } else if (mkdir(args[1], 0755) != 0) {
            perror("Erreur lors de la création du dossier");
        } else {
            printf("Dossier créé: %s\n", args[1]);
        }
    }
```
Crée un dossier avec **mkdir**. Le mode 0755 permet au propriétaire d'avoir tous les droits, les autres uniquement lecture/exécution.

```c
else if (strcmp(args[0], "nano") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Usage: nano <filename>\n");
        } else {
            char* editor = getenv("EDITOR");
            if (!editor) editor = "nano";
            char command[256];
            snprintf(command, sizeof(command), "%s %s", editor, args[1]);
            system(command);
        }
    }
```
Ouvre un fichier avec nano (ou un autre éditeur défini dans la variable d'environnement EDITOR) grâce à system().

```c
else if (strcmp(args[0], "help") == 0) {
        display_help();
    }
```
Affiche l'aide si l'utilisateur tape help.

```c
else {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror("Commande non trouvée");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("Erreur de fork");
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}
```
Toutes les autres commandes sont exécutées comme un vrai shell, avec **fork()** pour créer un processus fils et **execvp()** pour remplacer le processus courant par la commande saisie.


# Analyseur de ligne de commande
```c
char** parse_input(char* input) {
    int bufsize = MAX_TOKENS;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;
    int position = 0;

    token = strtok(input, DELIMITERS);
    while (token != NULL) {
        tokens[position++] = token;
        token = strtok(NULL, DELIMITERS);
    }
    tokens[position] = NULL;
    return tokens;
}
```
Cette fonction **divise une chaîne tapée par l'utilisateur en "mots" (tokens)**, qu'on peut ensuite analyser pour savoir quelle commande est appelée.

# Point d’entrée principal

```c
int main() {
    char input[MAX_INPUT_SIZE];
    char** args;

    printf("=== Super Mini Shell ===\n");
    printf("Commandes spéciales: nano, mkdir, ls, cd, help, exit\n");

    while (1) {
        printf("shell> ");
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            printf("\n");
            break;
        }

        args = parse_input(input);
        execute_command(args);
        free(args);
    }

    return 0;
}
```
Boucle principale du shell : **affiche une invite shell>**, attend l'entrée, l'analyse, exécute, puis recommence jusqu'à ce que l'utilisateur tape exit ou quitte le shell.

# Exemple d'exécution
```c
$ ./minishell
=== Super Mini Shell ===
Commandes spéciales: nano, mkdir, ls, cd, help, exit
shell> mkdir test
Dossier créé: test
shell> cd test
shell> nano notes.txt
# (ouvre nano, écrire et enregistrer)
shell> ls
notes.txt
shell> help
(exit, cd, ls, mkdir, nano, help)
shell> exit
```
