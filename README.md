# AGV_esp_simulation
Repository per progetto esame corso ARCHITETTURE DEI CALCOLATORI E CLOUD COMPUTING

Per la descrizione del progetto riferirsi alla [wiki](https://github.com/MarshallDz/AGV_esp_simulation/wiki) di Github! 
Nella wiki riportiamo i passaggi per replicare il progetto in modo più dettagliato.

# Replicare il progetto

Hardware necessario: 
| Componente|Quantità |
|--|--|
| ESP32|1  |
|Led|4|
| Push button|2  |
|Potenziometro 10K|1|
| Resistenza 220 OHM|4  |
| Resistenza 10 K-OHM|2  |
| Jumper wires|-  |

Software necessario: Arduino Ide, account AWS

Per poter replicare il progetto consigliamo per prima cosa di scaricare il file ZIP del progetto da Github e di estrarlo nel proprio computer, inoltre dato che il microcontrollore dovrà essere connesso al wifi, di munirsi dell'SSID e della password della connessione wifi con cui si è collegato il proprio computer. 

# Setup Arduino 

 1. Scaricare ed installare l'IDE [Arduino](https://www.arduino.cc/en/software) 
 2. Scaricare il [driver](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads) per la porta seriale 
 3. Installare la libreria ArduinoJson di Benoit Blanchon dal Gestore Librerie di Arduino IDE
 4. Nelle preferenze di Arduino IDE, aggiungere l'url aggiuntivo per il Gestore schede
 5. Creare uno sketch e incollare il codice contenuto nel file AGV_controller.ino contenuto nella cartella scaricata  
 Nota: nel codice vanno inserire di propria mano alcuni dati come SSID e password della connessione wifi, indirizzo url delle due funzioni Lambda
```
  const char* ssid = "";  
  const char* password = "";  
  const char* apiEndpoint = "";  
  const char* s3apiEndpoint = "";  
```
# Setup microcontrollore 
![375969740-ea3fd435-23ee-45e7-9d42-1200cbc30260](https://github.com/user-attachments/assets/8cf474dd-3840-4e54-8dcf-3dbbe93d3e4f)


# Setup AWS
In AWS andremo ad utilizzare: AWS Lambda, AWS S3, AWS Api Gateway
# AWS Lambda

[](https://github.com/MarshallDz/AGV_esp_simulation/wiki/Impostare-l'ambiente-cloud-su-AWS#aws-lambda)

In questa fase andremo a creare due funzioni lambda. Una funzione ci servirà per gestire la logica della macchina a stati dell'AGV, la seconda ci servirà esclusivamente per andare a recuperare l'ultimo stato dell'AGV in cui era stato lasciato.

1.  Accedere alla console AWS accedendo con il proprio account root;
2.  Digitare `Lambda` nella barra di ricerca in alto a sinistra
3.  Entrati nella console di AWS Lambda, cliccare su `Crea funzione`  
    3.1 Selezionare _Autore da zero_  
    3.2 Scegliere un nome per la funzione lambda. (ex: `MyLambda`)  
    3.3 Su runtime scegliere `python 3.12`  
    3.4 Lasciamo il resto delle opzione nei loro valori de default  
    3.5 Clicchiamo `Crea funzione`
4.  Dopo aver creato la funzione, nella sezione `Codice` incolliamo il codice contenuto in [lambda_agv_controller.py](https://github.com/MarshallDz/AGV_esp_simulation/blob/main/lamba_agv_controller.py) della repository di Github.
5.  Creiamo ora la seconda funzione lambda ripetendo i passaggi del punto 3, questa volta il codice che andremo a caricare nella seconda funzione lambda sarà quello contenuto nel file [lambda_lastState.py](https://github.com/MarshallDz/AGV_esp_simulation/blob/main/lambda_lastState.py).

Proseguire ora con la creazione di un endpoint

# AWS API Gateway

[](https://github.com/MarshallDz/AGV_esp_simulation/wiki/Impostare-l'ambiente-cloud-su-AWS#aws-api-gateway)

1.  Digitare `API Gateway` nella barra di ricerca in alto a sinistra.
2.  Entrare nella console di AWS API Gateway, cliccare su `Crea API`.
3.  Scegliere `API REST` e cliccare `Crea`.
4.  Su `Dettagli API` scegliere `Nuova API`.
5.  Scegliere un nome per l'API. (ex: `MyApi`)
6.  Lasciare il tipo di endpoint in `Regionale` e cliccare su `Crea API`.
7.  Dopo aver creato l'api procediamo a creare una risorsa, cliccare quindi `Crea risorsa`.
8.  Per la creazione della risorsa lasciamo sia `Risorsa proxy` sia `CORS` in off e scegliamo un nome per la risorsa. (ex: `myResource`)
9.  Cliccare su `Crea risorsa`.
10.  Dopo aver creato la risorsa dobbiamo creare un metodo, cliccare quindi su `Crea metodo`.
11.  In `Tipo di metodo` selezionare `ANY`
12.  Scegliere `Funzione Lambda`.
13.  Selezionare `Integrazione proxy lambda`.
14.  Su `Funzione Lambda` scegliere la funzione creata in precedenza.
15.  Lasciare il resto nelle impostazioni di default e cliccare `Crea metodo`.
16.  Creato il metodo procedere cliccando `Implementa API`.
17.  In `Fase` selezionare `Nuova fase`, scegliere un nome per la fase e cliccare `Distribuire`. Dopo aver implementato l'api possiamo vedere l'url dell'endpoint in `Dettaglio fase` --> `Richiama URL`.

Ora dobbiamo creare un oggetto S3 per salvare i logs.

# AWS S3

[](https://github.com/MarshallDz/AGV_esp_simulation/wiki/Impostare-l'ambiente-cloud-su-AWS#aws-s3)

Il servizio S3 ci permetterà di creare un Bucket ed inserire nel suo interno un oggetto(AWS chiama i file con il nome di oggetti). All'interno di un oggetto, che chiameremo ad esempio `logs.json`, salveremo tutti i log dettagliati.

1.  Cercare il servizio S3 con la barra di ricerca di AWS.
2.  Cliccare su `Crea bucket`.
3.  Scegliamo un nome per il bucket (es: agvbucket) e cliccare in fondo alla pagina `Crea bucket` (tutte le altre impostazione devono essere lasciate come di default).

----------

Nota: il nome del bucket deve contenere esclusivamente caratteri minuscoli e deve essere univoco, quindi probabilmente dovrete cercare di personalizzare il nome che darete al bucket)

----------

4.  Dopo aver creato il bucket, nel menu a tendina di sinistra cliccare su `Punti di accesso`.
5.  Cliccare su `Crea punto di accesso`.
6.  Nel processo di creazione del punto di accesso dovrete:  
    6.1 Dare un nome al punto di accesso (es: agv_access_point).  
    6.2 Cliccare su `Sfoglia S3` e selezionare il bucket precedentemente creato.  
    6.3 Su _Origine della rete_ selezionare _Internet_. (Lasciamo il resto delle impostazioni ai loro valori di default)
7.  In fondo alla pagina cliccare `Crea punto di accesso`.

# Creazione policy per Lambda

[](https://github.com/MarshallDz/AGV_esp_simulation/wiki/Impostare-l'ambiente-cloud-su-AWS#creazione-policy-per-lambda)

Le funzioni Lambda necessitano di una policy per poter lavorare con S3. Precediamo ora a creare la policy necessaria.

1.  Dalla barra di ricerca di AWS cerchiamo IAM.
2.  Dal menu verticale di sinistra cliccare su `Ruoli`.
3.  Cliccare su `Crea ruolo`.
4.  SU Tipo di entità attendibile lasciare Servizio AWS, mentre su Caso d'uso cercare Lambda.
5.  In fondo alla pagina cliccare `Successivo`
6.  Nella barra di ricerca di Policy di autorizzazione cerchiamo e spuntiamo la voce AWSLambdaBasicExecutionRole e successivamente AmazonS3FullAccess.
7.  In fondo alla pagina cliccare `Successivo`
8.  Scegliere un nome per il ruolo (es: agv_role_s3) e successivamente in fondo alla pagina cliccare `Crea ruolo`.

Ora che abbiamo creato il ruolo, dobbiamo aggiornare le policy di entrambe le funzioni Lambda create. La seguente procedura dovrà pertanto essere ripetuta per entrambe le funzione, gli step rimangono comunque invariati.

1.  Tornare alla pagina del servizio Lambda.
2.  Aprire la funzione Lambda.
3.  Andare alla voce Configurazione.
4.  Nel menu verticale di sinistra cliccare su Autorizzazioni.
5.  Cliccare sul pulsante `Modifica` all'inizio della sezione Ruolo di esecuzione.
6.  In fondo alla pagina, nella voce Ruolo esistente, dal menu a tendina scegliere il ruolo appena creato.
7.  Cliccare su `Salva`.

Ora dobbiamo far "puntare" alle funzioni Lambda il punto di accesso di S3. Seguire la seguente procedura.

1.  Tornare al servizio S3.
2.  Nel menu verticale di sinistra cliccare su `Punti di accesso oggetto Lambda`.
3.  Cliccare nel pulsante `Crea punto di accesso per le espressioni Lambda dell'oggetto`.
4.  Scegliere un nome (es: lambda_access_point) e su **Punto di accesso di supporto** scegliere il punto di accesso precedentemente creato cliccando su `Sfoglia S3`.
5.  Sotto la voce Trasformazione 1 spuntare solo getObject.
6.  Su funzione Lambda scegliamo la funzione Lambda che controlla la macchina a stati (la prima creata).
7.  Cliccare in fondo alla pagina `Crea punto di accesso per le espressioni Lambda dell'oggetto`

A questo punto abbiamo terminato la configurazione dell'ambiente cloud.


