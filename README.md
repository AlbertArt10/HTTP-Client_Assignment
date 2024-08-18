# Protocoale de Comunicatii
## Client CLI pentru Bibliotecă Virtuală cu Protocol HTTP
## ANTONESCU ALBERT


## Descriere:

Client HTTP în C/C++ pentru interacțiune cu REST API. Implementare a unui client care trimite comenzi către un server ce simulează o bibliotecă online. Include suport pentru JSON, sesiuni, JWT și manipularea cererilor POST și GET. Utilizează biblioteca parson pentru parsarea JSON.


Enunt Tema: https://pcom.pages.upb.ro/enunt-tema4/

Checker: https://gitlab.cs.pub.ro/pcom/homework4-public

## Biblioteca pentru parsare JSON

Aceasta tema se foloseste de parson pentru serializarea datelor in formatul json pentru trimiterea 
prin request-urile POST. Aceasta alegere a fost motivata de recomandarea din enunt din sectiunea
JSON si Token JWT pentru limbajul C. Am remarcat modul simplu de utilizare al API-ului pentru
formatarea datelor de care am avut nevoie.


## Client (client.c)

Clientul citeste de la tastatura comenzi pana cand se primeste comanda "exit".
In acest context sunt retinute de asemenea detalii de securitate precum:
    - cookie-ul pentru autorizare (primit in urma unui login)
    - token JWT (primit in urma autorizarii accesului la resurse protejate)
In aceasta implementare, am presupus ca poate sa fie doar un singur cont autentificat la un timp.
Motiv pentru care aceste date se reseteaza in urma autentificarii cu un alt cont.
De asemenea, clientul a fost realizat astfel incat sa permita accesul la resurse protejate prin 
token-ul JWT in urma delogarii (din nou insa, daca user-ul se autentifica cu un alt cont, token-ul va fi resetat).
Iar token-ul nu este salvat dupa ce programul se opreste.


## Comenzi (utils.c)

Functiile care executa comenzile si utilitarele folosite de acestea se afla toate in acelasi fisier.
Pentru fiecare comanda se aloca un buffer care constituie pachetul care va fi transmis la server.
Se citesc date de la tastatura (daca sunt necesare).
Datele citite sunt dupa aceea incluse in buffer-ul pentru pachet in functie de comanda:
    - pentru comenzi precum "register", "login" si "add_book" aceste date de intrare sunt formatate in pachet json, iar
    string-ul datelor serializate este amplasat in corpul pachetului
    - pentru comenzi care cer un id pentru o carte, acesta este adaugat in ruta de acces
Anumite comenzi necesita acces autorizat de la server, motiv pentru care acestea o sa adauge in pachetul trimis la server
cookie de autentificare sau token-ul JWT.
Dupa ce se realizeaza pachetul in formatul specific request-ului comenzii, se deschide o conexiune la server, 
se trimite pachetul si in urma primirii raspunsului se inchide conexiunea la server. 
Nicio conexiune nu este deschisa pe termen nelimitat, acest fapt denotat de natura stateless a request-urilor http.
In functie de raspunsul primit de la server, se afiseaza un mesaj de succes, esec sau orice date primite de la server,
precum la comanda "get_books".
