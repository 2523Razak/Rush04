#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

/* Constantes */
#define MAX_INPUT 1024
#define MAX_ARGS 64

/* Couleurs */
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

/* Prototypes */
void display_prompt();
char *read_input();
char **parse_input(char *input);
int execute_command(char **args);
int shell_exit(char **args);
int shell_cd(char **args);
int shell_help(char **args);
int shell_ls(char **args);
int shell_mkdir(char **args);
int shell_nano(char **args);

/* Commandes intégrées */
char *builtins[] = {"exit", "cd", "help", "ls", "mkdir", "nano"};
int (*builtin_funcs[])(char **) = {
    &shell_exit, &shell_cd, &shell_help, &shell_ls, &shell_mkdir, &shell_nano
};

/* ================= MAIN ================= */
int main() {
    char *input;
    char **args;
    int status = 1;

    printf(GREEN "\n=== Super Mini Shell ===\n" RESET);
    printf("Commandes spéciales: nano, mkdir, ls, cd, help, exit\n\n");

    while (status) {
        display_prompt();
        input = read_input();
        args = parse_input(input);
        status = execute_command(args);

        free(input);
        free(args);
    }

    printf("\nMerci d'avoir utilisé Super Mini Shell!\n");
    return 0;
}

/* ================= FONCTIONS DE BASE ================= */

void display_prompt() {
    printf(BLUE "shell> " RESET);
    fflush(stdout);
}

char *read_input() {
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&input, &len, stdin);
    if (read == -1) {
        if (feof(stdin)) exit(0);
        perror("getline");
        exit(1);
    }

    if (input[read-1] == '\n') input[read-1] = '\0';
    return input;
}

char **parse_input(char *input) {
    char **args = malloc(MAX_ARGS * sizeof(char*));
    if (!args) {
        perror("malloc");
        exit(1);
    }

    char *token;
    int i = 0;
    token = strtok(input, " ");
    
    while (token != NULL && i < MAX_ARGS-1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    
    return args;
}

int execute_command(char **args) {
    if (!args[0]) return 1;

    for (int i = 0; i < sizeof(builtins)/sizeof(char*); i++) {
        if (strcmp(args[0], builtins[i]) == 0) {
            return (*builtin_funcs[i])(args);
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        printf(RED "Commande inconnue: %s\n" RESET, args[0]);
        exit(1);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror(RED "fork" RESET);
    }
    return 1;
}

/* ================= COMMANDES INTÉGRÉES ================= */

int shell_exit(char **args) {
    return 0;
}

int shell_cd(char **args) {
    if (!args[1]) {
        fprintf(stderr, RED "Usage: cd <dossier>\n" RESET);
    } else {
        if (chdir(args[1]) != 0) {  // Correction ici: parenthèses complètes
            perror(RED "cd" RESET);
        }
    }
    return 1;
}

int shell_help(char **args) {
    printf("\n" GREEN "=== AIDE ===" RESET "\n");
    printf("exit      - Quitter le shell\n");
    printf("cd <dir>  - Changer de dossier\n");
    printf("ls        - Lister les fichiers\n");
    printf("mkdir <d> - Créer un dossier\n");
    printf("nano <f>  - Éditer un fichier (crée si inexistant)\n");
    printf("help      - Afficher cette aide\n\n");
    return 1;
}

int shell_ls(char **args) {
    DIR *dir = opendir(".");
    if (!dir) {
        perror(RED "ls" RESET);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {  // Correction ici: != NULL ajouté
        if (entry->d_name[0] != '.')
            printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
    return 1;
}

int shell_mkdir(char **args) {
    if (!args[1]) {
        fprintf(stderr, RED "Usage: mkdir <nom_dossier>\n" RESET);
    } else {
        if (mkdir(args[1], 0755) != 0) {  // Correction ici: parenthèses complètes
            perror(RED "mkdir" RESET);
        } else {
            printf(GREEN "Dossier créé: %s\n" RESET, args[1]);
        }
    }
    return 1;
}

/* ======= FONCTION NANO COMPLÈTE ======= */
int shell_nano(char **args) {
    if (!args[1]) {
        fprintf(stderr, RED "Usage: nano <fichier>\n" RESET);
        return 1;
    }

    // Vérification du chemin
    char *path = args[1];
    char *last_slash = strrchr(path, '/');
    
    if (last_slash != NULL) {  // Correction ici: != NULL ajouté
        char directory[MAX_INPUT];
        strncpy(directory, path, last_slash - path);
        directory[last_slash - path] = '\0';
        
        struct stat st;
        if (stat(directory, &st) != 0) {  // Correction ici: parenthèses complètes
            printf(RED "Erreur: Dossier parent inexistant\n" RESET);
            return 1;
        }
    }

    printf(GREEN "Ouverture de nano: %s\n" RESET, path);
    
    pid_t pid = fork();
    if (pid == 0) {
        execlp("nano", "nano", path, NULL);
        printf(RED "nano non trouvé, essai avec vi...\n" RESET);
        execlp("vi", "vi", path, NULL);
        printf(RED "Aucun éditeur texte trouvé (installez nano ou vi)\n" RESET);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf(GREEN "Édition terminée. " RESET);
            
            struct stat st;
            if (stat(path, &st) == 0) {  // Correction ici: == 0 ajouté
                printf("Taille: %ld octets\n", st.st_size);
            } else {
                printf(RED "Fichier non créé/modifié\n" RESET);
            }
        }
    } else {
        perror(RED "fork" RESET);
    }
    
    return 1;
}
