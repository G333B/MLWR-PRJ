# MLWR-PRJ
**Clément LANGUEDOC 3SI5**
**Johanna MEGUEDAD 3SI5**

Nous allons travailler sur des machines virtuelles Kali Linux et Ubuntu.
L'objectif est de créer un malware en langage C basé sur la manipulation de la variable d'environnement linux LD_PRELOAD.

**LD_PRELOAD** permet de forcer le chargement d'une bibliothèque indiquée avant TOUTES les autres, c'est pour cette raison que nous nous en servirons.

Ce malware permettra, lorsqu'il sera exécuté, de récupérer les identifiants de la victime qui se connecte en ssh (nous allons modifier le comportement des fonctions **write()** et **read()**).

Pour savoir quelle fonction redéfinir, nous devrons lancer la commande suivante : 

`strace -e trace=open,read,write,close -o toto.txt ssh username@IP-address`

Cette commande stockera dans un fichier toutes les logs (filtré sur open,read,write,close) du processus.
Cela nous donne une idée sur ce à quoi notre code devra ressembler. 

Nous allons stocker les identifiants volés dans un fichier caché (./ssh_logs).

_Scénario en assumed breach -> nous avons déjà accès à la machine de la victime._

Nous allons également faire en sorte que le malware ne soit exécutable qu'en tant qu'utilisateur normal pour garantir la portabilité de notre malware (pas besoin d'avoir tous les privilèges). 

Dans un second temps, nous ajouterons une fonctionnalité à notre malware : 
Redéfinir la fonction open() afin de bloquer l'accès à certains fichiers sur la machine de la victime.

En parallèle, nous allons créer un serveur C2 (Command & Control) qui nous permettra d'avoir le contrôle sur la machine victime. C'est ici que nous ferons du port knocking (nous avons choisi la suite de port 4000-5000-6000 pour l'autorisation d'ouverture du port de connexion au serveur, ici 4444).
Nous utilisons le port 4444 car il n'est pas standard (discrétion) et ne nécessite pas de droits root (sous linux, les ports au dessous de 1024 nécessitent les droits root).

Enfin, nous créerons un malware qui capturera les clés ssh de notre victime à l'aide d'un autre serveur d'exfiltration qui va nous permettre de stocker les clés SSH. La création d'un autre serveur pour l'exfiltration des clés, sécurise notre C2 serveur (éviter qu'il soit détectable). 

## Linker : 

Programme qui va assembler des fichiers objets pour créer un exécutable ou une bibliothèque partagée (notre cas actuel).
Linker statique -> pas besoin de bibliothèques externes mais charge toutes les bibliothèques utilisées dans l'exécutable final (donc plus lourd)
Linker dynamique -> Ce que nous avons utilisé. Utilise des bibliothèques partagées (.so) qui sont chargées au moment de l'exécution donc moins lourd.

## Threads :

Un thread linux ets une "partie" d'un processus (on peut exécuter des threads en parallèle à condition qu'il n'y ait pas de conflit -> le pointeur de 2 threads d'un même processus pointe sur le même élément -> conflit).
C'est le noyau qui gère l'exécution des processus et leur répartition dans le temps et dans la mémoire.

Le processus a son propre espace mémoire pour ne pas interférer avec les autres processus (ce qui signifie que le thread d'un processus possède le même espace mémoire que lui).
Le problème est que pour qu'un processus communique avec un autre, il doit obligatoirement passer par le système, c'est plus coûteux. 

On utilise donc des threads qui sont une version allégée d'un processus. Ce sont la représentation du processeur à un instant T (tout est identique au processus sauf qu'on a un pointeur sur telle partie de la pile par exemple, donc c'est l'état concret du processeur à telle exécution d'un processus).

Un processus avec 2 threads par xemple, aura donc 2 piles (une par thread). l'espace mémoire d'un processus comprend : une partie destinée au noyau qui va gérer la partie système, une partie pile, tas et bibliothèques qui concernent à elles trois les données, et une partie pour le code.
La pile est l'endroit d'un espace mémoire qui va stocker les variables locales et les adresses de retour des fonctions, elle fait partie des éléments de l'espace mémoire qui gère les données (avec les bibliothèques et le tas (malloc)).

Donc un thread est beaucoup plus judicieux qu'un processus complet car moins coûteux et plus abile.


Commandes linux notables :

### Suivre la trace de la connexion ssh  
``` strace -o toto.txt ssh username@IP-address```

### Compiler et créer notre bibliothèque partagée
```gcc -fPIC -shared -o lib.so code.c```

### Lancer la connexion ssh avec LD_PRELOAD pour charger NOTRE bibliothèque
```  LD_PRELOAD=./libmalware.so ssh username@IPaddress```

### Modifier les fichiers de config ssh  
```nano /etc/ssh/sshd_config```
```nano /etc/pam.d/sshd```

### permet de compiler et lancer le serveur 
```gcc -o ssh_serv ssh_serv.c```
```./ssh_serv```














