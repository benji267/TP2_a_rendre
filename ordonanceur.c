#include <stdarg.h>
#include <unistd.h>
#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

#define CHK(op)            \
    do                     \
    {                      \
        if ((op) == -1)    \
            raler(1, #op); \
    } while (0)

noreturn void raler(int syserr, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    if (syserr == 1)
        perror("");

    exit(EXIT_FAILURE);
}

void surp(int k)
{
    printf("SURP - process %d\n", k);
    fflush(stdout);
}
void evip(int k)
{
    printf("EVIP - process %d\n", k);
    fflush(stdout);
}

void finp(int k)
{
    printf("TERM - process %d\n", k);
    fflush(stdout);
}

volatile sig_atomic_t sig_user_un = 0;
volatile sig_atomic_t sig_user_deux = 0;
volatile sig_atomic_t sig_alarm = 0;
volatile sig_atomic_t sig_chld = 0;
;

void fct(int signal)
{
    switch (signal)
    {
    case SIGUSR1:
        sig_user_un = 1;
        break;
    case SIGUSR2:
        sig_user_deux = 1;
        break;
    case SIGALRM:
        sig_alarm = 1;
        break;
    case SIGCHLD:
        sig_chld = 1;
        break;
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        raler(0, "Erreur sur le nombre d'arguments");
    }
    int duree_qtum = atoi(argv[1]);
    if (duree_qtum < 1)
    {
        raler(0, "durée quantum trop faible");
    }
    int *tab_valeurs_atoi = malloc((argc - 2) * sizeof(int));
    if (tab_valeurs_atoi == NULL)
    {
        raler(0, "malloc");
    }
    int k;
    for (k = 0; k < argc - 2; k++)
    {
        tab_valeurs_atoi[k] = atoi(argv[k + 2]);
        if (tab_valeurs_atoi[k] < 1)
        {
            raler(0, "durée d'un process trop court");
        }
    }
    pid_t fils;
    int process_finie = 0;
    int raison;
    pid_t pere = getpid();
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    s.sa_handler = fct;
    s.sa_flags = 0;
    CHK(sigemptyset(&s.sa_mask));
    CHK(sigaction(SIGUSR1, &s, NULL));
    CHK(sigaction(SIGUSR2, &s, NULL));
    CHK(sigaction(SIGALRM, &s, NULL));
    CHK(sigaction(SIGCHLD, &s, NULL));
    sigset_t masque_un, vide, masque_total;
    CHK(sigemptyset(&masque_total));
    CHK(sigemptyset(&masque_un));
    CHK(sigemptyset(&vide));
    CHK(sigaddset(&masque_un, SIGUSR1));
    CHK(sigaddset(&masque_total, SIGUSR1));
    CHK(sigaddset(&masque_total, SIGCHLD));
    CHK(sigaddset(&masque_total, SIGALRM));
    int *pid_fils;
    pid_fils = malloc((argc - 2) * sizeof(int));
    if (pid_fils == NULL)
    {
        raler(0, "malloc");
    }
    for (int i = 0; i < argc - 2; i++)
    {
        switch (pid_fils[i] = fork())
        {
        case -1:
            raler(1, "fork");
        case 0:

            for (k = 0; k < tab_valeurs_atoi[i]; k++)
            {
                CHK(sigprocmask(SIG_BLOCK, &masque_un, NULL));
                if (sig_user_un == 0)
                {
                    sigsuspend(&vide);
                }
                surp(i);
                CHK(sigprocmask(SIG_UNBLOCK, &masque_un, NULL));
                sig_user_un = 0;
                while (!sig_user_deux)
                {
                    sleep(1);
                }
                CHK(kill(pere, SIGUSR1));
                sig_user_deux = 0;
            }
            exit(0);

        default:
            break;
        }
    }
    k = 0;
    CHK(kill(pid_fils[k], SIGUSR1));
    alarm(duree_qtum);
    while (1)
    {
        CHK(sigprocmask(SIG_BLOCK, &masque_total, NULL));
        if (sig_user_un != 1 && sig_chld != 1 && sig_alarm != 1)
        {
            sigsuspend(&vide);
        }
        CHK(sigprocmask(SIG_UNBLOCK, &masque_total, NULL));

        if (sig_user_un == 1)
        {
            evip(k);
            while (1)
            {
                k++;
                if (k == argc - 2)
                {
                    k = 0;
                }
                if (pid_fils[k] != -1)
                {
                    break;
                }
            }
            CHK(kill(pid_fils[k], SIGUSR1));
            alarm(duree_qtum);
            sig_user_un = 0;
        }

        if (sig_alarm == 1)
        {
            CHK(kill(pid_fils[k], SIGUSR2));
            sig_alarm = 0;
        }

        if (sig_chld == 1)
        {
            CHK(fils = wait(&raison));
            if (WIFEXITED(raison))
            {
                if (WEXITSTATUS(raison) != 0)
                {
                    printf("erreur dans l'exit d'un fils");
                    exit(1);
                }
            }
            else if (WIFSIGNALED(raison))
            {
                printf("erreur fils signal \n ");
                exit(1);
            }
            else
            {
                printf(" problème fils raison inconnue \n ");
                exit(1);
            }
            
            for (int j = 0; j < argc - 2; j++)
            {
                if (pid_fils[j] == fils)
                {
                    pid_fils[j] = -1;
                    finp(j);
                    process_finie++;
                    break;
                }
            }

            sig_chld = 0;
        }
        if (process_finie == argc - 2)
        {
            break;
        }
    }
    free(pid_fils);
    free(tab_valeurs_atoi);
    exit(0);
}
