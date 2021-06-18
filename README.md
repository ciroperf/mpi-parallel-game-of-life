# MPI Parallel Game of Life
Progetto accademico di [**Ciro Perfetto**](https://github.com/ciroperf)

Corso di Laurea Magistrale in informatica: **Programmazione concorrente e parallela su clpud**

Professori: **Vittorio Scarano** **Carmine Spagnuolo**

[Dipartimento di Informatica](http://www.di.unisa.it) - [Università degli Studi di Salerno](https://www.unisa.it/) (Italia)

## Definizione del problema

Il problema consiste nell'esecuzione di "Game of Life", un gioco la cui evoluzione è determinata dallo stato iniziale, senza bisogno di ulteriori input.
Si può interagire creando una configurazione iniziale e vedere come si evolve oppure creando pattern con particolari proprietà.
L'universo di Game of Life è una griglia bidimensionale di celle quadrate, ciascuna delle quali si può trovare in due stati diversi, viva o morta, oppure "popolato" o "non popolato".
Nello sviluppo del progetto è stata usata una matrice di caratteri indicando con 'd' la cella morta e con 'a' la cella viva.
La parte centrale del programma consiste proprio nel fatto che queste celle interagiscono con le celle vicine, che sono quelle orizzontali, verticali o diagonalmente adiacenti.
Ad ogni esecuzione vengono determinati un numero di passi, a ciascun passo nel tempo, occorrono le seguenti interazioni:
- Qualsiasi cella viva con meno di due celle vicine vive, muore come se ci fosse sotto popolazione.
- Qualsiasi cella viva con 2 o 3 celle vicine vive, sopravvive anche nella prossima generazione.
- Qualsiasi cella viva con più di 3 vicine muore, come se ci fosse sovrappopolamento.
- Qualsiasi cella morta con esattamente 3 vicini, diventa una cella viva, come per riproduzione.

La configurazione iniziale costituisce il seed del sistema. La prima generazione viene creata applicando tutte le regole precedenti simultaneamente a ciascuna cella del seed iniziale. Ciascuna generazione successiva viene creata solo in funzione di quella precedente, continuando ad applicare le regole.

## Soluzione proposta



