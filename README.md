# MPI Parallel Game of Life

Progetto accademico di [**Ciro Perfetto**](https://github.com/ciroperf)

Corso di Laurea Magistrale in informatica: **Programmazione concorrente e parallela su cloud**

Professori: **Vittorio Scarano** **Carmine Spagnuolo**

[Dipartimento di Informatica](http://www.di.unisa.it) - [Università degli Studi di Salerno](https://www.unisa.it/) (Italia)

## Definizione del problema

Il problema consiste nell'esecuzione di "Game of Life", un gioco la cui evoluzione è determinata dallo stato iniziale, senza bisogno di ulteriori input.
Si può interagire creando una configurazione iniziale per vedere come si evolve oppure creando pattern con particolari proprietà.
L'universo di Game of Life è una griglia bidimensionale di celle quadrate, ciascuna delle quali si può trovare in due stati diversi, viva o morta, oppure "popolato" o "non popolato".
Nello sviluppo del progetto è stata usata una matrice di caratteri indicando con 'd' la cella morta e con 'a' la cella viva.
La parte centrale del programma consiste proprio nel fatto che queste celle interagiscono con le celle vicine, che sono quelle orizzontali, verticali o diagonalmente adiacenti.
Ad ogni esecuzione vengono determinati un numero di passi, a ciascun passo nel tempo, occorrono le seguenti interazioni:
- Qualsiasi cella viva con meno di due celle vicine vive, muore come se ci fosse sottopopolamento.
- Qualsiasi cella viva con 2 o 3 celle vicine vive, sopravvive anche nella prossima generazione.
- Qualsiasi cella viva con più di 3 vicine muore, come se ci fosse sovrappopolamento.
- Qualsiasi cella morta con esattamente 3 vicini, diventa una cella viva, come per riproduzione.

La configurazione iniziale costituisce il seed del sistema. La prima generazione viene creata applicando tutte le regole precedenti simultaneamente a ciascuna cella del seed iniziale. Ciascuna generazione successiva viene creata solo in funzione di quella precedente, continuando ad applicare le regole.

![Definizione del problema](Gospers_glider_gun.gif)

## Soluzione proposta

Per il problema sono stati utilizzati due approcci:
- un programma funzionante su un singolo core che non usa librerie di OpenMPI
- un programma che sfrutta il parallelismo utilizzando le librerie di OpenMPI

Prima di spiegare le soluzioni adottate al problema bisogna anche considerare che a ogni iterazione del programma, nella matrice di caratteri utilizzata come seed, la riga della matrice alla posizione 0 e la riga alla posizione N-1 (dove N è il numero delle righe totali) sono considerate come vicine, quindi si influenzano a vicenda ad ogni generazione, creando una struttura toroidale. 
Il [primo](Code/OneThread.c) ha un approccio più semplice al problema, generando un seed iniziale casuale e poi applicando le regole ad ogni cella fino a che non si raggiunge il numero di generazioni date in input. Dopo ogni passo viene aggiornata la matrice in output finchè non si arriva al numero di passi inserito.
Il [secondo](Code/Game.c) offre una soluzione più complessa, in quanto il problema viene diviso equamente dal master che manda le porzioni di matrice agli altri processori, facilitando così l'esecuzione del programma su matrici di grandi dimensioni, e ognuno di essi elabora gli input applicando le regole. Successivamente vengono mandati i bordi ai processori adiacenti. Una volta eseguite il numero di generazioni richieste, il processore con il rank 0, che partecipa anche'esso alla computazione, raccoglie i dati di tutti gli altri processori e inserisce i risultati nella matrice che viene poi data in output.

## Struttura del progetto

Verrà analizzato nel dettaglio la struttura di [Game.c](Code/Game.c) analizzando gli aspetti cruciali della soluzione al problema.

### Funzioni MPI

Le funzioni di MPI usate oltre quelle fondamentali sono:
- MPI_Isend
- MPI_Irecv
- MPI_Wait
Sono state usate queste funzioni per sfruttare la comunicazione non bloccante, in quanto dopo che il master ha suddiviso e mandato gli input agli altri processori questi già possono elaborare le porzioni interne prima che ricevano i bordi dai processori adiacenti, velocizzando così il processo di computazione.

### Sezioni di codice

` 
MPI_Isend(&borders[0][0], M, MPI_CHAR, source, tag, MPI_COMM_WORLD, &request_send);
MPI_Isend(&borders[1][0], M, MPI_CHAR, dest, tag, MPI_COMM_WORLD, &request_send);
MPI_Irecv(&borders_received[0][0], M, MPI_CHAR, source, tag, MPI_COMM_WORLD, &request_receive);
MPI_Irecv(&borders_received[1][0], M, MPI_CHAR, dest, tag, MPI_COMM_WORLD, &request_receive1);
`







