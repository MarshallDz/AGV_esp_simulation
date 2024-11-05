import json
import boto3
from datetime import datetime

def lambda_handler(event, context):
    # Inizializza il client S3 e definisci il bucket e il percorso del file
    s3_client = boto3.client('s3')
    bucket_name = ''  # Sostituisci con il nome del tuo bucket S3
    s3_key = '' #Sostituisci con il nome dell'oggetto del bucket
    
    
    # Ottieni il contenuto di logs.json da S3
    s3_object = s3_client.get_object(Bucket=bucket_name, Key=s3_key)
    logs_data = json.loads(s3_object['Body'].read())
    
    # Controlla che ci siano log nel file
    if "logs" in logs_data and len(logs_data["logs"]) > 0:
        # Recupera l'ultimo log
        last_log = logs_data["logs"][-1]
        current_state = last_log['new_state']
        
    else: current_state = 'in_stazione'

    # Ottieni l'evento dall'input
    input_event = event["queryStringParameters"]["input_event"]

    # Gestione degli stati
    if current_state == 'in_stazione':
        if input_event == 'azione':
            new_state = 'navigazione'
            message = "Navigazione iniziata"
        else:
            new_state = 'in_stazione'
            message = "In stazione, in attesa di comando"
    
    elif current_state == 'navigazione':
        if input_event == 'azione':
            new_state = 'attesa_carico'
            message = "In attesa del carico"
        else:
            new_state = 'navigazione'
            message = "In navigazione, in attesa di comando"
    
    elif current_state == 'attesa_carico':
        if input_event == 'pot_up':
            new_state = 'caricato'
            message = "Carico completato"
        else:
            new_state = 'attesa_carico'
            message = "In carico, in attesa di comando"
    
    elif current_state == 'caricato':
        if input_event == 'azione':
            new_state = 'attesa_scarico'
            message = "In attesa dello scarico"
        else:
            new_state = 'caricato'
            message = "Caricato, in attesa di comando"
    
    elif current_state == 'attesa_scarico':
        if input_event == 'pot_down':
            new_state = 'in_stazione'
            message = "Scarico completato, in ritorno alla stazione"
        else:
            new_state = 'attesa_scarico'
            message = "In attesa scarico"
    
    elif current_state == 'emergenza':
        if input_event == 'azione':
            new_state = 'in_stazione'
            message = "Emergenza risolta, tornato in stazione"
        else:
            new_state = 'emergenza'
            message = "In emergenza, in attesa di risoluzione"
    
    # Gestione dell'evento di emergenza in qualsiasi stato
    if input_event == 'emergenza':
        new_state = 'emergenza'
        message = "Entrato in stato di emergenza"
    
    # Risposta che verrà restituita da AWS Lambda
    response = {
        'new_state': new_state,
        'message': message
    }

    # Aggiungi il nuovo log
    log_entry = {
        'timestamp': datetime.utcnow().isoformat(),
        'current_state': current_state,
        'input_event': input_event,
        'new_state': new_state,
        'message': message
    }
    logs_data['logs'].append(log_entry)

    # Sovrascrivi logs.json con il nuovo contenuto
    s3_client.put_object(
        Bucket=bucket_name,
        Key=s3_key,
        Body=json.dumps(logs_data),
        ContentType='application/json'
    )

    return {
        'statusCode': 200,
        'body': json.dumps(response)
    }
